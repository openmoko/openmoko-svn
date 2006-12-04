#include "mbdesktop_view.h"

static void
mbdesktop_view_paint_items(MBDesktop *mb, MBPixbufImage *img_dest, Bool popup_mode);

static void
_set_win_title(MBDesktop *mb, unsigned char *title)
{
  XChangeProperty(mbdesktop_xdisplay(mb), mb->win_top_level, 
		  mb->window_utf8_name_atom, 
		  mb->utf8_atom, 8, PropModeReplace, 
		  title, strlen(title));

  XStoreName(mb->dpy, mb->win_top_level, title );
}

void
mbdesktop_view_init_bg(MBDesktop *mb)
{
  MBPixbufImage *img_tmp;
  int dw, dh, dx, dy, r, g, b;

  mb->font_col_type = DKTP_FONT_COL_UNKOWN;

  if (mb->bg_img != NULL)
    mb_pixbuf_img_free(mb->pixbuf, mb->bg_img);

fprintf(stderr, "==== bg_type = [%d] bg_name = [%s]\n", mb->bg->type, mb->bg->data.filename);

  switch (mb->bg->type)
    {
    case BG_SOLID:
      mb->bg_img = mb_pixbuf_img_rgba_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);
      mb_pixbuf_img_fill(mb->pixbuf, mb->bg_img, 
			 mb->bg->data.cols[0], 
			 mb->bg->data.cols[1],
			 mb->bg->data.cols[2], 
			 0);

      mb->use_text_outline = False;

      if (!mb->user_overide_font_col)
	{
	  if ((((mb->bg->data.cols[0] * 54) + (mb->bg->data.cols[1] * 183) + (mb->bg->data.cols[2] * 19)) / 256) > 127 )
	    {
	      mbdesktop_set_font_color(mb, "black");
	      mb->font_col_type = DKTP_FONT_COL_BLACK;
	    } else {
	      mbdesktop_set_font_color(mb, "white");
	      mb->font_col_type = DKTP_FONT_COL_WHITE;
	    }
	}

      break;
    case BG_TILED_PXM:
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);

      if ((img_tmp = mb_pixbuf_img_new_from_file(mb->pixbuf, 
						 mb->bg->data.filename)) 
	  == NULL)
	{
	  fprintf(stderr,"Failed to load background : %s", 
		  mb->bg->data.filename);
	  mbdesktop_bg_parse_spec(mb, "col-solid:red");
	  mbdesktop_view_init_bg(mb);
	  return;
	}

      for (dy=0; dy < mb->desktop_height; dy += img_tmp->height)
	for (dx=0; dx < mb->desktop_width; dx += img_tmp->width)
	  {
	    if ( (dx + img_tmp->width) > mb->desktop_width )
	      dw = img_tmp->width - ((dx + img_tmp->width)-mb->desktop_width);
	    else
	      dw = img_tmp->width;

	    if ( (dy + img_tmp->height) > mb->desktop_height )
	      dh = img_tmp->height-((dy + img_tmp->height)-mb->desktop_height);
	    else
	      dh = img_tmp->height;
	    mb_pixbuf_img_copy(mb->pixbuf, mb->bg_img, img_tmp,
			       0, 0, dw, dh, dx, dy);
	  }
      mb_pixbuf_img_free(mb->pixbuf, img_tmp);

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_STRETCHED_PXM:
      if ((img_tmp = mb_pixbuf_img_new_from_file(mb->pixbuf, 
						 mb->bg->data.filename)) 
	  == NULL)
	{
	  fprintf(stderr,"Failed to load background : %s", mb->bg->data.filename);
	  mbdesktop_bg_parse_spec(mb, "col-solid:red");
	  mbdesktop_view_init_bg(mb);
	  return;
	}
      mb->bg_img = mb_pixbuf_img_scale(mb->pixbuf, img_tmp, mb->desktop_width, 
				       mb->desktop_height);
      mb_pixbuf_img_free(mb->pixbuf, img_tmp);

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_CENTERED_PXM:
      if ((img_tmp = mb_pixbuf_img_new_from_file(mb->pixbuf, 
						 mb->bg->data.filename)) 
	  == NULL)
	{
	  fprintf(stderr,"Failed to load background : %s", mb->bg->data.filename);
	  mbdesktop_bg_parse_spec(mb, "col-solid:red");
	  mbdesktop_view_init_bg(mb);
	  return;
	}
      dx = (mb->desktop_width - img_tmp->width) / 2;
      if (dx < 0) dx = 0;
      dy = (mb->desktop_height - img_tmp->height) / 2;
      if (dy < 0) dy = 0;
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
				     mb->desktop_height);
      mb_pixbuf_img_copy(mb->pixbuf, mb->bg_img, img_tmp,
			 0, 0,
			 (img_tmp->width > mb->desktop_width) ?
			 mb->desktop_width : img_tmp->width ,
			 (img_tmp->height > mb->desktop_height) ?
			 mb->desktop_height : img_tmp->height ,
			 dx, dy);
      mb_pixbuf_img_free(mb->pixbuf, img_tmp);

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_GRADIENT_HORIZ:
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);
      dw = mb->desktop_width;
      dh = mb->desktop_height;
      
      for(dx=0; dx<dw; dx++)
	{
	  r = mb->bg->data.gcols[0] + (( dx * (mb->bg->data.gcols[1] - mb->bg->data.gcols[0])) / dw); 
	  g = mb->bg->data.gcols[2] + (( dx * (mb->bg->data.gcols[3] - mb->bg->data.gcols[2])) / dw); 
	  b = mb->bg->data.gcols[4] + (( dx * (mb->bg->data.gcols[5] - mb->bg->data.gcols[4])) / dw); 

	  for(dy=0; dy<dh; dy++)
	    {
	      mb_pixbuf_img_plot_pixel (mb->pixbuf, mb->bg_img, 
					dx, dy, r ,g, b);
	    }
	}

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_GRADIENT_VERT:
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);

      dw = mb->desktop_width;
      dh = mb->desktop_height;
      
      for(dy=0; dy<dh; dy++)
	{
	  r = mb->bg->data.gcols[0] + (( dy * (mb->bg->data.gcols[1] - mb->bg->data.gcols[0])) / dh); 
	  g = mb->bg->data.gcols[2] + (( dy * (mb->bg->data.gcols[3] - mb->bg->data.gcols[2])) / dh); 
	  b = mb->bg->data.gcols[4] + (( dy * (mb->bg->data.gcols[5] - mb->bg->data.gcols[4])) / dh); 

	  for(dx=0; dx<dw; dx++)
	    {
	      mb_pixbuf_img_plot_pixel (mb->pixbuf, mb->bg_img, 
					dx, dy, r ,g, b);
	    }
	}

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    }

  /* Now set the background */
  mbdesktop_view_set_root_pixmap(mb, mb->bg_img);

  /* and figure out what font is best */

}

