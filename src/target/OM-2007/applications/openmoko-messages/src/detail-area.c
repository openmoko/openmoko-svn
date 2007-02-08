/* detail-area.c
 *
 * Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 * Copyright (C) 2006-2007 OpenMoko Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Public License as published by
 * the Free Software Foundation; version 2.1 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Public License for more details.
 *
 * Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: Alex $]
 *
 */ 

#include "detail-area.h"
#include "callbacks.h"
#include "main.h"

GtkWidget* detail_area_mode_edit (DetailArea *self);
GtkWidget* detail_area_new_mail (DetailArea* self);
GtkWidget* detail_area_mode_read (DetailArea* self);
GtkWidget* detail_area_mode_membership (DetailArea* self);

G_DEFINE_TYPE (DetailArea, detail_area, GTK_TYPE_SCROLLED_WINDOW)

#define DETAIL_AREA_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_DETAIL_AREA, DetailAreaPrivate))

typedef struct _DetailAreaPrivate{
	
} DetailAreaPrivate;

/* parent class pointer */
GtkWindowClass* parent_class = NULL;

/* forward declarations */
gboolean
_expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	
	return TRUE;
}

static void
detail_area_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (detail_area_parent_class)->dispose)
        G_OBJECT_CLASS (detail_area_parent_class)->dispose (object);
}

static void
detail_area_finalize (GObject *object)
{
    G_OBJECT_CLASS (detail_area_parent_class)->finalize (object);
}
    
static void
detail_area_class_init (DetailAreaClass *klass)
{
    /* hook parent */
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (DetailAreaPrivate));

    object_class->dispose = detail_area_dispose;
    object_class->finalize = detail_area_finalize;
    
}

static void
detail_area_init (DetailArea *self)
{
   	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(self), GTK_POLICY_NEVER, GTK_POLICY_NEVER );
    
    self->readAttributes = g_malloc (sizeof(ReadAttributes));
    self->editAttributes = g_malloc (sizeof(EditAttributes));
    self->notebook = GTK_NOTEBOOK( gtk_notebook_new() );
    //gtk_notebook_append_page (self->notebook,detail_area_mode_edit(self),NULL);
    //gtk_notebook_append_page (self->notebook,detail_area_new_mail(self),NULL);
    gtk_notebook_append_page (self->notebook,detail_area_mode_read(self),NULL);
    
    //gtk_notebook_append_page (self->notebook,detail_area_mode_membership(self),NULL);
    gtk_notebook_set_show_tabs (self->notebook,FALSE);
    gtk_notebook_set_show_border (self->notebook,FALSE);
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(self), GTK_WIDGET(self->notebook) );
    
}

GtkWidget* detail_area_new (void)
{
    return GTK_WIDGET(g_object_new(TYPE_DETAIL_AREA, NULL ));
}

GtkWidget* detail_area_mode_edit (DetailArea* self)
{
    /* create detail box */
    self->detailbox = GTK_VBOX(gtk_vbox_new(FALSE,0));
    EditAttributes* editAttributes = self->editAttributes;

    /* create tool box */
    self->toolbox = GTK_HBOX(gtk_hbox_new(FALSE,0)); 
    MokoToolBox* mokobox = MOKO_TOOL_BOX(moko_tool_box_new());
    GtkHBox* toolbox = moko_tool_box_get_button_box(mokobox);
    editAttributes->sendBtn = gtk_button_new_with_label("Send");
    gtk_box_pack_start( GTK_BOX(toolbox), GTK_WIDGET(editAttributes->sendBtn), FALSE, FALSE, 0 );
    editAttributes->addrBtn = gtk_button_new_with_label("Address");
    gtk_box_pack_start( GTK_BOX(toolbox), GTK_WIDGET(editAttributes->addrBtn), FALSE, FALSE, 0 );

    /* fill entry area */
    self->entryarea = MOKO_FIXED(moko_fixed_new());
    GtkAlignment* alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
    gtk_alignment_set_padding (alignment, 5, 5, 5, 5);
    GtkWidget* entrybox = gtk_hbox_new(FALSE,0);
    GtkWidget* toLabel = gtk_label_new("To:");
    gtk_widget_set_size_request (toLabel, 40, -1);
    editAttributes->toEntry = gtk_entry_new();
    gtk_widget_set_size_request (editAttributes->toEntry, 300, -1);
    GtkWidget* inputLabel = gtk_label_new("(120/1)");
    gtk_widget_set_size_request (inputLabel, 80, -1);
    gtk_box_pack_start (GTK_BOX(entrybox),toLabel,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(entrybox),editAttributes->toEntry,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(entrybox),inputLabel,FALSE,TRUE,0);
    gtk_container_add (GTK_CONTAINER(alignment), entrybox);

    moko_fixed_set_cargo(MOKO_FIXED(self->entryarea),GTK_WIDGET(alignment));

    /* fill textview */
    editAttributes->txtView = gtk_text_view_new();
    GtkWidget* viewAlign = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_alignment_set_padding (GTK_ALIGNMENT(viewAlign),0,0,0,50);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(editAttributes->txtView),GTK_WRAP_CHAR);
    gtk_container_add (GTK_CONTAINER(viewAlign),GTK_WIDGET(editAttributes->txtView));

    gtk_box_pack_start (GTK_BOX(self->detailbox),GTK_WIDGET(mokobox),FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(self->detailbox),GTK_WIDGET(self->entryarea),FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(self->detailbox),GTK_WIDGET(viewAlign),TRUE,TRUE,0);

    return GTK_WIDGET(self->detailbox);
}

