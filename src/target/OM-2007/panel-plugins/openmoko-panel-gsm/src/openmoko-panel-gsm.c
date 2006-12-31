#include <libmb/mb.h>
#include <X11/Xlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
//#include <libgsmd/libgsmd.h>
//#include <libgsmd/misc.h>

#define DIR_LONG	256
#define FONT_POPUP_DESC       "Sans bold 28px"
#define ADD_IMG "mbadd.png"  //use for test

enum {
	conn_err=0,
	level_1,
	level_2,
	level_3,
	level_4,
	level_5,
	MAX_ID,
};

static MBPixbuf *pb;
static MBPixbufImage *Img_icon[MAX_ID]; 
static MBPixbufImage *Img_Scaled[MAX_ID];
static int CurImg = 0;
static int LastImg = -1;
MBMenu* popupmenu;
static Display *dpy;
static int screen;
static int times = 0;

//static struct lgsm_handle *lgsmh;
/*
Bool
gsm_connect_init()
{
	lgsmh = lgsm_init(LGSMD_DEVICE_GSMD);
	
	if (!lgsmh) 
	{
		fprintf(stderr, "Can't connect to gsmd\n");
		return False;
	}
	else return True;
}
*/
/*
*
*/
int 
signal_update(void)
{
   int sig_quality = 0;
  // int result = 0;
  // result = lgsm_get_signal_quality (lgsmh, &sig_quality);

    if (CurImg<MAX_ID-1) 
    	CurImg++;
    else
    	CurImg=0;

    return CurImg;
}

/**
*@brief execute an application
*@param cmd 	command string
*@return none
*/
void 
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
om_gsm_connect_dialog_cb(MBMenuItem *item)
{
      fprintf(stdout, "open connect status dialog");
}

void
om_gsm_disconnect_dialog_cb(MBMenuItem *item)
{
      fprintf(stdout, "open disconnect status dialog");
}

/**
*@brief Xevent callback function, let the popupmenu handle Xevent.
*@param app		MBTrayApp reference
*@param e		XEvent
*@return none
*/
void
xevent_callback (MBTrayApp *app, XEvent *e)
{
  	mb_menu_handle_xevent (popupmenu, e);
}

/**
 * @brief get the popupmenu position
 * @param app	MBTrayApp reference
 * @param x 		X co-ord
 * @param y		Y co-ord
 * @return none
 */
static void
menu_get_popup_pos (MBTrayApp *app, int *x, int *y)
{
  int abs_x, abs_y, menu_h, menu_w;
  mb_tray_app_get_absolute_coords (app, &abs_x, &abs_y);
  mb_menu_get_root_menu_size(popupmenu, &menu_w, &menu_h);

  if (mb_tray_app_tray_is_vertical (app))
   {
      /* XXX need to figure out menu size before its mapped 
	     so we can figure out offset for east panel
      */
     *y = abs_y + mb_tray_app_height(app);

      if (abs_x > (DisplayWidth(mb_tray_app_xdisplay(app), mb_tray_app_xscreen(app)) /2))
	*x = abs_x - menu_w - 2;
      else
	*x = abs_x + mb_tray_app_width(app) + 2;
    }
  else
    {
      *x = abs_x;
      if (abs_y > (DisplayHeight(mb_tray_app_xdisplay(app), mb_tray_app_xscreen(app)) /2))
	*y = abs_y - 2;
      else
	*y = abs_y + mb_tray_app_height(app) + menu_h;
    }
}


