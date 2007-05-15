
#include "dialer-callbacks-connection.h"
#include "dialer-window-incoming.h"


void
network_registration_cb (MokoGsmdConnection *self, int type, int lac, int cell)
{
  /* network registration */
}

void
incoming_call_cb (MokoGsmdConnection *self, int type, MokoDialerData *data)
{
  /* incoming call */
  window_incoming_show (data);
}

void
incoming_clip_cb (MokoGsmdConnection *self, const char *number)
{
  /* caller id */
}
