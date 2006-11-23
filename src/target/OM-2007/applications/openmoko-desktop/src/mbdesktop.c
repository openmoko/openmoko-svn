/*
  mbdesktop - a desktop for handhelds etc. 

   Copyright 2002 Matthew Allum

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


#include "mbdesktop.h"
#include "mbdesktop_view.h"
#include "mbdesktop_module.h"
#include "mbdesktop_win_plugin.h"

#include <dlfcn.h>

#ifdef MB_HAVE_PNG
#define FOLDER_IMG "mbfolder.png"
#define HIDE_IMG    "mbdown.png"
#define ADD_IMG     "mbadd.png"
#define REMOVE_IMG  "mbremove.png"
#else
#define FOLDER_IMG "mbfolder.xpm"
#define ADD_IMG     "mbadd.xpm"
#define REMOVE_IMG  "mbremove.xpm"
#define HIDE_IMG    "mbdown.xpm"
#endif

#define DIALOG_BORDER 5 	/* 5 Pixel border for dialog */

static void
modules_init (MBDesktop *mb);

static void 
mbdesktop_switch_bg_theme (MBDesktop *mb);

static Bool
mbdesktop_get_theme_via_root_prop(MBDesktop *mb);

static Bool
mbdesktop_set_highlight_col(MBDesktop *mb, char *spec);

void
mbdesktop_menu_execute_cb(MBMenuItem *item);

void
mbdesktop_menu_add_to_launch_cb(MBMenuItem *item);

void
mbdesktop_menu_remove_from_launch_cb(MBMenuItem *item);


#ifdef USE_XSETTINGS
static void 
mbdesktop_xsettings_notify_cb (const char       *name,
			       XSettingsAction   action,
			       XSettingsSetting *setting,
			       void             *data);
#endif

static volatile Bool WantReload = False;

static void fork_exec(char *cmd)
{
    pid_t pid;
    
    switch (pid = fork()) 
      {
      case 0:
	mb_exec(cmd);
	/* execlp("/bin/sh", "sh", "-c", cmd, NULL); */
	fprintf(stderr, "mb-applet-menu-launcher: exec of '%s' failed, cleaning up child\n", cmd);
	exit(1);
      case -1:
	fprintf(stderr, "mb-applet-menu-launcher: can't fork '%s'\n", cmd); break;
      }
}

void mbdesktop_menu_execute_cb(MBMenuItem *item)
{
	fprintf(stdout, "steven-desktop: mbdesktop_menu_execute_cb\n");
	MBDesktop	 *mb = (MBDesktop *)item->cb_data;
	/*fprintf(stdout, "steven-desktop: mb->current_popup_item->name=[%s]\n", mb->current_popup_item->name);*/
	if (mb->current_popup_item->activate_cb)	mb->current_popup_item->activate_cb((void *)mb, 
	 			(void *)mb->current_popup_item); 	
	 			
	mb->popup_mode = False;
	/*
  MBPanel *panel = (MBPanel *)item->cb_data;
  panel_toggle_visibilty(panel);
  */
}

void mbdesktop_menu_add_to_launch_cb(MBMenuItem *item)
{
	fprintf(stdout, "steven-desktop: mbdesktop_menu_add_to_launch_cb\n");
	MBDesktop	 *mb = (MBDesktop *)item->cb_data;
	mb->popup_mode = False;
}

void mbdesktop_menu_remove_from_launch_cb(MBMenuItem *item)
{
	fprintf(stdout, "steven-desktop: mbdesktop_menu_remove_from_launch_cb\n");
	MBDesktop	 *mb = (MBDesktop *)item->cb_data;
	mb->popup_mode = False;
}

void  
modules_unload (MBDesktop *mb)
{
  /* XXX we probably need to free some stuff here ! */
  mb->modules = NULL;
}

#ifdef USE_DNOTIFY

MBDesktop *_Gmb;

static
void really_reload(int signum)
{
  DBG("Really Reload menu callback\n");

  WantReload = True;

}


static void 
sig_reload_handler(int sig, siginfo_t *si, void *data)
{
  signal (SIGALRM, really_reload);
  alarm(3);
}

#endif

static void 
sig_hup_reload_handler(int sig, siginfo_t *si, void *data)
{
  DBG("SIGHUP: Really Reload menu callback\n");

  WantReload = True;

}

#ifdef USE_XSETTINGS

#define XSET_UNKNOWN    0
#define XSET_THEME      1
#define XSET_BG         2
#define XSET_FONT       3
#define XSET_FONT_COL   6
#define XSET_TITLE_FONT 4
#define XSET_ICON       5

#define XSET_ACTION_CHANGED 1
#define XSET_ACTION_DELETED 2


/* XXX
   Im not sure if below is needed.
   we always seem to get a manager. 

 */
static void
mbdesktop_xsettings_watch_cb (Window   window,
			      Bool     is_start,
			      long     mask,
			      void    *cb_data)
{
  MBDesktop *mb = (MBDesktop *)cb_data;
  if (is_start)
    {
      mb->xsettings_have_manager = True;
      return;
    }
  mb->xsettings_have_manager = False;
}

static Bool
mbdesktop_xsettings_process (MBDesktop        *mb, 
			     const char       *name, 
			     XSettingsSetting *setting,
			     int               action)
{
  int i = 0;
  int key = XSET_UNKNOWN;
  
  struct _mb_xsettings { char *name; int value; } mb_xsettings[] = {
    { "Net/ThemeName",               XSET_THEME      },
    { "MATCHBOX/Background",         XSET_BG         },
    { "MATCHBOX/Desktop/Font",       XSET_FONT       },
    { "MATCHBOX/Desktop/TitleFont",  XSET_TITLE_FONT },
    { "MATCHBOX/Desktop/FontColor",  XSET_FONT_COL   },
    { "MATCHBOX/Desktop/IconSize",   XSET_ICON       },
    { NULL,       -1 } 
  };
  
  DBG("%s() called. key is '%s'\n", __func__, name);

  while(  mb_xsettings[i].name != NULL )
    {
      if (!strcmp(name, mb_xsettings[i].name)
	  && setting != NULL 	/* XXX set to NULL when action deleted */
	  && ( setting->type == XSETTINGS_TYPE_STRING
	       || setting->type == XSETTINGS_TYPE_INT ) )
	{
	  key = mb_xsettings[i].value;
	  break;
	}
      i++;
    }
    
  if (key == XSET_UNKNOWN) {
    /* xsettings_setting_free(setting); cause seg ? */
    return False;
  }

  if (action == XSETTINGS_ACTION_DELETED)
    {
      if (key == XSET_BG)
	{
	  DBG("%s() called. key XSET_BG deleted\n", __func__); 
	  mb->xsettings_have_bg = False; 
	  if (mb->bg_img != NULL)
	    {
	      mbdesktop_switch_bg_theme(mb);
	      mbdesktop_view_paint(mb, False, False);
	    }
	} 

      return True; 	      /* For now ignore other deleted keys */
    }

  switch (key)
    {
    case XSET_THEME:
      DBG("%s() called. XSET_THEME string is '%s'\n", __func__, 
	  setting->data.v_string);
      mbdesktop_switch_theme (mb, setting->data.v_string);
      break;
    case XSET_BG:
      if (setting->data.v_string != NULL && strlen(setting->data.v_string) > 0)
	{
	  mb->xsettings_have_bg = True;
	  mb->bg_def = strdup(setting->data.v_string);
	  DBG("%s() called. XSET_BG string is '%s'\n", __func__, mb->bg_def);
	  mbdesktop_bg_parse_spec(mb, mb->bg_def);
	  mbdesktop_view_init_bg(mb);

	  if (mb->top_head_item != NULL)
	    mbdesktop_view_paint(mb, False, False);
	}
      else
	{
	  DBG("%s() : XSET_BG data is null reverting to defualt theme\n", 
	      __func__);
	  mb->xsettings_have_bg = False; 
	  if (mb->top_head_item != NULL)
	    {
	      mbdesktop_switch_bg_theme(mb);
	      mbdesktop_view_paint(mb, False, False);
	    }
	}
      break;
    case XSET_FONT:
      if (strlen(setting->data.v_string) > 0)
	{
	  mbdesktop_set_font(mb, setting->data.v_string);
	  DBG("%s() called. XSET_FONT string is '%s'\n", __func__, 
	      setting->data.v_string);
	  if (mb->top_head_item != NULL)
	    mbdesktop_view_paint(mb, False, False);
	  break;
	}
    case XSET_TITLE_FONT:
      if (strlen(setting->data.v_string) > 0)
	{

	  DBG("%s() called. XSET_TITLE_FONT string is '%s'\n", __func__, 
	      setting->data.v_string);

	  mbdesktop_set_title_font(mb, setting->data.v_string);
	  if (mb->top_head_item != NULL)
	    mbdesktop_view_paint(mb, False, False);
	}
      break;
    case XSET_ICON:
      DBG("%s() called. XSET_ICON val is '%i'\n", __func__, 
	  setting->data.v_int);

      if (setting->data.v_int > 4)
	{
	  mb->icon_size = setting->data.v_int;
      
	  if (mb->icon_padding > 32)	  
	    mb->icon_padding = 32; 
	  else
	    mb->icon_padding = 28;

      // mbdesktop_calculate_item_dimentions(mb); 

	  if (mb->top_head_item != NULL)
	    mbdesktop_view_paint(mb, False, False);
	}
      break;
      /* CRACK ?
    case XSET_FONT_COL:
      DBG("%s() called. XSET_FONT_COL string is '%s'\n", __func__, 
	  setting->data.v_string);

      if (setting->data.v_string == NULL 
	  || strlen(setting->data.v_string) < 2)
	{
	  mb->user_overide_font_col = False;
	}
      else
	{
	  mb->user_overide_font_col = True;

	  mbdesktop_set_font_color(mb, setting->data.v_string);
	}

      if (mb->top_head_item != NULL)
	  mbdesktop_view_paint(mb, False, False);
      break;
      */
    }
  
  /* xsettings_setting_free(setting); */
  return True;
}



static void 
mbdesktop_xsettings_notify_cb (const char       *name,
			       XSettingsAction   action,
			       XSettingsSetting *setting,
			       void             *data)
{
  MBDesktop *mb = (MBDesktop *)data;

  DBG("%s() called. with action: %i ( new is %i )\n", __func__, action, XSETTINGS_ACTION_NEW);

  switch (action)
    {
    case XSETTINGS_ACTION_NEW:
    case XSETTINGS_ACTION_CHANGED:
      mbdesktop_xsettings_process (mb, name, setting, XSET_ACTION_CHANGED); 
      break;
    case XSETTINGS_ACTION_DELETED:
      DBG("%s() called. with XSETTINGS_ACTION_DELETED\n", __func__);
      mbdesktop_xsettings_process (mb, name, setting, XSET_ACTION_DELETED); 
      break;
    }

}

#endif


