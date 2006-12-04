#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <libmb/mb.h>

 MBPixbuf      *pixbuf = NULL;
 MBPixbufImage *img_icon = NULL, *img_icon_scaled = NULL, *img_graph=NULL;

 int 
gsm_signal_check()
 {
	int temp = 0;
 	temp = (int)rand();
 	return (temp);
 }
 
int
rand_num(int min, int max)
{float temp;
static int seed_val=0;
srand(RAND_MAX-seed_val);
temp=rand();
seed_val=temp;
temp=(int)((temp/(float)RAND_MAX)*(max-min));
return temp;
}

 void
 paint_callback ( MBTrayApp *app, Drawable drw )
 {
 	 static int prev_gsm_pixels= -1;
 	 int gsm_pixels = NULL;
 	 int gsm_size = NULL;  /*figure in 0-100*/
 	 int x, y, temp=0;
 	 int gsm_x, gsm_y, gsm_w, gsm_h;
 	 int gsm_signal_w = 0,gsm_signal_h = 0;
   	 MBPixbufImage *img_backing=NULL;
   
   /* example figures */
   /*********************/ 
   /*gsm_size = gsm_signal_check();*/
   gsm_size = rand_num(0,100);
   gsm_h=(mb_pixbuf_img_get_width(img_icon_scaled)*4)/5;
   gsm_pixels = (gsm_size*gsm_h)/10;
   /*********************/
   
   if (gsm_pixels == prev_gsm_pixels) return;
   
   img_backing = mb_tray_app_get_background(app, pixbuf);
   
   mb_pixbuf_img_composite(pixbuf, img_backing, img_icon_scaled, 0, 0);


	 /*make image*/
	 gsm_x=mb_pixbuf_img_get_width(img_backing)/4;
	 gsm_y=mb_pixbuf_img_get_width(img_backing)*6/10;
	 gsm_w=gsm_x*2;
	 gsm_signal_w=((((gsm_w/5)*4)/5)*gsm_size)/10;
	 gsm_signal_h=(((gsm_h*3)/4)*gsm_size)/10;
	 
	 if(gsm_size>0)
	 	{
	 		for(y=gsm_y;y>gsm_y-(gsm_h/4);y--)
	 			for(x=0;x<gsm_signal_w;x++)
	 				mb_pixbuf_img_plot_pixel(pixbuf, img_backing, gsm_x+x+2, y, 0, 0xff, 0);
	 		for(y=(gsm_y-gsm_h/4);y>gsm_h/4-gsm_signal_h;y--,temp++)
	 			for(x=(int)temp*2;x<gsm_signal_w;x++)  /*the figure temp need to be reset*/
	 				mb_pixbuf_img_plot_pixel(pixbuf, img_backing, gsm_x+x+2, y, 0, 0xff, 0);
	 	}
	 	
	 mb_pixbuf_img_render_to_drawable(pixbuf, img_backing, drw, 0, 0);

   mb_pixbuf_img_free(pixbuf, img_backing );

  prev_gsm_pixels = gsm_pixels;

}
 
 void 
 resize_callback(MBTrayApp *app,int w,int h)
 {
  if (img_icon_scaled) mb_pixbuf_img_free(pixbuf, img_icon_scaled);
  if (img_graph) mb_pixbuf_img_free(pixbuf, img_graph);
  img_icon_scaled = mb_pixbuf_img_scale(pixbuf, img_icon, w, h);
 }
 	
  void
timeout_callback ( MBTrayApp *app )
{
  mb_tray_app_repaint (app);
}


 int main(int argc, char *argv[])
 {
   MBTrayApp *app = NULL;
   
   struct timeval tv;

   app = mb_tray_app_new ( "GSM",
 			  resize_callback,
 			  paint_callback,
 			  &argc,
 			  &argv );

   if (app == NULL) return -1;
   	
   memset(&tv,0,sizeof(struct timeval));
   tv.tv_usec=1000000;

   pixbuf = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
 			 mb_tray_app_xscreen(app));

   img_icon = mb_pixbuf_img_new_from_file(pixbuf, "/usr/icon/gsm.png");
   resize_callback (app, mb_tray_app_width(app), mb_tray_app_width(app) );

   mb_tray_app_set_timeout_callback (app, timeout_callback, &tv);

   mb_tray_app_main(app);
 }
