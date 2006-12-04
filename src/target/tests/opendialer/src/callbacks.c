#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "contacts.h"
static gint hboxLowerX0=0;
static gint hboxLowerY0=0;
static gint hboxLowerX1=0;
static gint hboxLowerY1=0;
static gint hboxLowerWidth0=0;
static gint hboxLowerHeight0=0;
static gint hboxLowerWidth1=0;
static gint hboxLowerHeight1=0;

static gint eventboxNumberWidth0=0;
static gint eventboxNumberHeight0=0;
static gint eventboxNumberWidth1=0;
static gint eventboxNumberHeight1=0;


static gint eventboxFunctionWidth0=0;
static gint eventboxFunctionHeight0=0;
static gint eventboxFunctionWidth1=0;
static gint eventboxFunctionHeight1=0;
#define MAXDIALNUMBERLEN 50
#define MAXNAMEDESC 180
extern DIALER_CONTACTS_LIST_HEAD       contactlist; //in file opendialer.c
extern GtkWidget *menuopendialer;
extern GtkWidget *menuactions;
extern GtkWidget *window1;
void
on_buttonUpper_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *widget;
GtkWidget *fixed;
widget= lookup_widget (GTK_WIDGET(button), "hboxTop");
gtk_widget_hide(widget);
widget = lookup_widget (GTK_WIDGET(button), "vboxNameDisplay");
gtk_widget_show(widget);

	
fixed= lookup_widget (GTK_WIDGET(button), "fixed1");
widget= lookup_widget (GTK_WIDGET(button), "hboxLower");

if(hboxLowerX0==0&&hboxLowerY0==0)	
{
	hboxLowerX0=widget->allocation.x;
	hboxLowerY0=widget->allocation.y;
	hboxLowerWidth0=widget->allocation.width;
	hboxLowerHeight0=widget->allocation.height;
	
	hboxLowerX1=0;
	hboxLowerY1=72;
	hboxLowerWidth1=hboxLowerWidth0;
	hboxLowerHeight1=hboxLowerHeight0+hboxLowerY0-hboxLowerY1;
	
}

gtk_fixed_move(GTK_FIXED(fixed),widget,0,72);
gtk_widget_set_size_request(widget,hboxLowerWidth1,hboxLowerHeight1);

widget= lookup_widget (GTK_WIDGET(button), "eventboxNumber");
if(eventboxNumberWidth0==0&&eventboxNumberHeight0==0)
{
eventboxNumberWidth0=widget->allocation.width;
eventboxNumberHeight0=widget->allocation.height;
eventboxNumberWidth1=eventboxNumberWidth0;
eventboxNumberHeight1=eventboxNumberHeight0+hboxLowerHeight1-hboxLowerHeight0;
}
gtk_widget_set_size_request(widget,eventboxNumberWidth1,eventboxNumberHeight1);

widget= lookup_widget (GTK_WIDGET(button), "eventboxFunction");
if(eventboxFunctionWidth0==0&&eventboxFunctionHeight0==0)
{
eventboxFunctionWidth0=widget->allocation.width;
eventboxFunctionHeight0=widget->allocation.height;
eventboxFunctionWidth1=eventboxFunctionWidth0;
eventboxFunctionHeight1=eventboxFunctionHeight0+hboxLowerHeight1-hboxLowerHeight0;
}
gtk_widget_set_size_request(widget,eventboxFunctionWidth1,eventboxFunctionHeight1);


gtk_widget_show(fixed);


}

void
on_buttonStylusMode_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *widget;
GtkWidget *fixed;
widget= lookup_widget (GTK_WIDGET(button), "hboxTop");
gtk_widget_show(widget);
	
widget = lookup_widget (GTK_WIDGET(button), "vboxNameDisplay");
gtk_widget_hide(widget);
	
fixed= lookup_widget (GTK_WIDGET(button), "fixed1");
widget= lookup_widget (GTK_WIDGET(button), "hboxLower");
gtk_fixed_move(GTK_FIXED(fixed),widget,hboxLowerX0,hboxLowerY0);
gtk_widget_set_size_request(widget,hboxLowerWidth0,hboxLowerHeight0);

