
#ifndef _DIALER_CALLBACKS_CONNECTION_H
#define _DIALER_CALLBACKS_CONNECTION_H

#include <libmokogsmd/moko-gsmd-connection.h>
#include <dialer-main.h>

void network_registration_cb (MokoGsmdConnection *self, int type, int lac, int cell);
void incoming_call_cb (MokoGsmdConnection *self, int type, MokoDialerData *data);
void incoming_clip_cb (MokoGsmdConnection *self, const char *number);

#endif