static Bool
mbdesktop_get_theme_via_root_prop(MBDesktop *mb)
{

  Atom realType;
  unsigned long n;
  unsigned long extra;
  int format;
  int status;
  char * value;

#ifdef USE_XSETTINGS 
  /*
  if (mb->xsettings_have_manager)  let xsettings set theme 
    {
      printf("%s() settings client is not null\n", __func__ );
      return False;
    }
    */
#endif 
  
  status = XGetWindowProperty(mb->dpy, mb->root,
			      mb->atom_mb_theme, 
			      0L, 512L, False,
			      AnyPropertyType, &realType,
			      &format, &n, &extra,
			      (unsigned char **) &value);
  
  if (status != Success || value == 0 || *value == 0 || n == 0)
    {
      if (value) XFree(value);
      return False;
    }
  
  if (mb->theme_name == NULL || strcmp(mb->theme_name, value))
    {
      if (mb->theme_name) free(mb->theme_name);
      mb->theme_name = strdup(value);
      
      XFree(value);
      return True;
    }
  if (value) XFree(value);

  return False;
}

static Bool 
file_exists(char *filename)
{
  struct stat st;
  if (stat(filename, &st)) return False;
  return True;
}

static void 
mbdesktop_switch_bg_theme (MBDesktop *mb)
{
  char theme_filename[255] = { 0 };
  char *spec = NULL;

  mbdesktop_set_highlight_col(mb, "#333333");

  if (mb->theme_name == NULL) 
    {
      DBG("%s() called. theme name is null, aborting\n", __func__);
      return;
    }

  if (mb->theme_name[0] == '/')
      strncpy(theme_filename, mb->theme_name, 255);
  else
    {
      snprintf(theme_filename, 255, "%s/.themes/%s/matchbox/theme.desktop",
	       mb_util_get_homedir(), mb->theme_name);
      if (!file_exists(theme_filename))
	{
	  snprintf(theme_filename, 255, "%s/themes/%s/matchbox/theme.desktop",
		   DATADIR, mb->theme_name);
	}
    }

  DBG("%s() called. checking '%s' for details\n", __func__, theme_filename);

  if (file_exists(theme_filename))
    {
      MBDotDesktop *theme  = NULL;
      theme = mb_dotdesktop_new_from_file(theme_filename);
      if (theme)
	{
	  if (mb_dotdesktop_get(theme, "DesktopBgSpec"))
	    {
	      spec = strdup(mb_dotdesktop_get(theme, "DesktopBgSpec"));
	    }
	  if (mb_dotdesktop_get(theme, "DesktopHlCol"))
	    {
	      mbdesktop_set_highlight_col(mb, mb_dotdesktop_get(theme, "DesktopHlCol"));
	    }
	  else mbdesktop_set_highlight_col(mb, "#333333");

	  mb_dotdesktop_free(theme);
	}
    }

  DBG("%s() called. calling parse_spec with %s\n", __func__, 
      spec == NULL ? "NULL" : spec );

  mbdesktop_bg_parse_spec(mb, spec);

  if (mb->bg_img != NULL) {
    DBG("%s() called. bg_img is not null, calling init_bg()\n", __func__ );
    mbdesktop_view_init_bg(mb);
  }

  if (spec) free(spec);
}

void
mbdesktop_switch_theme (MBDesktop *mb, char *theme_name )
{
  if (theme_name != NULL)
    {
      if (mb->theme_name) free(mb->theme_name);
      mb->theme_name = strdup(theme_name);
    }

#ifdef USE_XSETTINGS
  if (!mb->xsettings_have_bg)
#endif
    mbdesktop_switch_bg_theme(mb);    

  mbdesktop_switch_icon_theme(mb, mb->top_head_item); 
  mbdesktop_set_scroll_buttons(mb);
  mbdesktop_set_popup_buttons(mb);

  if (mb->bg_img && mb->top_head_item != NULL)
    mbdesktop_view_paint(mb, False, False);

}

void
mbdesktop_switch_icon_theme (MBDesktop     *mb, 
			     MBDesktopItem *item )
{
  MBDesktopItem *item_cur = item;

  while (item_cur != NULL)
    {
      if (item_cur->item_child)
	mbdesktop_switch_icon_theme (mb, item_cur->item_child );

      if (item_cur->icon_name)
	mbdesktop_item_set_icon_from_theme (mb, item_cur);
      item_cur = item_cur->item_next_sibling;
    }
}

Bool
mbdesktop_get_workarea(MBDesktop *mb, int *x, int *y, int *w, int *h)
{
  Atom real_type; int real_format;
  unsigned long items_read, items_left;
  long *coords;

  Atom workarea_atom =
    XInternAtom(mb->dpy, "_NET_WORKAREA",False);

  if (XGetWindowProperty(mb->dpy, mb->root,
			 workarea_atom, 0L, 4L, False,
			 XA_CARDINAL, &real_type, &real_format,
			 &items_read, &items_left,
			 (unsigned char **) &coords) == Success
      && items_read) {
    *x = coords[0];
    *y = coords[1];
    *w = coords[2];
    *h = coords[3];
    XFree(coords);
    return True;
  }
  
  return False;
}

void
mbdesktop_bg_free_config(MBDesktop *mb)
{
  if (mb->bg->type == BG_TILED_PXM || mb->bg->type == BG_STRETCHED_PXM)
    free(mb->bg->data.filename);
  free(mb->bg);
  mb->bg = NULL;
}

Bool
mbdesktop_bg_parse_spec(MBDesktop *mb, char *spec)
{
  /*
  img-stretched:filename>
  img-tiled:<filename>
  col-solid:<color definition>
  col-gradient-vertical:<start color>,<end color>
  col-gradient-horizontal:<start color>,<end color>
  */

  XColor tmpxcol;
  int i, mapping_cnt, spec_offset = 0, type = 0;
  char *bg_def = NULL, *p = NULL;

  struct conf_mapping_t {
    char *name;
    int   id;
  } conf_mapping[] = {
    { "img-stretched:",           BG_STRETCHED_PXM  },
    { "img-tiled:",               BG_TILED_PXM      },
    { "img-centered:",            BG_CENTERED_PXM   },
    { "col-solid:",               BG_SOLID          },
    { "col-gradient-vertical:",   BG_GRADIENT_VERT  },
    { "col-gradient-horizontal:", BG_GRADIENT_HORIZ },
  };

  mapping_cnt = (sizeof(conf_mapping)/sizeof(struct conf_mapping_t));

  if (mb->bg != NULL) mbdesktop_bg_free_config(mb);

  mb->bg = malloc(sizeof(MBDesktopBG));
  memset(mb->bg, 0, sizeof(mb->bg));

  if (spec == NULL)
    {
      /* XXX we should probably check theme.desktop too for a bg_def */
      bg_def = "#8395ac"; 	/* Defualt col - red on error */
      type   = BG_SOLID;
    }
  else
    {
      for (i=0; i<mapping_cnt; i++)
	{
	  spec_offset = strlen(conf_mapping[i].name);
	  if (spec_offset < strlen(spec)
	      && !strncmp(conf_mapping[i].name, spec, spec_offset))
	    {
	      type = conf_mapping[i].id;
	      break;
	    }
	}

      if (!type)
	{
	  /* Assume we've just been passed an image filename */
	  mb->bg->type = BG_STRETCHED_PXM;
	  mb->bg->data.filename = strdup(spec);
	  return True;
	} else bg_def = spec + spec_offset;
    }


  mb->bg->type = type;

  switch(type)
    {
    case BG_SOLID:
      XParseColor(mb->dpy, DefaultColormap(mb->dpy, mb->scr), 
		  bg_def, &tmpxcol);
      mb->bg->data.cols[0] = tmpxcol.red   >> 8;
      mb->bg->data.cols[1] = tmpxcol.green >> 8;
      mb->bg->data.cols[2] = tmpxcol.blue  >> 8;
      break;
    case BG_TILED_PXM:
    case BG_STRETCHED_PXM:
    case BG_CENTERED_PXM:
      mb->bg->data.filename = strdup(bg_def);
      break;
    case BG_GRADIENT_HORIZ:
    case BG_GRADIENT_VERT:
      p = bg_def;
      while(*p != ',' && *p != '\0') p++;
      if (*p == '\0')
	{
	  mbdesktop_bg_free_config(mb);
	  return False; 	/* XXX need to reset on fail */
	}
      *p = '\0';
      XParseColor(mb->dpy, DefaultColormap(mb->dpy, mb->scr), 
		  bg_def, &tmpxcol);
      mb->bg->data.gcols[0] = (tmpxcol.red   >> 8);
      mb->bg->data.gcols[2] = (tmpxcol.green >> 8);
      mb->bg->data.gcols[4] = (tmpxcol.blue  >> 8);
      p++;
      XParseColor(mb->dpy, DefaultColormap(mb->dpy, mb->scr), 
		  p, &tmpxcol);
      mb->bg->data.gcols[1] = (tmpxcol.red   >> 8);
      mb->bg->data.gcols[3] = (tmpxcol.green >> 8);
      mb->bg->data.gcols[5] = (tmpxcol.blue  >> 8);
      break;
    }    

  return True;
}

void
mbdesktop_set_font(MBDesktop *mb, char *spec)
{
  mb_font_set_from_string(mb->font, spec);
}


static Bool
mbdesktop_set_highlight_col(MBDesktop *mb, char *spec)
{
  mb_col_set (mb->hl_col, spec);

  return True;
}


void
mbdesktop_set_title_font(MBDesktop *mb, char *spec)
{
  mb_font_set_from_string(mb->titlefont, spec) ;

}

void
mbdesktop_set_font_color(MBDesktop *mb, char *spec)
{
  mb_col_set (mb->fgcol, spec);
}

/* Calculate various pos/size params fro current view */
void
mbdesktop_calculate_item_dimentions(MBDesktop *mb)
{
  if (mb->use_title_header)
    mb->title_offset = mb_font_get_height(mb->titlefont) + 4;
  else
    mb->title_offset = 0;

  /*fprintf(stdout, "matchbox-desktop: item_height = [%d], item_width = [%d]\n", mb->item_height, mb->item_width);*/
  switch (mbdesktop_current_folder_view ( mb ))
    {
    case   VIEW_ICONS:
      mb->item_height  = mb->icon_size + ( 2 * (mb_font_get_height(mb->font)) );
      mb->item_width   = mb->icon_size + mb->icon_padding;
      break;

    case   VIEW_LIST:
      mb->item_height  = mb->icon_size;
      mb->item_width   = mb->icon_size + mb->icon_padding;
      break;

    case VIEW_ICONS_ONLY:
      mb->item_height  = mb->icon_size + (mb->icon_size/4);
      mb->item_width   = mb->icon_size + (mb->icon_size/4);
      break;

    case VIEW_TEXT_ONLY:
      mb->item_height  = ( 1.5 * (mb_font_get_height(mb->font)) );
      mb->item_width   = mb->workarea_width;
      break;
    }
   /*fprintf(stdout, "matchbox-desktop: item_height = [%d], item_width = [%d]\n", mb->item_height, mb->item_width);*/
}