widget= lookup_widget (GTK_WIDGET(button), "eventboxNumber");	
gtk_widget_set_size_request(widget,eventboxNumberWidth0,eventboxNumberHeight0);
	
widget= lookup_widget (GTK_WIDGET(button), "eventboxFunction");	
gtk_widget_set_size_request(widget,eventboxFunctionWidth0,eventboxFunctionHeight0);
	
gtk_widget_show(fixed);
	
}

int number_clicked   (GtkButton    *button, const gchar* number)
{
gchar* codestring;
int len=0;
GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter start;
GtkTextIter end;
GtkTextIter selectioniter,insertiter;
GtkTextMark *selectmark,*insertmark;


text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
 /* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	
selectmark=gtk_text_buffer_get_selection_bound(buffer);
insertmark=	gtk_text_buffer_get_insert(buffer);
//get current cursor iterator
gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
gtk_text_buffer_get_iter_at_mark(buffer,&selectioniter,selectmark);
	//to see whether there is a selection range.
if(gtk_text_iter_get_offset(&insertiter)!=gtk_text_iter_get_offset(&selectioniter))
	{
		//yes, first delete the range.
	gtk_text_buffer_delete(buffer,&selectioniter,&insertiter);
	insertmark=	gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
	}

 gtk_text_buffer_get_start_iter (buffer, &start);
 gtk_text_buffer_get_end_iter (buffer, &end);

  /* Get the entire buffer text. */
  codestring = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

	len=strlen(codestring);
	if(len>0&&len<MAXDIALNUMBERLEN)
	{
	gtk_text_buffer_insert_at_cursor( buffer,number,1);
	len=len+1;

	}
	else
	{
	gtk_text_buffer_set_text (buffer,number, -1);
	len=1;
	}
	rebuild_contact_view(text_view);
	gtk_widget_grab_focus(text_view);
	g_free (codestring );
	return len;
}



void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("1"));
}


void
on_button2ABC_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("2"));
}


void
on_button3DEF_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("3"));
}


void
on_button4GHI_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("4"));
}


void
on_button5JKL_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("5"));
}


void
on_button6MNO_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("6"));
}


void
on_button7PQRS_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("7"));
}


void
on_button8TUV_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("8"));
}


void
on_button9WXYZ_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("9"));
}


void
on_buttonAsterisk_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("*"));
}


void
on_button0_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("0"));
}


void
on_buttonSharp_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("#"));
}


void
on_buttonPlus_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
number_clicked(button,_("+"));
}
//set the text left to the cursor to black, and right to red.
void settextviewcolor(GtkWidget *text_view)
{
//GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter cursoriter,start,end;
gchar * tag_name;

//text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
 /* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	
tag_name = g_object_get_data (G_OBJECT (text_view), "tag");

if(tag_name==NULL)
{
  gtk_text_buffer_create_tag (buffer, "color",
                              "foreground", "#FF0000", 
                              NULL); 
  g_object_set_data (G_OBJECT (text_view), "tag", "color");	
  tag_name = g_object_get_data (G_OBJECT (text_view), "tag");
}

gtk_text_buffer_get_iter_at_mark(buffer,
		&cursoriter,gtk_text_buffer_get_insert(buffer));

 gtk_text_buffer_get_start_iter (buffer, &start);
 gtk_text_buffer_get_end_iter (buffer, &end);

//g_printf("outter tag_name=%s\n",tag_name);
  /* Get iters at the beginning and end of current selection. */
  

  /* Apply the tag to the selected text. */
  gtk_text_buffer_remove_tag_by_name (buffer, tag_name , &start, &cursoriter);	
  gtk_text_buffer_apply_tag_by_name (buffer, tag_name , &cursoriter, &end);	
  


}


