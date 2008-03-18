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
#include <libgsmd/pin.h>
#include <gsmd/usock.h>
#include <gsmd/ts0705.h>

#ifndef __GSMD__
#define __GSMD__
#include <gsmd/talloc.h>
#undef __GSMD__
#endif

#define STDIN_BUF_SIZE	1024

int pending_responses = 0;

/* this is the handler for receiving passthrough responses */
static int pt_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	char *payload = (char *)gmh + sizeof(*gmh);
	printf("RSTR=`%s'\n", payload);
	return 0;
}

/* this is the handler for receiving phonebook responses */
static int pb_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	struct gsmd_phonebook *gp;
	struct gsmd_phonebooks *gps;
	struct gsmd_phonebook_support *gpsu;
	struct gsmd_phonebook_storage *gpst;
	char *payload;
	int i;

	switch (gmh->msg_subtype) {
	case GSMD_PHONEBOOK_FIND:		
		gps = (struct gsmd_phonebooks *) ((char *)gmh + sizeof(*gmh));

		if (gps->pb.index > 0)
			printf("%d, %s, %d, %s\n",
					gps->pb.index, gps->pb.numb,
					gps->pb.type, gps->pb.text);
		else if (gps->pb.index < 0)
			/* If index < 0, error happens */
			printf("+CME ERROR %d\n", (0-(gps->pb.index)));
		else
			/* The record doesn't exist or could not read yet */
			printf("Doesn't exist or couldn't read it yet\n");

		if (gps->is_last)
			pending_responses --;
		break;
	case GSMD_PHONEBOOK_READRG:
		gps = (struct gsmd_phonebooks *) ((char *)gmh + sizeof(*gmh));

		if (gps->pb.index > 0)
			printf("%d, %s, %d, %s\n",
					gps->pb.index, gps->pb.numb,
					gps->pb.type, gps->pb.text);
		else if (gps->pb.index < 0)
			/* If index < 0, error happens */
			printf("+CME ERROR %d\n", (0-(gps->pb.index)));
		else
			/* The record doesn't exist or could not read yet */
			printf("Doesn't exist or couldn't read it yet\n");

		if (gps->is_last)
			pending_responses --;
		break;
	case GSMD_PHONEBOOK_READ:
		gp = (struct gsmd_phonebook *) ((char *)gmh + sizeof(*gmh));
		if (gp->index > 0)
			printf("%d, %s, %d, %s\n",
					gp->index, gp->numb,
					gp->type, gp->text);
		else if (gp->index < 0)
			/* If index < 0, error happens */
			printf("+CME ERROR %d\n", (0-(gp->index)));
		else
			/* The record doesn't exist or could not read yet */
			printf("Doesn't exist or couldn't read it yet\n");
		break;
	case GSMD_PHONEBOOK_GET_SUPPORT:
		gpsu = (struct gsmd_phonebook_support *) ((char *)gmh + sizeof(*gmh));
		printf("(1-%d), %d, %d\n", gpsu->index, gpsu->nlength, gpsu->tlength);
		pending_responses --;
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
	default:
		return -EINVAL;
	}
	return 0;
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
		if(sms->payload.is_voicemail)
			printf("it's a voicemail \n");
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
		if (sms->is_last)
			pending_responses --;
		break;
	case GSMD_SMS_SEND:
		pending_responses --;
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
		pending_responses --;
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
		pending_responses --;
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
		pending_responses --;
		break;
	case GSMD_SMS_GET_SERVICE_CENTRE:
		addr = (struct gsmd_addr *) ((void *) gmh + sizeof(*gmh));
		printf("Number of the default Service Centre is %s\n",
				addr->number);
		pending_responses --;
		break;
	default:
		pending_responses --;
		return -EINVAL;
	}
	return 0;
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
	const struct gsmd_voicemail *vmail = (struct gsmd_voicemail *)
		((void *) gmh + sizeof(*gmh));
	enum gsmd_netreg_state state = *(enum gsmd_netreg_state *) gmh->data;
	int result = *(int *) gmh->data;
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
		pending_responses --;
		break;
	case GSMD_NETWORK_OPER_GET:
	case GSMD_NETWORK_OPER_N_GET:
		if (oper[0])
			printf("Our current operator is %s\n", oper);
		else
			printf("No current operator\n");
		pending_responses --;
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
		pending_responses --;
		break;
	case GSMD_NETWORK_GET_NUMBER:
		printf("\t%s\t%10s%s%s%s\n", num->addr.number, num->name,
				(num->service == GSMD_SERVICE_UNKNOWN) ?
				"" : " related to ",
				(num->service == GSMD_SERVICE_UNKNOWN) ?
				"" : srvname[num->service],
				(num->service == GSMD_SERVICE_UNKNOWN) ?
				"" : " services");
		pending_responses --;
		break;
	case GSMD_NETWORK_VMAIL_SET:
		if (result)
			printf("Set voicemail error %i\n", result);
		else
			printf("Set voicemail OK \n");
		pending_responses --;
		break;
	case GSMD_NETWORK_VMAIL_GET:
		if(vmail->addr.number)
			printf ("voicemail number is %s \n",vmail->addr.number);
		pending_responses --;
		break;
	case GSMD_NETWORK_QUERY_REG:
		switch (state) {
			case GSMD_NETREG_UNREG:
				printf("not searching for network \n");
				break;
			case GSMD_NETREG_REG_HOME:
				printf("registered (home network) \n");
				break;
			case GSMD_NETREG_UNREG_BUSY:
				printf("searching for network \n");
				break;
			case GSMD_NETREG_DENIED:
				printf("registration denied \n");
				break;
			case GSMD_NETREG_REG_ROAMING:
				printf("registered (roaming) \n");
				break;
			default:
				break;
		}
		pending_responses --;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int phone_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	char *payload  = (char *)gmh + sizeof(*gmh);
	int *intresult = (void *)gmh + sizeof(*gmh);
	const struct gsmd_battery_charge *bc = (struct gsmd_battery_charge *)
		((void *) gmh + sizeof(*gmh));

	switch (gmh->msg_subtype) {
	case GSMD_PHONE_GET_IMSI:
		printf("imsi <%s>\n", payload);
		break;
	case GSMD_PHONE_GET_MANUF:
		printf("manufacturer: %s\n", payload);
		break;
	case GSMD_PHONE_GET_MODEL:
		printf("model: %s\n", payload);
		break;
	case GSMD_PHONE_GET_REVISION:
		printf("revision: %s\n", payload);
		break;
	case GSMD_PHONE_GET_SERIAL:
		printf("serial: %s\n", payload);
		break;
	case GSMD_PHONE_POWERUP:
		if (*intresult)
			printf("Modem power-up failed: %i\n", *intresult);
		else
			printf("Modem powered-up okay\n");
		break;
	case GSMD_PHONE_POWERDOWN:
		if (*intresult)
			printf("Modem power-down failed: %i\n", *intresult);
		else
			printf("Modem down\n");
		break;
	case GSMD_PHONE_POWER_STATUS:
		printf("Antenna Status: %s\n", payload);
		break;
	case GSMD_PHONE_GET_BATTERY:
		printf("<BCS>: %d <BCL>: %d \n", bc->bcs, bc->bcl);
		break;		
	case GSMD_PHONE_VIB_ENABLE:
		if(*intresult)
			printf("Vibrator enable failed: %i\n", *intresult);
		else
			printf("Vibrator enabled\n");
		break;
	case GSMD_PHONE_VIB_DISABLE:
		if(*intresult)
			printf("Vibrator disable failed: %i\n", *intresult);
		else
			printf("VIbrator disabled\n");
		break;
	default:
		return -EINVAL;
	}
	pending_responses --;
	return 0;
}

