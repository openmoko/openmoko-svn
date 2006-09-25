#include "footer.h"

enum {
    FOOTER_SIGNAL,
    LAST_SIGNAL
};

static void footer_class_init          (FooterClass *klass);
static void footer_init                (Footer      *f);

static guint footer_signals[LAST_SIGNAL] = { 0 };

GType footer_get_type (void) /* Typechecking */
{
    static GType f_type = 0;

    if (!f_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (FooterClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) footer_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (Footer),
            0,
            (GInstanceInitFunc) footer_init,
        };

        f_type = g_type_register_static(GTK_TYPE_HBOX, "Footer", &f_info, 0);
    }

    return f_type;
}

static void footer_class_init (FooterClass *klass) /* Class Initialization */
{
    footer_signals[FOOTER_SIGNAL] = g_signal_new ("footer",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (FooterClass, footer),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);


}

static void footer_init (Footer *f) /* Instance Construction */
{
    f->leftbutton = gtk_button_new_with_label( "A" );
    gtk_box_pack_start( f, GTK_WIDGET(f->leftbutton), FALSE, FALSE, 0 );

    f->statusbar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip( f->statusbar, FALSE );
    gtk_box_pack_start( f, GTK_WIDGET(f->statusbar), TRUE, TRUE, 0 );

    gtk_statusbar_push( f->statusbar, 1, "Ready." );

    f->rightbutton = gtk_button_new_with_label( "B" );
    gtk_box_pack_start( f, GTK_WIDGET(f->rightbutton), FALSE, FALSE, 0 );

    gtk_widget_show( f->leftbutton );
    gtk_widget_show( f->statusbar );
    gtk_widget_show( f->rightbutton );
}

GtkWidget* footer_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(footer_get_type(), NULL));
}

void footer_clear(Footer *f) /* Destruction */
{
}

void footer_set_status(Footer *f, const char* s)
{
    gtk_statusbar_push( f->statusbar, 1, s );
}
