#include "app-history.h"

void
moko_sample_hisory_app_fill(MokoPixmapButton *btn)
{
   GtkWidget *image;

   image = gtk_image_new_from_file ("/usr/share/pixmaps/gnome-eyes.png");

   moko_pixmap_button_set_finger_toolbox_btn_center_image(btn, image);
   
}

void
moko_add_history_app_image (MokoPixmapButton* btn, GdkPixbuf *pixbuf)
{
  moko_pixmap_button_set_finger_toolbox_btn_center_image(btn, gtk_image_new_from_pixbuf (pixbuf)); 
}