void textviewCodes_move_cursor(GtkButton       *button,int direction)
{
GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter cursoriter;
text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
 /* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
gtk_text_buffer_get_iter_at_mark(buffer,
		&cursoriter,gtk_text_buffer_get_insert(buffer));
if(direction==-1)
	gtk_text_iter_backward_cursor_position(&cursoriter);
else
	gtk_text_iter_forward_cursor_position(&cursoriter);
gtk_text_buffer_place_cursor(buffer,&cursoriter);
settextviewcolor(text_view);
gtk_widget_grab_focus(text_view);
}

void
on_buttonBack_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *text_view;
textviewCodes_move_cursor(button,-1);
text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
rebuild_contact_view(text_view);

}


void
on_buttonForward_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *text_view;
textviewCodes_move_cursor(button,1);
text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
rebuild_contact_view(text_view);
}


void
on_buttonDel_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter selectioniter,insertiter;
GtkTextMark *selectmark,*insertmark;
	text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
 /* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

selectmark=gtk_text_buffer_get_selection_bound(buffer);
insertmark=	gtk_text_buffer_get_insert(buffer);
//get current cursor iterator
gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
gtk_text_buffer_get_iter_at_mark(buffer,&selectioniter,selectmark);
	//to see whether there is a selection range.
	if(gtk_text_iter_get_offset(&insertiter)!=gtk_text_iter_get_offset(&selectioniter))
	{
		//yes, first delete the range.
	gtk_text_buffer_delete(buffer,&selectioniter,&insertiter);
	}
	else
	{
		//no selection, then just perform backspace.
	gtk_text_buffer_backspace(buffer,&insertiter,TRUE,TRUE);
	}
//textviewCodesSensentive(text_view);
rebuild_contact_view(text_view);
//gtk_widget_grab_focus(text_view);
}

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{
release_contact_list(&contactlist);
gtk_main_quit ();
}

void
on_textviewCodes_backspace             (GtkTextView     *textview,
                                        gpointer         user_data)
{

g_printf("on_textviewCodes_backspace\n");
}


void
on_textviewCodes_delete_from_cursor    (GtkTextView     *textview,
                                        GtkDeleteType    type,
                                        gint             count,
                                        gpointer         user_data)
{
g_printf("on_textviewCodes_delete_from_cursor    \n");
rebuild_contact_view(textview);
}


void
on_textviewCodes_insert_at_cursor      (GtkTextView     *textview,
                                        gchar           *string,
                                        gpointer         user_data)
{
g_printf("on_textviewCodes_insert_at_cursor    \n");
}


void
on_textviewCodes_move_cursor           (GtkTextView     *textview,
                                        GtkMovementStep  step,
                                        gint             count,
                                        gboolean         extend_selection,
                                        gpointer         user_data)
{
//	settextviewcolor(textview);

}


gboolean
on_textviewCodes_focus_out_event       (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data)
{
//g_printf("on_textviewCodes_focus_out_event\n");
  return FALSE;
}

//uppon clicked, user agrees with our sensentive fillin, so make the input 
//solid.
void
on_buttonMagic_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter start, end;
gchar *tag_name=NULL;
	text_view= lookup_widget (GTK_WIDGET(button), "textviewCodes");
 /* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
tag_name = g_object_get_data (G_OBJECT (text_view), "tag");

if(tag_name==NULL)
{
  gtk_text_buffer_create_tag (buffer, "color",
                              "foreground", "#FF0000", 
                              NULL); 
  g_object_set_data (G_OBJECT (text_view), "tag", "color");	
  tag_name = g_object_get_data (G_OBJECT (text_view), "tag");
}

 gtk_text_buffer_get_start_iter (buffer, &start);
 gtk_text_buffer_get_end_iter (buffer, &end);

//g_printf("outter tag_name=%s\n",tag_name);
  /* Get iters at the beginning and end of current selection. */
  //gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

  /* Apply the tag to the whole text. */
  gtk_text_buffer_remove_tag_by_name (buffer, tag_name , &start, &end);	
//set the cursor to the end of the buffer
  gtk_text_buffer_place_cursor(buffer,&end);
//refresh the treeview;
 rebuild_contact_view(text_view);
}
//get the input section of the textview
int  textviewCodesgetinput(GtkWidget *widget,gchar** input)
{
gchar* codestring;
GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter start;
GtkTextIter insertiter;
GtkTextMark *insertmark;
	

text_view= lookup_widget (GTK_WIDGET(widget), "textviewCodes");
/* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	
//get current cursor iterator
insertmark=	gtk_text_buffer_get_insert(buffer);
gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
//get start & end iterator
gtk_text_buffer_get_start_iter (buffer, &start);

if(gtk_text_iter_get_offset(&insertiter)==gtk_text_iter_get_offset(&start))
{
	strcpy(*input,"");
	return 0;
}
  /* Get the entire buffer text. */
codestring = gtk_text_buffer_get_text (buffer, &start, &insertiter, FALSE);
strcpy(*input,codestring);
g_free(codestring);
return 1;
}
int  settextviewCodesSensentiveString(GtkWidget *widget,gchar* string)
{

gchar* codestring;
GtkWidget *text_view;
GtkTextBuffer *buffer;
GtkTextIter start;
GtkTextIter end;
GtkTextIter insertiter;
GtkTextMark *insertmark;
gint offset;
gint offsetend;	
text_view= lookup_widget (GTK_WIDGET(widget), "textviewCodes");
/* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	//about tag_name
gchar * tag_name=NULL;
	
//get current cursor iterator
insertmark=	gtk_text_buffer_get_insert(buffer);
gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
//get start & end iterator
gtk_text_buffer_get_start_iter (buffer, &start);
gtk_text_buffer_get_end_iter (buffer, &end);
offsetend=gtk_text_iter_get_offset(&end);
offset=gtk_text_iter_get_offset(&insertiter);
//if insertpos=endpos, that means we didn't input anything
//so we just insert the text.
/*
if(offsetend==offset)
{
	
	gtk_text_buffer_set_text(buffer,string,-1);
	settextviewcolor(text_view);
	gtk_widget_grab_focus(text_view);
	return 1;

}
*/
  /* Get the entire buffer text. */
codestring = gtk_text_buffer_get_text (buffer, &start, &insertiter, FALSE);

gtk_text_buffer_delete(buffer,&insertiter,&end);


//reget current cursor iterator
insertmark=	gtk_text_buffer_get_insert(buffer);
gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);