void
mbdesktop_view_set_root_pixmap(MBDesktop *mb, MBPixbufImage *img)
{
  Atom atom_root_pixmap_id = XInternAtom(mb->dpy, "_XROOTPMAP_ID", False); 
  
  if (mb->root_pxm != None) XFreePixmap(mb->dpy, mb->root_pxm);
  
  mb->root_pxm = XCreatePixmap(mb->dpy, mb->root, 
			       img->width, img->height,
			       mb->pixbuf->depth ); 

  mb_pixbuf_img_render_to_drawable(mb->pixbuf, img, (Drawable)mb->root_pxm,
				   0, 0);

  XChangeProperty(mb->dpy, mb->root, atom_root_pixmap_id, XA_PIXMAP, 
		  32, PropModeReplace, (unsigned char *) &mb->root_pxm, 1);
}


void  /* called only when desktop/workarea changes size or bgimage changes */
mbdesktop_view_configure(MBDesktop *mb)
{
  /* Assume mb->desktop_width, etc is up to date */

  int x,y,w,h;

  /* This is a bit crap - probably already called */
  if (mbdesktop_get_workarea(mb, &x, &y, &w, &h))
    {
      mb->workarea_x = x; 
      mb->workarea_y = y ;
      mb->workarea_width = w;
      mb->workarea_height = h;
    }

  if (mb->workarea_width > mb->desktop_width
      || mb->workarea_height > mb->desktop_height)
    return; 			/* Abort - probably in mid rotation */

  mbdesktop_view_init_bg(mb); 	   /* reset the background */
  mbdesktop_view_paint(mb, False, False); /* repaint */

}

void
mbdesktop_view_advance(MBDesktop *mb, int n_items)
{
  /* 
     XXX NOT USED YET - JUST EXPERIMENTAL XXXX

  */

  int n = 0;

  /* 
     rendering starts from    :: mb->scroll_offset_item
     highlighted item is      :: mb->kbd_focus_item
     last visble item in view :: mb->last_visible_item
  */

  if (n_items > ( mb->current_view_columns * mb->current_view_rows ))
    {
      /* Means we expecting to scroll the view */

      /* if keyboard focus and advanced key focus position is 
       * pass last viewable item then we also need to scroll.
       */
    }

  while (mb->kbd_focus_item->item_next_sibling && n <= n_items)
    {
      if (mb->kbd_focus_item == mb->last_visible_item
	  && mb->kbd_focus_item->item_next_sibling != NULL)
	{
	  /* we will need to scroll  */
	}

      mb->kbd_focus_item = mb->kbd_focus_item->item_next_sibling;

      n++;
    }

  /* 
     if (we_need_to_scroll)
     {
       rows_to_scroll = 

     }
  */

}

void
mbdesktop_view_retreat(MBDesktop *mb, int n_items)
{

}


void  /* TODO: implement multiple views */
mbdesktop_set_view(MBDesktop *mb, int view)
{
  ;


}

