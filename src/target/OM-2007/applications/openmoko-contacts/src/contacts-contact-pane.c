/*
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>
#include <gtk/gtk.h>
#include <libebook/e-book.h>
#include "contacts-contact-pane.h"
#include "contacts-utils.h"
#include "contacts-callbacks-ebook.h"
#include "contacts-omoko.h"

G_DEFINE_TYPE (ContactsContactPane, contacts_contact_pane, GTK_TYPE_VBOX);

#define CONTACT_PANE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CONTACTS_TYPE_CONTACT_PANE, ContactsContactPanePrivate))


struct _ContactsContactPanePrivate
{
  EBookView *bookview;
  EContact *contact;
  gboolean dirty;
  gboolean editable;

  GtkSizeGroup *size_group; /* used to sizing the labels */
};

enum {
  FULLNAME_CHANGED,
  CELL_CHANGED,
  LAST_SIGNAL
};

static guint contacts_contact_pane_signals [LAST_SIGNAL];

typedef struct {
  char *display;
  char *vcard;
} VCardTypes;

static VCardTypes email_types[] = {
  { "Work", "WORK"},
  { "Home", "HOME" },
  { "Other", "OTHER" },
  {}
};

static VCardTypes phone_types[] = {
  { "Work", "VOICE;WORK"},
  { "Home", "VOICE;HOME" },
  { "Mobile", "CELL" },
  { "Fax", "FAX" },
  { "Other", "OTHER" },
  {}
};

typedef enum {
  FIELD_UNIQUE    = (1 << 1),
  FIELD_MULTILINE = (1 << 2),
  FIELD_NOLABEL   = (1 << 3),
} FieldOptions;

typedef struct {
  char *vcard_field; /* vCard field name */
  char *display_name; /* Human-readable name for display */
  char *icon; /* Icon name for the menu */
  FieldOptions options; /* If there can be only one of this field */
  char *format; /* format string */
  VCardTypes *types;
  /* TODO: add an extra changed callback so that N handler can update FN, etc */
} FieldInfo;

#define FIELD_IS_UNIQUE(x) (x->options & FIELD_UNIQUE)
#define FIELD_IS_MULTILINE(x) (x->options & FIELD_MULTILINE)
#define FIELD_IS_NOLABEL(x) (x->options & FIELD_NOLABEL)


static GQuark attr_quark = 0;
static GQuark field_quark = 0;
static GQuark entry_quark = 0;
static GQuark combo_quark = 0;

static FieldInfo fields[] = {
  { EVC_FN, "Name", NULL, FIELD_UNIQUE | FIELD_NOLABEL,  "<big><b>%s</b></big>", NULL },
  { EVC_ORG, "Organization", NULL, FIELD_UNIQUE | FIELD_NOLABEL, "<span size=\"small\">%s</span>", NULL },

  { EVC_EMAIL, "E-Mail", MOKO_STOCK_CONTACT_EMAIL, 0, NULL, email_types },
  { EVC_TEL, "Telephone", MOKO_STOCK_CONTACT_PHONE, 0, NULL, phone_types },
  { EVC_BDAY, "Birthday", "moko-stock-contact-birthday", FIELD_UNIQUE, NULL, NULL },
  { EVC_ADR, "Address", MOKO_STOCK_CONTACT_ADDRESS, FIELD_MULTILINE, NULL, email_types },

  { EVC_NOTE, "Notes", NULL, FIELD_UNIQUE | FIELD_MULTILINE, NULL, NULL },
};

/* Function prototypes */
static GtkWidget * make_widget (ContactsContactPane *pane, EVCardAttribute *attr, FieldInfo *info);
static void on_commit_cb (EBook *book, EBookStatus status, gpointer closure);
/*
 * TODO: 
 * - add EBookView and listen for changes
 */

/* Implementation */

/*
 * From a contact return a list of all EVCardAttributes that are the specified
 * name.
 */
