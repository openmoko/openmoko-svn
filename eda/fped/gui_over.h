/*
 * gui_over.h - GUI, canvas overlays
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_OVER_H
#define	GUI_OVER_H

/*
 * Dynamic changes around the pointer are affected by the following events:
 *
 * - enter: enter a circle where we're hovering
 * - leave: leave a circle where we've been hovering
 * - begin: begin dragging
 * - move: move with or without dragging
 * - end: end dragging
 * - reset: we got a redraw, just drop everything
 *
 * We have the following states:
 *
 * - NOTHING: neither hovering nor dragging
 * - HOVER: we're hovering but not dragging
 * - DRAG: we're dragging but not hovering, e.g., when searching for a place to
 *   end the drag
 * - BOTH: we're dragging and hovering
 *
 * Both drag and hover save the area being changed and restore it after a
 * change. We have to make sure the save/draw/restore operations are properly
 * sequenced. We call the hover area H, the drag area D. This is the state
 * machine that does the sequencing:
 *
 * NOTHING (saved: -)
 *	enter -> save H, draw H, HOVER
 *	begin -> save D, draw D, DRAG
 *	move -> NOTHING
 *	reset -> NOTHING
 *
 * HOVER: (saved: H)
 *	leave -> restore H, NOTHING
 *	begin -> save D, draw D, BOTH
 *	move -> HOVER
 *	reset -> drop H, NOTHING
 *
 * DRAG: (saved: D)
 *	end -> restore D, NOTHING
 *	enter -> restore D, save H, draw H, save D, draw D, BOTH
 *	move -> restore D, update, save D, draw D, DRAG
 *	reset -> drop D, NOTHING
 *
 * BOTH: (saved: D on top of H)
 *	end -> restore D, HOVER
 *	leave -> restore D, restore H, save D, draw D, DRAG
 *	move -> restore D, update, save D, draw D, BOTH
 *	reset -> drop D, drop H, NOTHING
 */

#include "coord.h"
#include "inst.h"


void over_enter(struct pix_buf *(*save_and_draw)(void *user), void *user);
void over_leave(void);

void over_begin(struct pix_buf *(*save_and_draw)(void *user, struct coord pos),
    void *user, struct coord pos);
void over_move(struct coord pos);
void over_end(void);

void over_reset(void);

#endif /* !GUI_OVER_H */
