#include "misc.h"

void
mbcommand(Display *dpy, int cmd_id, char *data)
{
    XEvent ev;
    Window root;
    Atom cmd_prop, desktop_manager_atom;

    desktop_manager_atom = XInternAtom(dpy, "_NET_DESKTOP_MANGER",False);

    root = DefaultRootWindow(dpy);

    if (cmd_id == MB_CMD_DESKTOP)
    {
       /* Check if desktop is running */
        if (!XGetSelectionOwner(dpy, desktop_manager_atom))
        {
            fprintf(stderr, "Desktop not running, exiting...\n");
            switch (fork())
            {
               case 0:
                 execvp ("mbdesktop", NULL);
                 break;
               case -1:
                 fprintf(stderr, "failed to exec mbdesktop");
                 break;
                 }
             exit(0);
             }
         }

   cmd_prop = XInternAtom(dpy, "_MB_COMMAND", False);

   memset(&ev, '\0', sizeof ev);
   ev.xclient.type = ClientMessage;
   ev.xclient.window = root; 	/* we send it _from_ root as we have no win  */
   ev.xclient.message_type = cmd_prop;
   ev.xclient.format = 8;

   ev.xclient.data.l[0] = cmd_id;

   XSendEvent(dpy, root, False,
      SubstructureRedirectMask|SubstructureNotifyMask, &ev);

}