tag_name = g_object_get_data (G_OBJECT (text_view), "tag");
if(tag_name==NULL)
{
  gtk_text_buffer_create_tag (buffer, "color",
                              "foreground", "#FF0000", 
                              NULL); 
  g_object_set_data (G_OBJECT (text_view), "tag", "color");	
  tag_name = g_object_get_data (G_OBJECT (text_view), "tag");
}
 
//here we have to call the get sensentivestring to get "139" or something.
//gtk_text_buffer_insert_with_tags_by_name(buffer,&insertiter,"139",3,tag_name);
if(string!=0)
{
	if(strlen(string)>0)
	{
	gtk_text_buffer_insert( buffer,&insertiter,string,strlen(string));

//reget current cursor iterator
	insertmark=	gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
	gtk_text_iter_set_offset(&insertiter,offset);
	}
}

//setback the cursor position
gtk_text_buffer_place_cursor(buffer,&insertiter);
	
settextviewcolor(text_view);
gtk_widget_grab_focus(text_view);
g_free (codestring );
return 1;
}

int  rebuild_contact_view(GtkWidget *text_view)
{
	
gchar* codestring;
GtkTextBuffer *buffer;
GtkTextIter start;
GtkTextIter insertiter;
GtkTextMark *insertmark;
gchar* sstring;
/* Obtaining the buffer associated with the widget. */
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	//about tag_name
sstring=(gchar *)calloc(sizeof(gchar),MAXDIALNUMBERLEN);

//get current cursor iterator
insertmark=	gtk_text_buffer_get_insert(buffer);
gtk_text_buffer_get_iter_at_mark(buffer,&insertiter,insertmark);
//get start & end iterator
gtk_text_buffer_get_start_iter (buffer, &start);
  /* Get the entire buffer text. */
codestring = gtk_text_buffer_get_text (buffer, &start, &insertiter, FALSE);

rebuild_contact_view_with_string(text_view,codestring,0); 
g_free (codestring );
free(sstring);
return 1;
	
}
//if the current selection is valid, then sensatively display the number_clicked
//if no sensentive found, display the first entry.

