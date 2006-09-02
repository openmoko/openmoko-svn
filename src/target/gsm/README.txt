

1. GSMD api towards libgsmd 
	- provides api for other processes to interact with GSM subsystem

2. GSMD api towards vendor-specific plugins
	- implement vendor-specific AT commands

3. libgsmd api towards applications
	- wraps GSMD-libgsmd api into C functions that can be used by applications



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

