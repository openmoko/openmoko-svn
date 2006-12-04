/***************************************************************************
 *            opendialer.h
 *
 *  Thu Aug 24 22:57:07 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/

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
 
#ifndef _OPENDIALER_H
#define _OPENDIALER_H

#ifdef __cplusplus
extern "C"
{
#endif
gint
init_contact_view (GtkWidget *window);
gint
rebuild_contact_view_with_string (GtkWidget *window,gchar* string,int personornum)
;
gint
init_contact_store (GtkTreeStore *store,
                    DIALER_CONTACT* contacts);
					gint
rebuild_contact_store_by_string(GtkTreeStore *store,
                    DIALER_CONTACT* contacts,gchar* string,int personornum);
int namehasstring(gchar* content,gchar * string);
	int hassensentive(gchar* content,gchar *string);
	void
insert_new_person (GtkTreeStore *store,
                         GtkTreeIter *sec,
                         DIALER_CONTACT *contact);
	int
insert_entry_to_person (GtkTreeStore *store,
                           GtkTreeIter *parent,
                           DIALER_CONTACT_ENTRY* entry);
gint get_select_line(GtkWidget * tree_view,gchar ** name,gchar **desc,gchar ** content);









#ifdef __cplusplus
}
#endif

#endif /* _OPENDIALER_H */
