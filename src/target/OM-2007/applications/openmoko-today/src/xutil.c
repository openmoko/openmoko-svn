#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

char *
x_strerror (int code)
{
#define BUFFER_SIZE 255
  char *s;

  s = g_malloc (BUFFER_SIZE);

  XGetErrorText (GDK_DISPLAY (), code, s, BUFFER_SIZE);

  return s;
}


gboolean
x_get_workarea (int *x, int *y, int *w, int *h)
{
  Atom real_type;
  int result, xres, real_format;
  unsigned long items_read, items_left;
  long *coords;

  Atom workarea_atom = XInternAtom (GDK_DISPLAY (), "_NET_WORKAREA", False);
  
  gdk_error_trap_push ();
  result = XGetWindowProperty (GDK_DISPLAY (), GDK_ROOT_WINDOW (),
                               workarea_atom, 0L, 4L, False,
                               XA_CARDINAL, &real_type, &real_format,
                               &items_read, &items_left,
                               (unsigned char **) (void*)&coords);
  if ((xres = gdk_error_trap_pop ()) != 0) {
    char *s = x_strerror (xres);
    g_warning ("Cannot get property: %s", s);
    g_free (s);
    return FALSE;
  }

  if (result == Success && items_read) {
    *x = coords[0];
    *y = coords[1];
    *w = coords[2];
    *h = coords[3];
    XFree(coords);
    return TRUE;
  }
  return FALSE;
}