static void 
mbdesktop_view_header_paint(MBDesktop *mb, unsigned char* folder_title, unsigned char* folder_title_path)
{
  int opts              = MB_FONT_RENDER_OPTS_CLIP_TRAIL | MB_FONT_RENDER_ALIGN_RIGHT;
  int opts2              = MB_FONT_RENDER_OPTS_CLIP_TRAIL | MB_FONT_RENDER_ALIGN_CENTER;
  int extra_render_opts = 0;

  if (!mb->use_title_header)
    {
      _set_win_title(mb, folder_title);
      return;
    }
/*
  XSetLineAttributes(mb->dpy, mb->gc, 1, LineSolid, CapRound, JoinRound);
*/
  if (mb->use_text_outline)
    extra_render_opts = MB_FONT_RENDER_EFFECT_SHADOW;

	mb_font_render_simple (mb->titlefontpath,
			 mb->backing_cache,
			 mb->workarea_x + ((48-32)/2) + 1,
			 mb->workarea_y + 10,
			 (mb->workarea_width - (48-32)),
			 (unsigned char *)folder_title_path,
			 MB_ENCODING_UTF8,
			 opts|extra_render_opts);
			 
	mb_font_render_simple (mb->titlefont,
			 mb->backing_cache,
			 mb->workarea_x + ((48-32)/2) + 1,
			 mb->workarea_y + 1,
			 (mb->workarea_width - (48-32)),
			 (unsigned char *)folder_title,
			 MB_ENCODING_UTF8,
			 opts2|extra_render_opts);			 
			 
/*
   XDrawLine(mb->dpy, mb_drawable_pixmap(mb->backing_cache), 
	     mb->gc, 
	     mb->workarea_x + ((48-32)/2),
	     mb->workarea_y + mb_font_get_height(mb->titlefont) + 2,
	     mb->workarea_x + mb->workarea_width - ((48-32)/2),
	     mb->workarea_y + mb_font_get_height(mb->titlefont) + 2);

  XSetLineAttributes(mb->dpy, mb->gc, 1, LineOnOffDash, CapRound, JoinRound);
*/
}


void 
mbdesktop_view_paint(MBDesktop *mb, Bool use_cache, Bool popup_mode)
{
  MBPixbufImage *img_dest;
  MBDesktopItem *item_tmp = NULL;
  MBDesktopItem *item_tmp2 = NULL;
  
  char *folder_title = "Home";
	char *folder_title_path = strdup("Main Menu");
	
	
  if (use_cache && mb->backing_cache != NULL)
    {
      if (mb->had_kbd_input && mb->kbd_focus_item)
			{
	  		if (mb->have_focus)
	   			 mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
					   HIGHLIGHT_OUTLINE); 
			}
      return;
    }

  if (mb->backing_cache != NULL)
    mb_drawable_unref(mb->backing_cache);

  mb->backing_cache = mb_drawable_new(mb->pixbuf, 
				      mb->desktop_width, mb->desktop_height); 

  img_dest = mb_pixbuf_img_clone(mb->pixbuf, mb->bg_img);

  mbdesktop_calculate_item_dimentions(mb);

	// 如果没有选中，重新定位需要高亮显示的项
	if(mb->kbd_focus_item == NULL)
	{
  	if(mb->scroll_offset_item->type == ITEM_TYPE_PREVIOUS)
  		if(mb->scroll_offset_item->item_next_sibling)
  			mb->kbd_focus_item = mb->scroll_offset_item->item_next_sibling;
  		else
  			mb->kbd_focus_item = NULL;
  	else
  		mb->kbd_focus_item = mb->scroll_offset_item;
  }


  /* no items to paint - current item is very top - no items loaded */
  if (mb->current_head_item == mb->top_head_item)
    {
      mb_pixbuf_img_render_to_drawable(mb->pixbuf, img_dest, 
				       mb_drawable_pixmap(mb->backing_cache), 
				       0, 0);
    }
  else
    {

	   if ( mbdesktop_current_folder_view (mb) == VIEW_ICONS 
	   		|| mbdesktop_current_folder_view (mb) == VIEW_ICONS_ONLY)
				mbdesktop_view_paint_items(mb, img_dest, popup_mode);
     else
				mbdesktop_view_paint_list(mb, img_dest);
    }
  
  
  // 以下代码为显示当前选中项的路径，但是目前不使用了
