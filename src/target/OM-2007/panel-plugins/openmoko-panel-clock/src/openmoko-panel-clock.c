 /**
 * @file openmoko-panel-clock.c
 * @brief Openmoko panel plug-in clock.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <X11/Xlib.h>
#include <string.h>
#define __USE_BSD
#include <sys/time.h>


//#define  USE_PANGO  //FIXME: check out whether can 
					//use Pango Font of MBFont to suit Moko theme.
#include <libmb/mb.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(text) gettext(text)
#else
# define _(text) (text)
#endif

#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif 

#define FONT_POPUP_DESC       "Sans bold 28px"
#define ADD_IMG "mbadd.png" 	//FIXME:image show in popup menu, changer it to suit with Moko theme

enum {
TIME_TYPE_12_HOUR,
TIME_TYPE_24_HOUR
};

int time_type = TIME_TYPE_12_HOUR;
MBFont     *Fnt = NULL;
MBDrawable *Drw  = NULL;
MBColor    *Col  = NULL;
MBPixbuf   *Pixbuf;
MBPixbufImage   *background;
MBTrayApp *app = NULL;
MBMenu* popupmenu;
static Display *dpy;
int bg_h;
int bg_w;

void usage()
{
  exit(1);
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

/**
 * @brief callback function of popupmenu item, change time format.
 * @param item	MBMenuItem  
 * @return none
 */
void panel_menu_time_change_cb(MBMenuItem *item)
{
  if (time_type == TIME_TYPE_24_HOUR) time_type = TIME_TYPE_12_HOUR;
  else time_type = TIME_TYPE_24_HOUR;
  mb_tray_app_repaint (app);
 }

/**
 * @brief initialize popupmenu
 * @param app	MBTrayApp reference
 * @return none
 */
void
menu_init(MBTrayApp* app)
{
 char* icon_path;
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

  mb_theme = mb_tray_app_get_theme_name (app);

 
  icon_path = mb_dot_desktop_icon_get_full_path (mb_theme, 
						 16, ADD_IMG  );
  
  mb_menu_add_item_to_menu(popupmenu, popupmenu->rootmenu, _("Time Format Change"), 
			  icon_path, NULL , 
			  panel_menu_time_change_cb, (void *)app, MBMENU_NO_SORT);

  if (icon_path) free(icon_path);
  if (mb_theme) free(mb_theme);
  
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


#ifdef USE_LIBSN

static SnDisplay *sn_dpy;

static void 
sn_activate(char *name, char *exec_str)
{
  SnLauncherContext *context;
  pid_t child_pid = 0;

  context = sn_launcher_context_new (sn_dpy, 0);
  
  sn_launcher_context_set_name (context, name);
  sn_launcher_context_set_binary_name (context, exec_str);
  
  sn_launcher_context_initiate (context, "monoluanch launch", exec_str,
				CurrentTime);

  switch ((child_pid = fork ()))
    {
    case -1:
      fprintf (stderr, "Fork failed\n" );
      break;
    case 0:
      sn_launcher_context_setup_child_process (context);
      mb_exec(exec_str);
      fprintf (stderr, "Failed to exec %s \n", exec_str);
      _exit (1);
      break;
    }
}

#endif

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
      fprintf(stderr, "openmoko-panel-clock: Failed to Launch '%s'\n", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "openmoko-panel-clock: Failed to Launch '%s'", cmd);
      break;
    }
}

