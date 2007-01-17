#include "app-history.h"

static int current = 0;

static void 
pointer_check()
{
    if (current < MAX_RECORD_APP )
    	return;
    else
    	current = 0;
}

void
moko_hisory_app_fill(MokoPixmapButton **btn, const char *path)
{
   GtkWidget *image;
   image = gtk_image_new_from_file (path);

   if (!path)
   	return;
   pointer_check();
   moko_pixmap_button_set_finger_toolbox_btn_center_image(btn[current], image);
   current++;
}