/*  
  free(mb->top_head_item->name);
  mb->top_head_item->name   = strdup("Main Menu");
  if (mb->current_head_item->item_parent)
  {
  	folder_title 
      = (mb->current_head_item->item_parent->name_extended) ? strdup(mb->current_head_item->item_parent->name_extended) : strdup(mb->current_head_item->item_parent->name);
    
    char tmppath[512];
    char *tmppartpath;
    
    tmppartpath 
      = (mb->top_head_item->name_extended) ? strdup(mb->top_head_item->name_extended) : strdup(mb->top_head_item->name);
    memset( tmppath, 0, 512);
    	
    strcpy( tmppath, tmppartpath);
    strcat( tmppath, "/" );
    	
    item_tmp = mb->current_head_item->item_parent;
    do
    {
    	if( item_tmp->type == ITEM_TYPE_ROOT )
    		break;
    		
    	tmppartpath = (item_tmp->name_extended) ? strdup(item_tmp->name_extended) : strdup(item_tmp->name);
			strcat(tmppath, tmppartpath);
    	
    }while((item_tmp = item_tmp->item_parent) != NULL);
    
    if(popup_mode)
    	{
    		tmppartpath = (mb->kbd_focus_item->name_extended) ? strdup(mb->kbd_focus_item->name_extended) : strdup(mb->kbd_focus_item->name);
    		strcat( tmppath, "/" );
    		strcat(tmppath, tmppartpath);
    	}
    
    folder_title_path = strdup(tmppath);
    
    if(tmppartpath)
    	free(tmppartpath);
  }
  else 
  {
    folder_title 
      = (mb->top_head_item->name_extended) ? strdup(mb->top_head_item->name_extended) : strdup(mb->top_head_item->name);
  }
*/

	// 计算当前组
	free(mb->top_head_item->name);
  mb->top_head_item->name   = strdup("Main Menu");
  if (mb->current_head_item->item_parent)
  {
  	folder_title 
      = (mb->current_head_item->item_parent->name_extended) ? strdup(mb->current_head_item->item_parent->name_extended) : strdup(mb->current_head_item->item_parent->name);
  }
  else 
  {
    folder_title 
      = (mb->top_head_item->name_extended) ? strdup(mb->top_head_item->name_extended) : strdup(mb->top_head_item->name);
  }

	// 计算当前组总数和当前项个数 9/25
	// fprintf(stdout, "=====: scroll_item = [%d] [%s]\n", mb->scroll_offset_item->type, mb->scroll_offset_item->name);
  if(mb->scroll_offset_item->type == ITEM_TYPE_PREVIOUS)
  	if(mb->scroll_offset_item->item_next_sibling)
  		item_tmp2 = mb->scroll_offset_item->item_next_sibling;
  	else
  		item_tmp2 = NULL;
  else
  	item_tmp2 = mb->scroll_offset_item;
  	
  // fprintf(stdout, "=====: item_tmp2 = [%d] [%s]\n", item_tmp2->type, item_tmp2->name);
  	
  free(mb->top_head_item->name);
  mb->top_head_item->name   = strdup("Main Menu");
  if (mb->current_head_item->item_parent)
  {
  	// 表示是一个应用程序组，不是第一层，比如office, game之类
  	int totalcount = 0;
  	int currcount = 0;
  	for(item_tmp = mb->current_head_item; 
      item_tmp != NULL; 
      item_tmp = item_tmp->item_next_sibling)
    { 
    	if( item_tmp->type == ITEM_TYPE_DOTDESKTOP_FOLDER ||
    		  item_tmp->type == ITEM_TYPE_DOTDESKTOP_ITEM ||
    		  item_tmp->type == ITEM_TYPE_FOLDER )
    	{
    		// fprintf(stdout, "=====: item_tmp = [%d] [%s]\n", item_tmp->type, item_tmp->name);
    		totalcount++;
    		if(item_tmp2)
    			currcount++;
    	}
    	
    	if(item_tmp2)
    		if(!strcmp(item_tmp->name, item_tmp2->name) && 
    			 (item_tmp->type == item_tmp2->type) )
    			 item_tmp2 = NULL;
    }
    
    int i3 = currcount / 3;
    
    if( (totalcount - i3 * 3) < 9 )
    	currcount = totalcount;
    else
    	currcount = i3 * 3 + 9;
    	
		sprintf(folder_title_path, "%d / %d", currcount, totalcount);
  } 
  else 
  {
  	// 表示是顶层，没有元素
  	folder_title = strdup("0 / 0");	
  }

	// fprintf(stdout, "steven-desktop: folder_title_path = [%s]\n", folder_title_path);
  mbdesktop_view_header_paint(mb, folder_title, folder_title_path);
	
  XSetWindowBackgroundPixmap(mb->dpy, mb->win_top_level, 
			     mb_drawable_pixmap(mb->backing_cache));
  XClearWindow(mb->dpy, mb->win_top_level);

// fprintf(stdout, "steven-desktop: had_kbd_input = [%d] have_focus = [%d]\n", mb->had_kbd_input, mb->have_focus);
/*  if (mb->kbd_focus_item && mb->had_kbd_input)*/
  if (mb->kbd_focus_item || mb->had_kbd_input)
    {
    	
      if (mb->have_focus)
					mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
				       HIGHLIGHT_OUTLINE); 
    }

  if (img_dest)
    mb_pixbuf_img_free(mb->pixbuf, img_dest);  

  if (folder_title)
    free(folder_title);
    
  if (folder_title_path)
    free(folder_title_path);    
}

void
mbdesktop_view_paint_list(MBDesktop *mb, MBPixbufImage *dest_img)
{
  MBDesktopItem *item;
  MBPixbufImage *icon_img_small;
  MBLayout *layout = NULL;

  int cur_y, cur_x, limit_y;


  if (mb->scroll_offset_item  == mb->current_head_item)
    mb->scroll_active = False;
  else
    mb->scroll_active = True;

  cur_x = mb->workarea_x + ((48-32)/2);
  cur_y = mb->workarea_y + mb->title_offset; /* + mb->win_plugin_rect.height; */
  limit_y = mb->workarea_y + mb->workarea_height - mb->title_offset; /*  - mb->win_plugin_rect.height; */

  mb->current_view_columns = 1;
  mb->current_view_rows 
    = ( mb->workarea_height - mb->title_offset /* - mb->win_plugin_rect.height */) / mb->icon_size ;


  for(item = mb->scroll_offset_item; 
      item != NULL; 
      item = item->item_next_sibling)
    {
      if (item->icon)
	{
	  if ( (cur_y + mb->item_height ) > limit_y) /* Off display ? */
	    {
	      mb->scroll_active = True;
	      break;
	    }
	  
	  if (mbdesktop_current_folder_view ( mb ) != VIEW_TEXT_ONLY)
	    {
	      
	      /* Arg, we shouldn't scale *every* render ! */
	      icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, 
						   item->icon,
						   mb->icon_size, 
						   mb->icon_size);
	      
	      mb_pixbuf_img_composite(mb->pixbuf, dest_img, 
				      icon_img_small, 
				      cur_x, 
				      cur_y );
	      
	      mb_pixbuf_img_free(mb->pixbuf, icon_img_small);
	    }
	  
	  item->x = cur_x;
	  item->y = cur_y;

	  item->width  = mb->workarea_width - ((48-32));
	  item->height = mb->item_height;

	  cur_y += mb->item_height;
	}
    }
  
