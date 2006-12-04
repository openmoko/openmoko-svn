#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

gchar *operation;
gchar *string_usrname = "Hobbes";
gchar *string_operation;
gchar *string_fullname = "Calvin & Hobbes";
gchar *phonenum = "7606729199";

/*used for timing */
GTimer *mytimer;
gulong dumb_API_needs_this_variable;
gdouble time_elapsed;
gint func_ref;
gchar h_buffer[8];
gchar m_buffer[8];
gchar s_buffer[8];
gchar *result;
double h=0,m=0,s=0;
gchar *str_array[4];

void
on_call_handlering_activate            (GtkMenuItem     *menuitem,
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

/* function display*/
void display						  (GtkWidget       *widget)
{
	GtkWidget * gwInfoArea;
	GdkFont *font;
	GdkGC *gc;
	GtkWidget *callerImg;
	GtkWidget *fixed1;
	GtkWidget *btnAnswer;
	GtkWidget *btnIgnore;
	GtkWidget *btnCancel;
	GtkWidget *window1;
	gint i,n;
	GString *shownum;

	/* initialize the string_operation decided by vaiable operation */
	if(!strcmp(operation,"ans"))
		string_operation = "Incoming Call...";
	else if(!strcmp(operation,"dial"))
		string_operation = "Dialing...";

	gwInfoArea = lookup_widget ( GTK_WIDGET(widget), "infoArea" );
	fixed1 = lookup_widget ( GTK_WIDGET(widget), "fixed1" );
	callerImg = lookup_widget ( GTK_WIDGET(widget), "callerImg" );
	window1 = lookup_widget ( GTK_WIDGET(widget), "window1" );

	gc = gdk_gc_new (gwInfoArea->window);
	font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
	if(font != NULL)
	{
		/* load the image */
		callerImg = gtk_image_new_from_file ("./gnome-emacs.png");
		gtk_widget_show (callerImg);
  		gtk_fixed_put (GTK_FIXED (fixed1), callerImg, 10, 30);
	  	gtk_widget_set_size_request (callerImg, 460, 150);

		/* 
		 * display the message 
		 * user name, full name(optional), operation type
	 	 */
		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(string_usrname)*5, 20, string_usrname);

		font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(string_fullname)*5, 40, string_fullname);

		font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(string_operation)*5, 70, string_operation);

		font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
		
		/* insert '.' into the phone number */
		i=3;
		n=1;
		shownum = g_string_new(phonenum);
		while(i<strlen(phonenum))
		{
			g_string_insert(shownum,i,".");
			i=i+3+n;
			n++;
		}

		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(shownum->str)*5, 100, shownum->str);
	}
	else
		g_print("font is NULL");

	/* load different buttons depending on the operation */
	if(!strcmp(operation,"ans"))
	{
		btnAnswer = gtk_button_new_with_mnemonic (_("Answer"));
		gtk_widget_set_name (btnAnswer, "answerbutton");
		gtk_widget_show (btnAnswer);
		gtk_fixed_put (GTK_FIXED (fixed1), btnAnswer, 80, 538);
		gtk_widget_set_size_request (btnAnswer, 80, 30);

		btnIgnore = gtk_button_new_with_mnemonic (_("Ignore"));
		gtk_widget_show (btnIgnore);
		gtk_widget_set_name (btnIgnore, "savebutton");
		gtk_fixed_put (GTK_FIXED (fixed1), btnIgnore, 320, 538);
		gtk_widget_set_size_request (btnIgnore, 80, 30);

		g_signal_connect ((gpointer) btnAnswer, "clicked",
    		                G_CALLBACK (on_btnAnswer_clicked),
    		                NULL);
  		g_signal_connect ((gpointer) btnIgnore, "clicked",
    		                G_CALLBACK (on_btnIgnore_clicked),
    		                NULL);

		GLADE_HOOKUP_OBJECT (window1, btnAnswer, "btnAnswer");
		GLADE_HOOKUP_OBJECT (window1, btnIgnore, "btnIgnore");
	}
	else if(!strcmp(operation,"dial"))
	/* dial function will transmit into inCall after calling the gsm interface*/
	{
		btnCancel = gtk_button_new_with_mnemonic (_("Cancel"));
		gtk_widget_set_name (btnCancel, "savebutton");
		gtk_widget_show (btnCancel);
		gtk_fixed_put (GTK_FIXED (fixed1), btnCancel, 320, 538);
		gtk_widget_set_size_request (btnCancel, 80, 30);

		g_signal_connect ((gpointer) btnCancel, "clicked",
        	            G_CALLBACK (on_btnCancel_clicked),
        	            NULL);

		GLADE_HOOKUP_OBJECT (window1, btnCancel, "btnCancel");
		
		/*sleep(g_random_int_range (1, 4));*/

		/*inCall(widget);*/
	}
}

