#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#define ENTRY_WIDTH 240
#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)
#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

gchar * operation = "sms";

/*
 * reset the txtView size and the input area
 */
void resetScrolledWindow(GtkWidget     *menuitem)
{
	GtkWidget * fixed1;
	GtkWidget * viewport1;
	GtkWidget * sms_input_label;
	GtkWidget * toFixed;

	fixed1 = lookup_widget (GTK_WIDGET(menuitem),"fixed1");		
	toFixed = lookup_widget (GTK_WIDGET(menuitem),"toFixed");	
	viewport1 = lookup_widget (GTK_WIDGET(menuitem),"viewport1");
	sms_input_label = lookup_widget (GTK_WIDGET(menuitem),"sms_input_label");
	
	gtk_widget_set_size_request (viewport1,480, 125);
	gtk_widget_set_size_request (toFixed, 480, 125);
	if(sms_input_label != NULL)
		gtk_widget_destroy(sms_input_label);
}

/*
 * main function for display the interface
 * depend on what function you will use
 * it will display the required interface
 */
void
display		(GtkWidget     *widget)
{
	/* used for operation */
	GtkWidget * toFixed;
	GtkWidget * fixed_old;
	GtkWidget * window;
	GtkWidget * fixed1;
	GtkWidget * viewport1;
	/* in order to get processed*/
	GtkWidget * btnSave;
	/* used to store the info*/
	GtkTextBuffer * buffer;
	GtkWidget * txtView;
	/*used to display email */
	GtkWidget * email_to_entry;
	GtkWidget * email_cc_entry;
	GtkWidget * email_bcc_entry;
	GtkWidget * email_subject_entry;
	GtkWidget * label1;	/* used for common*/
	GtkWidget * label2;
	GtkWidget * label3;
	GtkWidget * label4;
	/* used to display im*/
	GtkWidget *tmp_image;
	GtkWidget *im_to_entry;
	GtkWidget *toolbutton1;
	GtkWidget *toolbutton2;
	GtkWidget *toolbutton3;
	GtkWidget *toolbar1;
	GtkIconSize tmp_toolbar_icon_size;
	/* used to display sms*/
	GtkWidget *sms_to_entry;
	GtkWidget *sms_input_label;
	
	/* reset the window for the sms case */	
	resetScrolledWindow (widget);
	/* set the txtView */
	txtView = lookup_widget (GTK_WIDGET(widget),"txtView");	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(txtView));

	if(g_str_equal(operation,"email"))
	{
		gtk_text_buffer_set_text (buffer,"This is a sample email...",-1);
	}

	if(g_str_equal(operation,"im"))
	{
		gtk_text_buffer_set_text (buffer,"This is a sample IM...",-1);
	}
	
	if(g_str_equal(operation,"sms"))
	{
		gtk_text_buffer_set_text (buffer,"This is a sample SMS...",-1);
	}
	
	/* destroy the old display, create a new one below */
	fixed_old = lookup_widget (GTK_WIDGET(widget),"toFixed");
	window = lookup_widget (GTK_WIDGET(widget),"window1");
	btnSave = lookup_widget (GTK_WIDGET(widget),"btnSave");
	fixed1 = lookup_widget (GTK_WIDGET(widget),"fixed1");
	viewport1 = lookup_widget (GTK_WIDGET(widget),"viewport1");
	toFixed = lookup_widget (GTK_WIDGET(widget),"toFixed");

	gtk_widget_destroy(fixed_old);

	toFixed = gtk_fixed_new ();
  	gtk_widget_show (toFixed);
  	gtk_container_add (GTK_CONTAINER (viewport1), toFixed);
  	gtk_widget_set_size_request (toFixed, 480, 125);
  	gtk_container_set_border_width (GTK_CONTAINER (toFixed), 0);

	/* load the entry depend on the operation */
	if(g_str_equal(operation,"email"))
	{	
		email_to_entry = gtk_entry_new ();
		gtk_widget_show (email_to_entry);
		gtk_fixed_put (GTK_FIXED (toFixed), email_to_entry, 104, 5);
		gtk_widget_set_size_request (email_to_entry, ENTRY_WIDTH, 22);
	
		email_cc_entry = gtk_entry_new ();
		gtk_widget_show (email_cc_entry);
		gtk_fixed_put (GTK_FIXED (toFixed), email_cc_entry, 104, 32);
		gtk_widget_set_size_request (email_cc_entry, ENTRY_WIDTH, 22);
		
		email_bcc_entry = gtk_entry_new ();
		gtk_widget_show (email_bcc_entry);
		gtk_fixed_put (GTK_FIXED (toFixed), email_bcc_entry, 104, 59);
		gtk_widget_set_size_request (email_bcc_entry, ENTRY_WIDTH, 22);
		
		email_subject_entry = gtk_entry_new ();
		gtk_widget_show (email_subject_entry);
		gtk_fixed_put (GTK_FIXED (toFixed), email_subject_entry, 104, 86);
		gtk_widget_set_size_request (email_subject_entry, ENTRY_WIDTH, 22);
	
		label1 = gtk_label_new (_("To"));
		gtk_widget_show (label1);
		gtk_fixed_put (GTK_FIXED (toFixed), label1, 50, 5);
		gtk_widget_set_size_request (label1, 50, 16);
		gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
	
		label2 = gtk_label_new (_("Cc"));
		gtk_widget_show (label2);
		gtk_fixed_put (GTK_FIXED (toFixed), label2, 50, 32);
		gtk_widget_set_size_request (label2, 50, 16);
		gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (label2), TRUE);
		
		label3 = gtk_label_new (_("Bcc"));
		gtk_widget_show (label3);
		gtk_fixed_put (GTK_FIXED (toFixed), label3, 50, 59);
		gtk_widget_set_size_request (label3, 50, 16);
		gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (label3), TRUE);
	
		label4 = gtk_label_new (_("Subject"));
		gtk_widget_show (label4);
		gtk_fixed_put (GTK_FIXED (toFixed), label4, 50, 86);
		gtk_widget_set_size_request (label4, 50, 16);
		gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (label4), TRUE);

		GLADE_HOOKUP_OBJECT (window, toFixed, "toFixed");
		GLADE_HOOKUP_OBJECT (window, email_to_entry, "email_to_entry");
		GLADE_HOOKUP_OBJECT (window, email_cc_entry, "email_cc_entry");
		GLADE_HOOKUP_OBJECT (window, email_bcc_entry, "email_bcc_entry");
		GLADE_HOOKUP_OBJECT (window, email_subject_entry, "email_subject_entry");
		GLADE_HOOKUP_OBJECT (window, label1, "label1");
		GLADE_HOOKUP_OBJECT (window, label2, "label2");
		GLADE_HOOKUP_OBJECT (window, label3, "label3");
		GLADE_HOOKUP_OBJECT (window, label4, "label4");
		gtk_widget_show(btnSave);
	}

	if(g_str_equal(operation,"im"))
	{	
		label1 = gtk_label_new (_("To"));
		gtk_widget_show (label1);
		gtk_fixed_put (GTK_FIXED (toFixed), label1, 50, 10);
		gtk_widget_set_size_request (label1, 50, 16);
		gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);

		im_to_entry = gtk_entry_new ();
		gtk_widget_show (im_to_entry);
		gtk_fixed_put (GTK_FIXED (toFixed), im_to_entry, 104, 10);
		gtk_widget_set_size_request (im_to_entry, ENTRY_WIDTH, 22);
	
		toolbar1 = gtk_toolbar_new ();
		gtk_widget_show (toolbar1);
		gtk_fixed_put (GTK_FIXED (toFixed), toolbar1, 0, 90);
		gtk_widget_set_size_request (toolbar1, 480, 50);
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH);
		tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar1));
	
		tmp_image = gtk_image_new_from_stock ("gtk-dialog-authentication", tmp_toolbar_icon_size);
		gtk_widget_show (tmp_image);
		toolbutton1 = (GtkWidget*) gtk_tool_button_new (tmp_image, "");
		gtk_widget_show (toolbutton1);
		gtk_container_add (GTK_CONTAINER (toolbar1), toolbutton1);
		gtk_widget_set_size_request (toolbutton1, 48, 48);
		
		tmp_image = gtk_image_new_from_stock ("gtk-home", tmp_toolbar_icon_size);
		gtk_widget_show (tmp_image);
		toolbutton2 = (GtkWidget*) gtk_tool_button_new (tmp_image, "");
		gtk_widget_show (toolbutton2);
		gtk_container_add (GTK_CONTAINER (toolbar1), toolbutton2);
		gtk_widget_set_size_request (toolbutton2, 48, 48);
	
		tmp_image = gtk_image_new_from_stock ("gtk-network", tmp_toolbar_icon_size);
		gtk_widget_show (tmp_image);
		toolbutton3 = (GtkWidget*) gtk_tool_button_new (tmp_image, "");
		gtk_widget_show (toolbutton3);
		gtk_container_add (GTK_CONTAINER (toolbar1), toolbutton3);
		gtk_widget_set_size_request (toolbutton3, 48, 48);
		gtk_widget_hide (btnSave);

		GLADE_HOOKUP_OBJECT (window, toFixed, "toFixed");
		GLADE_HOOKUP_OBJECT (window, label1, "label1");
		GLADE_HOOKUP_OBJECT (window, im_to_entry, "im_to_entry");
		GLADE_HOOKUP_OBJECT (window, toolbar1, "toolbar1");
		GLADE_HOOKUP_OBJECT (window, toolbutton1, "toolbutton1");
		GLADE_HOOKUP_OBJECT (window, toolbutton2, "toolbutton2");
		GLADE_HOOKUP_OBJECT (window, toolbutton3, "toolbutton3");
	}

	if(g_str_equal(operation,"sms"))
	{	
		gtk_widget_set_size_request (viewport1, 480, 80);
  		gtk_widget_set_size_request (toFixed, 480, 80);

		sms_to_entry = gtk_entry_new ();
		gtk_widget_show (sms_to_entry);
		gtk_fixed_put (GTK_FIXED (toFixed), sms_to_entry, 104, 10);
		gtk_widget_set_size_request (sms_to_entry, ENTRY_WIDTH, 22);

		label1 = gtk_label_new (_("To"));
		gtk_widget_show (label1);
		gtk_fixed_put (GTK_FIXED (toFixed), label1, 50, 10);
		gtk_widget_set_size_request (label1, 50, 16);
		gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);

		sms_input_label = gtk_label_new (_("137/1"));
		gtk_widget_show (sms_input_label);
		gtk_fixed_put (GTK_FIXED (fixed1), sms_input_label, 10, 130);
		gtk_widget_set_size_request (sms_input_label, 450, 16);
		gtk_label_set_justify (GTK_LABEL (sms_input_label), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (sms_input_label), TRUE);

		gtk_widget_hide (btnSave);

		GLADE_HOOKUP_OBJECT (window, toFixed, "toFixed");
		GLADE_HOOKUP_OBJECT (window, sms_to_entry, "sms_to_entry");
		GLADE_HOOKUP_OBJECT (window, sms_input_label, "sms_input_label");
		GLADE_HOOKUP_OBJECT (window, label1, "label1");
	}
}