/* 没有测试,暂时注释
  if (mb->scroll_active)
    {
      mb_pixbuf_img_composite(mb->pixbuf, dest_img, mb->img_scroll_up,
			      mb->workarea_x + mb->workarea_width-24, 
			      mb->workarea_y + 2);
      
      mb_pixbuf_img_composite(mb->pixbuf, dest_img,
			      mb->img_scroll_down, 
			      mb->workarea_x + mb->workarea_width-40, 
			      mb->workarea_y + 2);
    }
*/

  mb_pixbuf_img_render_to_drawable(mb->pixbuf, dest_img, 
				   mb_drawable_pixmap(mb->backing_cache), 
				   0, 0);

  mb->last_visible_item = item;

  layout = mb_layout_new();

  mb_layout_set_font(layout, mb->font);


  for(item = mb->scroll_offset_item; 
      item != NULL; 
      item = item->item_next_sibling)
    {
      int offset_y  = 0;
      int offset_x  = 0;
      int extra_render_opt = 0;

      if (mbdesktop_current_folder_view ( mb ) != VIEW_TEXT_ONLY)
	{
	  offset_x = mb->icon_size + mb->icon_size/4;
	  offset_y = 0;
	}
      else
	offset_x = 0;

      if (offset_y < 0) offset_y = 0;
      
      if (item == mb->last_visible_item)
	break;
      
      if (mb->use_text_outline)
	extra_render_opt = MB_FONT_RENDER_EFFECT_SHADOW;

      mb_layout_set_geometry(layout, 
			     item->width - mb->icon_size - mb->icon_size/8, 
			     item->height);

      mb_layout_set_text(layout, item->name, MB_ENCODING_UTF8);

      mb_layout_render(layout, mb->backing_cache, 
		       item->x + offset_x,
		       item->y + offset_y,
		       extra_render_opt|MB_FONT_RENDER_OPTS_CLIP_TRAIL|MB_FONT_RENDER_VALIGN_MIDDLE);

    }

  mb_layout_unref(layout);

}


static void
mbdesktop_view_paint_items(MBDesktop *mb, MBPixbufImage *img_dest, Bool popup_mode)
{
  MBDesktopItem *item;
  MBDesktopItem *topfolderitem;
  
  MBPixbufImage *icon_img_small;

  int cur_x = 0, cur_y = 0, limit_x, limit_y, cur_row = 1;
  int item_horiz_border = (mb->item_width-(mb->icon_size))/2;

	Bool isHomeShow = False;  
	
  if (mb->scroll_offset_item  == mb->current_head_item)
    mb->scroll_active = False;
  else
    mb->scroll_active = True;

  cur_x = mb->workarea_x;
  
  // 第一行icon高度固定为79 
	// cur_y = mb->workarea_y + mb->title_offset + mb->icon_size / 16;
	cur_y = mb->workarea_y + 35;
		
  limit_x = mb->workarea_x + mb->workarea_width;
  limit_y = mb->workarea_y + mb->workarea_height - mb->title_offset; /* - mb->win_plugin_rect.height; */

	/* Commit By Steven
  mb->current_view_columns = mb->workarea_width  / mb->item_width;
  mb->current_view_rows  
    = ( mb->workarea_height - mb->title_offset ) / mb->item_height;
  */
  mb->current_view_columns= 3;
  mb->current_view_rows  = 3;
  
  fprintf(stdout, "steven-desktop: current_head_item->type = [%d]  current_head_item->item_parent->type = [%d] mb->scroll_offset_item->type [%d]\n",mb->current_head_item->type, mb->current_head_item->item_parent->type, mb->scroll_offset_item->type);
		
  for(item = mb->scroll_offset_item; 
      item != NULL; 
      item = item->item_next_sibling)
    {
      if (item->type == ITEM_TYPE_MODULE_WINDOW)
	{
	  /* 
	     May need to 'newline' thigs here.

	  */

	    item->x      = mb->workarea_x;
	    item->y      = cur_y;
	    item->width  = mb->workarea_width;
	    /* item->height stays the same
	    
	    mbdesktop_xembed_configure (mb, item);

	    */

	    cur_x = mb->workarea_x;
	    cur_y += mb->item_height;


	    /*
	     Its a window. 
	       - Move it ?
	       - Is it offscreen ?
	       - resize its width if changed ?
	       - send it an expose ?
	       - cur_y + window height. 
	    */
	} 
	/* Added By Steven Chen */
			else if (item->type == ITEM_TYPE_PREVIOUS)
	{
/* 现在回退按钮只要显示就可以了,这段代码不用了,会被后面的wheel图片遮住		
				item->x      = mb->workarea_x + 6;
		  	item->y      = mb->workarea_height - 76 - 6 + mb->workarea_y;
		  
				if (item->icon)
		    {
		      if (mb_pixbuf_img_get_width(item->icon) != mb->icon_size
			  || mb_pixbuf_img_get_height(item->icon) != mb->icon_size)
					{	  
			  		icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, item->icon,
							       mb->icon_size, 
							       mb->icon_size);
							       
			  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
						  icon_img_small, 
						  item->x, 
						  item->y );
			  
			  		mb_pixbuf_img_free(mb->pixbuf, icon_img_small);						       
							       			
					} else mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
						       item->icon, 
						       item->x, 
									item->y );
									
					isHomeShow = True;									
				}	
*/				
				isHomeShow = True;
	}
	/* End Added */
      else
	{
		if ((cur_x + mb->item_width) > limit_x) /* 'newline' */
	    {
	      cur_x  = mb->workarea_x;
	      cur_y += mb->item_height;

	      cur_row++;

	      //if ( (cur_y+mb->item_height) > limit_y) /* Off display ? */
	      if (cur_row >   mb->current_view_rows ) 
		{

		 	mb->scroll_active = True;
		  break;
		}
	    }

	  item->x      = cur_x;
	  item->y      = cur_y;
	  item->width  = mb->item_width;
	  item->height = mb->item_height;
	  
		if (item->icon)
	    {
	      if (mb_pixbuf_img_get_width(item->icon) != mb->icon_size
		  || mb_pixbuf_img_get_height(item->icon) != mb->icon_size)
		{
		  icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, item->icon,
						       mb->icon_size, 
						       mb->icon_size);
		  
		  mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  icon_img_small, 
					  item->x + item_horiz_border, 
					  item->y );
		  
		  mb_pixbuf_img_free(mb->pixbuf, icon_img_small);
		  
		} else mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					       item->icon, 
					       item->x + item_horiz_border, 
					   item->y );
	    }
	  
	  cur_x += mb->item_width;
	  
		// 是否显示高亮
		// fprintf(stderr, "=== item->name = [%s] mb->kbd_focus_item->name = [%s]\n", item->name, mb->kbd_focus_item->name);
		if(!strcmp(item->name, mb->kbd_focus_item->name))
		{
			
			if(mb->img_item_hot)
			{
	      if (mb_pixbuf_img_get_width(mb->img_item_hot) != mb->icon_size + 8
		  		|| mb_pixbuf_img_get_height(mb->img_item_hot) != mb->icon_size + 8)
				{
		  			icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, mb->img_item_hot,
						       mb->icon_size + 8, 
						       mb->icon_size + 8);
		  
		  			mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  			icon_img_small, 
					 			  item->x + item_horiz_border -4, 
								  item->y - 4);
		  
		 			 mb_pixbuf_img_free(mb->pixbuf, icon_img_small);
		  
				} else 
				{
					mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					       mb->img_item_hot, 
					       item->x + item_horiz_border - 4, 
					   		 item->y - 4);
	    	}				
			}