static GList *
contact_get_attributes (EContact *contact, const char *name)
{
  GList *attrs = NULL, *l;
  
  g_return_val_if_fail (E_IS_CONTACT (contact), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  
  for (l = e_vcard_get_attributes (E_VCARD (contact)); l; l = l->next) {
    EVCardAttribute *attr;
    const char *n;
    
    attr = (EVCardAttribute *) l->data;
    n = e_vcard_attribute_get_name (attr);
    
    if (strcmp (n, name) == 0)
      attrs = g_list_prepend (attrs, attr);
  }
  
  return g_list_reverse (attrs);
}

/*
 * Strip empty attributes from a vcard
 * Returns: the number of attributes left on the card
 */
static gint
strip_empty_attributes (EVCard *card)
{
  GList *attrs, *values;
  gboolean remove;
  EVCardAttribute *attribute;
  gint count = 0;

  attrs = e_vcard_get_attributes (card);
  while (attrs) {
    count++;
    attribute = attrs->data;
    remove = TRUE;
    values = e_vcard_attribute_get_values (attrs->data);

    while (values) {
      if (g_utf8_strlen (values->data, -1) > 0) {
        remove = FALSE;
        break;
      }
      values = g_list_next (values);
    }

    attrs = g_list_next (attrs);
    if (remove) {
      e_vcard_remove_attribute (card, attribute);
      count--;
    }
  }
  return count;
}

/*
 * Set the entry to display as a blank field (i.e. with the display name in the
 * "background" of the widget)
 */
static void
field_set_blank (GtkEntry *entry, FieldInfo *info)
{
  gtk_entry_set_text (GTK_ENTRY (entry), info->display_name);
  gtk_widget_modify_text (GTK_WIDGET (entry), 
                          GTK_STATE_NORMAL, 
                          &(GTK_WIDGET (entry)->style->dark[GTK_STATE_NORMAL]));
}

/*
 * Callback for when a field entry is modified
 */
static void
field_changed (GtkWidget *entry, ContactsContactPane *pane)
{
  EVCardAttribute *attr;
  FieldInfo *info;
  const char *value;
  
  attr = g_object_get_qdata (G_OBJECT (entry), attr_quark);
  g_assert (attr);

  info = g_object_get_qdata (G_OBJECT (entry), field_quark);
  g_assert (info);


  value = gtk_entry_get_text (GTK_ENTRY (entry));
  
  /* don't save the value if we're just displaying the field name */
  if (value && !strcmp (info->display_name, value))
    return;

  /* remove the current attributes */
  e_vcard_attribute_remove_values (attr);

  /* add the new attributes */
  int i = 0;
  gchar* s;
  gchar** values = g_strsplit (value, ";", 0);
  while ((s = values[i])) {
    g_strstrip (s);
    if (s)
      e_vcard_attribute_add_value (attr, s);
    i++;
  }
  g_strfreev (values);

  if (info->vcard_field == EVC_FN)
  {
    g_signal_emit (pane, contacts_contact_pane_signals[FULLNAME_CHANGED], 0, pane->priv->contact);
  }
  else if (info->vcard_field == EVC_TEL && (g_utf8_strlen (value, -1) > 0))
  {
    g_signal_emit (pane, contacts_contact_pane_signals[CELL_CHANGED], 0, pane->priv->contact);
  }
  pane->priv->dirty = TRUE;
}

static void
make_entry_visible (GtkWidget *entry, gboolean visible)
{
  GtkWidget *parent = NULL;
  
  g_return_if_fail (GTK_IS_ENTRY (entry));
  
  if (visible) {
    /* Restore the frame & set the base colour to normal */
    gtk_entry_set_has_frame (GTK_ENTRY (entry), TRUE);
    gtk_widget_modify_base (entry, GTK_STATE_NORMAL, NULL); 
    
  } else {
    /* Remove the frame & set the base colour to the background of the parent */
    gtk_entry_set_has_frame (GTK_ENTRY (entry), FALSE);  
  
    parent = gtk_widget_get_parent (entry);
    if (parent)
      gtk_widget_modify_base (entry, 
                            GTK_STATE_NORMAL,
    			    &(parent->style->bg[GTK_STATE_NORMAL]));       
  }
}

static void
change_state_cb (GtkWidget *widget, gpointer bool)
{
  if (GPOINTER_TO_INT  (bool)) {
    /* We want to 'active' the widget */
    gtk_widget_set_state (widget, GTK_STATE_PRELIGHT);
  } else {
    /* We want to return its state to normal */
    gtk_widget_set_state (widget, GTK_STATE_NORMAL);
  }
}

/*
 * Callback for when a field entry recieves focus
 */
static gboolean
field_focus_in (GtkWidget *entry, GdkEventFocus *event, FieldInfo *info)
{
  GtkWidget *combo;
  
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (entry)), info->display_name)) {
    gtk_widget_modify_text (GTK_WIDGET (entry), 
                            GTK_STATE_NORMAL, 
                            NULL);

    gtk_entry_set_text (GTK_ENTRY (entry), "");
    
  }
  
  /* Set the combo to 'active' */
  combo = (GtkWidget*)g_object_get_qdata (G_OBJECT (entry), combo_quark);
  if (GTK_IS_CONTAINER (combo)) {
    gtk_container_forall (GTK_CONTAINER (combo), 
                          (GtkCallback)change_state_cb,
                          GINT_TO_POINTER (1));    
    gtk_widget_queue_draw (combo);
  }
  
  /* Restore the frame & set the base colour to normal */
  make_entry_visible (entry, TRUE);  
  
  return FALSE;
}

