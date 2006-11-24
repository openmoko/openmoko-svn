#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <string.h>

/*! \file folder.h
    \brief A Documented file.
    
    Details.
*/

/*! \struct FOLDER
    \brief Struct definition for FOLDER .
    
    Details.
*/
typedef struct {
  int type;	    /**< folder type */	
  char fname[24];   /**< folde name */
} FOLDER;

/*! \fn int delete_folder(char *dfname)
    \brief delete folder handler
    \param dfname the folder name to delete
*/

/*! \fn void add_folder(char *fname)
    \brief add folder handler
    \param fname add fname folder
*/

/*! \fn void display()
    \brief display the messages in terminal, used for test
*/

/*! \fn void rename_folder(char *fname, char *new_name)
    \brief rename fname folder to new_name
    \param fname fname folder to rename
    \param new_name new folder name for fname folder
*/

/*! \fn int custom_folder_in ()
    \brief is there a custom folder
    \return custom_folder_in () 1 yes; 0 no
*/

/*! \fn int is_custom_folder (char *fname)
    \brief if fname is a custom folder handler
    \param fname fname folder to judge
    \return is_custom_folder (char *fname) 1 yes; 0 no
*/

int delete_folder(char *dfname);
void add_folder(char *fname);
void display();
void rename_folder(char *fname, char *new_name);
int custom_folder_in ();
int is_custom_folder (char *fname);
