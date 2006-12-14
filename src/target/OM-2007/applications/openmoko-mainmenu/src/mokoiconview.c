#include "mokoiconview.h"
#include "callbacks.h"

enum {
    ICON_VIEW_SIGNAL,
    LAST_SIGNAL
};

static void 
moko_icon_view_class_init(MokoIconViewClass *klass);

static void 
moko_icon_view_init(MokoIconView *self);

static guint icon_view_signals[LAST_SIGNAL] = { 0 };

/**
*@brief retrun 	MokoIconView type.
*@param none
*@return GType
*/
GType 
moko_icon_view_get_type (void) /* Typechecking */
{
    static GType icon_view_type = 0;

    if (!icon_view_type)
    {
        static const GTypeInfo icon_view_info =
        {
            sizeof (MokoIconViewClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_icon_view_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoIconView),
            0,
            (GInstanceInitFunc) moko_icon_view_init,
            NULL
        };

        icon_view_type = g_type_register_static (GTK_TYPE_VBOX, "MokoIconView", &icon_view_info, 0);
    }

    return icon_view_type;
}

/**
*@brief initialize	MokoIconView class.
*@param klass	MokoIconView Class
*@return none
*/
static void 
moko_icon_view_class_init(MokoIconViewClass* Klass) /* Class Initialization */
{
    icon_view_signals[ICON_VIEW_SIGNAL] = g_signal_new ("MokoIconView",
            G_TYPE_FROM_CLASS (Klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoIconViewClass, moko_icon_view_function),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 
            0);
}

/*@brief initialize 	MokoIconView instance
 *@param mm	MokoIconView*
 *@return none
 */
void
moko_icon_view_init(MokoIconView *self)
{
  PangoFontDescription* PangoFont = pango_font_description_new(); //get system default PangoFontDesc
  GtkWidget *table;
  gint i,j;
  
  table = gtk_table_new (3, 3, TRUE);
  gtk_container_add (GTK_CONTAINER(self), table);
  gtk_widget_show (table);

  for (i=0;i<3; i++)
    for (j=0;j<3; j++)
      {
	self->btns[i][j] = gtk_toggle_button_new ();
	gtk_table_attach_defaults (GTK_TABLE(table), self->btns[i][j], 
				   i, i+1, j, j+1);
	gtk_signal_connect (GTK_OBJECT (self->btns[i][j]), "clicked",
			    GTK_SIGNAL_FUNC (moko_item_select_cb), NULL);
	gtk_widget_set_size_request (self->btns[i][j], 20, 20);
	gtk_widget_show (self->btns[i][j]);
//	gtk_misc_set_padding (GTK_MISC (self->btns[i][j]), 30, 30);
//	sleep (1);
      }

  if (PangoFont)
    pango_font_description_free (PangoFont);
}


/* Construction */
GtkWidget* 
moko_icon_view_new()
{
  return GTK_WIDGET(g_object_new(moko_icon_view_get_type(), NULL));
}

/* Destruction */
void 
moko_icon_view_clear(MokoIconView *self)
{ 
  if (!self) g_free (self);
}

/*
*
*
*/
void
moko_icon_view_update(GtkListStore *store) {
    
}