/*
 * Callback for when a field entry looses focus
 */
static gboolean
field_focus_out (GtkWidget *entry, GdkEventFocus *event, FieldInfo *info)
{
  GtkWidget *combo;
  
  if (!strcmp (gtk_entry_get_text (GTK_ENTRY (entry)), "")) {
    field_set_blank (GTK_ENTRY (entry), info);
  }
  
  /* Set the combo to 'normal' */
  combo = (GtkWidget*)g_object_get_qdata (G_OBJECT (entry), combo_quark);
  if (GTK_IS_CONTAINER (combo)) {
    gtk_container_forall (GTK_CONTAINER (combo), 
                          (GtkCallback)change_state_cb,
                          GINT_TO_POINTER (0));
    gtk_widget_queue_draw (combo);
  }
  /* Remove the frame & set the base colour to the background of the parent */
  make_entry_visible (entry, FALSE);     
  return FALSE;
}

/*
 * Convenience function to set the type property of a vcard attribute
 */
static void
set_type (EVCardAttribute *attr, gchar *type)
{
  GList *params;
  EVCardAttributeParam *p = NULL;

  /* look for the TYPE parameter */
  for (params = e_vcard_attribute_get_params (attr); params;
      params = g_list_next (params))
  {
    if (!strcmp (e_vcard_attribute_param_get_name (params->data), "TYPE"))
    {
      p = params->data;
      break;
    }
  }

  /* if we didn't find the TYPE parameter, so create it now */
  if (p == NULL)
  {
    p = e_vcard_attribute_param_new ("TYPE");
    e_vcard_attribute_add_param (attr, p);
  }

  /* FIXME: we can only deal with one attribute type value at the moment */
  e_vcard_attribute_param_remove_values (p);

  gint i;
  gchar **values = g_strsplit (type, ";", -1);

  for (i = 0; (values[i]); i++)
  {
    e_vcard_attribute_param_add_value (p, values[i]);
  }

  g_strfreev (values);
}

/*
 * Convenience function to get the type property of a vcard attribute
 * Returns the 
 */
static gchar*
get_type (EVCardAttribute *attr)
{
  GList *list, *l;
  gchar *result = NULL;
  list = e_vcard_attribute_get_param (attr, "TYPE");

  for (l = list; l; l = g_list_next (l))
  {
    if (result)
    {
      gchar *old_result = result;
      result = g_strconcat (l->data, ";", old_result, NULL);
      g_free (old_result);
    }
    else
    {
      result = g_strdup (l->data);
    }
  }

  return result;
}

/* returns whether b is a subset of a, where a and b are semi-colon seperated
 * lists
 */
static gboolean
compare_types (gchar *a, gchar *b)
{
  gchar **alist, **blist;
  gboolean result = FALSE;
  int i, j;

  /* make sure a and b are not NULL */
  if (!(a && b))
    return FALSE;

  alist = g_strsplit (a, ";", -1);
  blist = g_strsplit (b, ";", -1);

  /* check each element of blist exists in alist */
  for (i = 0; blist[i]; i++)
  {
    gboolean exists = FALSE;
    for (j = 0; alist[j]; j++)
    {
      if (!strcmp (alist[j], blist[i]))
      {
        exists = TRUE;
        break;
      }
    }
    /* if any of the items from b don't exist in a, we return false */
    if (!exists)
    {
      result = FALSE;
      break;
    }
    else
    {
      result = TRUE;
    }
  }

  g_strfreev (alist);
  g_strfreev (blist);

  return result;
}

/*
 * Callback for when a menuitem in the attribute type menu is activated
 */
