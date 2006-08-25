/***************************************************************************
 *            opendialer.c
 *
 *  Thu Aug 17 21:31:25 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/
#include "contacts.h"
#include <gtk/gtk.h>
#include <string.h>
#include "support.h"
#include "contacts.h"
#include "opendialer.h"


  DIALER_CONTACTS_LIST_HEAD       contactlist;
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 

/**
 * @brief Init the contact tree
 *
 * This function should be called once at the initial process.
 *
 * @param window The main window widget pointer
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
gint
init_contact_view (GtkWidget *window)
{
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore        *treestore;
  GtkTreeModel        *model;

  GtkWidget           *contactview=NULL;

//  gint                ret = 1;
  gint                res;

  contactview = lookup_widget (window, "treeviewContacts");

  if(contactview == NULL)
    return 0;

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Name"));
  gtk_tree_view_column_set_resizable (col, TRUE);
  //gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  //gtk_tree_view_column_set_fixed_width (col, 300);

  gtk_tree_view_append_column (GTK_TREE_VIEW (contactview), col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_NAME);

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Type"));

  gtk_tree_view_append_column (GTK_TREE_VIEW (contactview), col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_DESC);

  
  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Number"));

  gtk_tree_view_append_column (GTK_TREE_VIEW (contactview), col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_CONTENT);

  treestore = gtk_tree_store_new (COL_NUMS,
                                  G_TYPE_STRING,
                                  G_TYPE_STRING,
								  G_TYPE_STRING);

  model = GTK_TREE_MODEL (treestore);

  //get_package_list (treestore);

  res = init_contact_list (&contactlist);

  if(res == 0)
  {
    res = init_contact_list_cmd (&contactlist);

    if(contactlist.contacts!= NULL)
        init_contact_store (treestore, contactlist.contacts);
    
  }
  

  gtk_tree_view_set_model (GTK_TREE_VIEW(contactview), model);

  g_object_unref (model);

  //gtk_tree_view_set_expander_column(contactview,col);
  
  gtk_tree_view_expand_all(contactview);
  
  GtkTreePath *path;
  path=gtk_tree_path_new_first();
  gtk_tree_path_down(path);

  //gtk_tree_path_next(path);  
  gtk_tree_view_set_cursor(contactview,path,0,0);   
   
  gtk_tree_path_free(path);
  //gtk_tree_selection_set_mode (gtk_tree_view_get_selection(GTK_TREE_VIEW(contactview)),
    //                           GTK_SELECTION_MULTIPLE);


  return 1;


}

/**
 * @brief rebuild the contact tree
 *
 * This function should be called once at the initial process.
 *+
 * @param window The main window widget pointer
 * @param string the inputed string
 * @param  int personornum
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
gint
rebuild_contact_view_with_string (GtkWidget *window,gchar* string,int personornum)
{
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore        *treestore;
  GtkTreeModel        *model;

  GtkTreePath * path;
  GtkWidget           *contactview=NULL;
  gint                ret = 0;
   
  contactview = lookup_widget (window, "treeviewContacts");

  if(contactview == NULL)
    return 0;

  model=gtk_tree_view_get_model(contactview);
  
  g_object_ref(model);

  gtk_tree_view_set_model(contactview,NULL);
  
  treestore=GTK_TREE_STORE(model);

  gtk_tree_store_clear(treestore);

   if(contactlist.contacts!= NULL)
     ret= rebuild_contact_store_by_string(treestore, contactlist.contacts,string,personornum);
   else
   {
	  g_printf("contactlist.contacts=0\n");
   }
  

  gtk_tree_view_set_model (GTK_TREE_VIEW(contactview), model);

  g_object_unref (model);

  gtk_tree_view_expand_all(contactview);
  
  path=gtk_tree_path_new_first();
  gtk_tree_path_down(path);

  //gtk_tree_path_next(path);  
  gtk_tree_view_set_cursor(contactview,path,0,0);   
   
  gtk_tree_path_free(path);
  return ret;


}


/**
 * @brief Convert package list from IPK_PACKAGE to GtkTreeStore
 *
 * @param store The GtkTreeStore
 * @param pkg The package list
 * 
 * @return Error code
 */
gint
init_contact_store (GtkTreeStore *store,
                    DIALER_CONTACT* contacts)
{

  GtkTreeIter   isec;

//  gint    res;

  DIALER_CONTACT_ENTRY* entry;
  while(contacts!= NULL)
  {
	insert_new_person(store, &isec, contacts);
    entry=contacts->entry;
	  
	 while(entry)
	 {
     insert_entry_to_person (store, &isec, entry);
     entry=entry->next;
	 }
    
	 contacts= contacts->next;
  }

 return 1;
}


/**
 * @brief build the store with the filter string,if personornum=0,number,1-personname
 *
 * @param store The GtkTreeStore
 * @param contacts the contact list
 * @param string the filter string, it has be included by the item to be added.
 * 
 * @return Error code
 */
gint
rebuild_contact_store_by_string(GtkTreeStore *store,
                    DIALER_CONTACT* contacts,gchar* string,int personornum)
{

  GtkTreeIter   isec;
  DIALER_CONTACT_ENTRY* entry;
  //strcpy(*output,"");
  int inserted=0;
  
  int len;
  
  if(string)
	  len=strlen(string);
  else
	  len=0;
  
  if(personornum==0)
  {
 // g_printf("rebuild_contact_store_by_string\n");
  while(contacts!= NULL)
  {
	//g_printf("%s\n",contacts->name);
	  
    entry=contacts->entry;
	  
	 while(entry)
	 {
	 //g_printf("%s,%s\n",entry->desc,entry->content);
	 //judge if the entry includes the string
	 if(hassensentive(entry->content,string))
	 {	
		//if the person not inserted, then insert first
     if(inserted==0)
	 {
		// g_printf("insert %s\n",contacts->name);
		 insert_new_person(store, &isec, contacts);	 
		 inserted=1;
	 }		 
     insert_entry_to_person (store, &isec, entry);
	 }
     entry=entry->next;
	 }
    
	 contacts= contacts->next;
	 inserted=0;
  }
  }
  else
  {//person name;
	  // g_printf("rebuild_contact_store_by_string\n");
  while(contacts!= NULL)
  {
	//g_printf("%s\n",contacts->name);

	if(!namehasstring(contacts->name,string))
		
	{
		contacts= contacts->next;
		continue;
	}
		// g_printf("insert %s\n",contacts->name);
    //yes, we insert every entry it has.
	insert_new_person(store, &isec, contacts);	 
   
    entry=contacts->entry;
	  
	 while(entry)
	 {
	    insert_entry_to_person (store, &isec, entry);
	     entry=entry->next;
	 }
    
	 contacts= contacts->next;
	
  }
 
  }

 return 1;
}

/**
 * @brief judge if the contact name includes the string 
 *
 * @param name the contact name
 * @param string   the sensatvie string
 * 
 * @return 0-no 1-yes
 */

int namehasstring(gchar* content,gchar * string)
{
	int i,j,lenn,lens,find;
	//g_printf("namehasstring:%s,%s\n",content,string);
	if(content==NULL) 
	return 0;
	
	if(string==NULL) 
	return 1;
	
	lenn=strlen(content);
	lens=strlen(string);
	if(lens==0)
		return 1;
	if(lens>lenn)
		return 0;
	for(i=0;i<=lenn-lens;i=i+1)
	{
		//g_printf("abitger \n");
		find=1;
		for(j=0;j<lens;j=j+1)
		{
			//g_printf("%c,%s[%d]=%c\n",content[i+j],string,j,string[j]);
			if(content[i+j]!=string[j])
			{
				find=0;
				//j=0;
			    break;
			}
			
		}
		if(j==lens&&find==1)
			break;
		
	}
	return find;

}
/**
 * @brief judge if the content includes the sensative string 
 *
 * @param content  the content
 * @param string   the sensatvie string
 * 
 * @return 0-no 1-yes
 */
int hassensentive(gchar* content,gchar *string)
{
	int i;
	//g_printf("hassensentive:%s,%s\n",content,string);
	if(content==NULL) 
	return 0;
	
	if(string==NULL) 
	return 1;
	
	if(strlen(string)==0)
		return 1;
	if(strlen(string)>strlen(content))
		return 0;
	for(i=0;string[i];i++)
	{
		if(content[i]==0)
			return 1;
		if(string[i]==content[i])
			continue;
		return 0;
		
	}
	return 1;
}

void
insert_new_person (GtkTreeStore *store,
                         GtkTreeIter *sec,
                         DIALER_CONTACT *contact)
{
  
  //gtk_tree_store_insert_before (store, &parent, NULL, sec);

 gtk_tree_store_append (store, sec, NULL);
  gtk_tree_store_set (store, sec,
                      COL_NAME, contact->name,
                      -1);

  

  
                      
}

int
insert_entry_to_person (GtkTreeStore *store,
                           GtkTreeIter *parent,
                           DIALER_CONTACT_ENTRY* entry)
{
  GtkTreeIter iter;

  gtk_tree_store_append (store, &iter, parent);

  gtk_tree_store_set (store, &iter,
                      COL_DESC, entry->desc,
                      COL_CONTENT, entry->content,
                      -1);

	return 1;
}

gint get_select_line(GtkWidget * tree_view,gchar ** name,gchar **desc,gchar ** content)
{
  GtkTreeSelection    *selection;
  GtkTreeModel        *model;
  GtkTreeIter         iter;
    GtkTreeIter         parentiter;
  if(tree_view == NULL) {
    g_printf("The widget \"apptree\" not init correctly\n");
    return 0;
  }

selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));

  if (gtk_tree_selection_get_selected(selection, &model, &iter))
  {
	  //is the person selected?
	if(gtk_tree_store_iter_depth(GTK_TREE_STORE(model),&iter)==0)
	{
		//gtk_tree_model_iter_children(model,&parentiter,&iter);
		//gtk_tree_view_set_cursor();
		//??>?
		return 0;
	}
	
	if(gtk_tree_model_iter_parent(model,&parentiter,&iter))
	{
	gtk_tree_model_get (model, &parentiter, COL_NAME,name, -1);
	
    gtk_tree_model_get (model, &iter, COL_DESC,desc,COL_CONTENT,content, -1);
	
	return 1;
	}
    

  }

  else
  {
    g_printf("no row selected.\n");

    return 0;
  }
  return 1;
  }
