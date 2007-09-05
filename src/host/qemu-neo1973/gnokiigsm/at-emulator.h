/*

  $Id: at-emulator.h,v 1.12 2004/09/20 16:05:38 bozo Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  This file is part of gnokii.

  Gnokii is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Gnokii is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with gnokii; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copyright (C) 1999, 2000 Hugh Blemings & Pavel Janík ml.

  Header file for AT emulator code.

*/

#ifndef _gnokii_data_at_emulator_h
#define _gnokii_data_at_emulator_h

#include "compat.h"
#include "gnokii.h"

/* XXX: The values >6 are TI-specific */
enum gsmd_call_progress {
	GSMD_CALLPROG_SETUP		= 0,
	GSMD_CALLPROG_DISCONNECT	= 1,
	GSMD_CALLPROG_ALERT		= 2,
	GSMD_CALLPROG_CALL_PROCEED	= 3,
	GSMD_CALLPROG_SYNC		= 4,
	GSMD_CALLPROG_PROGRESS		= 5,
	GSMD_CALLPROG_CONNECTED		= 6,
	GSMD_CALLPROG_RELEASE		= 7,
	GSMD_CALLPROG_REJECT		= 8,
	GSMD_CALLPROG_UNKNOWN		= 9,
	__NUM_GSMD_CALLPROG
};

enum gsmd_call_direction {
	GSMD_CALL_DIR_MO	= 0,	/* Mobile Originated (Outgoing) */
	GSMD_CALL_DIR_MT	= 1,	/* Mobile Terminated (Incoming) */
	GSMD_CALL_DIR_CCBS	= 2,	/* network initiated MO */
	GSMD_CALL_DIR_MO_REDIAL	= 3,	/* Mobile Originated Redial */
};

	/* Prototypes */
bool	gn_atem_initialise(struct gn_statemachine *sm);
void	gn_atem_incoming_data_handle(const char *buffer, int length);
void	gn_atem_registers_init(void);
void	gn_atem_string_out(char *buffer);
void	gn_atem_at_parse(char *cmd_buffer);
void	gn_atem_sms_parse(char *cmd_buffer);
void	gn_atem_dir_parse(char *cmd_buffer);
bool	gn_atem_command_plusc(char **buf);
bool	gn_atem_command_plusg(char **buf);
bool	gn_atem_command_percent(char **buf);
int	gn_atem_num_get(char **p);
void	gn_atem_modem_result(int code);
void    gn_atem_call_passup(gn_call_status call_status,
		gn_call_info *call_info, struct gn_statemachine *state);
void	gn_atem_cid_out(gn_call_info *callinfo);
bool	gn_atem_command_diesis(char **buf);
void	gn_atem_cpi(enum gsmd_call_progress msg, enum gsmd_call_direction dir,
		int inband);

	/* Global variables */
extern bool gn_atem_initialised;
extern struct gn_statemachine	*sm;
extern gn_data		data;

	/* Definition of modem result codes - these are returned to "terminal"
       numerically or as a string depending on the setting of S12 */

#define 	MR_OK			(0)
#define		MR_CONNECT		(1)
#define         MR_RING                 (2)
#define		MR_NOCARRIER		(3)
#define		MR_ERROR		(4)
#define		MR_CARRIER		(5)
#define		MR_NODIALTONE		(6)
#define		MR_BUSY			(7)
#define		MR_NOANSWER		(8)

#endif	/* _gnokii_data_at_emulator_h */