static void
set_type_cb (GtkWidget *widget, EVCardAttribute *attr)
{
  int i;
  gchar *vcard_type = NULL;
  GtkWidget *box;
  FieldInfo *info;
  gpointer entry;
  
  /* TODO: use quarks here */
  gchar *display_type = gtk_combo_box_get_active_text (GTK_COMBO_BOX (widget));
  ContactsContactPane *pane = g_object_get_data (G_OBJECT (widget), "contact-pane");

  box = widget->parent;
  if (!GTK_IS_HBOX (box))
    return;

  info = g_object_get_qdata (G_OBJECT (box), field_quark);


  for (i = 0; &(info->types[i]); i++)
  {
     if (!info->types[i].display)
       continue;
     /* search the types array */
     if (!strcmp (info->types[i].display, display_type))
     {
        vcard_type = info->types[i].vcard;
        break;
     }
  }

  set_type (attr, vcard_type);
  pane->priv->dirty = TRUE;
  
  /* Finally, activate the associated entry */  
  entry = g_object_get_data (G_OBJECT (widget), "entry");
  if (GTK_IS_ENTRY (entry)) {
    gtk_widget_grab_focus (GTK_WIDGET (entry));
  }
}

static void
field_button_add_cb (GtkWidget *button, ContactsContactPane *pane)
{
  EVCardAttribute *attr;
  FieldInfo *info;
  GtkWidget *w, *box;
  GValue *v;
  gint p;

  /* widget path is: hbox.alignment.button*/
  box = button->parent->parent;
  if (!GTK_IS_HBOX (box))
    return;

  info = g_object_get_qdata (G_OBJECT (box), field_quark);

  attr = e_vcard_attribute_new ("", info->vcard_field);
  e_vcard_add_attribute (E_VCARD (pane->priv->contact), attr);

  v = g_new0 (GValue, 1);
  v = g_value_init (v, G_TYPE_INT);
  gtk_container_child_get_property (GTK_CONTAINER (pane), box, "position", v);
  p = g_value_get_int (v);

  w = make_widget (pane, attr, info);
  gtk_box_pack_start (GTK_BOX (pane), w, FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (pane), w, p + 1);
  gtk_widget_show_all (w);

}

static void
field_button_remove_cb (GtkWidget *button,  ContactsContactPane *pane)
{
  GtkWidget *box, *entry;
  EVCardAttribute *attr;
  FieldInfo *info;
  GList *rows = gtk_container_get_children (GTK_CONTAINER (pane));
  GList *r;
  gint res = 0;
  

  box = button->parent->parent;
  if (!GTK_IS_HBOX (box))
    return;

  attr = g_object_get_qdata (G_OBJECT (box), attr_quark);
  entry = g_object_get_qdata (G_OBJECT (box), entry_quark);
  info = g_object_get_qdata (G_OBJECT (box), field_quark);

  /* Check this isn't the last field of this type */
  for (r = rows; r != NULL; r = r->next) {
    GtkWidget *row = r->data;
    FieldInfo *i = g_object_get_qdata (G_OBJECT (row), field_quark);
    
    if (GTK_IS_HBOX (row) && i != NULL) {
      if (strcmp (info->vcard_field, i->vcard_field) == 0)
        res++;
    }
  }
  
  if (res > 1) {
    gtk_container_remove (GTK_CONTAINER (pane), box);
    e_vcard_remove_attribute (E_VCARD (pane->priv->contact), attr);
  }
  else {
    /* clear the attribute and entry widget */
    e_vcard_attribute_remove_values (attr);
    field_set_blank (GTK_ENTRY (entry), info);
  }

}

static void
combo_popup_changed_cb (GObject *gobject, GParamSpec *arg1, GtkWidget *entry)
{
  g_return_if_fail (GTK_IS_COMBO_BOX (gobject));
  g_return_if_fail (GTK_IS_ENTRY (entry));

  gboolean shown = FALSE;
  g_object_get (gobject, "popup-shown", &shown, NULL);
  
  if (shown) {
    make_entry_visible (entry, TRUE);
  } else {
    make_entry_visible (entry, FALSE);
  }  
}

