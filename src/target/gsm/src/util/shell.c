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

#ifndef __GSMD__
#define __GSMD__
#include <gsmd/talloc.h>
#undef __GSMD__
#endif

#define STDIN_BUF_SIZE	1024
static int nFIND = 0;
static int nREADRG = 0;

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
	struct gsmd_phonebook_storage *gpst;
	char *payload;
	char *fcomma, *lcomma, *ptr = NULL;
	int *num;
	char buf[128];
	int i;

	switch (gmh->msg_subtype) {
	case GSMD_PHONEBOOK_FIND:		
		num = (int *) ((char *)gmh + sizeof(*gmh));
		printf("Records:%d\n", *num);

		nFIND = *num;
		break;
	case GSMD_PHONEBOOK_READRG:
		num = (int *) ((char *)gmh + sizeof(*gmh));
		printf("Records:%d\n", *num);

		nREADRG = *num;
		break;
	case GSMD_PHONEBOOK_READ:
		gp = (struct gsmd_phonebook *) ((char *)gmh + sizeof(*gmh));
		if (gp->index)
			printf("%d, %s, %d, %s\n",
					gp->index, gp->numb,
					gp->type, gp->text);
		else
			printf("Empty\n");
		break;
	case GSMD_PHONEBOOK_GET_SUPPORT:
		gps = (struct gsmd_phonebook_support *) ((char *)gmh + sizeof(*gmh));
		printf("(1-%d), %d, %d\n", gps->index, gps->nlength, gps->tlength);
		break;

	case GSMD_PHONEBOOK_LIST_STORAGE:
		gpst = (struct gsmd_phonebook_storage *)((char *)gmh + sizeof(*gmh));

		for (i = 0; i < gpst->num; i++) {
			printf("%s, ", gpst->mem[i].type);
		}

		printf("\n");

		break;

	case GSMD_PHONEBOOK_WRITE:
	case GSMD_PHONEBOOK_DELETE:
	case GSMD_PHONEBOOK_SET_STORAGE:
		/* TODO: Need to handle error */
		payload = (char *)gmh + sizeof(*gmh);
		printf("%s\n", payload);
		break;
	case GSMD_PHONEBOOK_RETRIEVE_READRG:
		gp = (struct gsmd_phonebook *) ((char *)gmh + sizeof(*gmh));

		for (i=0; i<nREADRG; i++) {
			printf("%d,%s,%d,%s\n", gp->index, gp->numb, gp->type, gp->text);
			gp++;
		}

		nREADRG = 0;
		break;
	case GSMD_PHONEBOOK_RETRIEVE_FIND:
		gp = (struct gsmd_phonebook *) ((char *)gmh + sizeof(*gmh));

		for (i = 0; i < nFIND; i++) {
			printf("%d,%s,%d,%s\n", gp->index, gp->numb, gp->type, gp->text);
			gp++;
		}

		nFIND = 0;
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
	struct gsmd_addr *addr;
	struct gsmd_sms_storage *mem;
	static const char *msgtype[] = {
		"Unread", "Received", "Unsent", "Sent"
	};
	static const char *memtype[] = {
		"Unknown", "Broadcast", "Me message", "MT", "SIM", "TA", "SR"
	};

	switch (gmh->msg_subtype) {
	case GSMD_SMS_LIST:
	case GSMD_SMS_READ:
		sms = (struct gsmd_sms_list *) ((void *) gmh + sizeof(*gmh));
		printf("%s message %i from/to %s%s, at %i%i-%i%i-%i%i "
				"%i%i:%i%i:%i%i, GMT%c%i\n",
				msgtype[sms->stat], sms->index,
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
	case GSMD_SMS_GET_MSG_STORAGE:
		mem = (struct gsmd_sms_storage *)
			((void *) gmh + sizeof(*gmh));
		printf("mem1: %s (%i)       Occupied: %i / %i\n",
				memtype[mem->mem[0].memtype],
				mem->mem[0].memtype,
				mem->mem[0].used,
				mem->mem[0].total);
		printf("mem2: %s (%i)       Occupied: %i / %i\n",
				memtype[mem->mem[1].memtype],
				mem->mem[1].memtype,
				mem->mem[1].used,
				mem->mem[1].total);
		printf("mem3: %s (%i)       Occupied: %i / %i\n",
				memtype[mem->mem[2].memtype],
				mem->mem[2].memtype,
				mem->mem[2].used,
				mem->mem[2].total);
		break;
	case GSMD_SMS_GET_SERVICE_CENTRE:
		addr = (struct gsmd_addr *) ((void *) gmh + sizeof(*gmh));
		printf("Number of the default Service Centre is %s\n",
				addr->number);
		break;
	default:
		return -EINVAL;
	}
}

/* this is the handler for responses to network/operator commands */
static int net_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	const struct gsmd_signal_quality *sq = (struct gsmd_signal_quality *)
		((void *) gmh + sizeof(*gmh));
	const char *oper = (char *) gmh + sizeof(*gmh);
	const struct gsmd_msg_oper *opers = (struct gsmd_msg_oper *)
		((void *) gmh + sizeof(*gmh));
	const struct gsmd_own_number *num = (struct gsmd_own_number *)
		((void *) gmh + sizeof(*gmh));
	static const char *oper_stat[] = {
		[GSMD_OPER_UNKNOWN] = "of unknown status",
		[GSMD_OPER_AVAILABLE] = "available",
		[GSMD_OPER_CURRENT] = "our current operator",
		[GSMD_OPER_FORBIDDEN] = "forbidden",
	};
	static const char *srvname[] = {
		[GSMD_SERVICE_ASYNC_MODEM] = "asynchronous modem",
		[GSMD_SERVICE_SYNC_MODEM] = "synchronous modem",
		[GSMD_SERVICE_PAD_ACCESS] = "PAD Access (asynchronous)",
		[GSMD_SERVICE_PACKET_ACCESS] = "Packet Access (synchronous)",
		[GSMD_SERVICE_VOICE] = "voice",
		[GSMD_SERVICE_FAX] = "fax",
	};

	switch (gmh->msg_subtype) {
	case GSMD_NETWORK_SIGQ_GET:
		if (sq->rssi == 99)
			printf("Signal undetectable\n");
		else
			printf("Signal quality %i dBm\n", -113 + sq->rssi * 2);
		if (sq->ber == 99)
			printf("Error rate undetectable\n");
		else
			printf("Bit error rate %i\n", sq->ber);
		break;
	case GSMD_NETWORK_OPER_GET:
		if (oper[0])
			printf("Our current operator is %s\n", oper);
		else
			printf("No current operator\n");
		break;
	case GSMD_NETWORK_OPER_LIST:
		for (; !opers->is_last; opers ++)
			printf("%8.*s   %16.*s,   %.*s for short, is %s\n",
					sizeof(opers->opname_num),
					opers->opname_num,
					sizeof(opers->opname_longalpha),
					opers->opname_longalpha,
					sizeof(opers->opname_shortalpha),
					opers->opname_shortalpha,
					oper_stat[opers->stat]);
		break;
	case GSMD_NETWORK_GET_NUMBER:
		printf("\t%s\t%10s%s%s%s\n", num->addr.number, num->name,
				(num->service == GSMD_SERVICE_UNKNOWN) ?
				"" : " related to ",
				(num->service == GSMD_SERVICE_UNKNOWN) ?
				"" : srvname[num->service],
				(num->service == GSMD_SERVICE_UNKNOWN) ?
				"" : " services");
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
		"\tr\tRegister to network\n"
		"\tR\tRegister to given operator (R=number)\n"
		"\tU\tUnregister from netowrk\n"
		"\tP\tPrint current operator\n"
		"\tL\tDetect available operators\n"
		"\tQ\tRead signal quality\n"
		"\tT\tSend DTMF Tone\n"
		"\tn\tPrint subscriber numbers\n"
		"\tpd\tPB Delete (pb=index)\n"
		"\tpr\tPB Read (pr=index)\n"
		"\tprr\tPB Read Range (prr=index1,index2)\n"
		"\tpf\tPB Find (pf=indtext)\n"
		"\tpw\tPB Write (pw=index,number,text)\n"
		"\tps\tPB Support\n"
		"\tpm\tPB Memory\n"
		"\tpp\tPB Set Memory (pp=storage)\n"
		"\tpRr\tRetrieve Readrg Records\n"
		"\tpRf\tRetrieve Find Records\n"
		"\tsd\tSMS Delete (sd=index,delflg)\n"
		"\tsl\tSMS List (sl=stat)\n"
		"\tsr\tSMS Read (sr=index)\n"
		"\tss\tSMS Send (ss=ask_ds,number,text|[\"text\"])\n"
		"\tsw\tSMS Write (sw=stat,number,text)\n"
		"\tsm\tSMS Storage stats\n"
		"\tsM\tSMS Set preferred storage (sM=mem1,mem2,mem3)\n"
		"\tsc\tSMS Show Service Centre\n"
		"\tsC\tSMS Set Service Centre (sC=number)\n"
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
	lgsm_register_handler(lgsmh, GSMD_MSG_NETWORK, &net_msghandler);

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
			} else if (!strcmp(buf, "r")) {
				printf("Register\n");
				lgsm_netreg_register(lgsmh, "\0     ");
			} else if (buf[0] == 'R') {
				printf("Register to operator\n");
				ptr = strchr(buf, '=');
				if (!ptr || strlen(ptr) < 6)
					printf("No.\n");
				else
					lgsm_netreg_register(lgsmh, ptr + 1);
			} else if (!strcmp(buf, "U")) {
				printf("Unregister\n");
				lgsm_netreg_deregister(lgsmh);
			} else if (!strcmp(buf, "P")) {
				printf("Read current opername\n");
				lgsm_oper_get(lgsmh);
			} else if (!strcmp(buf, "L")) {
				printf("List operators\n");
				lgsm_opers_get(lgsmh);
			} else if (!strcmp(buf, "Q")) {
				printf("Signal strength\n");
				lgsm_signal_quality(lgsmh);
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
				lgsm_pb_del_entry(lgsmh, atoi(ptr+1));
			} else if ( !strncmp(buf, "prr", 3)) {	
				printf("Read Phonebook Entries\n");
				struct lgsm_phonebook_readrg pb_readrg;

				ptr = strchr(buf, '=');
				pb_readrg.index1 = atoi(ptr+1);				
				ptr = strchr(buf, ',');
				pb_readrg.index2 = atoi(ptr+1);
				lgsm_pb_read_entries(lgsmh, &pb_readrg);
			} else if ( !strncmp(buf, "pr", 2)) {
				ptr = strchr(buf, '=');
				lgsm_pb_read_entry(lgsmh, atoi(ptr+1));
			} else if ( !strncmp(buf, "pf", 2)) {
				printf("Find Phonebook Entry\n");
				struct lgsm_phonebook_find pb_find;

				ptr = strchr(buf, '=');
				strncpy(pb_find.findtext,
						ptr + 1,
						sizeof(pb_find.findtext) - 1);
				pb_find.findtext[strlen(ptr+1)] = '\0';	
			
				lgsm_pb_find_entry(lgsmh, &pb_find);
			} else if ( !strncmp(buf, "pw", 2)) {
				printf("Write Phonebook Entry\n");
				struct lgsm_phonebook pb;

				ptr = strchr(buf, '=');
				pb.index = atoi(ptr+1);

				fcomma = strchr(buf, ',');
				lcomma = strchr(fcomma+1, ',');
				strncpy(pb.numb, fcomma + 1, (lcomma - fcomma - 1));
				pb.numb[(lcomma - fcomma - 1)] = '\0';
				if ('+' == pb.numb[0])
					pb.type = LGSM_PB_ATYPE_INTL;
				else 
					pb.type = LGSM_PB_ATYPE_OTHE;				
				strncpy(pb.text, lcomma + 1, strlen(lcomma + 1));
				pb.text[strlen(lcomma + 1)] = '\0';

				lgsm_pb_write_entry(lgsmh, &pb);
			} else if ( !strncmp(buf, "pm", 2)) {
				lgsm_pb_list_storage(lgsmh);
			} else if ( !strncmp(buf, "pp", 2)) {
				char storage[3];

				ptr = strchr(buf, '=');
				strncpy(storage, (ptr+1), 2);

				lgsm_pb_set_storage(lgsmh, storage);
			} else if ( !strncmp(buf, "ps", 2)) {	
				printf("Get Phonebook Support\n");
				lgsm_pb_get_support(lgsmh);
			} else if( !strncmp(buf, "pRr", 3) ) {
				printf("Retrieve Readrg Records\n");

				if ( nREADRG )
					lgsm_pb_retrieve_readrg(lgsmh, nREADRG);
			} else if( !strncmp(buf, "pRf", 3) ) {
				printf("Retrieve Find Records\n");

				if ( nFIND )
					lgsm_pb_retrieve_find(lgsmh, nFIND);
			} else if ( !strncmp(buf, "sd", 2)) {		
				printf("Delete SMS\n");			
				struct lgsm_sms_delete sms_del;

				ptr = strchr(buf, '=');
				sms_del.index = atoi(ptr+1);
				ptr = strchr(buf, ',');
				sms_del.delflg = atoi(ptr+1);	
			
				lgsm_sms_delete(lgsmh, &sms_del);
			} else if ( !strncmp(buf, "sl", 2)) {		
				printf("List SMS\n");	
				ptr = strchr(buf, '=');	
					
				lgsm_sms_list(lgsmh, atoi(ptr+1));			
			} else if ( !strncmp(buf, "sr", 2)) {				
				printf("Read SMS\n");	
				ptr = strchr(buf, '=');	
					
				lgsm_sms_read(lgsmh, atoi(ptr+1));				
			} else if ( !strncmp(buf, "ss", 2)) {
				struct lgsm_sms sms;

				ptr = strchr(buf, '=');
				sms.ask_ds = atoi(ptr+1);
				fcomma = strchr(buf, ',');
				lcomma = strchr(fcomma+1, ',');
				strncpy(sms.addr, fcomma+1, lcomma-fcomma-1);
				sms.addr[lcomma-fcomma-1] = '\0';
				/* todo define \" to allow " in text */
				if (lcomma[1]=='"' &&
						!strchr(lcomma+2, '"')) {
						/* read until closing '"' */
						rc = fscanf(stdin, "%[^\"]\"",
							lcomma+strlen(lcomma));
						if (rc == EOF) {
							printf("EOF\n");
							return -1;
						}
						/* remove brackets */
						lcomma++;
						lcomma[strlen(lcomma)] = '\0';
				}
				printf("Send SMS\n");
				packing_7bit_character(lcomma+1, &sms);

				lgsm_sms_send(lgsmh, &sms);
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
				sms_write.sms.ask_ds = 0;

				lgsm_sms_write(lgsmh, &sms_write);
			} else if (!strncmp(buf, "sm", 2)) {
				printf("Get SMS storage preferences\n");
				lgsm_sms_get_storage(lgsmh);
			} else if (!strncmp(buf, "sM", 2)) {
				int mem[3];

				printf("Set SMS storage preferences\n");
				if (sscanf(buf, "sM=%i,%i,%i", mem, mem + 1,
							mem + 2) < 3)
					printf("No.\n");
				else
					lgsm_sms_set_storage(lgsmh, mem[0],
							mem[1], mem[2]);
			} else if (!strncmp(buf, "sc", 2)) {
				printf("Get the default SMSC\n");
				lgsm_sms_get_smsc(lgsmh);
			} else if (!strncmp(buf, "sC", 2)) {
				printf("Set the default SMSC\n");
				ptr = strchr(buf, '=');
				if (!ptr || strlen(ptr) < 6)
					printf("No.\n");
				else
					lgsm_sms_set_smsc(lgsmh, ptr + 1);
			} else if (!strcmp(buf, "n")) {
				lgsm_get_subscriber_num(lgsmh);
			} else {
				printf("Unknown command `%s'\n", buf);
			}
		}
		fflush(stdout);
	}
}