void
on_message_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_sms_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	operation="sms";
	display(GTK_WIDGET(menuitem));
}


void
on_email_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	operation="email";
	display(GTK_WIDGET(menuitem));
}


void
on_im_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	operation="im";
	display(GTK_WIDGET(menuitem));
}

void
on_btnSend_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	/*used to display email */
	GtkWidget * email_to_entry;
	GtkWidget * email_cc_entry;
	GtkWidget * email_bcc_entry;
	GtkWidget * email_subject_entry;
	/* used to display im*/
	GtkWidget *im_to_entry;
	/* used to display sms*/
	GtkWidget *sms_to_entry;
	/* get txtView text */
	GtkTextIter start, end;
	GtkWidget * gwTxtView;
	GtkTextBuffer * buffer;
	
	/** 	get the GtkTextView object "txtView"   	 */
	gwTxtView = lookup_widget (GTK_WIDGET(button),"txtView");
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gwTxtView));
	
	/*
	 * set the start and end point
	 * then get the characters
	 */
 	gtk_text_buffer_get_iter_at_offset(buffer,&start,0);
	gtk_text_buffer_get_iter_at_offset(buffer,&end,140);

	GString * s_msg;

	if(g_str_equal(operation,"email"))
	{
		email_to_entry = lookup_widget(GTK_WIDGET(button),"email_to_entry");
		email_cc_entry = lookup_widget(GTK_WIDGET(button),"email_cc_entry");
		email_bcc_entry = lookup_widget(GTK_WIDGET(button),"email_bcc_entry");
		email_subject_entry = lookup_widget(GTK_WIDGET(button),"email_subject_entry");
		g_print("this is an sending email\n");
		s_msg = g_string_new("");
		s_msg = g_string_append(s_msg, gtk_entry_get_text (GTK_ENTRY(email_to_entry)));
		s_msg = g_string_append(s_msg, gtk_entry_get_text (GTK_ENTRY(email_cc_entry)));
		s_msg = g_string_append(s_msg, gtk_entry_get_text (GTK_ENTRY(email_bcc_entry)));
		s_msg = g_string_append(s_msg, gtk_entry_get_text (GTK_ENTRY(email_subject_entry)));
		s_msg = g_string_append(s_msg, gtk_text_buffer_get_text(buffer,&start,&end,FALSE));
		g_print(s_msg->str);
	}

	if(g_str_equal(operation,"im"))
	{
		g_print("this is an sending im\n");
		im_to_entry = lookup_widget(GTK_WIDGET(button),"im_to_entry");
		s_msg = g_string_new("");
		s_msg = g_string_append(s_msg, gtk_entry_get_text (GTK_ENTRY(im_to_entry)));
		s_msg = g_string_append(s_msg, gtk_text_buffer_get_text(buffer,&start,&end,FALSE));
		g_print(s_msg->str);
	}
	
	if(g_str_equal(operation,"sms"))
	{
		g_print("this is an sending sms\n");
		sms_to_entry = lookup_widget(GTK_WIDGET(button),"sms_to_entry");
		s_msg = g_string_new("");
		s_msg = g_string_append(s_msg, gtk_entry_get_text (GTK_ENTRY(sms_to_entry)));
		s_msg = g_string_append(s_msg, gtk_text_buffer_get_text(buffer,&start,&end,FALSE));
		g_print(s_msg->str);
	}
}