void
on_treeviewContacts_cursor_changed     (GtkTreeView     *treeview,
                                        gpointer         user_data)
{
gchar *desc;
gchar *content;
gchar *name;
gchar *temp;
GtkWidget *label;
GtkTreeIter iter;
GtkTreePath* path;
GtkTreeModel* model;
GtkTreeSelection * selection;
label=lookup_widget (GTK_WIDGET(treeview), "labelPersonName");	
if(label==NULL)
		return;


selection = gtk_tree_view_get_selection(treeview);
 if (!gtk_tree_selection_get_selected(selection, &model, &iter))
 {	 //g_printf("no current selection\n");
	 gtk_label_set_text(GTK_LABEL(label),"New Contact");
	 settextviewCodesSensentiveString((GtkWidget*)treeview,"");
	 return ;
 }
	
	
if(get_select_line(treeview,&name,&desc,&content)!=0)
{
   // g_printf("view selected row is:%s, %s,%s\n",name, desc,content);	

	//buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

	temp=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC);
	
	
	snprintf(temp,MAXNAMEDESC,"%s-%s",name,desc);
	
	
	gtk_label_set_text(GTK_LABEL(label),temp);

	textviewCodesgetinput((GtkWidget*)treeview,&temp);
	
	int i=strlen(temp);
	//g_printf("%s,%d\n",&content[i],i);
	
	//if(hassensentive(content,temp))
    settextviewCodesSensentiveString((GtkWidget*)treeview,&content[i]);
	
	//gtk_text_buffer_set_text (buffer,content, -1);	
	
	g_free(name);
	g_free(desc);
	g_free(content);
	g_free(temp);
	
}
else
{
model=gtk_tree_view_get_model(treeview);
path=gtk_tree_model_get_path(model,&iter);
	
gtk_tree_view_expand_row(treeview,path,TRUE);
gtk_tree_path_down(path);
gtk_tree_view_set_cursor(treeview,path,0,0);
gtk_tree_path_free(path);
}
}

void
on_buttonPrevious_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
 GtkTreeSelection    *selection;
 GtkTreeModel        *model;
 GtkTreeIter         iter;
// GtkTreeIter         parentiter;
// GtkTreeIter         son;
 GtkTreePath* path;
 GtkTreeView * treeview;
 treeview=(GtkTreeView*)lookup_widget (GTK_WIDGET(button), "treeviewContacts");
 if(treeview==0)return ;
	 
 selection = gtk_tree_view_get_selection(treeview);

 if (!gtk_tree_selection_get_selected(selection, &model, &iter))
 {
	 //g_printf("no current selection\n");
	 
	 return ;
 }

	  //is the person selected?
	if(gtk_tree_store_iter_depth(GTK_TREE_STORE(model),&iter)==0)
	{
		//g_printf("the person is selected,we need to change the selection to prev and down.\n");
		path=gtk_tree_model_get_path(model,&iter);
		if(!gtk_tree_path_prev(path))
		{
			//g_printf("no prev for the top level\n");
				gtk_tree_path_free(path);
				return;
		}
		if(!gtk_tree_model_get_iter(model,&iter,path))
			{
			gtk_tree_path_free(path);
			return;
			}

		//we now get the number of the son.
				gint sonnum=gtk_tree_model_iter_n_children(model,&iter);
				gint i;
				if(sonnum>0)
					gtk_tree_path_down(path);
				for(i=1;i<sonnum;i++)
				{
					//g_printf("i=%d\n",i);
					gtk_tree_path_next(path);
				}
				gtk_tree_view_set_cursor(treeview,path,0,0);
				gtk_tree_path_free(path);
				return;
	}
	else
	{
		//from iter, we get path, then use path_prev to get the prev iter.
		//we need to go to the next son.
	    //g_printf("we need to go to the prev son\n");	
		path=gtk_tree_model_get_path(model,&iter);
		if(!gtk_tree_path_prev(path))
		{//no previous iterm
			//we have to get parent path here
			gtk_tree_path_up(path);
			if(!gtk_tree_path_prev(path))
			{//parent has no prev,has to return;
				gtk_tree_path_free(path);
				return;
			}
			if(!gtk_tree_model_get_iter(model,&iter,path))
			{
			gtk_tree_path_free(path);
			return;
			}
			else
			{//we get the number of the son.
				gint sonnum=gtk_tree_model_iter_n_children(model,&iter);
				gint i;
				if(sonnum>0)
					gtk_tree_path_down(path);
				for(i=1;i<sonnum;i++)
				{
					gtk_tree_path_next(path);
				}
				gtk_tree_view_set_cursor(treeview,path,0,0);
				gtk_tree_path_free(path);
				return;
				 
	}
	return ;

}
  else
		{
			//g_printf("select previous one\n");
				gtk_tree_view_set_cursor(treeview,path,0,0);
				gtk_tree_path_free(path);

			
		}
}
}