//TODO move mail function to edit mode
GtkWidget* detail_area_new_mail (DetailArea* self) {
    
    /* create detail box */
    self->detailbox = GTK_VBOX(gtk_vbox_new(FALSE,0));

    /* create tool box */
    self->toolbox = GTK_HBOX(gtk_hbox_new(FALSE,0));
    MokoToolBox* mokobox = MOKO_TOOL_BOX(moko_tool_box_new());
    GtkHBox* toolbox = moko_tool_box_get_button_box(mokobox);
    GtkWidget* button = gtk_button_new_with_label("Send");
    gtk_box_pack_start( GTK_BOX(toolbox), GTK_WIDGET(button), FALSE, FALSE, 0 );
    button = gtk_button_new_with_label("Attach");
    gtk_box_pack_start( GTK_BOX(toolbox), GTK_WIDGET(button), FALSE, FALSE, 0 );
    button = gtk_button_new_with_label("Address");
    gtk_box_pack_start( GTK_BOX(toolbox), GTK_WIDGET(button), FALSE, FALSE, 0 );

    /* fill entry area */
    self->entryarea = MOKO_FIXED(moko_fixed_new());
    GtkAlignment* alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
    gtk_alignment_set_padding (alignment, 5, 5, 5, 5);

    GtkVBox* entrybox = GTK_VBOX(gtk_vbox_new(FALSE,0));
    gtk_box_set_spacing (GTK_BOX(entrybox),2);
    GtkWidget* hbox = gtk_hbox_new(FALSE,0);
    GtkWidget* toLabel = gtk_label_new("To:");
    gtk_widget_set_size_request (toLabel, 110, -1);
    gtk_misc_set_alignment (GTK_MISC (toLabel),0.8,0.5);
    GtkWidget* toEntry = gtk_entry_new();
    gtk_widget_set_size_request (toEntry, 300, -1);
    gtk_box_pack_start (GTK_BOX(hbox),GTK_WIDGET(toLabel),FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(hbox),GTK_WIDGET(toEntry),FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(entrybox),GTK_WIDGET(hbox),FALSE,TRUE,0);

    hbox = gtk_hbox_new(FALSE,0);
    GtkWidget* ccLabel = gtk_label_new("CC:");
    gtk_widget_set_size_request (ccLabel, 110, -1);
    gtk_misc_set_alignment (GTK_MISC (ccLabel),0.8,0.5);
    GtkWidget* ccEntry = gtk_entry_new();
    gtk_widget_set_size_request (ccEntry, 300, -1);
    gtk_box_pack_start (GTK_BOX(hbox),ccLabel,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(hbox),ccEntry,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(entrybox),GTK_WIDGET(hbox),FALSE,TRUE,0);

    hbox = gtk_hbox_new(FALSE,0);
    GtkWidget* bccLabel = gtk_label_new("Bcc:");
    gtk_widget_set_size_request (bccLabel, 110, -1);
    gtk_misc_set_alignment (GTK_MISC (bccLabel),0.8,0.5);
    GtkWidget* bccEntry = gtk_entry_new();
    gtk_widget_set_size_request (bccEntry, 300, -1);
    gtk_box_pack_start (GTK_BOX(hbox),bccLabel,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(hbox),bccEntry,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(entrybox),hbox,FALSE,TRUE,0);

    hbox = gtk_hbox_new(FALSE,0);
    GtkWidget* subjectLabel = gtk_label_new("Subject:");
    gtk_widget_set_size_request (subjectLabel, 110, -1);
    gtk_misc_set_alignment (GTK_MISC (subjectLabel),0.55,0.5);
    GtkWidget* subjectEntry = gtk_entry_new();
    gtk_widget_set_size_request (subjectEntry, 300, -1);
    gtk_box_pack_start (GTK_BOX(hbox),subjectLabel,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(hbox),subjectEntry,FALSE,TRUE,0);
    gtk_box_pack_start (GTK_BOX(entrybox),hbox,FALSE,TRUE,0);

    gtk_container_add (GTK_CONTAINER(alignment), GTK_WIDGET(entrybox));

    moko_fixed_set_cargo(self->entryarea,GTK_WIDGET(alignment));

     /* fill textview */
     /*self->textview = GTK_TEXT_VIEW(gtk_text_view_new ());
     GtkWidget* viewAlign = gtk_alignment_new (0.5, 0.5, 1, 1);
     gtk_alignment_set_padding (GTK_ALIGNMENT(viewAlign),0,0,0,50);
     gtk_text_view_set_wrap_mode (self->textview,GTK_WRAP_CHAR);
     gtk_container_add (GTK_CONTAINER(viewAlign),GTK_WIDGET(self->textview));

     gtk_box_pack_start (GTK_BOX(self->detailbox),GTK_WIDGET(mokobox),FALSE,TRUE,0);
     gtk_box_pack_start (GTK_BOX(self->detailbox),GTK_WIDGET(self->entryarea),FALSE,TRUE,0);
     gtk_box_pack_start (GTK_BOX(self->detailbox),GTK_WIDGET(viewAlign),TRUE,TRUE,0);*/

    return GTK_WIDGET(self->detailbox);
}