/* 高亮方式不同
			if(mb->img_item_hot)
			{
	      if (mb_pixbuf_img_get_width(mb->img_item_hot) != mb->item_width
		  		|| mb_pixbuf_img_get_height(mb->img_item_hot) != mb->item_height)
				{
		  			icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, mb->img_item_hot,
						       mb->item_width, 
						       mb->item_height);
		  
		  			mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  			icon_img_small, 
					 			  item->x , 
								  item->y);
		  
		 			 mb_pixbuf_img_free(mb->pixbuf, icon_img_small);
		  
				} else 
				{
					mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					       mb->img_item_hot, 
					       item->x , 
					   		 item->y );
	    	}				
			}			
*/			
		}
	  
	  /* 先判断当前屏回退按钮是否已经显示 */
	  if(!isHomeShow) 	  
		{
		  /*if(mb->scroll_offset_item->type == ITEM_TYPE_FOLDER) */
		  /*判断现在是否在顶层,如果是则显示回退图标*/
		  if(mb->current_head_item->item_parent->type == ITEM_TYPE_ROOT ||
		  	mb->scroll_offset_item->type == ITEM_TYPE_DOTDESKTOP_ITEM ||
		  	mb->scroll_offset_item->type == ITEM_TYPE_DOTDESKTOP_FOLDER )
		  	{
		  		if(mb->current_head_item->item_parent->type == ITEM_TYPE_ROOT )
				  	topfolderitem = mb->scroll_offset_item->item_child;
				  else
				  	topfolderitem = mb->current_head_item->item_parent->item_child;
				  
					if (topfolderitem->type == ITEM_TYPE_PREVIOUS)
						{
							topfolderitem->x      = mb->workarea_x;
					  	topfolderitem->y      = mb->workarea_height - 76 + mb->workarea_y;
					  
					  /* 回退按钮后面画
							if (topfolderitem->icon)
					    {
					      if (mb_pixbuf_img_get_width(topfolderitem->icon) != mb->icon_size
						  || mb_pixbuf_img_get_height(topfolderitem->icon) != mb->icon_size)
								{	  
						  		icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, topfolderitem->icon,
										       mb->icon_size, 
										       mb->icon_size);
										       
						  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
									  icon_img_small, 
									  topfolderitem->x, 
									  topfolderitem->y );
						  
						  		mb_pixbuf_img_free(mb->pixbuf, icon_img_small);						       
										       			
								} else mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
									       topfolderitem->icon, 
									       topfolderitem->x, 
												topfolderitem->y );
							}	
							*/
							isHomeShow = True;	
						}
				} 			
			}
	}
    }

			// 显示Title Bar
			if(mb->img_title_bar)	
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_title_bar, 
					  		mb->workarea_x, 
					  		mb->workarea_y);
			
			// 显示wheel scroll 
	    if(mb->wheel_mode == True )
			{
					if (mb_pixbuf_img_get_width(mb->img_scroll_hot) != 152
				  || mb_pixbuf_img_get_height(mb->img_scroll_hot) != 152)	
				  {
				  		icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, mb->img_scroll_hot,
										152,  152);
				  
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		icon_img_small, 
							  		mb->workarea_x, 
							  		mb->workarea_height - 152 + mb->workarea_y);
				  
				  		mb_pixbuf_img_free(mb->pixbuf, icon_img_small);		 
				  } else 			
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		mb->img_scroll_hot, 
							  		mb->workarea_x, 
							  		mb->workarea_height - 152 + mb->workarea_y);					
			} else {
					if (mb_pixbuf_img_get_width(mb->img_scroll) != 152
				  || mb_pixbuf_img_get_height(mb->img_scroll) != 152)	
				  {
				  		icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, mb->img_scroll,
										152,  152);
				  
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		icon_img_small, 
							  		mb->workarea_x, 
							  		mb->workarea_height - 152 + mb->workarea_y);
				  
				  		mb_pixbuf_img_free(mb->pixbuf, icon_img_small);		 
				  } else 			
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		mb->img_scroll, 
							  		mb->workarea_x, 
							  		mb->workarea_height - 152 + mb->workarea_y);				
			}	
    
			// 显示回退按钮图案
    	if(mb->current_head_item->item_parent->type == ITEM_TYPE_ROOT )
    	{
    			//最顶层显示disable
			    if (mb_pixbuf_img_get_width(mb->img_back_dis) != 32
				  || mb_pixbuf_img_get_height(mb->img_back_dis) != 32)	
				  {
				  		icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, mb->img_back_dis,
										32,  32);
				  
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		icon_img_small, 
							  		mb->workarea_x + 22, 
							  		mb->workarea_height - 76 + 22 + mb->workarea_y);
				  
				  		mb_pixbuf_img_free(mb->pixbuf, icon_img_small);		 
				  } else 			
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		mb->img_back_dis, 
							  		mb->workarea_x + 7, 
							  		mb->workarea_height - 76 + 37 + mb->workarea_y);    			
    	} else {
    			//最顶层显示disable
			    if (mb_pixbuf_img_get_width(mb->img_back_hot) != 32
				  || mb_pixbuf_img_get_height(mb->img_back_hot) != 32)	
				  {
				  		icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, mb->img_back_hot,
										32,  32);
				  
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		icon_img_small, 
							  		mb->workarea_x + 6, 
							  		mb->workarea_height - 76 + 7 + mb->workarea_y);
				  
				  		mb_pixbuf_img_free(mb->pixbuf, icon_img_small);		 
				  } else 			
				  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
							  		mb->img_back_hot, 
							  		mb->workarea_x + 7, 
							  		mb->workarea_height - 76 + 37 + mb->workarea_y);       		
    	}
    
    
			// 显示History APP 
			int scalewide = (mb->workarea_width - 152) / 4;
			if(mb->img_hisapp)
			{
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp, 
					  		mb->workarea_x + 152, 
					  		mb->workarea_height - 108 + mb->workarea_y);
					  		
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp, 
					  		mb->workarea_x + 152 + scalewide, 
					  		mb->workarea_height - 108 + mb->workarea_y);
					  		
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp, 
					  		mb->workarea_x + 152 + scalewide*2, 
					  		mb->workarea_height - 108 + mb->workarea_y);				
					  		
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp, 
					  		mb->workarea_x + 152 + scalewide*3, 
					  		mb->workarea_height - 108 + mb->workarea_y);						  							  		
			}
