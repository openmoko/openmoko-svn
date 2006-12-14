/* moko-dialer-tip.c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */

 #include "moko-dialer-tip.h"
 #include "error.h"


G_DEFINE_TYPE (MokoDialerTip, moko_dialer_tip, GTK_TYPE_EVENT_BOX)


#define MOKO_DIALER_TIP_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_DIALER_TIP, MokoDialerTipPrivate))



struct _MokoDialerTipPrivate
{

  GtkWidget * label; ///<the displaying label;
  gint index; ///<the index of this tip widget
  gboolean selected;///<is this tip currently selected?
  
};



typedef struct _MokoDialerTipPrivate MokoDialerTipPrivate;

GtkWidget*      moko_dialer_tip_new()
{

return moko_dialer_tip_new_with_label_and_index("",-1);
}


/**
 * @brief new a MokoDialerTip with the a string.
 * @param stringname  the name for the user;
 * @param index the index of this widget
 */
GtkWidget*      moko_dialer_tip_new_with_label_and_index(const gchar * stringname,const gint index)
{



 MokoDialerTip *  dialertip= ( MokoDialerTip * )g_object_new (MOKO_TYPE_DIALER_TIP, NULL);


 GdkColor  color;
 

 MokoDialerTipPrivate* priv = ( MokoDialerTipPrivate*)MOKO_DIALER_TIP_GET_PRIVATE(dialertip);

 priv->index=index;

GtkWidget*  label= gtk_label_new(stringname);

 priv->selected=FALSE;
  
  gtk_widget_show (label);


  gtk_container_add (GTK_CONTAINER (dialertip), label);

  GTK_WIDGET_SET_FLAGS (label, GTK_CAN_FOCUS);

 gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

//set the default color

 	gdk_color_parse("orange",&color);
	
	gtk_label_set_line_wrap(GTK_LABEL(label),TRUE);
	
	gtk_widget_modify_fg(label,GTK_STATE_NORMAL,&color);
	
	gtk_label_set_pattern(GTK_LABEL(label),"___________________");



	priv->label=label;

 	gdk_color_parse("black",&color);
 	
	gtk_widget_modify_fg(GTK_WIDGET(dialertip),GTK_STATE_NORMAL,&color); 	

 return GTK_WIDGET(dialertip);

}

gboolean moko_dialer_tip_set_label(GtkWidget* widget,const gchar * stringname)
{

g_return_val_if_fail(MOKO_IS_DIALER_TIP(widget),FALSE);

 MokoDialerTipPrivate* priv = ( MokoDialerTipPrivate*)MOKO_DIALER_TIP_GET_PRIVATE(widget);

g_return_val_if_fail(priv!=NULL,FALSE);

gtk_label_set_text( GTK_LABEL(priv->label),stringname);

gtk_label_set_pattern(GTK_LABEL(priv->label),"___________________");



return TRUE;
}

gboolean moko_dialer_tip_set_index(GtkWidget* widget,const gint index)
{

g_return_val_if_fail(MOKO_IS_DIALER_TIP(widget),FALSE);

 MokoDialerTipPrivate* priv = ( MokoDialerTipPrivate*)MOKO_DIALER_TIP_GET_PRIVATE(widget);

g_return_val_if_fail(priv!=NULL,FALSE);

priv->index=index;
return TRUE;
}

	
static void
moko_dialer_tip_class_init(MokoDialerTipClass *klass)
{
//g_print("moko_dialer_tip_class_init\n");

// GObjectClass *object_class = G_OBJECT_CLASS (klass);

//DBG_ENTER();
 g_type_class_add_private (klass, sizeof (MokoDialerTipPrivate));

return;
}

/**
 * @brief  set the digit button digit field to be -1.
 */
static void
moko_dialer_tip_init(MokoDialerTip *self)
{

//DBG_ENTER();

MokoDialerTipPrivate* priv = MOKO_DIALER_TIP_GET_PRIVATE(self);
priv->label=0;
priv->index=-1;
priv->selected=FALSE;

// gtk_widget_set_size_request (GTK_WIDGET(self), 64, 64);
return;
}

gint moko_dialer_tip_get_index(MokoDialerTip* tip)
{
g_return_val_if_fail(MOKO_IS_DIALER_TIP(tip),-1);

 MokoDialerTipPrivate* priv = ( MokoDialerTipPrivate*)MOKO_DIALER_TIP_GET_PRIVATE(tip);

g_return_val_if_fail(priv!=NULL,-1);


return(priv->index);

}

gboolean moko_dialer_tip_set_selected(MokoDialerTip* tip,gboolean selected)
{
g_return_val_if_fail(MOKO_IS_DIALER_TIP(tip),FALSE);

 MokoDialerTipPrivate* priv = ( MokoDialerTipPrivate*)MOKO_DIALER_TIP_GET_PRIVATE(tip);

g_return_val_if_fail(priv!=NULL,FALSE);

GdkColor  colornormal,colorselected;

priv->selected=selected;

if(selected)
{
colorselected.red=255<<8;colorselected.green=255<<8;colorselected.blue=0;
gtk_widget_modify_fg(priv->label,GTK_STATE_NORMAL,&colorselected);
}
else
{
gdk_color_parse("orange",&colornormal);
gtk_widget_modify_fg(priv->label,GTK_STATE_NORMAL,&colornormal);

}
return TRUE;

}

gboolean  moko_dialer_tip_is_selected(MokoDialerTip* tip)
{
g_return_val_if_fail(MOKO_IS_DIALER_TIP(tip),FALSE);

 MokoDialerTipPrivate* priv = ( MokoDialerTipPrivate*)MOKO_DIALER_TIP_GET_PRIVATE(tip);

g_return_val_if_fail(priv!=NULL,FALSE);

return (priv->selected);
}
/*
GType moko_dialer_tip_get_type(void)
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoDialerTipClass),
            NULL, // base_init 
            NULL, // base_finalize
            (GClassInitFunc) moko_dialer_tip_class_init,
            NULL, // class_finalize 
            NULL, // class_data 
            sizeof (MokoDialerTip),
            0,
            (GInstanceInitFunc) moko_dialer_tip_init,
        };

       // add the type of your parent class here 
        self_type = g_type_register_static(GTK_TYPE_BUTTON, "MokoDialerTip", &f_info, 0);
    }

    return self_type;
}

*/