static GtkWidget *
make_widget (ContactsContactPane *pane, EVCardAttribute *attr, FieldInfo *info)
{
#define BUTTON_WIDTH 24
#define BUTTON_HEIGHT 21

  GtkWidget *box, *type_label = NULL, *key = NULL, *value;
  gchar *attr_value = NULL, *escaped_str, *type, *s;
  gint i = 0;

  box = gtk_hbox_new (FALSE, 0);

  if (attr)
    type = get_type (attr);
  else
    type = NULL;


  if (type == NULL && info->types != NULL)
  {
    type = g_strdup (info->types[0].vcard);
  }

  /* insert add/remove buttons */
  if (pane->priv->editable && !FIELD_IS_UNIQUE (info))
  {
    /* need to use an alignment here to stop the button expanding vertically */
    GtkWidget *btn, *alignment;
    btn = gtk_button_new ();
    gtk_widget_set_name (btn, "addbutton");
    alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_widget_set_size_request (btn, BUTTON_WIDTH, BUTTON_HEIGHT);
    gtk_container_add (GTK_CONTAINER (alignment), btn);
    gtk_box_pack_start (GTK_BOX (box), alignment, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (btn), "clicked", G_CALLBACK (field_button_add_cb), pane);

    btn = gtk_button_new ();
    gtk_widget_set_name (btn, "removebutton");
    gtk_widget_set_size_request (btn, BUTTON_WIDTH, BUTTON_HEIGHT);
    alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), btn);
    gtk_box_pack_start (GTK_BOX (box), alignment, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (btn), "clicked", G_CALLBACK (field_button_remove_cb), pane);
  }


  /* The label (if required) */
  if (!FIELD_IS_NOLABEL (info) && (!pane->priv->editable || FIELD_IS_UNIQUE(info)))
  {
    s = NULL;
    GtkWidget *align;
    
    /* Unique fields don't have different types, so just use the display name
     * for the label */
    if (FIELD_IS_UNIQUE (info))
    {
      s = g_strdup_printf ("%s:", info->display_name);
    }
    else
    {
      /* find the display name for the current type */
      for (i = 0; info->types[i].display; i++)
      {
         if (compare_types (type, info->types[i].vcard))
         {
            s = g_strdup_printf ("%s:", info->types[i].display);
            break;
         }
      }
      /* if we couldn't find a display name, use the raw vcard name */
      if (!s)
      {
        s = g_strdup_printf ("%s:", type);
      }
    }
    /* Pack into an alignment with a left padding of twice the BUTTON_WIDTH size
       so we take into account the missing buttons-width, which the comboboxes
       will have */
    align  = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_alignment_set_padding (GTK_ALIGNMENT (align), 0, 0, BUTTON_WIDTH *2, 0);

    type_label = gtk_label_new (s);
    key = type_label;
    gtk_widget_set_name (type_label, "fieldlabel");
    gtk_widget_set_size_request (type_label, -1, 46);
    gtk_misc_set_alignment (GTK_MISC (type_label), 0.0, 0.5);
    
    if (pane->priv->size_group)
      gtk_size_group_add_widget (pane->priv->size_group, type_label);
    gtk_container_add (GTK_CONTAINER (align), type_label);
    gtk_box_pack_start (GTK_BOX (box), align, FALSE, FALSE, 4);
    g_free (s);
  }

  /* Create the type selector, or label, depending on field */
  if (!FIELD_IS_UNIQUE(info) && pane->priv->editable)
  {
    GtkWidget *combo;
    gboolean is_custom_type = TRUE;
    combo = gtk_combo_box_new_text ();
    key = combo;
    gtk_widget_set_size_request (combo, -1, 46);
    i = 0;

    /* add items to the types drop down (Home, Work, etc) */
    for (s = info->types[i].display; (s = info->types[i].display); i++) {
      gtk_combo_box_append_text (GTK_COMBO_BOX (combo), s);

      /* if the vcard type matches the current type, then select it */
      if (compare_types (type, info->types[i].vcard)) {
        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), i);
        is_custom_type = FALSE;
      }
    }

    /* this type isn't in our list of types, so add it now as a custom entry */
    if (is_custom_type) {
       gtk_combo_box_append_text (GTK_COMBO_BOX (combo), type);
       gtk_combo_box_set_active (GTK_COMBO_BOX (combo), i);
    }
    g_object_set_data (G_OBJECT (combo), "contact-pane", pane);
    g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (set_type_cb), attr);
    g_object_notify (G_OBJECT (combo), "popup-shown");
    if (pane->priv->size_group)
      gtk_size_group_add_widget (pane->priv->size_group, combo);
    gtk_box_pack_start (GTK_BOX (box), combo, FALSE, FALSE, 4);
  }


  /* The value field itself */

  /* load the attribute value, returning a semicolon seperated string for
   * multivalue attributes
   */
  if (attr)
  {
    GList *l = e_vcard_attribute_get_values (attr);
    if (l)
    {
      attr_value = g_strdup (l->data);

      while ((l = g_list_next (l)))
      {
        gchar *old = attr_value;
        attr_value = g_strdup_printf ("%s; %s", old, (gchar*) l->data);
        g_free (old);
      }
    }
  }


  if (pane->priv->editable) {
    value = gtk_entry_new ();
    gtk_widget_set_size_request (value, -1, 34);
    
    /* Make the entry "transparent" by setting the base colour to that of
       the parent, and removing the frame */
    gtk_entry_set_has_frame (GTK_ENTRY (value), FALSE);
    gtk_widget_modify_base (value, 
                            GTK_STATE_NORMAL,
	                    &(GTK_WIDGET (pane)->style->bg[GTK_STATE_NORMAL]));
    
    if (attr_value)
      gtk_entry_set_text (GTK_ENTRY (value), attr_value);
    else
    {
      gtk_entry_set_text (GTK_ENTRY (value), info->display_name);
      gtk_widget_modify_text (GTK_WIDGET (value), 
                          GTK_STATE_NORMAL, 
                          &(GTK_WIDGET (pane)->style->dark[GTK_STATE_NORMAL]));
    }

    g_object_set_qdata (G_OBJECT (value), attr_quark, attr);
    g_object_set_qdata (G_OBJECT (value), field_quark, (gpointer)info);
    if (key) {
      g_object_set_qdata (G_OBJECT (value), combo_quark, (gpointer)key);
      if (GTK_IS_COMBO_BOX (key)) {
        g_signal_connect (G_OBJECT (key), "notify", 
                          G_CALLBACK (combo_popup_changed_cb), (gpointer)value);
        g_object_set_data (G_OBJECT (key), "entry", (gpointer)value);
      }
    }
    g_signal_connect (value, "changed", G_CALLBACK (field_changed), pane);
    g_signal_connect (value, "focus-in-event", G_CALLBACK (field_focus_in), info);
    g_signal_connect (value, "focus-out-event", G_CALLBACK (field_focus_out), info);
  
  } else {
    GtkWidget *label, *image;
    GdkPixbuf *pixbuf = NULL;
    gchar *icon = NULL;
    
    if (!attr_value)
      attr_value = g_strdup ("");
    if (info->format)
    {
      escaped_str = g_markup_printf_escaped (info->format, attr_value);
      label = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label), escaped_str);
      g_free (escaped_str);
    }
    else
      label = gtk_label_new (attr_value);
    
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    
    /* Load the correct pixbuf */
    icon = info->icon;
    if (icon)
      pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                         icon,
                                         24,
                                         0, NULL);
    if (pixbuf) {
      value = gtk_hbox_new (FALSE, 10);
      
      image = gtk_image_new_from_pixbuf (pixbuf);
      gtk_box_pack_start (GTK_BOX (value), image, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (value), label, TRUE, TRUE, 0);
      
    } else
      value = label;
    
    /* We want to remove any size-requests as we are in viewmode */
    if (key) {
      gtk_widget_set_size_request (key, -1, 24);
      gtk_misc_set_alignment (GTK_MISC (key), 1, 0.5);
    }
  }
  gtk_box_pack_start (GTK_BOX (box), value, TRUE, TRUE, 4);

  /****/
  g_object_set_qdata (G_OBJECT (box), attr_quark, attr);
  g_object_set_qdata (G_OBJECT (box), field_quark, (gpointer)info);
  g_object_set_qdata (G_OBJECT (box), entry_quark, value);


  gtk_widget_show_all (box);
  g_free (attr_value);
  g_free (type);
  return box;
}