/* 
 * used to display the timer 
 * in order to record the time of the current call
 */
gboolean timing(gpointer data)
{
	GdkFont *font;
	GdkGC *gc;
	GtkWidget *widget;
	GdkColor color;

	/* get the drawing area for timer display */
	widget = (GtkWidget*)data;
	gc = gdk_gc_new (widget->window);
	
	/* 
	 * get the consuming time till now
	 * variables h,m,s must be int, or
	 * there would be some trouble 
	 */
	time_elapsed = g_timer_elapsed (mytimer, &dumb_API_needs_this_variable);
	h = (gint)(time_elapsed/3600);
	m = (gint)((time_elapsed - 3600*h)/60);
	s = (gint)(time_elapsed - 3600*h - 60*m);
	
	/* 
	 * use the g_ascii_formatd to convert double to string
	 * in our specified mode by "%1.f"
	 * in order to display 
	 */
	str_array[0]= g_ascii_formatd(h_buffer,8,"%1.f",h);
	str_array[1]= g_ascii_formatd(m_buffer,8,"%1.f",m);
	str_array[2]= g_ascii_formatd(s_buffer,8,"%1.f",s);
	str_array[3]= NULL;

	/* insert the ':' between hour,minute and second */
	result = g_strjoinv(":",str_array);

	/* 
	 * repaint the drawing area so that the info drawed last second
	 * would be clear
	 */
	gdk_color_parse ("white",&color);
   	gdk_colormap_alloc_color ( gtk_widget_get_colormap(widget), &color, FALSE, TRUE );
	gdk_gc_set_foreground(gc, &color);
   	gdk_draw_rectangle(widget->window, gc, 1, 0, 0, 300, 250);

	gdk_color_parse ("black",&color);
	gdk_colormap_alloc_color ( gtk_widget_get_colormap(widget), &color, FALSE, TRUE );
	gdk_gc_set_foreground(gc, &color);
	font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
	gdk_draw_string (widget->window, font, gc, 10, 20, "Talk Time");
	font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
	gdk_draw_string (widget->window, font, gc, 10, 40, result);

	/* 
	 * return TRUE to continue the loop
	 * FALSE would make it stopped 
	 * you can find this usage in GTK reference manual
	 */
	return TRUE;
}

/* display the inCall state */
void inCall						(GtkWidget       *widget)
{
	GtkWidget *btnRecord;
	GtkWidget *btnHangup;	
	GtkWidget *fixed1;
	GtkWidget *window1;
	GtkWidget *callerImg;
	GtkWidget *gwInfoArea;
	GtkWidget *gwOldArea;
	GtkWidget *timerArea;

	fixed1 = lookup_widget ( GTK_WIDGET(widget), "fixed1" );
	callerImg = lookup_widget ( GTK_WIDGET(widget), "callerImg" );
	window1 = lookup_widget ( GTK_WIDGET(widget), "window1" );
	gwOldArea = lookup_widget ( GTK_WIDGET(widget), "infoArea" );
	timerArea = lookup_widget ( GTK_WIDGET(widget), "timerArea" );

	/* clean the infomation displaying board */
	gtk_widget_destroy (gwOldArea);

	/* create a new one */
	gwInfoArea = gtk_drawing_area_new ();
	gtk_widget_set_name (gwInfoArea, "main");
  	gtk_widget_show (gwInfoArea);
  	gtk_fixed_put (GTK_FIXED (fixed1), gwInfoArea, 10, 200);
  	gtk_widget_set_size_request (gwInfoArea, 460, 320);
	GLADE_HOOKUP_OBJECT (window1, gwInfoArea, "gwInfoArea");
	/* 
	 * set the expose event function 
	 * the code of displaying the timer, infomation and singal strength
	 * in function on_gwInfoArea_expose_event
	 */
	g_signal_connect ((gpointer) gwInfoArea, "expose_event",
                    G_CALLBACK (on_gwInfoArea_expose_event),
                    NULL);

	/* load the related buttons */
	btnRecord = gtk_button_new_with_mnemonic (_("Record"));
	gtk_widget_set_name (btnRecord, "sendbutton");
	gtk_widget_show (btnRecord);
	gtk_fixed_put (GTK_FIXED (fixed1), btnRecord, 80, 538);
	gtk_widget_set_size_request (btnRecord, 80, 30);

	btnHangup = gtk_button_new_with_mnemonic (_("Hangup"));
	gtk_widget_set_name (btnHangup, "savebutton");
	gtk_widget_show (btnHangup);
	gtk_fixed_put (GTK_FIXED (fixed1), btnHangup, 320, 538);
	gtk_widget_set_size_request (btnHangup, 80, 30);

	g_signal_connect ((gpointer) btnRecord, "clicked",
        	            G_CALLBACK (on_btnRecord_clicked),
        	            NULL);
  	g_signal_connect ((gpointer) btnHangup, "clicked",
        	            G_CALLBACK (on_btnHangup_clicked),
        	            NULL);

	GLADE_HOOKUP_OBJECT (window1, btnRecord, "btnRecord");
	GLADE_HOOKUP_OBJECT (window1, btnHangup, "btnHangup");

	
	mytimer = g_timer_new ();
	func_ref = g_timeout_add(1000,timing,timerArea);
}