void
menu_init(MBTrayApp* app)
{
 char* icon_path_ucon;
 char* icon_path_con;
 char* mb_theme;

if (popupmenu == NULL)
    {
     popupmenu = mb_menu_new (mb_tray_app_xdisplay(app), 
								mb_tray_app_xscreen(app));
     mb_menu_set_icon_size(popupmenu, 32);
     mb_menu_set_font (popupmenu, FONT_POPUP_DESC);   
     mb_menu_set_trans(popupmenu, 0);
    }
  else mb_menu_free(popupmenu);

  //if ((mb_theme = mb_tray_app_get_theme_name (app) ) == NULL) exit(1);
  mb_theme = mb_tray_app_get_theme_name (app);

 
  icon_path_con = mb_dot_desktop_icon_get_full_path (mb_theme, 
						 16, ADD_IMG  );

   icon_path_ucon = mb_dot_desktop_icon_get_full_path (mb_theme, 
						 16, ADD_IMG  );
  
  mb_menu_add_item_to_menu(popupmenu, popupmenu->rootmenu, "Connect", 
			  icon_path_con, NULL , 
			  om_gsm_connect_dialog_cb, (void *)app, MBMENU_NO_SORT);

  mb_menu_add_item_to_menu(popupmenu, popupmenu->rootmenu, "Disconnect", 
			  icon_path_ucon, NULL , 
			  om_gsm_disconnect_dialog_cb, (void *)app, MBMENU_NO_SORT);

  if (icon_path_con) free (icon_path_con);
  if (icon_path_ucon) free (icon_path_ucon);
  if (mb_theme) free (mb_theme);
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{
  MBPixbufImage *img_backing = NULL;

  CurImg = signal_update();

  if (LastImg == CurImg) 
  	return;
  
  img_backing = mb_tray_app_get_background (app, pb);

  mb_pixbuf_img_copy_composite(pb, img_backing, 
			       Img_Scaled[CurImg], 0, 0,
			       mb_pixbuf_img_get_width(Img_Scaled[0]),
			       mb_pixbuf_img_get_height(Img_Scaled[0]),
			       mb_tray_app_tray_is_vertical(app) ? 
			       (mb_pixbuf_img_get_width(img_backing)-mb_pixbuf_img_get_width(Img_Scaled[0]))/2 : 0,
			       mb_tray_app_tray_is_vertical(app) ? 0 : 
			       (mb_pixbuf_img_get_height(img_backing)-mb_pixbuf_img_get_height(Img_Scaled[0]))/2 );

  mb_pixbuf_img_render_to_drawable(pb, img_backing, drw, 0, 0);

  mb_pixbuf_img_free( pb, img_backing );

  LastImg = CurImg;
}

void
resize_callback (MBTrayApp *app, int w, int h )
{
  int  i;
  int  base_width  = mb_pixbuf_img_get_width(Img_icon[0]);
  int  base_height = mb_pixbuf_img_get_height(Img_icon[0]);
  int  scale_width = base_width, scale_height = base_height;
  Bool want_resize = True;

  if (mb_tray_app_tray_is_vertical(app) && w < base_width)
    {

      scale_width = w;
      scale_height = ( base_height * w ) / base_width;

      want_resize = False;
    }
  else if (!mb_tray_app_tray_is_vertical(app) && h < base_height)
    {
      scale_height = h;
      scale_width = ( base_width * h ) / base_height;
      want_resize = False;
    }

  if (w < base_width && h < base_height
      && ( scale_height > h || scale_width > w))
    {

       /* Something is really wrong to get here  */
      scale_height = h; scale_width = w;
      want_resize = False;
    }

  if (want_resize)  /* we only request a resize is absolutely needed */
    {
      LastImg = -1;
      mb_tray_app_request_size (app, scale_width, scale_height);
    }

  for (i=0; i<MAX_ID; i++)
    {
      if (Img_Scaled[i] != NULL) 
	mb_pixbuf_img_free(pb, Img_Scaled[i]);

      Img_Scaled[i] = mb_pixbuf_img_scale(pb, 
					  Img_icon[i], 
					  scale_width, 
					  scale_height);
    }
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
   Time click_time = 800;
   int click_x,click_y;
   int counter = 10;

   fprintf(stdout, "openmoko-panel-gsm: %d times call function buttoncallback", ++times);

   if (!popupmenu) return; /* menu disabled */

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
       mb_tray_app_tray_send_message(app, "Run openmoko-gsm application (openmoko-preference)", 6000);
       //fork_exec("openmoko-preference");  //launch openmoko-preference.
      return;
    }
//function for "tap with hold" action.
 else if (done == 2 && !mb_menu_is_active(popupmenu))
    {
       mb_tray_app_tray_send_message(app, "Connect or reconnect gsm signal", 6000);

       // menu_get_popup_pos(app, &click_x, &click_y);
       //fprintf(stdout, "openmoko-panel-gsm: x = %d, y = %d\n", click_x, click_y);
       //mb_menu_activate(popupmenu, click_x, click_y);
    }
}

void
timeout_callback ( MBTrayApp *app )
{
  mb_tray_app_repaint (app);
}


int
main( int argc, char *argv[])
{
   MBTrayApp *app = NULL;
   struct timeval tv;
   int i =0;

   app = mb_tray_app_new ( "gsm Monitor",
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  

   dpy    = mb_tray_app_xdisplay(app);
   screen = mb_tray_app_xscreen(app);
  
   pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
		      mb_tray_app_xscreen(app));
  
   memset (&tv,0,sizeof(struct timeval));
   tv.tv_sec = 2;

    menu_init (app);

  Img_icon[conn_err]= mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/SignalStrength25g_00.png");
  Img_icon[level_1]= mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/SignalStrength25g_01.png");
  Img_icon[level_2]= mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/SignalStrength25g_02.png");
  Img_icon[level_3]= mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/SignalStrength25g_03.png");
  Img_icon[level_4]= mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/SignalStrength25g_04.png");
  Img_icon[level_5]= mb_pixbuf_img_new_from_file(pb, PKGDATADIR"/SignalStrength25g_05.png");
  
   mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 
   
   mb_tray_app_set_button_callback (app, button_callback );

   mb_tray_app_set_xevent_callback(app, xevent_callback);
   
   mb_tray_app_set_icon(app, pb, Img_icon[level_3]);

   mb_tray_app_request_offset (app, 1);

//   gsm_connect_init();

   mb_tray_app_main (app);
   
   XCloseDisplay(dpy);

}