static void
choose_photo_cb (GtkWidget *button, ContactsContactPane *pane)
{
	pane->priv->dirty = TRUE;
	contacts_choose_photo (button, pane->priv->contact);
}

/*
 * Update the widgets, called when the contact or editable mode has changed.
 */
static void
update_ui (ContactsContactPane *pane)
{
  int i;
  GtkWidget *w;
  EVCardAttribute *attr;

  g_assert (CONTACTS_IS_CONTACT_PANE (pane));

  /* First, clear the pane */
  gtk_container_set_border_width (GTK_CONTAINER (pane), 6);
  gtk_container_foreach (GTK_CONTAINER (pane),
                         (GtkCallback)gtk_widget_destroy, NULL);
  

  if (pane->priv->contact == NULL) {
    if (pane->priv->editable) {
      pane->priv->contact = e_contact_new ();
      /* TODO: check for error here */
      e_book_add_contact (e_book_view_get_book (pane->priv->bookview), pane->priv->contact, NULL);
    } else {
      w = gtk_label_new ("No contact to display");
      gtk_widget_show (w);
      gtk_box_pack_start (GTK_BOX (pane), w, TRUE, TRUE, 0);
      return;
    }
  }

  pane->priv->size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /* Add Name, Organisation and Photo fields into a special arrangement */
  GtkWidget *table = gtk_table_new (2, 4, FALSE);
  gtk_box_pack_start (GTK_BOX (pane), table, FALSE, FALSE, 4);

  /* Name Field */
  attr = e_vcard_get_attribute (E_VCARD (pane->priv->contact), fields[0].vcard_field);
  if (!attr && pane->priv->editable) {
    attr = e_vcard_attribute_new ("", fields[0].vcard_field);
    e_vcard_add_attribute (E_VCARD (pane->priv->contact), attr);
  }
  w = make_widget (pane, attr, &fields[0]);
  gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 0, 1);

  /* Organisation Field (if required) */
  attr = e_vcard_get_attribute (E_VCARD (pane->priv->contact), fields[1].vcard_field);
  if (!attr && pane->priv->editable) {
    attr = e_vcard_attribute_new ("", fields[1].vcard_field);
    e_vcard_add_attribute (E_VCARD (pane->priv->contact), attr);
  }
  gboolean has_org_field = FALSE;
  if (attr || pane->priv->editable)
  {
    w = make_widget (pane, attr, &fields[1]);
    gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 1, 2);
    has_org_field = TRUE;
  }
  
  /* Add Photo */
  GtkImage *photo = contacts_load_photo (pane->priv->contact);
  if (pane->priv->editable)
  {
    w = gtk_button_new ();
    gtk_widget_set_name (w, "mokofingerbutton-big");
    gtk_button_set_image (GTK_BUTTON (w), GTK_WIDGET (photo));
    g_signal_connect (w, "clicked", (GCallback) choose_photo_cb, pane);
  }
  else
  {
    w = GTK_WIDGET (photo);
  }
  if (has_org_field)
    gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 2, 0, 0, 0, 0);
  else
    gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1, 0, 0, 0, 0);

  gtk_widget_show_all (table);
  
  /* The HSep between the name/org and the fields */
  GtkWidget *sep = gtk_hseparator_new ();
  gtk_widget_show (sep);
  gtk_box_pack_start (GTK_BOX (pane), sep, FALSE, FALSE, 5);

  for (i = 2; i < G_N_ELEMENTS (fields); i++) {
    FieldInfo *info;

    info = &fields[i];
    if (FIELD_IS_UNIQUE (info)) {
      /* Fast path unique fields, no need to search the entire contact */
      attr = e_vcard_get_attribute (E_VCARD (pane->priv->contact), info->vcard_field);
      if (!attr && pane->priv->editable) {
         attr = e_vcard_attribute_new ("", info->vcard_field);
         e_vcard_add_attribute (E_VCARD (pane->priv->contact), attr);
      }
      if (attr) {
        w = make_widget (pane, attr, info);
        gtk_box_pack_start (GTK_BOX (pane), w, FALSE, FALSE, 4);
      }
    } else {
      GList *attrs, *l;
      attrs = contact_get_attributes (pane->priv->contact, info->vcard_field);
      if (g_list_length (attrs) < 1 && pane->priv->editable) {
        attr = e_vcard_attribute_new ("", info->vcard_field);
        e_vcard_add_attribute (E_VCARD (pane->priv->contact), attr);
        w = make_widget (pane, attr, info);
        gtk_box_pack_start (GTK_BOX (pane), w, FALSE, FALSE, 4);
      }
      else
      for (l = attrs; l ; l = g_list_next (l)) {
        EVCardAttribute *attr = l->data;
        w = make_widget (pane, attr, info);
        gtk_box_pack_start (GTK_BOX (pane), w, FALSE, FALSE, 4);
      }
    }
  }

  g_object_unref (pane->priv->size_group);
}

