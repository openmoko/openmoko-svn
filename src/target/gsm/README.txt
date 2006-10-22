

1. GSMD api towards libgsmd 
	- provides api for other processes to interact with GSM subsystem
	- is not documented and subject to change

2. GSMD api towards vendor-specific plugins
	- implement vendor-specific AT commands
	- is not documented and will initially only support TI 
	- later, a Motorola plugin is planned.

3. libgsmd api towards applications
	- wraps GSMD-libgsmd api into C functions that can be used by applications
	- will be documented and is supposed to be stable.

3.1. Events

The application registers callback handlers for events.  libgsmd can call these callback
handlers at any given time.  Events include "incoming call", "remote end hangup", ...

3.2. Commands

Commands have the form of command-reply.  Usually they will be performed
synchronously, i.e. the the caller will be blocked until a response to the command has been
received.

If this behaviour is not desired, a completion handler callback function can be
registered.  In this case, the function call will return immediately after the
command has been sent to gsmd.  Further incoming packets on the gsmd socket
will be handled to libgsmd by calling lgsm_handle_packet().  If one such packet is the
response to a currently-executing command, the completion handler will be
called from within lgsm_handle_packet().  Matching of responses to requests is done by 
assigning a unique id to each request.  gsmd responses will have a matching id.


code flow in gsmd:

- select loop detects data has arrived on user socket
	- gsmd_usock_user_cb()
		- if this is atcmd passthrough, 
			- alloc + fill in gsmd_atcmd
			- atcmd_submit()
				- append it to pending_atcmds
				- mark interest of writing to UART
		- if this is not passthrough
			- do whatever handling, including enqueueing of atcmds
- select loop detects UART is marked writable
	- atcmd_select_cb()
		- iterate over list of pending_atcmds()
			- write a particular atcmd to UART
			- move atcmd to busy_atcmds
- select loop detects UART is marked readable
	- atcmd_select_cb()
		- read up to 1024 bytes into buffer
		- llparse_string()
			- llparse_byte()
				- llparse_append()
			- llp->cb == ml_parse()
				- if this is not unsolicited
					- call cmd->cb() 
						- alloc + fill ucmd reply
						- add to finished_ucmds
						- mark user sock writable
					- unlink + free atcmd
				- if this is unsolicited
					- unsolicited_parse()
						- usock_evt_send()
							- alloc + fill ucmd reply
							- add to finished_ucmds
							- mark user sock writable
- select loop detects user sock writeability
	- gsmd_usock_user_cb()
		- iterate over finished_ucmds
			- write ucmd
			- unlink + free ucmd