static int pin_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	int result = *(int *) gmh->data;
	switch (gmh->msg_subtype) {
		case GSMD_PIN_GET_STATUS:
			printf("PIN STATUS: %i\n", result);
			break;
		case GSMD_PIN_INPUT:
			if (result)
				printf("PIN error %i\n", result);
			else
				printf("PIN accepted!\n");
			break;
		default:
			return -EINVAL;	
	}
	pending_responses --;
	return 0;
}

static int call_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	struct gsmd_call_status *gcs;
	struct gsmd_call_fwd_stat *gcfs;
	int *ret;

	switch (gmh->msg_subtype) {
	case GSMD_VOICECALL_GET_STAT:
		gcs = (struct  gsmd_call_status*) ((char *)gmh + sizeof(*gmh));
		
		if (gcs->idx > 0)
			printf("%d, %d, %d, %d, %d, %s, %d\n",
					gcs->idx, gcs->dir,
					gcs->stat, gcs->mode,
					gcs->mpty, gcs->number,
					gcs->type);
		else if (gcs->idx < 0)
			/* If index < 0, error happens */
			printf("+CME ERROR %d\n", (0-(gcs->idx)));
		else
			/* No existing call */
			printf("Doesn't exist\n");

		if (gcs->is_last)
			pending_responses --;
		break;
	case GSMD_VOICECALL_CTRL:
		ret = (int*)((char *)gmh + sizeof(*gmh));
		 (*ret)? printf("+CME ERROR %d\n", *ret) : printf("OK\n");
		pending_responses --;
		break;
	case GSMD_VOICECALL_FWD_DIS:
		pending_responses --;
		break;
	case GSMD_VOICECALL_FWD_EN:
		pending_responses --;
		break;
	case GSMD_VOICECALL_FWD_STAT:
		gcfs = (struct  gsmd_call_fwd_stat*) ((char *)gmh + sizeof(*gmh));
		
		printf("+CCFC:%d, %d, %s\n",gcfs->status, gcfs->classx, gcfs->addr.number);

		if (gcfs->is_last)
			pending_responses --;
		break;
	case GSMD_VOICECALL_FWD_REG:
		pending_responses --;
		break;
	case GSMD_VOICECALL_FWD_ERAS:
		pending_responses --;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static const struct msghandler_s {
	int type;
	lgsm_msg_handler *fn;
} msghandlers[] = {
	{ GSMD_MSG_PASSTHROUGH,	pt_msghandler },
	{ GSMD_MSG_PHONEBOOK,	pb_msghandler },
	{ GSMD_MSG_SMS,		sms_msghandler },
	{ GSMD_MSG_NETWORK,	net_msghandler },
	{ GSMD_MSG_PHONE,	phone_msghandler },
	{ GSMD_MSG_PIN,		pin_msghandler },
	{ GSMD_MSG_VOICECALL,	call_msghandler },

	{ 0, 0 }
};

static void shell_help(void)
{
	printf( "\tA\tAnswer incoming call\n"
		"\tD\tDial outgoing number\n"
		"\tH\tHangup call\n"
		"\tO\tAntenna Power On\n"
		"\to\tAntenna Power Off\n"
		"\tgos\tGet Antenna Status\n"
		"\tV\tVibrator Enable (CVIB=1)\n"
		"\tv\tVibrator Disable (CVIB=0)\n"
	       	"\tM\tModem Power On\n"
		"\tm\tModem Power Off\n"
		"\tr\tRegister to network\n"
		"\tR\tRegister to given operator (R=number)\n"
		"\tU\tUnregister from netowrk\n"
		"\tP\tPrint current operator\n"
		"\tN\tPrint current operator in numeric\n"
		"\tL\tList available operators\n"
		"\tQ\tRead signal quality\n"
		"\tnr\tQuery network registration\n"
		"\tS\tSleep (S[=second], default 5)\n"
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
		"\tsd\tSMS Delete (sd=index,delflg)\n"
		"\tsl\tSMS List (sl=stat)\n"
		"\tsr\tSMS Read (sr=index)\n"
		"\tss\tSMS Send (ss=ask_ds,number,text|[\"text\"])\n"
		"\tsw\tSMS Write (sw=stat,number,text)\n"
		"\tsm\tSMS Storage stats\n"
		"\tsM\tSMS Set preferred storage (sM=mem1,mem2,mem3)\n"
		"\tsc\tSMS Show Service Centre\n"
		"\tsC\tSMS Set Service Centre (sC=number)\n"
		"\tgvm\tGet Voicemail number\n"
		"\tsvm\tSet Voicemail number(svm=number)\n"
		"\tim\tGet imsi\n"
		"\tmf\tGet manufacturer\n"
		"\tml\tGet model\n"
		"\trv\tGet revision\n"
		"\tsn\tGet serial number\n"
		"\tcs\tGet Call status\n"
		"\tgp\tGet PIN status\n"
		"\tcbc\tGet Battery status\n"
		"\tRh\tRelease all held calls (+CHLD=0)\n"
		"\tUDUB\tUser Determined User Busy (+CHLD=0)\n"
		"\tRa\tRelease all active calls (+CHLD=1)\n"
		"\tRx\tRelease specific active call x (Rx=x)(+CHLD=1x)\n"
		"\tHa\tHold all active calls and accept held or waiting call (+CHLD=2)\n"
		"\tHx\tHold all active calls except call x (Hx=x)(+CHLD=2x)\n"
		"\tMP\tAdd a held call to the conversation (+CHLD=3)\n"
		"\tCFD\tDisable call forwarding (CFD=reason)\n"
		"\tCFE\tEnable call forwarding (CFE=reason)\n"
		"\tCFQ\tQuery the status of call forwarding (CFQ=reason)\n"
		"\tCFR\tRegister call forwarding (CFR=reason,number)\n"
		"\tCFe\tErase a record of call forwarding (CFe=reason)\n"
		"\tq\tQuit\n"
		);
}

int shell_main(struct lgsm_handle *lgsmh, int sync)
{
	int rc;
	char buf[STDIN_BUF_SIZE+1];
	fd_set readset;
	char *ptr, *fcomma, *lcomma;
	int gsm_fd = lgsm_fd(lgsmh);
	const struct msghandler_s *hndl;

	for (hndl = msghandlers; hndl->fn; hndl ++)
		lgsm_register_handler(lgsmh, hndl->type, hndl->fn);

	fcntl(0, F_SETFD, O_NONBLOCK);
	fcntl(gsm_fd, F_SETFD, O_NONBLOCK);

	FD_ZERO(&readset);

	printf("# ");

	while (1) {
		fd_set readset;
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
				printf("ERROR reading from gsm_fd\n");
				break;
			}
			rc = lgsm_handle_packet(lgsmh, buf, rc);
			if (rc < 0)
				printf("ERROR processing packet: %d(%s)\n", rc, strerror(-rc));
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
			} else if (!strcmp(buf, "gos")) {
				printf("Get Antenna status\n");
				lgsm_get_power_status(lgsmh);
			} else if (!strcmp(buf, "V")) {
				printf("Vibrator-Enable\n");
				lgsm_phone_vibrator(lgsmh, 1);
			} else if (!strcmp(buf, "v")) {
				printf("Vibrator-Disable\n");
				lgsm_phone_vibrator(lgsmh, 0);
			} else if (!strcmp(buf, "r")) {
				printf("Register\n");
				lgsm_netreg_register(lgsmh, "\0     ");
			} else if (!strcmp(buf,"R")) {
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
				pending_responses ++;
			} else if (!strcmp(buf, "N")) {
				printf("Read current opername in numeric format\n");
				lgsm_oper_n_get(lgsmh);
				pending_responses ++;
			} else if (!strcmp(buf, "L")) {
				printf("List operators\n");
				lgsm_opers_get(lgsmh);
				pending_responses ++;
			} else if (!strcmp(buf, "Q")) {
				printf("Signal strength\n");
				lgsm_signal_quality(lgsmh);
				pending_responses ++;
			} else if (!strcmp(buf, "nr")) {
				printf("Query network registration\n");
				lgsm_netreg_query(lgsmh);
				pending_responses ++;
			} else if (!strcmp(buf, "q")) {
				exit(0);
			} else if (buf[0] == 'S' ) {
				if(!strchr(buf,'=') || atoi((strchr(buf,'=')+1)) < 0) {
					printf("Sleep 5 secs\n");
					sleep(5);
				}else {
					printf("Sleep %d secs\n",atoi(strchr(buf,'=')+1));
					sleep(atoi(strchr(buf,'=')+1));
				}
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
				pending_responses ++;
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
				pending_responses ++;
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
				pending_responses ++;
			} else if ( !strncmp(buf, "sd", 2)) {		
				printf("Delete SMS\n");			
				struct lgsm_sms_delete sms_del;

				ptr = strchr(buf, '=');
				sms_del.index = atoi(ptr+1);
				ptr = strchr(buf, ',');
				sms_del.delflg = atoi(ptr+1);	
			
				lgsm_sms_delete(lgsmh, &sms_del);
				pending_responses ++;
			} else if ( !strncmp(buf, "sl", 2)) {
				printf("List SMS\n");
				ptr = strchr(buf, '=');

				lgsm_sms_list(lgsmh, atoi(ptr+1));
				pending_responses ++;
			} else if ( !strncmp(buf, "sr", 2)) {
				printf("Read SMS\n");
				ptr = strchr(buf, '=');

				lgsm_sms_read(lgsmh, atoi(ptr+1));
				pending_responses ++;
			} else if ( !strncmp(buf, "ss", 2)) {
				struct lgsm_sms sms;

				ptr = strchr(buf, '=');
				sms.ask_ds = atoi(ptr+1);
				fcomma = strchr(buf, ',');
				lcomma = strchr(fcomma+1, ',');
				strncpy(sms.addr, fcomma+1, lcomma-fcomma-1);
				sms.addr[lcomma-fcomma-1] = '\0';
				/* todo define \" to allow " in text */
				if (lcomma[1]=='"' && !strchr(lcomma+2, '"')) {
					/* read until closing '"' */
					rc = fscanf(stdin, "%[^\"]\"", lcomma+strlen(lcomma));
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
				pending_responses ++;
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
				pending_responses ++;
			} else if (!strncmp(buf, "sm", 2)) {
				printf("Get SMS storage preferences\n");
				lgsm_sms_get_storage(lgsmh);
				pending_responses ++;
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
				pending_responses ++;
			} else if (!strncmp(buf, "sC", 2)) {
				printf("Set the default SMSC\n");
				ptr = strchr(buf, '=');
				if (!ptr || strlen(ptr) < 6)
					printf("No.\n");
				else
					lgsm_sms_set_smsc(lgsmh, ptr + 1);
			} else if (!strcmp(buf, "n")) {
				lgsm_get_subscriber_num(lgsmh);
				pending_responses ++;
			} else if ( !strncmp(buf, "gvm", 3)) {
				printf("Get Voicemail Number\n");
				lgsm_voicemail_get(lgsmh);
				pending_responses ++;
			} else if ( !strncmp(buf, "svm", 3)) {
				printf("Set Voicemail Number\n");
				ptr = strchr(buf, '=');
				if (!ptr || strlen(ptr) < 3)
					printf("No.\n");
				else
					lgsm_voicemail_set(lgsmh, ptr + 1);
				pending_responses ++;
			} else if (!strncmp(buf, "im", 2)) {
				printf("Get imsi\n");
				lgsm_get_imsi(lgsmh);
				pending_responses ++;
			} else if (!strncmp(buf, "mf", 2)) {
				printf("Get manufacturer\n");
				lgsm_get_manufacturer(lgsmh);
				pending_responses ++;
			} else if (!strncmp(buf, "ml", 2)) {
				printf("Get model\n");
				lgsm_get_model(lgsmh);
				pending_responses ++;
			} else if (!strncmp(buf, "rv", 2)) {
				printf("Get revision\n");
				lgsm_get_revision(lgsmh);
				pending_responses ++;
			} else if (!strncmp(buf, "sn", 2)) {
				printf("Get serial number\n");
				lgsm_get_serial(lgsmh);
				pending_responses ++;
			} else if ( strlen(buf)==1 && !strncmp(buf, "M", 1)) {
				printf("Modem Power On\n");
				lgsm_modem_power(lgsmh, 1);
				pending_responses ++;
			} else if (!strncmp(buf, "m", 1)) {
				printf("Modem Power Off\n");
				lgsm_modem_power(lgsmh, 0);
				pending_responses ++;
			} else if ( !strncmp(buf, "cs", 2)) {
				printf("List current call status\n");
				lgsm_voice_get_status(lgsmh);
				pending_responses ++;
			} else if ( !strncmp(buf, "gp", 2)) {
				printf("Get PIN status\n");
				lgsm_pin_status(lgsmh);
				pending_responses ++;
			} else if ( !strncmp(buf, "Rh", 2)) {
				struct lgsm_voicecall_ctrl ctrl;	
				ctrl.proc = LGSM_VOICECALL_CTRL_R_HLDS; 
				printf("Release all held calls\n");
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "UDUB", 4)) {
				struct lgsm_voicecall_ctrl ctrl;	
				ctrl.proc = LGSM_VOICECALL_CTRL_UDUB; 
				printf("User Determined User Busy\n");
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "Ra", 2)) {
				struct lgsm_voicecall_ctrl ctrl;	
				ctrl.proc = LGSM_VOICECALL_CTRL_R_ACTS_A_HLD_WAIT; 
				printf("Release all active calls\n");
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "Rx", 2)) {
				struct lgsm_voicecall_ctrl ctrl;	
				ctrl.proc = LGSM_VOICECALL_CTRL_R_ACT_X; 
				printf("Release specific active call x\n");
				ptr = strchr(buf, '=');
				ctrl.idx = atoi(ptr+1);
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "Ha", 2)) {
				struct lgsm_voicecall_ctrl ctrl;	
				ctrl.proc = LGSM_VOICECALL_CTRL_H_ACTS_A_HLD_WAIT; 
				printf("Hold all active calls and accept held or waiting"
				        " call\n");
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "Hx", 2)) {
				struct lgsm_voicecall_ctrl ctrl;	
				ctrl.proc = LGSM_VOICECALL_CTRL_H_ACTS_EXCEPT_X; 
				printf("Hold all active calls except call x\n");
				ptr = strchr(buf, '=');
				ctrl.idx = atoi(ptr+1);
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "MP", 2)) {
				struct lgsm_voicecall_ctrl ctrl;
				ctrl.proc = LGSM_VOICECALL_CTRL_M_HELD; 
				printf("Add a held call to the conversation\n");
				lgsm_voice_ctrl(lgsmh, &ctrl);
				pending_responses ++;
			} else if ( !strncmp(buf, "CFD", 3)) {
				printf("Disable call forwarding\n");
				ptr = strchr(buf, '=');
				lgsm_voice_fwd_disable(lgsmh, atoi(ptr+1));
				pending_responses ++;
			}else if ( !strncmp(buf, "CFE", 3)) {
				printf("Enable call forwarding\n");
				ptr = strchr(buf, '=');
				lgsm_voice_fwd_enable(lgsmh, atoi(ptr+1));
				pending_responses ++;
			}else if ( !strncmp(buf, "CFQ", 3)) {
				printf("Query the status of call forwarding\n");
				ptr = strchr(buf, '=');
				lgsm_voice_fwd_stat(lgsmh, atoi(ptr+1));
				pending_responses ++;
			}else if ( !strncmp(buf, "CFR", 3)) {
				struct lgsm_voicecall_fwd_reg lvfr;
				printf("Register call forwarding\n");
				ptr = strchr(buf, '=');
				lvfr.reason = atoi(ptr+1);
				ptr = strchr(buf, ',');
				strcpy(lvfr.number.addr, ptr+1);
				lgsm_voice_fwd_reg(lgsmh, &lvfr);
				pending_responses ++;
			}else if ( !strncmp(buf, "CFe", 3)) {
				printf("Erase a record of call forwarding\n");
				ptr = strchr(buf, '=');
				lgsm_voice_fwd_erase(lgsmh, atoi(ptr+1));
				pending_responses ++;
			}else if ( !strncmp(buf, "cbc", 3)) {
				printf("Battery Connection status and Battery Charge Level\n");
				lgsm_get_battery(lgsmh);
				pending_responses++;
			}else {
				printf("Unknown command `%s'\n", buf);
			}
		}
		fflush(stdout);
	}
	return 0;
}