/* GObject methods */

static void
contacts_contact_pane_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
contacts_contact_pane_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
contacts_contact_pane_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (contacts_contact_pane_parent_class)->dispose)
    G_OBJECT_CLASS (contacts_contact_pane_parent_class)->dispose (object);
}

static void
contacts_contact_pane_finalize (GObject *object)
{
  G_OBJECT_CLASS (contacts_contact_pane_parent_class)->finalize (object);
}

static void
contacts_contact_pane_class_init (ContactsContactPaneClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ContactsContactPanePrivate));

  object_class->get_property = contacts_contact_pane_get_property;
  object_class->set_property = contacts_contact_pane_set_property;
  object_class->dispose = contacts_contact_pane_dispose;
  object_class->finalize = contacts_contact_pane_finalize;

  /* TODO: properties for editable and contact */
  
  /* Initialise the quarks */
  attr_quark = g_quark_from_static_string("contact-pane-attribute");
  field_quark = g_quark_from_static_string("contact-pane-fieldinfo");
  entry_quark = g_quark_from_static_string("contact-pane-entry");
  combo_quark = g_quark_from_static_string("contact-pane-combo");

  contacts_contact_pane_signals[FULLNAME_CHANGED] = g_signal_new (("fullname-changed"),
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (ContactsContactPaneClass, fullname_changed),
      NULL, NULL,
      g_cclosure_marshal_VOID__OBJECT,
      G_TYPE_NONE, 1,
      E_TYPE_CONTACT);

  contacts_contact_pane_signals[CELL_CHANGED] = g_signal_new (("cell-changed"),
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (ContactsContactPaneClass, cell_changed),
      NULL, NULL,
      g_cclosure_marshal_VOID__OBJECT,
      G_TYPE_NONE, 1,
      E_TYPE_CONTACT);


}

