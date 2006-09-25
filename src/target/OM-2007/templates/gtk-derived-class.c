#include "myobject.h"

/* add your signals here */
enum {
    MYOBJECT_SIGNAL,
    LAST_SIGNAL
};

static void myobject_class_init          (MyobjectClass *klass);
static void myobject_init                (Myobject      *self);

static guint myobject_signals[LAST_SIGNAL] = { 0 };

GType myobject_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MyobjectClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) myobject_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (Myobject),
            0,
            (GInstanceInitFunc) myobject_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(MYOBJECT_PARENT, "Myobject", &f_info, 0);
    }

    return self_type;
}

static void myobject_class_init (MyobjectClass *klass) /* Class Initialization */
{
    myobject_signals[MYOBJECT_SIGNAL] = g_signal_new ("myobject",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MyobjectClass, myobject),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void myobject_init (Myobject *self) /* Instance Construction */
{
    /* populate your widget here */
}

GtkWidget* myobject_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(myobject_get_type(), NULL));
}

void myobject_clear(Myobject *self) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */