#include <libmb/mb.h>
#include <X11/Xlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <gtk/gtk.h>

static MBPixbuf *pb;
static MBPixbufImage *image; 
static Display *dpy;

/**
*@brief execute an application
*@param cmd 	command string
*@return none
*/
static void 
fork_exec(char *cmd)
{
  switch (fork())
    {
    case 0:
      setpgid(0, 0); /* Stop us killing child */
      mb_exec(cmd);
      fprintf(stderr, "openmoko-panel-gsm: Failed to Launch '%s'\n", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "openmoko-panel-gsm: Failed to Launch '%s'", cmd);
      break;
    }
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{
  MBPixbufImage *img_backing = NULL;
  img_backing = mb_tray_app_get_background (app, pb);

  mb_pixbuf_img_copy_composite (pb, img_backing, 
			       image, 
			       0, 0,
			       mb_tray_app_width(app),
			       mb_tray_app_height(app),
			       0, 0 );

  mb_pixbuf_img_render_to_drawable(pb, img_backing, drw, 0, 0);

  mb_pixbuf_img_free( pb, img_backing );
}

void
resize_callback (MBTrayApp *app, int w, int h )
{
  int  width  = mb_pixbuf_img_get_width(image);
  int  height = mb_pixbuf_img_get_height(image);

  mb_tray_app_request_size (app, width, height);
}

/**
*@brief button callback function.
*/
void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
   XEvent ev;
   int done = 0;
   struct timeval then, now;
   Time click_time=800;

   gettimeofday(&then, NULL);
    
 //check the click type: tap "done = 1 "; tap with hold "done = 2";
   while (!done && !is_released)
    {
       if (XCheckMaskEvent(dpy,ButtonReleaseMask, &ev))
         if (ev.type == ButtonRelease)
      	   {
      	     done = 1;
          }
       gettimeofday(&now, NULL);
       
       if ((now.tv_usec-then.tv_usec) > (click_time*50000))
        {
          done = 2;
        }
    }

//function for "tap" action.
  if (done == 1)  
    {   
       mb_tray_app_tray_send_message(app, "Run openmoko-mainmenu stylus based application", 6000);
       //fork_exec("opemoko-mainmenu");  //launch openmoko-preference.
      return;
    }
//function for "tap with hold" action.
 else if (done == 2)
    {
       mb_tray_app_tray_send_message(app, "Run openmoko-mainmenu figure based application", 6000);
       fork_exec("openmoko-mainmenu"); 
       //fork_exec("openmoko-mainmenu --figure");  //launch openmoko-mainmenu.
    }
}

int
main( int argc, char *argv[])
{
   MBTrayApp *app = NULL;
   struct timeval tv;

   app = mb_tray_app_new ( "OpenMoko Mainmenu",
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  

   dpy = mb_tray_app_xdisplay(app);
  
   pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
		      mb_tray_app_xscreen(app));
  
   memset (&tv,0,sizeof(struct timeval));
   tv.tv_sec = 10;  
  image = mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/btn_menu.png");
  
  if (!image)
      fprintf (stderr, "openmoko-panel-mainmenu: Failed to load mainmenu plugin icon");
  
   mb_tray_app_set_button_callback (app, button_callback );

   mb_tray_app_request_offset(app, -1);

   mb_tray_app_main (app);
   
   XCloseDisplay(dpy);
   
   return 1;
}