void
mbdesktop_set_scroll_buttons(MBDesktop *mb)
{
  /* XXX free existing */
  MBPixbufImage *img_tmp = NULL;
	MBPixbufImage *img_tmp2 = NULL;
	
#ifdef MB_HAVE_PNG
#define UP_IMG   "mbup.png"
#define DOWN_IMG "mbdown.png"
#define SCROLL_IMG "mbscroll.png"
#define SCROLL_HOT_IMG "mbscroll_hot.png"
#else
#define UP_IMG   "mbup.xpm"
#define DOWN_IMG "mbdown.xpm"
#define SCROLL_IMG "mbscroll.xpm"
#define SCROLL_HOT_IMG "mbscroll_hot.xpm"
#endif

//  if (mb->img_scroll_up) mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll_up);
//  if (mb->img_scroll_down) mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll_down);
	if (mb->img_scroll) mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll);
	if (mb->img_scroll_hot) mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll_hot);
		
  if ((mb->img_scroll = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, SCROLL_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", SCROLL_IMG);
      exit(1);
    }		

  if (mb_pixbuf_img_get_width(mb->img_scroll) != 152
      || mb_pixbuf_img_get_height(mb->img_scroll) != 152)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_scroll, 152, 152);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll);
      mb->img_scroll = img_tmp;
    }
    
  if ((mb->img_scroll_hot = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, SCROLL_HOT_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", SCROLL_HOT_IMG);
      exit(1);
    }		

  if (mb_pixbuf_img_get_width(mb->img_scroll_hot) != 152
      || mb_pixbuf_img_get_height(mb->img_scroll_hot) != 152)
    {
      img_tmp2 = mb_pixbuf_img_scale (mb->pixbuf, mb->img_scroll_hot, 152, 152);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll_hot);
      mb->img_scroll_hot = img_tmp2;
    }    
      
/*  
  if ((mb->img_scroll_up = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, UP_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", UP_IMG);
      exit(1);
    }

  if ((mb->img_scroll_down = mb_pixbuf_img_new_from_file(mb->pixbuf, 
							 mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, DOWN_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-desktop: Cannot load %s  - is matchbox-common installed ? Cannot continue.\n", DOWN_IMG);
      exit(1);
    }
      

  if (mb_pixbuf_img_get_width(mb->img_scroll_up) != 16
      || mb_pixbuf_img_get_height(mb->img_scroll_up) != 16)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_scroll_up, 16, 16);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll_up);
      mb->img_scroll_up = img_tmp;
    }

  if (mb_pixbuf_img_get_width(mb->img_scroll_down) != 16
      || mb_pixbuf_img_get_height(mb->img_scroll_down) != 16)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_scroll_down, 16, 16);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_scroll_down);
      mb->img_scroll_down = img_tmp;
    }
*/
}


void
mbdesktop_set_back_buttons(MBDesktop *mb)
{
  MBPixbufImage *img_tmp = NULL;
  MBPixbufImage *img_tmp2 = NULL;

#ifdef MB_HAVE_PNG
#define BACK_HOT_IMG   "mbback_hot.png"
#define BACK_DIS_IMG   "mbback_dis.png"
#else
#define BACK_HOT_IMG   "mbback_hot.xpm"
#define BACK_DIS_IMG   "mbback_dis.xpm"
#endif

	if (mb->img_back_hot) mb_pixbuf_img_free(mb->pixbuf, mb->img_back_hot);
	if (mb->img_back_dis) mb_pixbuf_img_free(mb->pixbuf, mb->img_back_dis);
		
  if ((mb->img_back_hot = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, BACK_HOT_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", BACK_HOT_IMG);
      exit(1);
    }		

  if (mb_pixbuf_img_get_width(mb->img_back_hot) != 32
      || mb_pixbuf_img_get_height(mb->img_back_hot) != 32)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_back_hot, 32, 32);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_back_hot);
      mb->img_back_hot = img_tmp;
    }
    
    
  if ((mb->img_back_dis = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, BACK_DIS_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", BACK_DIS_IMG);
      exit(1);
    }		

  if (mb_pixbuf_img_get_width(mb->img_back_dis) != 32
      || mb_pixbuf_img_get_height(mb->img_back_dis) != 32)
    {
      img_tmp2 = mb_pixbuf_img_scale (mb->pixbuf, mb->img_back_dis, 32, 32);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_back_dis);
      mb->img_back_dis = img_tmp2;
    }      

}


void
mbdesktop_set_item_hot(MBDesktop *mb)
{
  /* XXX free existing */
  MBPixbufImage *img_tmp = NULL;

#ifdef MB_HAVE_PNG
#define ITEM_HOT_IMG "mbitem_hot.png"
#else
#define ITEM_HOT_IMG "mbitem_hot.xpm"
#endif

	if (mb->img_item_hot) mb_pixbuf_img_free(mb->pixbuf, mb->img_item_hot);
	
  if ((mb->img_item_hot = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, ITEM_HOT_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", ITEM_HOT_IMG);
      exit(1);
    }		

  if (mb_pixbuf_img_get_width(mb->img_item_hot) != mb->icon_size + 8
      || mb_pixbuf_img_get_height(mb->img_item_hot) != mb->icon_size + 8)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_item_hot, mb->icon_size + 8, mb->icon_size + 8);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_item_hot);
      mb->img_item_hot = img_tmp;
    }
  
}

void
mbdesktop_set_title_bar(MBDesktop *mb)
{
  /* XXX free existing */
  MBPixbufImage *img_tmp = NULL;

#ifdef MB_HAVE_PNG
#define TITLE_BAR_IMG "mbtitlebar.png"
#else
#define TITLE_BAR_IMG "mbtitlebar.xpm"
#endif

	if (mb->img_title_bar) mb_pixbuf_img_free(mb->pixbuf, mb->img_title_bar);
	
  if ((mb->img_title_bar = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, TITLE_BAR_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", TITLE_BAR_IMG);
      exit(1);
    }		

  if (mb_pixbuf_img_get_width(mb->img_title_bar) != 480
      || mb_pixbuf_img_get_height(mb->img_title_bar) != 34)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_title_bar, 480, 34);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_title_bar);
      mb->img_title_bar = img_tmp;
    }
  
}


void
mbdesktop_set_popup_buttons(MBDesktop *mb)
{
  /* XXX free existing */
  MBPixbufImage *img_tmp = NULL;

#ifdef MB_HAVE_PNG
#define MBPOP_IMG   "mbpopmenu.png"
#else
#define MBPOP_IMG   "mbpopmenu.xpm"
#endif

  if (mb->img_popmenu) mb_pixbuf_img_free(mb->pixbuf, mb->img_popmenu);
  
  if ((mb->img_popmenu = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBPOP_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBPOP_IMG);
      exit(1);
    }

  if (mb_pixbuf_img_get_width(mb->img_popmenu) != 160
      || mb_pixbuf_img_get_height(mb->img_popmenu) != 30)
    {
      img_tmp = mb_pixbuf_img_scale (mb->pixbuf, mb->img_popmenu, 160, 30);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_popmenu);
      mb->img_popmenu = img_tmp;
    }
}

void
mbdesktop_set_hisapp_buttons(MBDesktop *mb)
{
  /* XXX free existing */
  MBPixbufImage *img_tmp1 = NULL;
  MBPixbufImage *img_tmp2 = NULL;
  MBPixbufImage *img_tmp3 = NULL;
  MBPixbufImage *img_tmp4 = NULL;

#ifdef MB_HAVE_PNG
#define MBHISAPP_IMG       "mbhisapp.png"
#define MBHISAPP_HOT_IMG   "mbhisapp-hot.png"
#define MBHISAPP_IMG1      "mbhisapp1.png"
#define MBHISAPP_IMG2      "mbhisapp2.png"
#define MBHISAPP_IMG3      "mbhisapp3.png"
#define MBHISAPP_IMG4      "mbhisapp4.png"
#else
#define MBHISAPP_IMG        "mbhisapp.xpm"
#define MBHISAPP_HOT_IMG    "mbhisapp-hot.xpm"
#define MBHISAPP_IMG1       "mbhisapp1.xpm"
#define MBHISAPP_IMG2       "mbhisapp2.xpm"
#define MBHISAPP_IMG3       "mbhisapp3.xpm"
#define MBHISAPP_IMG4       "mbhisapp4.xpm"
#endif

  if (mb->img_hisapp) mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp);
  	if (mb->img_hisapp_hot) mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp_hot);
 	if (mb->img_hisapp1) mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp1);
  if (mb->img_hisapp2) mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp2);
  if (mb->img_hisapp3) mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp3);
  if (mb->img_hisapp4) mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp4);
  	  	  	  
  if ((mb->img_hisapp = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBHISAPP_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBHISAPP_IMG);
      exit(1);
    }
    
  if ((mb->img_hisapp_hot = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBHISAPP_HOT_IMG)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBHISAPP_HOT_IMG);
      exit(1);
    }    

  if ((mb->img_hisapp1 = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBHISAPP_IMG1)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBHISAPP_IMG1);
    }
  else  
  	if (mb_pixbuf_img_get_width(mb->img_hisapp1) != 32
      || mb_pixbuf_img_get_height(mb->img_hisapp1) != 32)
    {
      img_tmp1 = mb_pixbuf_img_scale (mb->pixbuf, mb->img_hisapp1, 32, 32);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp1);
      mb->img_hisapp1 = img_tmp1;
    }    
    
  if ((mb->img_hisapp2 = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBHISAPP_IMG2)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBHISAPP_IMG2);
    }
  else  
  	if (mb_pixbuf_img_get_width(mb->img_hisapp2) != 32
      || mb_pixbuf_img_get_height(mb->img_hisapp2) != 32)
    {
      img_tmp2 = mb_pixbuf_img_scale (mb->pixbuf, mb->img_hisapp2, 32, 32);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp2);
      mb->img_hisapp2 = img_tmp2;
    }     
    
  if ((mb->img_hisapp3 = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBHISAPP_IMG3)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBHISAPP_IMG3);
    }
	else
  	if (mb_pixbuf_img_get_width(mb->img_hisapp3) != 32
      || mb_pixbuf_img_get_height(mb->img_hisapp3) != 32)
    {
      img_tmp3 = mb_pixbuf_img_scale (mb->pixbuf, mb->img_hisapp3, 32, 32);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp3);
      mb->img_hisapp3 = img_tmp3;
    }    
/*    
  if ((mb->img_hisapp4 = mb_pixbuf_img_new_from_file(mb->pixbuf,
						       mb_dot_desktop_icon_get_full_path (mb->theme_name, 16, MBHISAPP_IMG4)))
      == NULL)
    {
      fprintf(stderr, "matchbox-bdesktop: Cannot load %s - is matchbox-common installed ? Cannot continue.\n", MBHISAPP_IMG4);
    }
  else            
  	if (mb_pixbuf_img_get_width(mb->img_hisapp4) != 32
      || mb_pixbuf_img_get_height(mb->img_hisapp4) != 32)
    {
      img_tmp4 = mb_pixbuf_img_scale (mb->pixbuf, mb->img_hisapp4, 32, 32);
      mb_pixbuf_img_free(mb->pixbuf, mb->img_hisapp4);
      mb->img_hisapp4 = img_tmp4;
    }    
*/   
}