/**
*@brief paint callback, to draw the layout of the app.
*/
void
paint_callback ( MBTrayApp *app, Drawable pxm )
{
  struct timeval   tv;
  struct timezone  tz;
  struct tm       *localTime = NULL; 
  time_t           actualTime;
  char             timestr[6] = { 0 };
  MBPixbufImage   *img_bg = NULL;
  MBDrawable      *drw;

  /* Figure out  the actual time */
  gettimeofday(&tv, &tz);
  actualTime = tv.tv_sec;
  localTime = localtime(&actualTime);

  if ((time_type == TIME_TYPE_12_HOUR) 
  		&& (localTime->tm_hour > 12))
    snprintf(timestr, sizeof(timestr), _("%.2d:%.2d"),
           (localTime->tm_hour-12), localTime->tm_min);
  else 
    snprintf(timestr, sizeof(timestr), _("%.2d:%.2d"),
          localTime->tm_hour, localTime->tm_min);

  img_bg = mb_tray_app_get_background (app, Pixbuf);
  drw = mb_drawable_new_from_pixmap (Pixbuf, pxm);

 if (bg_w != mb_pixbuf_img_get_width (img_bg)
 	||  bg_h != mb_pixbuf_img_get_height (img_bg))
   {
       MBPixbufImage   *bg = NULL;
       bg = mb_pixbuf_img_new_from_file (Pixbuf, PKGDATADIR"/clock-bg.png");
       bg_w = mb_pixbuf_img_get_width (img_bg);
       bg_h = mb_pixbuf_img_get_height (img_bg);
     	background = mb_pixbuf_img_scale(Pixbuf, bg, 
   				  bg_w ,
			         bg_h-3);
   }
 
   if (background)
     {
        mb_pixbuf_img_copy_composite(Pixbuf, img_bg, 
			       background, 0, 0,
			        bg_w,
			       bg_h,
			       0, 0 );
     	}
      mb_pixbuf_img_render_to_drawable (Pixbuf, img_bg, pxm, 0, 0);
      //mb_pixbuf_img_free( Pixbuf, img_backing );

  if (mb_tray_app_tray_is_vertical(app))
    {
      /* 
	 - create a new HxW pixmap
	 - rotate background img +90 onto it
	 - render the text to it
	 - call drawable to pixbuf
	 - rotate the new pixbuf -90 
      */
      MBDrawable *drw_rot;
      MBPixbufImage *img_bg_rot, *img_txt, *img_txt_rot;

      int font_y = ((mb_tray_app_width(app) - (mb_font_get_height(Fnt)))/2);

      img_bg_rot = mb_pixbuf_img_transform (Pixbuf, img_bg,
					    MBPIXBUF_TRANS_ROTATE_90);

      drw_rot = mb_drawable_new(Pixbuf, 
				mb_pixbuf_img_get_width(img_bg_rot), 
				mb_pixbuf_img_get_height(img_bg_rot));

      mb_pixbuf_img_render_to_drawable (Pixbuf, img_bg_rot, 
					mb_drawable_pixmap(drw_rot), 0, 0);

      mb_font_render_simple (Fnt, 
			     drw_rot, 
			     1, font_y,
			     mb_pixbuf_img_get_width(img_bg_rot),
			     (unsigned char *) timestr,
			     MB_ENCODING_UTF8,
			     0);

      img_txt = mb_pixbuf_img_new_from_drawable (Pixbuf,
						 mb_drawable_pixmap(drw_rot),
						 None,
						 0, 0,
						 mb_pixbuf_img_get_width(img_bg_rot), 
						 mb_pixbuf_img_get_height(img_bg_rot)); 

      img_txt_rot = mb_pixbuf_img_transform (Pixbuf, img_txt,
					     MBPIXBUF_TRANS_ROTATE_90);
      
      mb_pixbuf_img_render_to_drawable (Pixbuf, img_txt_rot, 
					mb_drawable_pixmap(drw), 0, 0);

      mb_pixbuf_img_free(Pixbuf, img_bg_rot); 
      mb_pixbuf_img_free(Pixbuf, img_txt);
      mb_pixbuf_img_free(Pixbuf, img_txt_rot);

      mb_drawable_unref(drw_rot);

    } 
  else 
    {
      int font_y = ((mb_tray_app_height(app) - (mb_font_get_height(Fnt)))/2);
      
      mb_pixbuf_img_render_to_drawable (Pixbuf, img_bg, 
					mb_drawable_pixmap(drw), 0, 0);

      mb_font_render_simple (Fnt, 
			     drw, 
			     11, font_y/2+2,
			     mb_tray_app_width(app),
			     (unsigned char *) timestr,
			     MB_ENCODING_UTF8,
			     MB_FONT_RENDER_VALIGN_MIDDLE);
    }

  mb_drawable_unref(drw);
  mb_pixbuf_img_free(Pixbuf, img_bg);

}

