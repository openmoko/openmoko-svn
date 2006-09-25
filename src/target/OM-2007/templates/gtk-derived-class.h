#ifndef OPENMOKO_MYOBJECT_H
#define OPENMOKO_MYOBJECT_H

#include <glib.h>
#include <glib-object.h>
/* include your parent object here */

G_BEGIN_DECLS

#define MYOBJECT_TYPE            (myobject_get_type())
#define MYOBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MYOBJECT_TYPE, Myobject))
#define MYOBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MYOBJECT_TYPE, MyobjectClass))
#define IS_MYOBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MYOBJECT_TYPE))
#define IS_MYOBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MYOBJECT_TYPE))

typedef struct _Myobject       Myobject;
typedef struct _MyobjectClass  MyobjectClass;

struct _Myobject
{
    /* add your parent type here */
    MyObjectParent parent;
    /* add pointers to new members here */

};

struct _MyobjectClass
{
    /* add your parent class here */
    MyObjectParentClass parent_class;
    void (*myobject) (Myobject *f);
};

GType          myobject_get_type        (void);
GtkWidget*     myobject_new             (void);
void           myobject_clear           (Myobject *f);

/* add additional methods here */

G_END_DECLS

#endif /* OPENMOKO_MYOBJECT_H */