GtkWidget* detail_area_mode_read (DetailArea* self)
{
     /* create detail box */
     self->detailbox = GTK_VBOX(gtk_vbox_new(FALSE,0));
     
     ReadAttributes* readAttributes = self->readAttributes;

     GtkWidget* headerbox = gtk_vbox_new(FALSE,0);
     GtkWidget* hbox = gtk_hbox_new(FALSE,0);
     readAttributes->from_label = gtk_label_new ("Alex");
     gtk_misc_set_alignment (GTK_MISC (readAttributes->from_label),1,0.5);
     readAttributes->date_label = gtk_label_new ("Hello");
     gtk_misc_set_alignment (GTK_MISC (readAttributes->date_label),1,0.5);
     
     GtkWidget* cellalign = gtk_alignment_new (0.5, 0.5, 1, 1);
     gtk_alignment_set_padding (GTK_ALIGNMENT(cellalign), 5,5,5,5);
     GtkWidget* label = gtk_label_new ("From:");      
     gtk_misc_set_alignment (GTK_MISC (label),1,0.5);
     gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
     gtk_box_pack_start(GTK_BOX(hbox),readAttributes->from_label,FALSE,FALSE,0);
     gtk_container_add(GTK_CONTAINER(cellalign),hbox);
     gtk_box_pack_start(GTK_BOX(headerbox),cellalign,FALSE,FALSE,0);

     cellalign = gtk_alignment_new (0.5, 0.5, 1, 1);
     gtk_alignment_set_padding (GTK_ALIGNMENT(cellalign), 5,5,5,5);
     hbox = gtk_hbox_new(FALSE,0);
     label = gtk_label_new ("Date:");
     gtk_misc_set_alignment (GTK_MISC (label),1,0.5);
     gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
     gtk_box_pack_start(GTK_BOX(hbox),readAttributes->date_label,FALSE,FALSE,0);
     gtk_container_add(GTK_CONTAINER(cellalign),hbox);
     gtk_box_pack_start(GTK_BOX(headerbox),cellalign,FALSE,FALSE,0);

     GtkWidget* hseparator = gtk_hseparator_new();
     GtkWidget* detailAlign = gtk_alignment_new(0, 0, 0, 0);
     gtk_alignment_set_padding (GTK_ALIGNMENT(detailAlign), 10, 10, 10, 50);
     /*GtkWidget* details = gtk_label_new("this is the detail");*/
     readAttributes->details = gtk_label_new( "Add your widget for showing details for the selected\n"
     "\ndata entry here\n \n \n \n \n \n \n \nThis particular label\n \nis very long\n"
     "\nto make the fullscreen\n \ntrigger more interesting\n \n \n");
     gtk_widget_set_size_request (readAttributes->details,420,-1);
     gtk_label_set_line_wrap (GTK_LABEL(readAttributes->details),TRUE);
     gtk_misc_set_alignment (GTK_MISC (readAttributes->details),0.1,0.5);
     gtk_container_add (GTK_CONTAINER(detailAlign),readAttributes->details);
     gtk_box_pack_start(GTK_BOX(self->detailbox),headerbox,FALSE,TRUE,0);
     gtk_box_pack_start(GTK_BOX(self->detailbox),hseparator,FALSE,TRUE,0);
     gtk_box_pack_start(GTK_BOX(self->detailbox),detailAlign,FALSE,TRUE,0);
     
     return GTK_WIDGET(self->detailbox);
}

GtkWidget* detail_area_mode_membership (DetailArea* self)
{

    /* create detail box */
    self->detailbox = GTK_VBOX(gtk_vbox_new(FALSE,0));
    GtkWidget *title = gtk_label_new ("Message Membership");
    gtk_misc_set_alignment (GTK_MISC(title),0.4,1);
    gtk_box_pack_start (GTK_BOX(self->detailbox),title,FALSE,TRUE,0);

    GSList *c;
    GtkWidget *rdo_btn = NULL;
    GSList *rdo_btn_group;
    c = self->folderlist;

    for (; c; c = g_slist_next(c) ) {
        gchar* folder = (gchar*) c->data;
	      g_debug( "find folder '%s'", folder );
	      if(!g_strcasecmp(folder,"Inbox")){
	         rdo_btn = gtk_radio_button_new_with_label (NULL, folder);
	         rdo_btn_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rdo_btn));
	         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rdo_btn), TRUE);
        }
	      else
	         rdo_btn = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rdo_btn), folder);
	
	      g_signal_connect_swapped (G_OBJECT(rdo_btn), 
	                                "released",
			                            G_CALLBACK (on_mmode_rdo_btn_clicked),
			                            c->data);
	      gtk_box_pack_start (GTK_BOX (self->detailbox), rdo_btn, FALSE, TRUE, 0);
    }

    return GTK_WIDGET(self->detailbox);
}

void detail_new_sms (DetailArea* self)
{
		EditAttributes* editAttributes = self->editAttributes;
		gtk_entry_set_text (GTK_ENTRY(editAttributes->toEntry),"");
		GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editAttributes->txtView));
		gtk_text_buffer_set_text (buffer,"",0);
		gtk_notebook_set_current_page (self->notebook,PAGE_EDIT_MODE);
}

void detail_read_message (DetailArea* self, message* msg)
{
		ReadAttributes* readAttributes = self->readAttributes;
		if(msg != NULL)
			{
		    gtk_label_set_text(GTK_LABEL(readAttributes->from_label), msg->name);
		    gtk_label_set_text(GTK_LABEL(readAttributes->date_label), msg->subject);
		    gtk_label_set_text(GTK_LABEL(readAttributes->details), msg->folder);
		    g_free(msg);
		  }
		else
		  {
			  gtk_label_set_text(GTK_LABEL(readAttributes->from_label), "");
		    gtk_label_set_text(GTK_LABEL(readAttributes->date_label), "");
		    gtk_label_set_text(GTK_LABEL(readAttributes->details), "please select a message");
		  }
		gtk_notebook_set_current_page (self->notebook,PAGE_MODE_READ);
}

void detail_reply_message (DetailArea* self, message* msg)
{
		EditAttributes* editAttributes = self->editAttributes;
		if(msg != NULL)
			{
	  	  gtk_entry_set_text(GTK_ENTRY(editAttributes->toEntry), msg->name);
	   	 g_free(msg);
	 		}
	 	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editAttributes->txtView));
	 	gtk_text_buffer_set_text (buffer,"",0);
		gtk_notebook_set_current_page (self->notebook,PAGE_EDIT_MODE);
}

void detail_forward_message (DetailArea* self, message* msg)
{
		EditAttributes* editAttributes = self->editAttributes;
		if(msg != NULL)
			{
	  	  gtk_entry_set_text(GTK_ENTRY(editAttributes->toEntry), msg->name);
	   	 g_free(msg);
	 		}
	  GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editAttributes->txtView));
	  gchar *fwdStr = g_strdup_printf("\n\n\n>%s",msg->content);
		gtk_text_buffer_set_text (buffer,fwdStr,strlen(fwdStr));
		gtk_notebook_set_current_page (self->notebook,PAGE_EDIT_MODE);
}