void
mbdesktop_set_icon(MBDesktop *mb)
{
#ifdef MB_HAVE_PNG
#define MBDESKTOP_ICON "mbdesktop.png"
#else
#define MBDESKTOP_ICON "mbdesktop.xpm"
#endif

  MBPixbufImage *win_top_level_img_icon = NULL;
  int *win_top_level_icon_data = NULL;
  Atom atom_net_wm_icon;


  if ((win_top_level_img_icon 
       = mb_pixbuf_img_new_from_file(mb->pixbuf, 
				     DATADIR "/pixmaps/" MBDESKTOP_ICON)) != NULL)
    {
      win_top_level_icon_data 
	= malloc(sizeof(CARD32)* ((win_top_level_img_icon->width * 
				   win_top_level_img_icon->height)+2));
      
      if (win_top_level_icon_data)
	{
	  int i = 2, x, y;
	  atom_net_wm_icon = XInternAtom(mb->dpy, "_NET_WM_ICON",False);
	  win_top_level_icon_data[0] = win_top_level_img_icon->width;
	  win_top_level_icon_data[1] = win_top_level_img_icon->height;

	  for (y=0; y<win_top_level_img_icon->height; y++)	  
	    for (x=0; x<win_top_level_img_icon->width; x++)
	      {
		unsigned char r,g,b,a;
		mb_pixbuf_img_get_pixel (mb->pixbuf, 
					 win_top_level_img_icon, 
					 x, y, &r, &g, &b, &a);
		win_top_level_icon_data[i] = (a << 24 | r << 16 | g << 8|b );
		i++;
	      }

	  XChangeProperty(mb->dpy, mb->win_top_level, 
			  atom_net_wm_icon ,
			  XA_CARDINAL, 32, PropModeReplace,
			  (unsigned char *)win_top_level_icon_data, 
			  (win_top_level_img_icon->width * 
			   win_top_level_img_icon->height )+2);
	  
	  free(win_top_level_icon_data);
	}
    }
}

static void
usage(char *name)
{
  fprintf(stderr, 
	  "Usage: %s [options..]\n"
	  "Where options are:\n"
	  "   -display       Display to connect to\n"
	  "  --icon-size    <int>         Icon size ( defualt: 32 )\n"
          "  --icon-padding <int>         Specify padding between icons\n"
	  "  --font         <font>        Icon font\n"
	  "  --titlefont    <font>        Title font\n"
	  "  --fontcol      <col>         Font color\n"
          "  --no-outline                 Dont outline text\n"
	  "  --no-title                   Dont render current folder title\n"
	  "  --bg            <background definition>, like;\n\n"
	  "\t\timg-stretched:<filename>\n"
	  "\t\timg-tiled:<filename>\n"
	  "\t\timg-centered:<filename>\n"
	  "\t\tcol-solid:<color definition>\n"
	  "\t\tcol-gradient-vertical:<start color>,<end color>\n"
          "\t\tcol-gradient-horizontal:<start color>,<end color>\n\n"
	  "Notes;\n"
	  "  <col> is specified as a color name or an rgb def in the form 'rgb:r/g/b' or '#RGB\n"
	  "\n%s is copyright Matthew Allum 2003\n",
	  name, name
	  );
  exit(1);
}


