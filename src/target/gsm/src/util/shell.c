/* libgsmd tool
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/voicecall.h>
#include <libgsmd/misc.h>
#include <libgsmd/phonebook.h>
#include <libgsmd/sms.h>
#include <gsmd/usock.h>
#include <gsmd/ts0705.h>

#define STDIN_BUF_SIZE	1024

/* this is the handler for receiving passthrough responses */
static int pt_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	char *payload = (char *)gmh + sizeof(*gmh);
	printf("RSTR=`%s'\n", payload);
}

/* this is the handler for receiving phonebook responses */
static int pb_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	struct gsmd_phonebook *gp;
	struct gsmd_phonebook_support *gps;
	char *payload;

	switch (gmh->msg_subtype) {
	case GSMD_PHONEBOOK_FIND:		
		break;
	case GSMD_PHONEBOOK_READRG:
		payload = (char *)gmh + sizeof(*gmh);
		printf("%s\n", payload);	
		break;
	case GSMD_PHONEBOOK_READ:
		gp = (struct gsmd_phonebook *) ((char *)gmh + sizeof(*gmh));
		printf("%d, %s, %d, %s\n", gp->index, gp->numb, gp->type, gp->text);
		break;
	case GSMD_PHONEBOOK_WRITE:			
	case GSMD_PHONEBOOK_DELETE:
		payload = (char *)gmh + sizeof(*gmh);
		printf("%s\n", payload);		
		break;
	case GSMD_PHONEBOOK_GET_SUPPORT:
		gps = (struct gsmd_phonebook_support *) ((char *)gmh + sizeof(*gmh));
		printf("(1-%d), %d, %d\n", gps->index, gps->nlength, gps->tlength);
		break;
	default:
		return -EINVAL;
	}	
}

/* this is the handler for receiving sms responses */
static int sms_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	char payload[GSMD_SMS_DATA_MAXLEN];
	int *result;
	struct gsmd_sms_list *sms;
	static const char *type[] = { "Unread", "Received", "Unsent", "Sent" };

	switch (gmh->msg_subtype) {
	case GSMD_SMS_LIST:
	case GSMD_SMS_READ:
		sms = (struct gsmd_sms_list *) ((void *) gmh + sizeof(*gmh));
		printf("%s message %i from/to %s%s, at %i%i-%i%i-%i%i "
				"%i%i:%i%i:%i%i, GMT%c%i\n", type[sms->stat],
				sms->index,
				((sms->addr.type & __GSMD_TOA_TON_MASK) ==
				 GSMD_TOA_TON_INTERNATIONAL) ? "+" : "",
				sms->addr.number,
				sms->time_stamp[0] & 0xf,
				sms->time_stamp[0] >> 4,
				sms->time_stamp[1] & 0xf,
				sms->time_stamp[1] >> 4,
				sms->time_stamp[2] & 0xf,
				sms->time_stamp[2] >> 4,
				sms->time_stamp[3] & 0xf,
				sms->time_stamp[3] >> 4,
				sms->time_stamp[4] & 0xf,
				sms->time_stamp[4] >> 4,
				sms->time_stamp[5] & 0xf,
				sms->time_stamp[5] >> 4,
				(sms->time_stamp[6] & 8) ? '-' : '+',
				(((sms->time_stamp[6] << 4) |
				  (sms->time_stamp[6] >> 4)) & 0x3f) >> 2);
		if (sms->payload.coding_scheme == ALPHABET_DEFAULT) {
			unpacking_7bit_character(&sms->payload, payload);
			printf("\"%s\"\n", payload);
		} else if (sms->payload.coding_scheme == ALPHABET_8BIT)
			printf("8-bit encoded data\n");
		else if (sms->payload.coding_scheme == ALPHABET_UCS2)
			printf("Unicode-16 encoded text\n");
		break;
	case GSMD_SMS_SEND:
		result = (int *) ((void *) gmh + sizeof(*gmh));
		if (*result >= 0) {
			printf("Send: message sent as ref %i\n", *result);
			break;
		}

		switch (-*result) {
		case 42:
			printf("Store: congestion\n");
			break;
		default:
			printf("Store: error %i\n", *result);
			break;
		}
		break;
	case GSMD_SMS_WRITE:
		result = (int *) ((void *) gmh + sizeof(*gmh));
		if (*result >= 0) {
			printf("Store: message stored with index %i\n",
					*result);
			break;
		}

		switch (-*result) {
		case GSM0705_CMS_SIM_NOT_INSERTED:
			printf("Store: SIM not inserted\n");
			break;
		default:
			printf("Store: error %i\n", *result);
			break;
		}
		break;
	case GSMD_SMS_DELETE:
		result = (int *) ((void *) gmh + sizeof(*gmh));
		switch (*result) {
		case 0:
			printf("Delete: success\n");
			break;
		case GSM0705_CMS_SIM_NOT_INSERTED:
			printf("Delete: SIM not inserted\n");
			break;
		case GSM0705_CMS_INVALID_MEMORY_INDEX:
			printf("Delete: invalid memory index\n");
			break;
		default:
			printf("Delete: error %i\n", *result);
			break;
		}
		break;
	default:
		return -EINVAL;
	}
}

