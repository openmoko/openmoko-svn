/**
 *  @file tool-box.c
 *  @brief The tool box in the main window
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */

#include "tool-box.h"

/**
 * @brief Create a new tool box for the application manager data
 * @param appdata The application manager data
 * @return The toplevel widget of the tool box
 */
MokoToolBox *
tool_box_new (ApplicationManagerData *appdata)
{
  MokoToolBox *toolbox;
  MokoPixmapButton *buttonapply;
  MokoPixmapButton *bupgrade;

  toolbox = MOKO_TOOL_BOX (moko_tool_box_new_with_search ());

  bupgrade = moko_tool_box_add_action_button (toolbox);
  gtk_button_set_label (GTK_BUTTON (bupgrade), "Upgrade");

  buttonapply = moko_tool_box_add_action_button (toolbox);
  gtk_button_set_label (GTK_BUTTON (buttonapply), "Apply");

  return toolbox;
}
