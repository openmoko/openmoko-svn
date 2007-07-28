#include <libecal/e-cal.h>
#include <libecal/e-cal-component.h>
#include <gtk/gtk.h>
#include "src/today-events-area.h"

#define LOG_ERROR \
g_warning ("Got error '%s', code '%d'", \
           error->message, error->code);

#define FREE_ERROR g_error_free (error) ; error = NULL ;

static GList* get_calendar_events (ECal *a_cal) ;

static GList*
get_calendar_events (ECal *a_cal)
{
  char   *query = NULL ;
  GError *error = NULL ;
  GList *objects = NULL ;
  GList *events = NULL ;
  GList *cur = NULL ;


  if (!e_cal_open (a_cal, TRUE, &error))
  {
    g_warning ("failed to open the calendar") ;
    goto out;
  }
  if (error)
  {
      LOG_ERROR ;
      FREE_ERROR ;
      goto out;
  }
  /*
  query = g_strdup_printf ("(occur-in-time-range? "
                               "(time-day-begin (time-now)) "
                               "(time-day-end   (time-now))"
                           ")");
   */
  query = g_strdup_printf ("#t");

  if (!e_cal_get_object_list (a_cal, query, &objects, &error))
  {
    g_message ("Querying system calendar failed for query '%s'",
               query) ;
    goto out ;
  }
  if (error)
  {
      LOG_ERROR ;
      FREE_ERROR ;
      goto out;
  }

  for (cur = objects ; cur ; cur = cur->next)
  {
    ECalComponent *comp ;
    comp = e_cal_component_new () ;
    e_cal_component_set_icalcomponent (comp, cur->data) ;
    events = g_list_prepend (events, comp) ;
    cur->data = NULL ;
  }
  g_list_free (objects) ;
  objects = NULL ;

out:
  g_free (query) ;
  if (objects)
  {
    e_cal_free_object_list (objects) ;
  }

  return events ;
}

int
main (int argc, char **argv)
{
  ECal *cal = NULL ;
  GtkWidget *window = NULL ;
  GtkWidget *ta = NULL ;
  GList *events = NULL ;

  gtk_init (&argc, &argv) ;

  cal = e_cal_new_system_calendar () ;
  g_return_val_if_fail (cal, -1) ;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL) ;
  g_signal_connect (G_OBJECT (window),
                    "destroy",
                    G_CALLBACK (gtk_exit),
                    NULL) ;
  ta = today_events_area_new () ;
  today_events_area_set_max_visible_events (TODAY_EVENTS_AREA (ta),
                                            2) ;
  events = get_calendar_events (cal) ;
  g_return_val_if_fail (events, -1) ;
  today_events_area_set_events (TODAY_EVENTS_AREA (ta), events) ;
  gtk_container_add (GTK_CONTAINER (window), ta) ;

  gtk_widget_show_all (window) ;

  gtk_main () ;

  if (cal)
  {
    g_object_unref (G_OBJECT (cal)) ;
  }
  return 0 ;
}