/**
*@brief button callback function.
*/
void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  XEvent         ev;
  int            done = 0;
  struct timeval then, now;
  Time click_time=800;
  int click_x,click_y;
  int counter = 10;

  //if (!popupmenu) return; /* menu disabled */
  
  gettimeofday(&then, NULL);
    
 //check the click type: tap "done = 1 "; tap with hold "done = 2";
  while (!done && !is_released)
    {
      counter ++;
      if (XCheckMaskEvent(dpy,ButtonReleaseMask, &ev))
      if (ev.type == ButtonRelease)
      	{
      	   done = 1;
       }
     gettimeofday(&now, NULL);
     if ((now.tv_usec-then.tv_usec) > (click_time*10000))
      {
         done = 2;
      }
  }
  
  //function for "tap" action, execute "openmoko-clocks application".
  if (done == 1)  
      {  
      mb_tray_app_tray_send_message(app, "launch openmoko-clock application", 6000);

      //fork_exec("/usr/games/same-gnome");//FIXME: enjoy it :), launch "openmoko-clocks application" instead.
      return;
  	}
  //function for "tap with hold" action, pop a popupmenu to change time format.
 else if (done == 2 ) //&& !mb_menu_is_active(popupmenu))
    {
      //menu_get_popup_pos(app, &click_x, &click_y);
      //fprintf(stdout, "openmoko-panel-clock: x = %d, y = %d\n", click_x, click_y);
      //mb_menu_activate(popupmenu, click_x, click_y);
      mb_tray_app_tray_send_message(app, "Change time display format", 6000);
      panel_menu_time_change_cb (NULL); 
    }
}


/**
*@brief resize callback function.
*/
void
resize_callback ( MBTrayApp *app, int w, int h )
{
  int   req_size   = 0;

  mb_font_set_size_to_pixels (Fnt, 
			      (mb_tray_app_tray_is_vertical(app) ? w : h), 
			      NULL);

  req_size = mb_font_get_txt_width (Fnt, 
				    (unsigned char *) "99999", 5,
				    MB_ENCODING_UTF8);

  if (mb_tray_app_tray_is_vertical(app))
    {
      mb_tray_app_request_size (app, w, req_size+ 12);
    } else {
      mb_tray_app_request_size (app, req_size + 12, h);
    }
}

/**
*@brief timeout callback function.
*/
void
timeout_callback (MBTrayApp *app)
{
  struct timeval tv;

  mb_tray_app_repaint (app);

  /* Make sure we get called again in 60 secs - we should get called 
   * exactly on the minute initially.
  */
  tv.tv_usec = 0;
  tv.tv_sec  = 60;

  mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 
}

int 
main(int argc, char **argv)
{
  struct timeval tv;
  char *icon_path;
  struct timezone  tz;
  struct tm *localTime = NULL; 
  time_t actualTime;

  app = mb_tray_app_new ( _("OpenMoko Panel Clock"),
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  

  if (app == NULL) usage();

  dpy = mb_tray_app_xdisplay(app);
  Pixbuf = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
			 mb_tray_app_xscreen(app));
  

  Col  = mb_col_new_from_spec (Pixbuf, "#FFEEDD");
  Fnt = mb_font_new_from_string (mb_tray_app_xdisplay(app), "Sans bold"); 
  mb_font_set_weight (Fnt, 1);
  mb_font_set_color (Fnt, Col);

  memset(&tv, 0, sizeof(struct timeval));

  menu_init (app);

  /* Figure out number of seconds till next minute */
  gettimeofday(&tv, &tz);
  actualTime = tv.tv_sec;
  localTime = localtime(&actualTime);
  tv.tv_usec = 0;
  tv.tv_sec  = 60 - localTime->tm_sec;

  /* This we then get reset when first called to 60 */
  mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 

  mb_tray_app_set_button_callback (app, button_callback );

  mb_tray_app_set_xevent_callback(app, xevent_callback);

  mb_tray_app_main (app);

  XCloseDisplay(dpy);

  return 1;
}