//			fprintf(stderr, "===== mb->hisapp_mode = [%d]\n", mb->hisapp_mode);
			
			if(mb->hisapp_mode != 0)
					if( mb->img_hisapp_hot)
		  			mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp_hot, 
					  		mb->workarea_x + 152 + (mb->hisapp_mode - 1) * scalewide, 
					  		mb->workarea_height - 108 + mb->workarea_y);			
			
			// 显示内嵌应用程序图标
			if(mb->img_hisapp1)	
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp1, 
					  		mb->workarea_x + 152 + 18, 
					  		mb->workarea_height - 108 + mb->workarea_y + 18 );
					  		
			if(mb->img_hisapp2)	
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp2, 
					  		mb->workarea_x + 152 + scalewide + 18, 
					  		mb->workarea_height - 108 + mb->workarea_y + 18 );
					  		
			if(mb->img_hisapp3)	
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp3, 
					  		mb->workarea_x + 152 + scalewide*2 + 18, 
					  		mb->workarea_height - 108 + mb->workarea_y + 18 );
					  		
			if(mb->img_hisapp4)	
		  		mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  		mb->img_hisapp4, 
					  		mb->workarea_x + 152 + scalewide*3 + 18, 
					  		mb->workarea_height - 108 + mb->workarea_y + 18 );					  							  		
					  							  		
					  					
			/* 显示Popup Menu */
			if(popup_mode)
				{
				 if (!mb_menu_is_active(mb->mbmenu))
				   {
				     /*int dpy_h = DisplayHeight(mb->dpy, mb->scr);*/
				     mb_menu_activate(mb->mbmenu,
					       mb->pop_x,
					       mb->pop_y);
				   }						
			}	

  mb_pixbuf_img_render_to_drawable(mb->pixbuf, img_dest, 
				   mb_drawable_pixmap(mb->backing_cache), 
				   0, 0);

  mb->last_visible_item = item;

  if (mbdesktop_current_folder_view (mb) != VIEW_ICONS_ONLY)
