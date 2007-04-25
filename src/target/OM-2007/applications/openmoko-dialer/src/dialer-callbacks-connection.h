
#include <libmokogsmd/moko-gsmd-connection.h>

void network_registration_cb (MokoGsmdConnection *self, int type, int lac, int cell);
void incoming_call_cb (MokoGsmdConnection *self, int type);
void incoming_clip_cb (MokoGsmdConnection *self, const char *number);
