/*
 * gui.h - Editor GUI core
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>


extern GtkWidget *root;
extern int show_all;
extern int show_stuff;
extern int show_meas;
extern int show_bright;

extern int no_save;


/* update everything after a model change */
void change_world(void);

/* like change_world, but select the object again */
void change_world_reselect(void);

int gui_init(int *argc, char ***argv);
int gui_main(void);

#endif /* !GUI_H */