MBDesktop *
mbdesktop_init(int argc, char **argv)
{
  MBDesktop *mb;
  XGCValues gv;

  struct sigaction act;

  char *font_def       = FONT_DESC;
  char *font_title_def = FONT_TITLE_DESC;
  char *font_title_path_def = FONT_TITLE_PATH_DESC;
  char *font_col_def   = FONT_COL;
	char *icon_path = NULL;
	
  int i;
  char *display_name = (char *)getenv("DISPLAY");  

  Bool font_user_set = False;

  /* Non-portable ? will work ok on BSD's ? */
  signal( SIGCHLD, SIG_IGN );

  mb = malloc(sizeof(MBDesktop));
  memset(mb, 0, sizeof(MBDesktop));

  mb->dd_dir          = DD_DIR;
  mb->icon_size       = 0;
  mb->dd_dir_mtime    = 0;

  mb->top_level_name   = strdup("Home");
  mb->bg_def           = NULL;
  mb->view_type        = VIEW_ICONS; 
  mb->use_text_outline = True;
  mb->icon_padding     = 0;

  mb->user_overide_font_col     = False;
  mb->user_overide_font_outline = False;
  mb->use_title_header          = True;
  
 // mb->popup_mode	= False;
  mb->hisapp_mode	= 0;
 // mb->wheel_mode	= False;


  for (i = 1; i < argc; i++) {
    if (!strcmp ("-display", argv[i]) || !strcmp ("-d", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      display_name = argv[i];
      setenv ("DISPLAY", display_name, 1);
      continue;
    }
    if (!strcmp ("--bg", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      mb->bg_def = strdup(argv[i]);
      continue;
    }
    if (!strcmp("--help", argv[i]) || !strcmp("-h", argv[i])) {
      usage(argv[0]);
    }
    /* XXX --default-view list|icons|icons-only|text-only?
    if (!strcmp ("--listview", argv[i])) {
      mb->view_type = VIEW_LIST;
      continue;
    }
    */
    if (!strcmp ("--no-title", argv[i])) {
      mb->use_title_header = False;
      continue;
    }

    if (!strcmp ("--icon-size", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      mb->icon_size = atoi(argv[i]);
      if (!mb->icon_size) usage(argv[0]);
      continue;
    }
    if (!strcmp ("--icon-padding", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      mb->icon_padding = atoi(argv[i]);
      if (!mb->icon_padding) usage(argv[0]);
      continue;
    }
    if (!strcmp ("--font", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      font_def = argv[i];
      font_user_set = True;
      continue;
    }
    if (!strcmp ("--titlefont", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      font_title_def = argv[i];
      font_title_path_def = argv[i];
      continue;
    }
    if (!strcmp ("--fontcol", argv[i])) {
      if (++i>=argc) usage (argv[0]);
      mb->user_overide_font_col = True;
      font_col_def = argv[i];
      continue;
    }
    if (!strcmp ("--no-outline", argv[i])) {
      mb->user_overide_font_outline = True;
      mb->use_text_outline          = False;
      continue;
    }

    usage(argv[0]);
  }

  if ((mb->dpy = XOpenDisplay(display_name)) == NULL)
    {
      fprintf(stderr, "matchbox-desktop: unable to open display !\n");
      exit(1);
    }


  mb->scr = DefaultScreen(mb->dpy);
  mb->root = RootWindow(mb->dpy, mb->scr); 
  mb->pixbuf = mb_pixbuf_new(mb->dpy, mb->scr);
  mb->root_pxm = None;

  mb->ddfolders = NULL;

  mb->had_kbd_input = False;
  mb->theme_name    = NULL;
  mb->bg_img        = NULL;
  mb->bg = NULL;

  mb->top_head_item = NULL;

  /* Dimentions */
  /*
  mb->desktop_width  = DisplayWidth(mb->dpy, mb->scr);
  mb->desktop_height = DisplayHeight(mb->dpy, mb->scr);
  */
  mb->desktop_width  = 480;
  mb->desktop_height = 640;


  
  mb->hl_col = mb_col_new_from_spec(mb->pixbuf, NULL);
  mb->fgcol = mb_col_new_from_spec(mb->pixbuf, NULL);
  mb->bgcol = mb_col_new_from_spec(mb->pixbuf, NULL);

  /* Figure out defualts */
  if (mb->icon_size == 0)
    {
      if (mb->desktop_width > 320)
	{
	  mb->icon_size = 48;
	  if (!mb->icon_padding) mb->icon_padding = 32; 
	  if (!font_user_set) font_def = FONT_DESC;
	}
      else
	{
	  mb->icon_size = 32;
	  if (!mb->icon_padding) mb->icon_padding = 28;
	}
    }
  else if (!mb->icon_padding) 
    mb->icon_padding = mb->icon_size / 2; 
    
/* 我们定制窗口工作区域
  if (mbdesktop_get_workarea(mb, &mb->workarea_x, &mb->workarea_y, 
			     &mb->workarea_width, 
			     &mb->workarea_height ) == False )
    {
      mb->workarea_x = 0;
      mb->workarea_y = 0;
      mb->workarea_width = DisplayWidth(mb->dpy, mb->scr);
      mb->workarea_height = DisplayHeight(mb->dpy, mb->scr);
   
    }
*/
   mb->workarea_x = 0;
   mb->workarea_y = 44;
   mb->workarea_width = 480;
   mb->workarea_height = 552;
          
  if(mb->workarea_height < 500) {
  	mb->icon_size=72;
  	mb->icon_padding = 88;
  }


  mb->atom_mb_theme = XInternAtom(mb->dpy, "_MB_THEME_NAME",False);

  mbdesktop_get_theme_via_root_prop(mb);

  if (mb->bg_def == NULL)
    {
      mbdesktop_bg_parse_spec(mb, NULL);
      mbdesktop_switch_bg_theme(mb);
    }
  else
    mbdesktop_bg_parse_spec(mb, mb->bg_def);

  mbdesktop_set_font_color(mb, font_col_def);

  mb->titlefont = mb_font_new_from_string(mb->dpy, font_title_def);
  mb_font_set_color(mb->titlefont, mb->fgcol);
  
  mb->titlefontpath = mb_font_new_from_string(mb->dpy, font_title_path_def);
  mb_font_set_color(mb->titlefontpath, mb->fgcol);

  mb->font      = mb_font_new_from_string(mb->dpy, font_def);
  mb_font_set_color(mb->font, mb->fgcol);


#ifdef USE_XSETTINGS
  mb->xsettings_have_bg      = False; 
  mb->xsettings_have_manager = False;

  /* This will trigger callbacks instantly */

  mb->xsettings_client = xsettings_client_new(mb->dpy, mb->scr,
					      mbdesktop_xsettings_notify_cb,
					      mbdesktop_xsettings_watch_cb, 
					      (void *)mb );
#endif

  DBG("%s() finished creating xsettings client \n", __func__);

  mb->folder_img_path = NULL;

	/* Added By Steven Chen For Popup menu */
	mb->click_time          = 400;




   /* GC's */
  gv.function = GXcopy;
  gv.graphics_exposures = 0;
  
  mb->gc = XCreateGC(mb->dpy, mb->root,
		     GCFunction|GCGraphicsExposures, 
		     &gv);

  gv.function = GXinvert;
  gv.subwindow_mode = IncludeInferiors;

  mb->invert_gc = XCreateGC(mb->dpy, mb->root,
			    GCFunction|GCSubwindowMode|GCGraphicsExposures
			    , &gv);


  //  mbdesktop_calculate_item_dimentions(mb);

  mbdesktop_set_scroll_buttons(mb);

	mbdesktop_set_title_bar(mb);
	
	mbdesktop_set_item_hot(mb);
	
	mbdesktop_set_popup_buttons(mb);
	
	mbdesktop_set_hisapp_buttons(mb);
	
	mbdesktop_set_back_buttons(mb);
	
  if (mb->mbmenu == NULL)
    {
      mb->mbmenu = mb_menu_new(mb->dpy, mb->scr ); 
      mb_menu_set_icon_size(mb->mbmenu, 32);
			mb_menu_set_font (mb->mbmenu, FONT_POPUP_DESC);   
			mb_menu_set_trans(mb->mbmenu, 0);
    }
  else mb_menu_free(mb->mbmenu); /* XXX should be mb_menu_empty */	
	
  icon_path = mb_dot_desktop_icon_get_full_path (mb->theme_name, 
						 16, HIDE_IMG  );

  mb_menu_add_item_to_menu(mb->mbmenu, mb->mbmenu->rootmenu, _("Execute Application"), 
			  icon_path, NULL , 
			  mbdesktop_menu_execute_cb, (void *)mb, MBMENU_NO_SORT); 	
			  
  icon_path = mb_dot_desktop_icon_get_full_path (mb->theme_name, 
						 16, ADD_IMG  );	

  mb_menu_add_item_to_menu(mb->mbmenu, mb->mbmenu->rootmenu, _("Add To Launcher"), 
			  icon_path, NULL , 
			  mbdesktop_menu_add_to_launch_cb, (void *)mb, MBMENU_NO_SORT); 	

  icon_path = mb_dot_desktop_icon_get_full_path (mb->theme_name, 
						 16, REMOVE_IMG  );	

  mb_menu_add_item_to_menu(mb->mbmenu, mb->mbmenu->rootmenu, _("Remove From Launcher"), 
			  icon_path, NULL , 
			  mbdesktop_menu_remove_from_launch_cb, (void *)mb, MBMENU_NO_SORT); 	
			  			  						 
  if (icon_path) free(icon_path);
	
	
  /* ewmh hints */
  mb->window_type_atom =
    XInternAtom(mb->dpy, "_NET_WM_WINDOW_TYPE" , False);
  mb->window_type_desktop_atom =
    XInternAtom(mb->dpy, "_NET_WM_WINDOW_TYPE_DESKTOP",False);
  mb->desktop_manager_atom =
    XInternAtom(mb->dpy, "_NET_DESKTOP_MANGER",False);

  mb->window_type_dialog_atom =
    XInternAtom(mb->dpy, "_NET_WM_WINDOW_TYPE_DIALOG",False);
  mb->window_state_atom =
    XInternAtom(mb->dpy, "_NET_WM_STATE",False);
  mb->window_state_modal_atom =
    XInternAtom(mb->dpy, "_NET_WM_STATE_MODAL",False);
  mb->window_utf8_name_atom =
    XInternAtom(mb->dpy, "_NET_WM_NAME",False);
  mb->utf8_atom =
    XInternAtom(mb->dpy, "UTF8_STRING",False);
  
  mb->win_top_level = XCreateWindow(mb->dpy, mb->root, 
		                    0, 0,  
				    mb->desktop_width, 
				    mb->desktop_height, 0, 
				    CopyFromParent,  
				    CopyFromParent, 
				    mb->pixbuf->vis,
				    0, NULL);

  /* Make sure with mess with no other already running desktops */
  if (XGetSelectionOwner(mb->dpy, mb->desktop_manager_atom))
    {
      fprintf(stderr, "matchbox-desktop: Desktop Already running on this display.\n");
      exit(1);
    }

  /* ...And they hopefully dont mess with us.... */
  XSetSelectionOwner(mb->dpy, mb->desktop_manager_atom, 
		     mb->win_top_level, CurrentTime);

  XStoreName(mb->dpy, mb->win_top_level, "Desktop" );

  XChangeProperty(mb->dpy, mb->win_top_level, mb->window_type_atom, XA_ATOM, 32, 
		  PropModeReplace, 
		  (unsigned char *) &mb->window_type_desktop_atom, 1);

  mbdesktop_set_icon(mb);

  mbdesktop_progress_dialog_init(mb);

#ifdef USE_DNOTIFY
  _Gmb = mb; 			/* arg, global mb for sig callback */

  act.sa_sigaction = sig_reload_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGRTMIN, &act, NULL);
#endif 

  act.sa_sigaction = sig_hup_reload_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGHUP, &act, NULL);

  mb->type_register_cnt = ITEM_TYPE_CNT;

  /* get the window ready */
  XSelectInput(mb->dpy, mb->win_top_level, 
	       ExposureMask | ButtonPressMask | ButtonReleaseMask |
	       KeyPress | KeyRelease | StructureNotifyMask |
	       /* Below for reparented module plugin */
	       SubstructureRedirectMask | SubstructureNotifyMask |
	       FocusChangeMask );

  XSelectInput(mb->dpy, mb->root, PropertyChangeMask );

  XMapWindow(mb->dpy, mb->win_top_level);

  return mb;
}

void 
mbdesktop_scroll_up(MBDesktop *mb)
{
  int i, items_per_row;

  if (mb->scroll_offset_item->item_prev_sibling)
    {
      if (mbdesktop_current_folder_view ( mb ) == VIEW_LIST)
	{
	  mb->scroll_offset_item = mb->scroll_offset_item->item_prev_sibling;

	  if (mb->kbd_focus_item->item_prev_sibling)
	    mb->kbd_focus_item = mb->kbd_focus_item->item_prev_sibling;
	}
      else
	{
	  items_per_row = mb->workarea_width / mb->item_width;
	  for(i = 0; i < items_per_row; i++)
	    {
	    	fprintf(stdout, "steven_desktop: scroll_offset_item->type = [%d] name = [%s];item_prev_sibling->type = [%d] name = [%s]\n", mb->scroll_offset_item->type, mb->scroll_offset_item->name, mb->scroll_offset_item->item_prev_sibling->type, mb->scroll_offset_item->item_prev_sibling->name);
	    	mb->scroll_offset_item = mb->scroll_offset_item->item_prev_sibling;
	      if(mb->scroll_offset_item->item_prev_sibling)
	        if(mb->scroll_offset_item->item_prev_sibling->type == ITEM_TYPE_PREVIOUS)
	      		mb->scroll_offset_item = mb->scroll_offset_item->item_prev_sibling;
	      if (mb->kbd_focus_item->item_prev_sibling)
		mb->kbd_focus_item = mb->kbd_focus_item->item_prev_sibling;
	    }
	}
    }
}

void 
mbdesktop_scroll_down(MBDesktop *mb)
{
  MBDesktopItem *orig = mb->scroll_offset_item;
  int n = 0;

  /* Check we dont scroll to far */
  do { n++; } while ((orig = orig->item_next_sibling) != NULL);

  if (n < (mb->current_view_columns * mb->current_view_rows))
    return;

  /* now do the actual scroll */

  orig = mb->scroll_offset_item;
  
  /* Added By Steven Chen */
  /* 如果刚进入一个FOLDER需要多移动一个ITEM */
  if( mb->scroll_offset_item->type == ITEM_TYPE_PREVIOUS )
  {
  	if(mb->scroll_offset_item->item_next_sibling == NULL)
  		{
  			mb->scroll_offset_item = orig;
  			return;
  		}
  	else
  		mb->scroll_offset_item = mb->scroll_offset_item->item_next_sibling;
  }
  		
  

  if (mbdesktop_current_folder_view ( mb ) == VIEW_LIST)
    {
      if (mb->scroll_offset_item->item_next_sibling != NULL)
	{
	  mb->scroll_offset_item = mb->scroll_offset_item->item_next_sibling;

	  if (mb->kbd_focus_item->item_next_sibling)
	    mb->kbd_focus_item = mb->kbd_focus_item->item_next_sibling;
	}

      
    } else {
      if (mb->scroll_offset_item->item_next_sibling)
	{
	  int i, items_per_row = mb->workarea_width / mb->item_width;
	  for(i = 0; i < items_per_row; i++)
	    {
	      if (mb->scroll_offset_item->item_next_sibling == NULL)
		{
		  mb->scroll_offset_item = orig;
		  return;
		} 
	      else
		mb->scroll_offset_item = mb->scroll_offset_item->item_next_sibling;
	      
	      if (mb->kbd_focus_item->item_next_sibling)
		mb->kbd_focus_item = mb->kbd_focus_item->item_next_sibling;
	    }
	}
    }
}

void
handle_button_event(MBDesktop *mb, XButtonEvent *e)
{
  XEvent ev;
  MBDesktopItem *active_item = NULL;
	struct timeval then, now;					/* 用来计算指针释放的时间 */
	int done = 0;											/* done = 1 表示单击；done = 2 表示长按 */
	
	gettimeofday(&then, NULL);				/* 开始按下的时间 */
	

  active_item = mbdesktop_item_get_from_coords(mb, e->x, e->y); 
  
  if (active_item)
    {
    	fprintf(stdout, "steven-desktop: active_item=[true] active_item->type = [%d]\n", active_item->type);
    	/*Bool canceled = False;*/
      
      if(active_item->type == ITEM_TYPE_PREVIOUS && mb->popup_mode)
      {
      	mb->popup_mode = False;
      	mbdesktop_view_paint(mb, False, False);   
      	return;
      }
      
     	/*if(mb_menu_is_active(mb->mbmenu))*/
     	if(mb->popup_mode || mb_menu_is_active(mb->mbmenu))
     	{
      	mb->popup_mode = False;
      	mbdesktop_view_paint(mb, False, False);  
      	mbdesktop_view_item_highlight (mb, active_item, HIGHLIGHT_FILL);  		    		
				return;
			}
	
      
      if (XGrabPointer(mb->dpy, mb->win_top_level, False,
		       ButtonPressMask|ButtonReleaseMask|
		       PointerMotionMask|EnterWindowMask|LeaveWindowMask,
		       GrabModeAsync,
		       GrabModeAsync, None, None, CurrentTime)
	  == GrabSuccess)
	{
	  if (mb->have_focus && mb->had_kbd_input)
	    {
	      mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
					     HIGHLIGHT_OUTLINE_CLEAR); 
	    }
/*
		// 如果是回退按钮就不用高亮显示了
		if(active_item->type != ITEM_TYPE_PREVIOUS)
	  	mbdesktop_view_item_highlight (mb, active_item, HIGHLIGHT_FILL); 
*/
		
	  
	  /*for (;;) */
	  done = 0;
	  
  while (!done) {
    /*if (XCheckMaskEvent(mb->dpy,	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, &ev))*/
    	if (XCheckMaskEvent(mb->dpy,	ButtonReleaseMask|PointerMotionMask, &ev))
       switch (ev.type)
				{
					case MotionNotify:
						done = 1;
						break;
					case ButtonRelease:
						done = 2;
						break;
				}
      
      gettimeofday(&now, NULL);
		  if ( (now.tv_usec-then.tv_usec) > (mb->click_time*1000) )
      		done = 3;
  }
  /*
 	int x1 = active_item->x;
	int x2 = active_item->x + active_item->width;
	int y1 = active_item->y;
	int y2 = active_item->y + active_item->height;
	*/
	
	switch(done)
  {
  	case 1:
		 if(!mb->popup_mode)
			{
				/*
				if(mb->kbd_focus_item)
					mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, HIGHLIGHT_OUTLINE_CLEAR); 		  
				
				if(active_item->type != ITEM_TYPE_PREVIOUS)	
		  		mbdesktop_view_item_highlight (mb, active_item, HIGHLIGHT_FILL);  		
		  	*/
		  	
		  	mb->kbd_focus_item = active_item;
		  	
		  				mb->popup_mode = False;
							mb->current_popup_item = NULL;			  	
		  }
		  else
		  {
		  	/*
		  	if(e->x > mb->pop_x &&
		  		 e->x < mb->pop_x + mb->item_width &&
		  		 e->y > mb->pop_y &&
		  		 e->y < mb->pop_y + 30*3)
		  		 {
		  		 		fprintf(stdout, "steven-desktop: MotionNotify in Popup Mode.\n");	
		  		 		return;
		  		 }
		  	else
						  	
		  		 {
		  				mb->popup_mode = False;
							mb->current_popup_item = NULL;	
							mbdesktop_view_paint(mb, False, False);   
							return;  
					 }
   				*/
			} 	
  		break;
  	case 2:
		  XUngrabPointer(mb->dpy, CurrentTime);
		     		
					if(!mb->popup_mode)
						{
									/*
									if(mb->kbd_focus_item)
										mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, HIGHLIGHT_OUTLINE_CLEAR); 
							
									if(active_item->type != ITEM_TYPE_PREVIOUS)
										mbdesktop_view_item_highlight (mb, active_item, HIGHLIGHT_FILL); 
									*/
									
									mb->kbd_focus_item = active_item;
							
									mb->current_popup_item = NULL;	
									mb->popup_mode = False;
																
						      if (active_item->activate_cb)	active_item->activate_cb((void *)mb, 
										 (void *)active_item); 
							
						      return;			      
						}
					else
						{
							/*
					  	if(e->x > mb->pop_x &&
					  		 e->x < mb->pop_x + mb->item_width &&
					  		 e->y > mb->pop_y &&
					  		 e->y < mb->pop_y + 30*3)
					  		 {
					  		 		fprintf(stdout, "steven-desktop: ButtonRelease in Popup Mode.\n");	
					  		 		
					  		 		if(e->y > mb->pop_y && e->y < mb->pop_y + 30)
					  		 		{
					  		 			fprintf(stdout, "steven-desktop: Excute Application. Name = [%s].\n", mb->current_popup_item->name);	
						      		
						      		if (mb->current_popup_item->activate_cb)	mb->current_popup_item->activate_cb((void *)mb, 
										 			(void *)mb->current_popup_item); 					  		 			
										 			
					  		 			mb->popup_mode = False;
					  		 			mb->current_popup_item = NULL;	
					  		 			mbdesktop_view_paint(mb, False, False);  
					  		 		}else if(e->y > mb->pop_y + 30 && e->y < mb->pop_y + 30*2)
					  		 		{	
					  		 			fprintf(stdout, "steven-desktop: Add To Lanuch .\n");	
					  		 	
					  		 			
					  		 			mb->popup_mode = False;
					  		 			mb->current_popup_item = NULL;	
					  		 			mbdesktop_view_paint(mb, False, False);  					  		 			

					  		 		}else
					  		 		{
					  		 			fprintf(stdout, "steven-desktop: Remove From Lanuch  .\n");	
					  		 		
					  		 			
					  		 			mb->popup_mode = False;
					  		 			mb->current_popup_item = NULL;		
					  		 			mbdesktop_view_paint(mb, False, False);  				  		 			
					  		 		}
					  		 			
					  		 		return;
					  		 }
					  	else
					  		 {
					  				mb->popup_mode = False;
					  				mb->current_popup_item = NULL;			
										mbdesktop_view_paint(mb, False, False);   
										return;  
								 }	
						 */						
						}   		
  		break;
  	case 3:

					if( active_item->type == ITEM_TYPE_DOTDESKTOP_ITEM )
						{
					  	if(e->x > mb->pop_x &&
					  		 e->x < mb->pop_x + mb->item_width &&
					  		 e->y > mb->pop_y &&
					  		 e->y < mb->pop_y + 30*3 &&
					  		 mb->popup_mode)
					  	{
					  		fprintf(stdout, "steven-desktop: With Hold in Popup Mode.\n");	
					  		return;
					  	}
					  	else
					  	{								
								/* ?? 鼠标跟随还是固定位置 ? */
								mb->pop_x = e->x;
								mb->pop_y = e->y;
								/*
								mb->pop_x = mb->workarea_x + mb->item_width;
								mb->pop_y = mb->workarea_y + mb->title_offset + mb->icon_size / 16 + mb->item_height;				
								*/
								/*fprintf(stdout, "steven-desktop: Hold - done = [%d] popup_mode = [%d] x = [%d] y = [%d]\n", done, mb->popup_mode, mb->pop_x, mb->pop_y);   */
								
								/*
								if(mb->kbd_focus_item)
									mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, HIGHLIGHT_OUTLINE_CLEAR); 
								
								if(active_item->type != ITEM_TYPE_PREVIOUS)
									mbdesktop_view_item_highlight (mb, active_item, HIGHLIGHT_OUTLINE); 
								*/	
								mb->kbd_focus_item = active_item;
								
								mb->current_popup_item = active_item;
								
								mbdesktop_view_paint(mb, False, True);   		
								
								/* 显示Popup menu */
								mb->popup_mode = True;
								return;
							}
						}
					else
						{
							/*
							if(mb->kbd_focus_item)
								mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, HIGHLIGHT_OUTLINE_CLEAR); 		  
				
							if(active_item->type != ITEM_TYPE_PREVIOUS)	
		  					mbdesktop_view_item_highlight (mb, active_item, HIGHLIGHT_FILL);  		
		  				*/
		  				
		  				mb->kbd_focus_item = active_item;		
		  				
		  				mb->popup_mode = False;
		  				mb->current_popup_item = NULL;				
							return;  
						}
				
  		break;
    	}
  	}
  }
  else
  {
  	 int scalewide = (mb->workarea_width - 152) / 4;
  	 
				 	   mb->popup_mode = False;
  				   mb->current_popup_item = NULL;	
  				   
  				   if( e->x > mb->workarea_x && e->x < mb->workarea_x + 76
  				   	&& e->y > mb->workarea_height - 152 + mb->workarea_y && e->y < mb->workarea_height - 76 + mb->workarea_y )
  				   {
  				   		mb->wheel_mode = True;
  				   } else if( e->x > mb->workarea_x + 76 && e->x < mb->workarea_x + 152
  				   	       && e->y > mb->workarea_height - 76 + mb->workarea_y && e->y < mb->workarea_height + mb->workarea_y)
  				   {
  				   		mb->wheel_mode = True;
  				   } else if( e->x > mb->workarea_x + 152 && e->x <  mb->workarea_x + 152 + 72
  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
  				   {
  				   		mb->hisapp_mode = 1;
  				   } else if( e->x > mb->workarea_x + 152 + scalewide && e->x <  mb->workarea_x + 152  + scalewide + 72
  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
  				   {
  				   		mb->hisapp_mode = 2;
  				   } else if( e->x > mb->workarea_x + 152 + scalewide * 2 && e->x <  mb->workarea_x + 152  + scalewide * 2 + 72
  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
  				   {
  				   		mb->hisapp_mode = 3;
  				   } else if( e->x > mb->workarea_x + 152 + scalewide * 3 && e->x <  mb->workarea_x + 152  + scalewide * 3 + 72
  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
  				   {
  				   		mb->hisapp_mode = 4;
  				   }			      
			      
			       mbdesktop_view_paint(mb, False, False);  	 
  	 
  	 for (;;) 
	   {
	       if (XGrabPointer(mb->dpy, mb->win_top_level, False,
		       ButtonPressMask|ButtonReleaseMask|
		       PointerMotionMask|EnterWindowMask|LeaveWindowMask,
		       GrabModeAsync,
		       GrabModeAsync, None, None, CurrentTime)
					  == GrabSuccess)
				 {

			       XMaskEvent(mb->dpy, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, &ev);  	 
			      	
			       switch (ev.type)
					   {
								case MotionNotify:	      	
												 break;
								case ButtonRelease:
		  									 XUngrabPointer(mb->dpy, CurrentTime);
						  				   if( e->x > mb->workarea_x && e->x < mb->workarea_x + 76
						  				   	&& e->y > mb->workarea_height - 152 + mb->workarea_y && e->y < mb->workarea_height - 76 + mb->workarea_y )
						  				   {
						  				   		mbdesktop_scroll_up(mb);
						  				   		mb->popup_mode = False;
						  				   		mb->current_popup_item = NULL;	
						  				   		mb->wheel_mode = False;
						  				   		mbdesktop_view_paint(mb, False, False);
						  				   } else if( e->x > mb->workarea_x + 76 && e->x < mb->workarea_x + 152
						  				   	       && e->y > mb->workarea_height - 76 + mb->workarea_y && e->y < mb->workarea_height + mb->workarea_y)
						  				   {
						  				   		mbdesktop_scroll_down(mb);
						  				   		mb->popup_mode = False;
						  				   		mb->current_popup_item = NULL;	
						  				   		mb->wheel_mode = False;
						  				   		mbdesktop_view_paint(mb, False, False);
						  				   } else if( e->x > mb->workarea_x + 152 && e->x <  mb->workarea_x + 152 + 72
						  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
						  				   {
						  				   		mb->hisapp_mode = 0;
						  				   		mbdesktop_view_paint(mb, False, False);
						  				   		fprintf(stdout, "steven-desktop: History Application One!\n");
						  				   } else if( e->x > mb->workarea_x + 152 + scalewide && e->x <  mb->workarea_x + 152  + scalewide + 72
						  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
						  				   {
						  				   		mb->hisapp_mode = 0;
						  				   		mbdesktop_view_paint(mb, False, False);
						  				   		fprintf(stdout, "steven-desktop: History Application Two!\n");
						  				   } else if( e->x > mb->workarea_x + 152 + scalewide * 2 && e->x <  mb->workarea_x + 152  + scalewide * 2 + 72
						  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
						  				   {
						  				   		mb->hisapp_mode = 0;
						  				   		mbdesktop_view_paint(mb, False, False);
						  				   		fprintf(stdout, "steven-desktop: History Application Three!\n");
						  				   } else if( e->x > mb->workarea_x + 152 + scalewide * 3 && e->x <  mb->workarea_x + 152  + scalewide * 3 + 72
						  				   				 && e->y > mb->workarea_height - 108 + mb->workarea_y && e->y < mb->workarea_height - 108 + mb->workarea_y + 71 )
						  				   {
						  				   		mb->hisapp_mode = 0;
						  				   		mbdesktop_view_paint(mb, False, False);
						  				   		fprintf(stdout, "steven-desktop: History Application Four!\n");
						  				   }			  									
		  									 return;
		  			 }
				 }
			}
   }
}

static void
handle_key_event(MBDesktop *mb, XKeyEvent *e)
{
  MBDesktopItem *active_item = NULL, *tmp_item = NULL;
  KeySym         key;
  int            i = 0;
  Bool           not_scrolled = True;
  int max_items_horiz = mb->workarea_width / mb->item_width;

  /* No items - no keys */
  if (mb->current_head_item == mb->top_head_item)
    return;

  if (mb->had_kbd_input == False)
    {
      mb->had_kbd_input = True;
      mbdesktop_view_paint(mb, True, False);      
      return;
    }

  switch (key = XKeycodeToKeysym (mb->dpy, e->keycode, 0))
    {
    case XK_Left:
      if (mbdesktop_current_folder_view (mb) == VIEW_LIST)
	{
	  mbdesktop_item_folder_prev_activate_cb((void *)mb, 
						 (void *)mbdesktop_item_get_first_sibling(mb->kbd_focus_item));
	  return;
	}
      if (mb->kbd_focus_item->item_prev_sibling &&
      	mb->kbd_focus_item->item_prev_sibling->type != ITEM_TYPE_PREVIOUS)
	    {
	      active_item = mb->kbd_focus_item->item_prev_sibling;
	    } else return;
      break;
    case XK_Right:
      /* 
       * Follow folders for list views
       */
      if (mbdesktop_current_folder_view (mb) == VIEW_LIST)
	{
	  if (mb->kbd_focus_item->type == ITEM_TYPE_FOLDER)
	    mb->kbd_focus_item->activate_cb((void *)mb, 
					    (void *)mb->kbd_focus_item);
	  return;
	}

      if (mb->kbd_focus_item->item_next_sibling)
	{
	  active_item = mb->kbd_focus_item->item_next_sibling;
	} else return;
      break;
    case XK_Up:
      if (mbdesktop_current_folder_view ( mb ) == VIEW_LIST)
	{
	  if (mb->kbd_focus_item->item_prev_sibling)
	    {
	      active_item = mb->kbd_focus_item->item_prev_sibling;
	    } else return;
	} else {
	  active_item = mb->kbd_focus_item;
	  while (i++ < max_items_horiz)
	    if ((active_item = active_item->item_prev_sibling) == NULL)
	      return;
	}
      break;
    case XK_Down:	    
      if (mbdesktop_current_folder_view ( mb ) == VIEW_LIST)
	{
	  if (mb->kbd_focus_item->item_next_sibling)
	    {
	      active_item = mb->kbd_focus_item->item_next_sibling;
	    } 
	  else return;
	} else {
	  active_item = mb->kbd_focus_item;
	  while (i++ < max_items_horiz)
	  	if (active_item->item_next_sibling)
	  		active_item = active_item->item_next_sibling;
	  	else
	  		if( i > 0 )
	  			break;
	  		else
	  			return;
	  	/*	
	    if ((active_item = active_item->item_next_sibling) == NULL)
	      return;
	    */  
	}
      break;
    case XK_Return:
    case XK_KP_Enter:
      mb->kbd_focus_item->activate_cb((void *)mb, 
				      (void *)mb->kbd_focus_item);
      return;
    case XK_BackSpace:
      mbdesktop_item_folder_prev_activate_cb((void *)mb, 
					     (void *)mbdesktop_item_get_first_sibling(mb->kbd_focus_item));
	return;
    case XK_Prior:
    	
    	fprintf(stdout, "steven-desktop: XK_Prior\n");
/* Commit By Steven Chen    	
      if (mb->scroll_offset_item) 
	{
	  mb->last_visible_item = mb->scroll_offset_item;
	  mbdesktop_scroll_up(mb);
	}
*/	
	  break;
    case XK_Next:
      fprintf(stdout, "steven-desktop: XK_Next\n");
/* Commit By Steven Chen       
      if (mb->last_visible_item)
	{
	  active_item = mb->scroll_offset_item = mb->last_visible_item;
	  mbdesktop_scroll_down(mb);
	}
*/	
      break;
    case XK_Home:
      mb->kbd_focus_item = mb->current_head_item 
	= mb->scroll_offset_item = mb->top_head_item->item_child;
      mbdesktop_view_paint(mb, False, False);
      break;

    case XK_End:
      /* XXX TODO */
      break;


    default:
      if (key >= XK_a && key <= XK_z) /* XXX To complete */
	{
	  ; 			/* Seek to this item */
	}
      break;
    }
 
  if (active_item)
    {
      if ( key == XK_Down || key == XK_Right)
	{
	  /* do we need to scroll ? */
	  if (mb->last_visible_item)
	    {

	      /* Handle 'special' case of keyboard scroll to end of list */

	      if (active_item->item_next_sibling == NULL
		  && mbdesktop_current_folder_view ( mb ) == VIEW_LIST)
		{
		  /* clear existing highlight */
		  mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
						 HIGHLIGHT_OUTLINE_CLEAR); 

		  mb->scroll_offset_item 
		    = mb->scroll_offset_item->item_next_sibling;
		  
		  mb->kbd_focus_item = active_item;

		  mbdesktop_view_paint(mb, False, False);

		  return;
		}
	      else
		{
		  tmp_item = active_item;
		  while ((tmp_item = tmp_item->item_prev_sibling) != NULL)
		    if (tmp_item == mb->last_visible_item->item_prev_sibling )
		      {
			not_scrolled = False;
			mbdesktop_scroll_down(mb);
			break;
		      }
		}
	    }
	}
      else
	{
	  /* do we need to scroll ? */
	  if (mb->scroll_offset_item)
	    {
	      tmp_item = active_item;
	      while ((tmp_item = tmp_item->item_next_sibling) != NULL)
		if (tmp_item == mb->scroll_offset_item)
		  {
		    not_scrolled = False;
		    mbdesktop_scroll_up(mb);
		    break;
		  }
	    }
	}
      
      /* Clear the old dashed border */
      if (not_scrolled && mb->had_kbd_input)
	{
	  mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
					 HIGHLIGHT_OUTLINE_CLEAR); 
	  mb->kbd_focus_item = active_item;
	  mbdesktop_view_paint(mb, True, False);

	}
      else
	{
	  mb->kbd_focus_item = active_item;
	  mbdesktop_view_paint(mb, False, False);

	}
    }
}

int
mbdesktop_current_folder_view ( MBDesktop *mb )
{
  //return VIEW_LIST;
/*  
  if (mb->current_head_item->item_parent)
    return mb->current_head_item->item_parent->view; 
  else
    return mb->current_head_item->view; 
*/
	return  mb->view_type;   
}

void
mbdesktop_main(MBDesktop *mb)
{
  XEvent ev;
  Atom workarea_atom = XInternAtom(mb->dpy, "_NET_WORKAREA",False);
  MBDesktopModuleslist  *module_current = NULL;
	
#ifdef USE_DNOTIFY
  sigset_t block_sigset;
  sigemptyset(&block_sigset);
  sigaddset(&block_sigset, SIGRTMIN); /* XXX should also add stuff 
					 like a HUP etc .. */
#endif
/*
	   strcpy(data, "finger");
      mode_prop = XInternAtom(mb->dpy, "_MOKO_TYPE_NAME", False);
      XChangeProperty(mb->dpy, mb->win_top_level, mode_prop, XA_STRING, 8,
		      PropModeReplace, (unsigned char*)data, strlen(data));  

		 cmd_prop = XInternAtom(mb->dpy, "_MB_COMMAND", False);
		 
	   memset(&ev1, '\0', sizeof ev1);
	   ev1.xclient.type = ClientMessage;
	   ev1.xclient.window = mb->win_top_level; 
	   ev1.xclient.message_type = cmd_prop;
	   ev1.xclient.format = 8;
	
	   ev1.xclient.data.l[0] = 10;
	
	   XSendEvent(mb->dpy, mb->win_top_level, False, 
		      SubstructureRedirectMask|SubstructureNotifyMask, &ev1);
*/

	fork_exec("matchbox-remote -s finger");


  mbdesktop_view_paint(mb, True, False);
  XFlush(mb->dpy);

  while (1)
    {
	  if (WantReload) 	/* Triggered by dnotify signals etc */
	    {
	      mbdesktop_item_folder_contents_free(mb, mb->top_head_item);

	      mb->current_folder_item = mb->top_head_item;

	      modules_unload(mb);  /* XXX more eficient way ?  */

	      modules_init(mb);

	      mb->kbd_focus_item = mb->current_head_item 
		= mb->scroll_offset_item = mb->top_head_item->item_child;

	      mbdesktop_view_paint(mb, False, False);

	      WantReload = False;

	      XSync(mb->dpy, False);
	    }

	  XNextEvent(mb->dpy, &ev);

#ifdef USE_DNOTIFY 		/* Block dnotify signal */
	  sigprocmask(SIG_BLOCK, &block_sigset, NULL); 
#endif

#ifdef USE_XSETTINGS
	  if (mb->xsettings_client != NULL)
	    xsettings_client_process_event(mb->xsettings_client, &ev);
#endif

		mb_menu_handle_xevent(mb->mbmenu, &ev);

	  switch (ev.type) 
	    {
	    case MappingNotify:
	    	
	      XRefreshKeyboardMapping(&ev.xmapping);
	      break;

	    case FocusIn:
	    	fprintf(stdout, "steven-desktop: sev.type = [FocusIn]\n");
	      mb->have_focus = True;
	      mbdesktop_view_paint(mb, True, False);
	      break;

	    case FocusOut:
	    	fprintf(stdout, "steven-desktop: sev.type = [FocusOut]\n");
	      mbdesktop_view_paint(mb, True, False);
	      mb->have_focus = False;
	      break;

	    case Expose:
	    	
	      if (ev.xexpose.count > 0) 
		mbdesktop_view_paint(mb, True, False);
	      break;
	    case PropertyNotify:
	    	
	      if (ev.xproperty.atom == workarea_atom)
		{
		  int wx, wy, ww, wh;
		  if (mbdesktop_get_workarea(mb, &wx, &wy, &ww, &wh))
		    {
		      if (mb->workarea_x != wx 
			  || mb->workarea_y != wy
			  || mb->workarea_width != ww 
			  || mb->workarea_height != wh)
			mbdesktop_view_configure(mb);
		    }
		}
	      else if (ev.xproperty.atom == mb->atom_mb_theme)
		{
		  if (mbdesktop_get_theme_via_root_prop(mb))
		    mbdesktop_switch_theme (mb, NULL );
		}
	      break;
	      /*
	    case ConfigureRequest:
	      mbdesktop_win_plugin_configure_request(mb, 
						     &ev.xconfigurerequest);
	      break;
	      */
	    case ConfigureNotify:
	      fprintf(stdout, "steven-desktop: sev.type = [ConfigureNotify]\n");
	      if ( ev.xconfigure.width != mb->desktop_width
		   || ev.xconfigure.height != mb->desktop_height)
		{
		  mb->desktop_width = ev.xconfigure.width;
		  mb->desktop_height = ev.xconfigure.height;
		  mbdesktop_view_configure(mb);
		}
	      break;
	    case ButtonPress:
	    	fprintf(stdout, "steven-desktop: sev.type = [ButtonPress]\n");
	    	handle_button_event(mb, &ev.xbutton);
	      break;
	    case KeyPress:
	    	fprintf(stdout, "steven-desktop: sev.type = [KeyPress] - Don;t deal!!!\n");
//	    	if(!mb->popup_mode)
//	      	handle_key_event(mb, &ev.xkey);
	      break;
	    }

	  module_current = mb->modules;
	  while (module_current != NULL)
	    {
	      if (module_current->module_handle->mod_xevent)
		module_current->module_handle->mod_xevent(mb, module_current->module_handle, &ev);
	      module_current = module_current->next;
	    }
	  
	  /*
	  if (mb->current_folder_item 
	      && mb->current_folder_item->module_handle
	      && mb->current_folder_item->module_handle->folder_xevent)
	    {
	      mb->current_folder_item->module_handle->folder_xevent(mb, mb->current_folder_item, &ev);
	    }
	  */
#ifdef USE_DNOTIFY 		/* Unblock dnotify signal */
	  sigprocmask(SIG_UNBLOCK, &block_sigset, NULL); 
#endif

    }

}

static char **
get_module_list(MBDesktop *mb, int *module_cnt)
{
  char **result = NULL;
  int n_modules = 0;
  FILE *fp;
  char data[1024];
  char modules_config_path[512];
  struct stat stat_info;

  int pass = 1, i = 0;

  snprintf(modules_config_path, 512, 
	   "%s/.matchbox/mbdesktop_modules", mb_util_get_homedir());

  if (stat(modules_config_path, &stat_info))
    {
      snprintf(modules_config_path, 512, MBCONFDIR "/mbdesktop_modules");
    }
  
  if (!(fp = fopen(modules_config_path, "r"))) return NULL;

  while (pass != 3)
    {
      while (fgets(data,1024,fp) != NULL)
	{
	  int j = 0;
	  while (isspace(data[j])) j++;
	  
	  if (data[j] == '#' || data[j] == '\n' || data[j] == '\0')
	    continue;
	  
	  if (pass == 2)
	    {
	      if (data[strlen(data)-1] == '\n')
		data[strlen(data)-1] = '\0';
	      result[i] = strdup(&data[j]);
	      i++;
	    } 
	  else n_modules++;
	}

      if (pass == 1)
	{
	  result = (char **)malloc(sizeof(char *)*n_modules);
	  rewind(fp);
	}

      pass++;
    } 

  fclose(fp);

  /*
  result[2] = strdup(PKGDATADIR "/desktop/modules/simplefilebrowser.so");
  result[1] = strdup(PKGDATADIR "/desktop/modules/tasks.so");
  result[0] = strdup(PKGDATADIR "/desktop/modules/dotdesktop.so");
  */

  *module_cnt = n_modules;
  return result;
}


static void
modules_init (MBDesktop *mb)
{
  MBDesktopFolderModule *mod_details = NULL;
  void                  *handle = NULL;
  char                  *error;
  char                 **mods   = NULL;
  int                    n_mods = 0;
  int                    i, successes = 0;
  MBDesktopModuleslist  *module_current = NULL;

  mods = get_module_list(mb, &n_mods);

  if (mods == NULL)
    {
      fprintf(stderr, "matchbox-desktop: failed to load modules. Exiting ...\n");
      exit(1);
    }

  for (i = 0; i < n_mods; i++ )
    { 
      char *args = NULL;

      args = strchr(mods[i], ' ');

      if (args != NULL)
	{
	  *args = '\0'; args++;
	}

      printf("matchbox-desktop: loading %s with args %s\n", mods[i], 
	     args ? args : "( None )");

      if (i == 0 || (i > 0 && strcmp(mods[i], mods[i-1])))
	{
	  handle = dlopen (mods[i], RTLD_LAZY |RTLD_GLOBAL);

	  if (!handle) 
	    {
	      fputs (dlerror(), stderr);
	      continue;
	    }
	  
	  mod_details = dlsym(handle, "folder_module" );

	  if ((error = dlerror()) != NULL) 
	    {
	      fputs(error, stderr);
	      free(error);
	      continue;
	    }
	}

      if (mod_details->mod_init(mb, mod_details, args) != -1)
	{
	  /* add to out list of modules */
	  if (mb->modules == NULL)
	    {
	      mb->modules = malloc(sizeof(MBDesktopModuleslist));
	      memset(mb->modules, 0, sizeof(MBDesktopModuleslist));
	      
	      mb->modules->module_handle = mod_details;
	      mb->modules->dl_handle  = handle;
	      mb->modules->next          = NULL;
	      
	      module_current = mb->modules;
	    }
	  else
	    {
	      module_current->next = malloc(sizeof(MBDesktopModuleslist));
	      memset(module_current->next, 0, sizeof(MBDesktopModuleslist));
	      
	      module_current = module_current->next;
	      
	      module_current->module_handle = mod_details;
	      module_current->dl_handle     = handle;
	      module_current->next          = NULL;
	    }
	  successes++;
	}
      else
	{
	  fprintf(stderr,"matchbox-desktop: failed to initialise module %s\n",
		  mods[i]);
	  dlclose(handle);
	}
    }

  if (successes == 0)
    {
      fprintf(stderr, "matchbox-desktop: failed to load any item modules.\n");
    }
}

int 
main(int argc, char **argv)
{
  MBDesktop *mb;

  mb = mbdesktop_init(argc, argv);

  mbdesktop_view_init_bg(mb);

	/* Add the root topelevel item */
  mb->top_head_item  = mbdesktop_item_new_with_params (mb, 
						       mb->top_level_name, 
						       NULL,
						       NULL,
						       ITEM_TYPE_ROOT );
  mb->current_folder_item = mb->top_head_item;

	
  modules_init(mb);

	/* Do we have item modules loaded ? */

  if (mb->top_head_item->item_child)
    mb->kbd_focus_item = mb->current_head_item 
      = mb->scroll_offset_item = mb->top_head_item->item_child;
  else
    mb->kbd_focus_item = mb->current_head_item 
      = mb->scroll_offset_item = mb->top_head_item;

	mbdesktop_calculate_item_dimentions(mb);

	/*
  mbdesktop_win_plugin_init (mb);

  if (mbdesktop_win_plugin_load (mb, "gpe-today"))
    mbdesktop_win_plugin_reparent (mb);
  */

  mbdesktop_main(mb);

  return 0;
}

/* calls for modules ? */

Display 
*mbdesktop_xdisplay (MBDesktop *mb) 
{
  return mb->dpy; 
}

Window
mbdesktop_xrootwin (MBDesktop *mb) 
{
  return mb->root;
}

MBPixbuf*
mbdesktop_mbpixbuf (MBDesktop *mb) 
{
  return mb->pixbuf;
}

int
mbdesktop_xscreen (MBDesktop *mb) 
{
  return mb->scr;
}

int
mbdesktop_icon_size (MBDesktop *mb) 
{
  return mb->icon_size;
}

void
mbdesktop_progress_dialog_init (MBDesktop   *mb)
{
#define DIALOG_TEXT "Generating thumbnails ..."

  mb->win_dialog_w = (mb->desktop_width/2)  + (2 * DIALOG_BORDER);
  mb->win_dialog_h = (mb->desktop_height/6) + (2 * DIALOG_BORDER);

  mb->win_dialog_backing = XCreatePixmap(mbdesktop_xdisplay(mb), 
					 mbdesktop_xrootwin(mb),
					 mb->win_dialog_w,
					 mb->win_dialog_h,
					 mb->pixbuf->depth ); 
}

void
mbdesktop_progress_dialog_set_percentage (MBDesktop   *mb, 
					  int          percentage)
{
  int barwidth = ((mb->desktop_width/2) * percentage) / 100 ;

  XSetForeground(mbdesktop_xdisplay(mb), mb->gc, 
		 WhitePixel(mbdesktop_xdisplay(mb), 
			    mbdesktop_xscreen(mb)));

  /* Clear Background */
  XFillRectangle(mbdesktop_xdisplay(mb), 
		 mb->win_dialog_backing, 
		 mb->gc, 
		 0, 0, 
		 mb->win_dialog_w, mb->win_dialog_h);

  XSetForeground(mbdesktop_xdisplay(mb), mb->gc, 
		 mb_col_xpixel(mb->hl_col));


  XFillRectangle(mbdesktop_xdisplay(mb), 
		 mb->win_dialog_backing, 
		 mb->gc, 
		 DIALOG_BORDER, DIALOG_BORDER, 
		 barwidth, (mb->desktop_height/4));

  XSetWindowBackgroundPixmap(mbdesktop_xdisplay(mb), mb->win_dialog, 
			     mb->win_dialog_backing);

  XClearWindow(mbdesktop_xdisplay(mb), mb->win_dialog);

  XFlush(mbdesktop_xdisplay(mb));
}


void
mbdesktop_progress_dialog_open (MBDesktop   *mb)
{
  unsigned char *win_title = "Loading ...";

  mb->win_dialog
    = XCreateSimpleWindow(mbdesktop_xdisplay(mb),
			  mbdesktop_xrootwin(mb),
			  0, 0, mb->win_dialog_w, mb->win_dialog_h, 0,
			  BlackPixel(mbdesktop_xdisplay(mb), 
				     mbdesktop_xscreen(mb)),
			  WhitePixel(mbdesktop_xdisplay(mb), 
				     mbdesktop_xscreen(mb)));

  XChangeProperty(mbdesktop_xdisplay(mb), mb->win_dialog, 
		  mb->window_type_atom, XA_ATOM, 32, 
		  PropModeReplace, 
		  (unsigned char *) &mb->window_type_dialog_atom, 1);

  XChangeProperty(mbdesktop_xdisplay(mb), mb->win_dialog, 
		  mb->window_state_atom, XA_ATOM, 32, 
		  PropModeReplace, 
		  (unsigned char *) &mb->window_state_modal_atom, 1);

  XChangeProperty(mbdesktop_xdisplay(mb), mb->win_dialog, 
		  mb->window_utf8_name_atom, 
		  mb->utf8_atom, 8, PropModeReplace, 
		  win_title, strlen(win_title));

  XStoreName(mbdesktop_xdisplay(mb), mb->win_dialog, win_title);

  XMapWindow(mbdesktop_xdisplay(mb), mb->win_dialog);

  XFlush(mbdesktop_xdisplay(mb));

}


void
mbdesktop_progress_dialog_close (MBDesktop   *mb)
{
  XDestroyWindow(mbdesktop_xdisplay(mb), mb->win_dialog);
}