void
on_buttonDown_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{

 GtkTreeSelection    *selection;
 GtkTreeModel        *model;
 GtkTreeIter         iter;
 GtkTreeIter         parentiter;
 GtkTreeIter         son;
 GtkTreePath* path;
 GtkTreeView * treeview;
 treeview=lookup_widget (GTK_WIDGET(button), "treeviewContacts");
 if(treeview==0)return ;
	 
 selection = gtk_tree_view_get_selection(treeview);

 if (!gtk_tree_selection_get_selected(selection, &model, &iter))
 {
	 g_printf("no current selection\n");
	 return ;
 }

	  //is the person selected?
	if(gtk_tree_store_iter_depth(GTK_TREE_STORE(model),&iter)==0)
	{
		g_printf("the person is selected,we need to change the selection to down.\n");
		if(gtk_tree_model_iter_children(model,&son,&iter))
		{
		path=gtk_tree_model_get_path(model,&son);
		gtk_tree_view_set_cursor(treeview,path,0,0);
		gtk_tree_path_free(path);
		}
		//??>?
		return ;
	}
	else
	{
		
		//we need to go to the next son.
	 if(gtk_tree_model_iter_next(model,&iter))
	 {
		path=gtk_tree_model_get_path(model,&iter);
		gtk_tree_view_set_cursor(treeview,path,0,0);
		gtk_tree_path_free(path);
		return ;
	 }
	 else
	 {
		 //iter destroied, reget it.
		 gtk_tree_selection_get_selected(selection, &model, &iter);
		 //find the parent
		if(!gtk_tree_model_iter_parent(model,&parentiter,&iter))
			 return ;
		 			 //get the first son of the parent
		if(!gtk_tree_model_iter_next(model,&parentiter))
				 return ;
			 
		if(!gtk_tree_model_iter_children(model,&iter,&parentiter))
				return;
				//reset the selection.
		path=gtk_tree_model_get_path(model,&iter);
		gtk_tree_view_set_cursor(treeview,path,0,0);
		gtk_tree_path_free(path);
		return ;

		 		 
	 }
			 
	}
	return ;
}

void
on_treeviewContacts_row_collapsed      (GtkTreeView     *treeview,
                                        GtkTreeIter     *iter,
                                        GtkTreePath     *path,
                                        gpointer         user_data)
{
gtk_tree_view_expand_row(treeview,path,TRUE);
gtk_tree_path_down(path);
gtk_tree_view_set_cursor(treeview,path,0,0);
return;
}


void
on_entrySearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
GtkTextBuffer *buffer;
	GtkTextView *text_view;
GtkTextIter start;
GtkTextIter end;
//GtkTextMark *insertmark;
gchar* sstring;

	
text_view= lookup_widget (GTK_WIDGET(editable), "textviewCodes");
buffer=gtk_text_view_get_buffer(text_view);
//gtk_text_buffer_set_text(buffer,"",-1);
gtk_text_buffer_get_start_iter (buffer, &start);
gtk_text_buffer_get_end_iter (buffer, &end);
gtk_text_buffer_delete(buffer,&start,&end);

	
sstring=gtk_editable_get_chars(editable,0,-1);	