void
on_btnSave_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	g_print("saving code will be write here\n");
}

gboolean
on_txtView_key_release_event         (GtkWidget       *widget,
                                       GdkEventKey     *event,
                                       gpointer         user_data)
{
	GtkTextBuffer * buffer;
	GtkLabel * sms_input_label;
	GtkTextView * txtView;	
        gint n,m;

        sms_input_label = lookup_widget(GTK_WIDGET(widget),"sms_input_label");
	if(sms_input_label!=NULL)
	{
		txtView = lookup_widget(GTK_WIDGET(widget),"txtView");
	        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (txtView));
	        n = 158 - gtk_text_buffer_get_char_count(buffer)%158;
		m = gtk_text_buffer_get_char_count(buffer)/158 + 1;
	        gtk_label_set_text(GTK_LABEL(sms_input_label),g_strdup_printf("%d/%d",n,m));
	}
	return FALSE;
}

void
on_txtView_backspace                 (GtkTextView     *textview,
                                       gpointer         user_data)
{
	GtkTextBuffer * buffer;
	GtkLabel * sms_input_label;
	gint n,m;

	sms_input_label = lookup_widget(GTK_WIDGET(textview),"sms_input_label");
	if(sms_input_label!=NULL)
	{
	        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
		n = 158 - gtk_text_buffer_get_char_count(buffer)%158;
		m = gtk_text_buffer_get_char_count(buffer)/158 + 1;
		gtk_label_set_text(GTK_LABEL(sms_input_label),g_strdup_printf("%d/%d",n,m));	
	}
}



void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{
	gtk_main_quit();
}