/* the application first start here */
gboolean
on_infoArea_expose_event               (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
	/* 
	 * display the interface depending on the variable operation
	 * which is set in main at the beginning 
	 */
	display(widget);
	
	return FALSE;
}

/* signal display */
gboolean signal(gpointer data)
{
    GdkColor color;
    GdkGC *gc;
    gint i,m;
    GtkWidget *gwInfoArea;
    GRand *rand;
    
    /* get the drawing area for timer display */
    gwInfoArea = (GtkWidget*)data;

    rand = g_rand_new();
    gc = gdk_gc_new (gwInfoArea->window);

    gdk_color_parse ("black",&color);
    gdk_colormap_alloc_color ( gtk_widget_get_colormap(gwInfoArea), &color, FALSE, TRUE );
    gdk_gc_set_foreground(gc, &color);
    for (i=0;i<7;i++)
	gdk_draw_rectangle(gwInfoArea->window, gc, 0, 120+i*30,180-10*i, 20, 20+i*10);

    m = g_rand_int_range(rand,4,6);
    for (i=0;i<7;i++)
    {
	if(i<m)
	{
	   gdk_color_parse ("orange",&color);
	   gdk_colormap_alloc_color ( gtk_widget_get_colormap(gwInfoArea), &color, FALSE, TRUE );
	   gdk_gc_set_foreground(gc, &color);
	   gdk_draw_rectangle(gwInfoArea->window, gc, 1, 120+i*30,180-10*i, 20, 20+i*10);
	 }
	else
	{
	    gdk_color_parse ("white",&color);
            gdk_colormap_alloc_color ( gtk_widget_get_colormap(gwInfoArea), &color, FALSE, TRUE );
            gdk_gc_set_foreground(gc, &color);
            gdk_draw_rectangle(gwInfoArea->window, gc, 1, 120+i*30,180-10*i, 20, 20+i*10);
	}
    }
}

/* the incall interface display function*/
gboolean
on_gwInfoArea_expose_event               (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
	GtkWidget *fixed1;
	GtkWidget *window1;
	GtkWidget *callerImg;
	GdkFont  *font;
	GtkWidget *gwInfoArea;
	GdkGC *gc;
	gint i,n;
	GString *shownum;

	fixed1 = lookup_widget ( GTK_WIDGET(widget), "fixed1" );
	callerImg = lookup_widget ( GTK_WIDGET(widget), "callerImg" );
	window1 = lookup_widget ( GTK_WIDGET(widget), "window1" );
	gwInfoArea = lookup_widget ( GTK_WIDGET(widget), "gwInfoArea" );

	/* display the info*/
	gc = gdk_gc_new(gwInfoArea->window);
	font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
	if(font != NULL)
	{
		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(string_usrname)*5, 20, string_usrname);

		font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");
		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(string_fullname)*5, 40, string_fullname);

		font = gdk_font_load ("-misc-fixed-medium-r-semicondensed--13-100-100-100-c-60-iso8859-1");

		i=3;
		n=1;
		shownum = g_string_new(phonenum);
		while(i<strlen(phonenum))
		{
			g_string_insert(shownum,i,".");
			i=i+3+n;
			n++;
		}

		gdk_draw_string (gwInfoArea->window, font, gc, 240-strlen(shownum->str)*5, 80, shownum->str);
	}
	else
		g_print("font is NULL");

	/* display the signal strength every 3 seconds*/
	g_timeout_add(1000,signal,gwInfoArea);
	
	return FALSE;
}

/* 
 * split the infomation into different pieces 
 * store them in the corresponding string 
 */
void
infoConstruction				(gchar* info)
{
	gint nIndex = 0;
	gchar ** infostruct;
	gchar *myspliter=",";

	infostruct = g_strsplit(info,myspliter,255);
	while(infostruct[nIndex]!=NULL)
	{
		if(nIndex == 0)
			string_usrname = infostruct[nIndex];
		if(nIndex == 1)
			string_fullname = infostruct[nIndex];
		if(nIndex == 2)
			phonenum = infostruct[nIndex];
		nIndex++;
	}
}

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_btnAnswer_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	inCall(GTK_WIDGET(button));
}


void
on_btnIgnore_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_btnCancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_btnRecord_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{

}

/* hangup function, we will add gsm call in it*/
void
on_btnHangup_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	g_timer_destroy (mytimer);
	g_source_remove (func_ref);
	gtk_main_quit();
}