rebuild_contact_view_with_string(text_view,sstring,1); 
//settextviewcolor(text_view);
	
g_free(sstring);

gtk_widget_grab_focus(GTK_WIDGET(editable));
gtk_editable_select_region(editable,-1,-1);
return 1;

	
}

void
on_entrySearch_activate                (GtkEntry        *entry,
                                        gpointer         user_data)
{
   gchar* sstring;
   sstring=gtk_entry_get_text(entry);	
   g_printf("%s\n",sstring);
if(namehasstring(sstring,"Search Person"))
		gtk_entry_set_text(entry,"");
	
}

void
on_menuhelp_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_menuquit_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
release_contact_list(&contactlist);
gtk_main_quit();
}

void
on_buttonDialer_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{

	//GtkWidget * menudialer;
//menudialer= lookup_widget (GTK_WIDGET(button), "menuOpenDialer");
gtk_menu_popup(GTK_MENU(menuopendialer),0,0,0,0,0,0);
}

void
on_menudial_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkMessageDialog * dialog;
gchar*	temp;
gchar*	temp2;
gchar*	temp3;
	temp=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC);
	temp3=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC*4);
GtkWidget *label;
label=lookup_widget (GTK_WIDGET(window1), "labelPersonName");	
temp2=gtk_label_get_text(GTK_LABEL(label));
	textviewCodesgetinput((GtkWidget*)window1,&temp);
	if(strlen(temp)>0)
	{
		snprintf(temp3,MAXNAMEDESC*4,"./openmoko-callhandlering dial \"%s,-,%s\"",temp2,temp);
		
		int ret=system(temp3);
		g_printf("system(%s) returns:%d\n",temp3,ret);
		if(ret!=0)
	{
	dialog=gtk_message_dialog_new(0,
	GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,
	GTK_BUTTONS_CLOSE,"We are about to make a call to %s, the number is %s,but the sysem call failed.",temp2,temp);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
		}
		
	}
	
free(temp);
free(temp3);
}


void
on_menusend_message_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkMessageDialog * dialog;
gchar*	temp;
gchar*	temp3;
	temp=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC);
	temp3=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC*3);
	textviewCodesgetinput((GtkWidget*)window1,&temp);
		if(strlen(temp)>0)
	{
	snprintf(temp3,MAXNAMEDESC*4,"./messager %s",temp);
    
	int ret=system(temp3);
	g_printf("system(%s) returns:%d\n",temp3,ret);
		if(ret!=0)
		{
			
	dialog=gtk_message_dialog_new(0,
	GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,
	GTK_BUTTONS_CLOSE,"We are about to send a SMS to the number %s,but the sysem call failed.",temp);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
			
			
		}
	}
	free(temp);


}


void
on_menuadd_number_to_contact1_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkMessageDialog * dialog;
gchar*	temp;
	temp=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC);
	textviewCodesgetinput((GtkWidget*)window1,&temp);
		if(strlen(temp)>0)
		{
	dialog=gtk_message_dialog_new(0,
	GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,
	GTK_BUTTONS_CLOSE,"We are about to add number %s to the contact.",temp);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
		}
	free(temp);


}


void
on_menunew_contact_with_this_number1_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkMessageDialog * dialog;
gchar*	temp;
	temp=(gchar*)calloc(sizeof(gchar),MAXNAMEDESC);
	textviewCodesgetinput((GtkWidget*)window1,&temp);
		if(strlen(temp)>0)
		{
	dialog=gtk_message_dialog_new(0,
	GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,
	GTK_BUTTONS_CLOSE,"We are about to new a contact with the number %s",temp);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
		}
	free(temp);

}

void
on_buttonActions_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
gtk_menu_popup(GTK_MENU(menuactions),0,0,0,0,0,0);
}

void
on_buttonIcon_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	

}