/*		if (mb->view_type != VIEW_ICONS_ONLY) */
    {
      MBLayout *layout = NULL;

      layout = mb_layout_new();

      mb_layout_set_font(layout, mb->font);

      for(item = mb->scroll_offset_item; 
	  (item != NULL && item != mb->last_visible_item); 
	  item = item->item_next_sibling)
	{
		if(item->type == ITEM_TYPE_PREVIOUS)
			continue;
		
	  int extra_render_opt = 0;
	  mb_layout_set_geometry(layout, item->width, 600);

// fprintf(stderr, "=====item_type = [%d] item-name=[%s]\n", item->type, item->name);

	  mb_layout_set_text(layout, item->name, MB_ENCODING_UTF8);

	  if (mb->use_text_outline)
	    extra_render_opt = MB_FONT_RENDER_EFFECT_SHADOW;

	  mb_layout_render(layout, mb->backing_cache, 
			   item->x, item->y + mb->icon_size,
			   MB_FONT_RENDER_OPTS_CLIP_TRAIL
			   |MB_FONT_RENDER_ALIGN_CENTER|extra_render_opt );

	}
	
      mb_layout_unref(layout);
    }

}

void
mbdesktop_view_item_highlight (MBDesktop     *mb, 
			       MBDesktopItem *item,
			       int            highlight_style)
{
/*	
  MBPixbufImage    *img_cache = NULL;
  MBDrawable       *pxm;
  MBLayout         *layout;
  MBFontRenderOpts  opts = 0;

  int text_y_offset = 0;
  int text_x_offset = 0;
  int cur_view = 0;
  int x = 0, y = 0, w = 0, h = 0, xx = 0, yy = 0;
  unsigned char r,g,b;

  cur_view = mbdesktop_current_folder_view(mb);

  switch (cur_view)
    {
    case VIEW_ICONS:
      x = item->x;
      y = item->y      + mb->icon_size;
      w = item->width;
      h = item->height - mb->icon_size;
      opts = MB_FONT_RENDER_OPTS_CLIP_TRAIL|MB_FONT_RENDER_ALIGN_CENTER;
      text_x_offset = 0;
      break;

    case VIEW_ICONS_ONLY:
    case VIEW_TEXT_ONLY:
      x = item->x;
      y = item->y;
      w = item->width;
      h = item->height;
      opts = MB_FONT_RENDER_OPTS_CLIP_TRAIL;
      break;

    case VIEW_LIST:
      x = item->x + mb->icon_size + mb->icon_size/8;
      text_x_offset = mb->icon_size/8;
      y = item->y;
      w = item->width - mb->icon_size - mb->icon_size/8;
      h = item->height;

     
      text_y_offset = 0; 

      opts = MB_FONT_RENDER_OPTS_CLIP_TRAIL|MB_FONT_RENDER_VALIGN_MIDDLE;
      break;
    }
    
  switch (highlight_style)
    {
    case HIGHLIGHT_OUTLINE:
    case HIGHLIGHT_FILL:

      r = mb_col_red(mb->hl_col);
      g = mb_col_green(mb->hl_col);
      b = mb_col_blue(mb->hl_col);

      img_cache = mb_pixbuf_img_rgba_new(mb->pixbuf, w, h);
     
      mb_pixbuf_img_copy (mb->pixbuf, img_cache, mb->bg_img,
			  x, y, w, h, 0, 0);

      for ( xx=2; xx < (w - 2); xx++)
	{
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, 0,
				   r, g, b);
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, h-2,
				   r, g, b);
	}
      
      for ( xx=1; xx < (w - 1); xx++)
	{
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, 1,
				   r, g, b);
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, h-3,
				   r, g, b);
	}
      
      for ( xx=0; xx < w; xx++)
	for ( yy=2; yy < h-3; yy++)
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, yy,
				   r, g, b);

      pxm = mb_drawable_new(mb->pixbuf, w, h);

      mb_pixbuf_img_render_to_drawable(mb->pixbuf, img_cache, 
				       mb_drawable_pixmap(pxm), 0, 0);

      if (cur_view != VIEW_ICONS_ONLY)
	{
	  if (mb->font_col_type == DKTP_FONT_COL_BLACK) {
	    mbdesktop_set_font_color(mb, "white");
	  }
	  else  if (mb->font_col_type == DKTP_FONT_COL_WHITE){
	    mbdesktop_set_font_color(mb, "black");
	  }
	  else if (mb->use_text_outline) {
	    opts |= MB_FONT_RENDER_EFFECT_SHADOW;
	  }

	  layout = mb_layout_new();

	  mb_layout_set_font(layout, mb->font);

	  mb_layout_set_geometry(layout, w, h);

	  mb_layout_set_text(layout, item->name, MB_ENCODING_UTF8);

	  mb_layout_render(layout, pxm, 
			   text_x_offset, text_y_offset,
			   opts);

	  mb_layout_unref(layout);

	  if (mb->font_col_type == DKTP_FONT_COL_BLACK)
	    mbdesktop_set_font_color(mb, "black");
	  else  if (mb->font_col_type == DKTP_FONT_COL_WHITE)
	    mbdesktop_set_font_color(mb, "white");
	}
      else 
	{

	  mbdesktop_view_header_paint(mb, item->name, "");
	}


      XCopyArea(mb->dpy, mb_drawable_pixmap(pxm), mb->win_top_level, mb->gc, 0, 0, w, h, x, y);

      mb_pixbuf_img_free(mb->pixbuf, img_cache);

      mb_drawable_unref(pxm);

      break;

    case HIGHLIGHT_OUTLINE_CLEAR:
    case HIGHLIGHT_FILL_CLEAR:
      XClearWindow(mb->dpy, mb->win_top_level);
      break;
    }

  XFlush(mb->dpy);
  */
}