static int shell_help(void)
{
	printf( "\tA\tAnswer incoming call\n"
		"\tD\tDial outgoing number\n"
		"\tH\tHangup call\n"
		"\tO\tPower On\n"
		"\to\tPower Off\n"
		"\tR\tRegister Network\n"
		"\tU\tUnregister from netowrk\n"
		"\tT\tSend DTMF Tone\n"
		"\tpd\tPB Delete (pb=index)\n"
		"\tpr\tPB Read (pr=index)\n"
		"\tprr\tPB Read Range (prr=index1,index2)\n"
		"\tpf\tPB Find (pff=indtext)\n"
		"\tpw\tPB Write (pw=index,number,text)\n"
		"\tps\tPB Support\n"
		"\tsd\tSMS Delete (sd=index,delflg)\n"
		"\tsl\tSMS List (sl=stat)\n"
		"\tsr\tSMS Read (sr=index)\n"
		"\tss\tSMS Send (ss=number,text)\n"
		"\tsw\tSMS Write (sw=stat,number,text)\n"
		"\tq\tQuit\n"
		);
}

int shell_main(struct lgsm_handle *lgsmh)
{
	int rc;
	char buf[STDIN_BUF_SIZE+1];
	char rbuf[STDIN_BUF_SIZE+1];
	int rlen = sizeof(rbuf);
	fd_set readset;
	char *ptr, *fcomma, *lcomma;

	lgsm_register_handler(lgsmh, GSMD_MSG_PASSTHROUGH, &pt_msghandler);
	lgsm_register_handler(lgsmh, GSMD_MSG_PHONEBOOK, &pb_msghandler);
	lgsm_register_handler(lgsmh, GSMD_MSG_SMS, &sms_msghandler);

	fcntl(0, F_SETFD, O_NONBLOCK);
	fcntl(lgsm_fd(lgsmh), F_SETFD, O_NONBLOCK);

	FD_ZERO(&readset);

	printf("# ");

	while (1) {
		fd_set readset;
		int gsm_fd = lgsm_fd(lgsmh);
		FD_SET(0, &readset);
		FD_SET(gsm_fd, &readset);

		rc = select(gsm_fd+1, &readset, NULL, NULL, NULL);
		if (rc <= 0)	
			break;
		if (FD_ISSET(gsm_fd, &readset)) {
			/* we've received something on the gsmd socket, pass it
			 * on to the library */
			rc = read(gsm_fd, buf, sizeof(buf));
			if (rc <= 0) {
				printf("ERROR reding from gsm_fd\n");
				break;
			}
			rc = lgsm_handle_packet(lgsmh, buf, rc);
		}
		if (FD_ISSET(0, &readset)) {
			/* we've received something on stdin.  */
			printf("# ");
			rc = fscanf(stdin, "%s", buf);
			if (rc == EOF) {
				printf("EOF\n");
				return -1;
			}
			if (rc <= 0) {
				printf("NULL\n");
				continue;
			}
			if (!strcmp(buf, "h")) {
				shell_help();
			} else if (!strcmp(buf, "?")) {
				shell_help();
			} else if (!strcmp(buf, "H")) {
				printf("Hangup\n");
				lgsm_voice_hangup(lgsmh);
			} else if (buf[0] == 'D') {
				struct lgsm_addr addr;
				if (strlen(buf) < 2)
					continue;
				printf("Dial %s\n", buf+1);
				addr.type = 129;
				strncpy(addr.addr, buf+1, sizeof(addr.addr)-1);
				addr.addr[sizeof(addr.addr)-1] = '\0';
				lgsm_voice_out_init(lgsmh, &addr);
			} else if (!strcmp(buf, "A")) {
				printf("Answer\n");
				lgsm_voice_in_accept(lgsmh);
			} else if (!strcmp(buf, "O")) {
				printf("Power-On\n");
				lgsm_phone_power(lgsmh, 1);
			} else if (!strcmp(buf, "o")) {
				printf("Power-Off\n");
				lgsm_phone_power(lgsmh, 0);
			} else if (!strcmp(buf, "R")) {
				printf("Register\n");
				lgsm_netreg_register(lgsmh, 0);
			} else if (!strcmp(buf, "U")) {
				printf("Unregister\n");
				lgsm_netreg_register(lgsmh, 2);
			} else if (!strcmp(buf, "q")) {
				exit(0);
			} else if (buf[0] == 'T') {
				if (strlen(buf) < 2)
					continue;
				printf("DTMF: %c\n", buf[1]);
				lgsm_voice_dtmf(lgsmh, buf[1]);
			} else if ( !strncmp(buf, "pd", 2)) {
				printf("Delete Phonebook Entry\n");				
				ptr = strchr(buf, '=');
				lgsmd_pb_del_entry(lgsmh, atoi(ptr+1));				
			} else if ( !strncmp(buf, "prr", 3)) {	
				printf("Read Phonebook Entries\n");
				struct lgsm_phonebook_readrg pb_readrg;

				ptr = strchr(buf, '=');
				pb_readrg.index1 = atoi(ptr+1);				
				ptr = strchr(buf, ',');
				pb_readrg.index2 = atoi(ptr+1);	

				lgsm_pb_read_entryies(lgsmh, &pb_readrg);
			} else if ( !strncmp(buf, "pr", 2)) {	
				printf("Read Phonebook Entry\n");
				ptr = strchr(buf, '=');				
				lgsm_pb_read_entry(lgsmh, atoi(ptr+1));	
			} else if ( !strncmp(buf, "pf", 2)) {
				printf("Find Phonebook Entry\n");
				struct lgsm_phonebook_find pb_find;

				ptr = strchr(buf, '=');
				strncpy(pb_find.findtext, ptr+1, sizeof(pb_find.findtext)-1);
				pb_find.findtext[strlen(ptr+1)] = '\0';	
			
				lgsm_pb_find_entry(lgsmh, &pb_find);
			} else if ( !strncmp(buf, "pw", 2)) {
				printf("Write Phonebook Entry\n");
				struct lgsm_phonebook pb;

				ptr = strchr(buf, '=');
				pb.index = atoi(ptr+1);

				fcomma = strchr(buf, ',');
				lcomma = strchr(fcomma+1, ',');
				strncpy(pb.numb, fcomma+1, (lcomma-fcomma-1));
				pb.numb[(lcomma-fcomma-1)] = '\0';				
				if ( '+' == pb.numb[0] )
					pb.type = LGSM_PB_ATYPE_INTL;
				else 
					pb.type = LGSM_PB_ATYPE_OTHE;				
				strncpy(pb.text, lcomma+1, strlen(lcomma+1));
				pb.text[strlen(lcomma+1)] = '\0';
				
				lgsmd_pb_write_entry(lgsmh, &pb);
			} else if ( !strncmp(buf, "ps", 2)) {	
				printf("Get Phonebook Support\n");
				lgsm_pb_get_support(lgsmh);				
			} else if ( !strncmp(buf, "sd", 2)) {		
				printf("Delete SMS\n");			
				struct lgsm_sms_delete sms_del;

				ptr = strchr(buf, '=');
				sms_del.index = atoi(ptr+1);
				ptr = strchr(buf, ',');
				sms_del.delflg = atoi(ptr+1);	
			
				lgsmd_sms_delete(lgsmh, &sms_del);				
			} else if ( !strncmp(buf, "sl", 2)) {		
				printf("List SMS\n");	
				ptr = strchr(buf, '=');	
					
				lgsm_sms_list(lgsmh, atoi(ptr+1));			
			} else if ( !strncmp(buf, "sr", 2)) {				
				printf("Read SMS\n");	
				ptr = strchr(buf, '=');	
					
				lgsm_sms_read(lgsmh, atoi(ptr+1));				
			} else if ( !strncmp(buf, "ss", 2)) {
				printf("Send SMS\n");		
				struct lgsm_sms sms;

				ptr = strchr(buf, '=');
				fcomma = strchr(buf, ',');
				strncpy(sms.addr, ptr+1, fcomma-ptr-1);
				sms.addr[fcomma-ptr-1] = '\0';
				packing_7bit_character(fcomma+1, &sms);

				lgsmd_sms_send(lgsmh, &sms);
			} else if ( !strncmp(buf, "sw", 2)) {	
				printf("Write SMS\n");				
				struct lgsm_sms_write sms_write;

				ptr = strchr(buf, '=');
				sms_write.stat = atoi(ptr+1);
				fcomma = strchr(buf, ',');
				lcomma = strchr(fcomma+1, ',');
				strncpy(sms_write.sms.addr,
						fcomma+1, lcomma-fcomma-1);
				sms_write.sms.addr[lcomma-fcomma-1] = '\0';
				packing_7bit_character(
						lcomma+1, &sms_write.sms);

				lgsmd_sms_write(lgsmh, &sms_write);
			} else {
				printf("Unknown command `%s'\n", buf);
			}
		}
		fflush(stdout);
	}
}