static void
contacts_contact_pane_init (ContactsContactPane *self)
{
  self->priv = CONTACT_PANE_PRIVATE (self);

}


/* Public API */

GtkWidget *
contacts_contact_pane_new (void)
{
  return g_object_new (CONTACTS_TYPE_CONTACT_PANE, NULL);
}

void
contacts_contact_pane_set_editable (ContactsContactPane *pane, gboolean editable)
{
  g_return_if_fail (CONTACTS_IS_CONTACT_PANE (pane));

  if (pane->priv->editable != editable) {
    pane->priv->editable = editable;

    /* strip empty attributes and save the contact if we're switching to view
     * mode
     */
    if (editable == FALSE && pane->priv->contact) {
      strip_empty_attributes (E_VCARD (pane->priv->contact));
      if (pane->priv->dirty && pane->priv->bookview) {
        e_book_async_commit_contact (e_book_view_get_book (pane->priv->bookview),
                                     pane->priv->contact,
                                     on_commit_cb, pane);
      }
    }

    update_ui (pane);
  }
}

void
contacts_contact_pane_set_book_view (ContactsContactPane *pane, EBookView *view)
{
  g_return_if_fail (CONTACTS_IS_CONTACT_PANE (pane));
  g_return_if_fail (E_IS_BOOK_VIEW (view));
  
  if (pane->priv->bookview) {
    g_object_unref (pane->priv->bookview);
  }
  
  pane->priv->bookview = g_object_ref (view);
}

static void
on_commit_cb (EBook *book, EBookStatus status, gpointer closure)
{
  if (status != E_BOOK_ERROR_OK) {
    /* TODO: show error dialog */
    g_warning ("Cannot commit contact: %d", status);
  }
}

void
contacts_contact_pane_set_contact (ContactsContactPane *pane, EContact *contact)
{
  ContactsContactPanePrivate *priv;
  priv = pane->priv;
  gint attr_count;

  /* check to see if the contact is the same as the current one */
  if (priv->contact && contact) {
    if (strcmp (e_contact_get_const (contact, E_CONTACT_UID),
                e_contact_get_const (priv->contact, E_CONTACT_UID)) == 0) {
      return;
    }
  }

  if (priv->contact) {
    attr_count = strip_empty_attributes (E_VCARD (priv->contact));
    if (attr_count == 1)
      e_book_remove_contact (e_book_view_get_book (priv->bookview),
                             e_contact_get_const (priv->contact, E_CONTACT_UID),
                             NULL);
    if (priv->dirty && priv->bookview) {
      e_book_async_commit_contact (e_book_view_get_book (priv->bookview),
                                   priv->contact,
                                   on_commit_cb, pane);
    }
    g_object_unref (priv->contact);
  }

  if (contact) {
    priv->contact = g_object_ref (contact);
  } else {
    priv->contact = NULL;
  }
  priv->dirty = FALSE;

  update_ui (pane);
}
