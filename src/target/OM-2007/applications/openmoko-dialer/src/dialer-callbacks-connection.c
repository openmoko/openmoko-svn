#include "dialer-callbacks-connection.h"


void
network_registration_cb (MokoGsmdConnection *self, int type, int lac, int cell)
{
  /* network registration */
}

void
incoming_call_cb (MokoGsmdConnection *self, int type)
{
  /* incoming call */
}

void
incoming_clip_cb (MokoGsmdConnection *self, const char *number)
{
  /* caller id */
}
