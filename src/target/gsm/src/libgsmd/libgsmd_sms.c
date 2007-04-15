#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include <libgsmd/sms.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

int lgsm_sms_list(struct lgsm_handle *lh, enum gsmd_msg_sms_type stat)
{
	/* FIXME: only support PDU mode now */
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_LIST, sizeof(int));
	if (!gmh)
		return -ENOMEM;
	*(int *) gmh->data = stat;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_sms_read(struct lgsm_handle *lh, int index)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_READ, sizeof(int));
	if (!gmh)
		return -ENOMEM;
	*(int *) gmh->data = index;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsmd_sms_delete(struct lgsm_handle *lh, 
		const struct lgsm_sms_delete *sms_del)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_sms_delete *gsd;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_DELETE, sizeof(*gsd));
	if (!gmh)
		return -ENOMEM;
	gsd = (struct gsmd_sms_delete *) gmh->data;
	gsd->index = sms_del->index;
	gsd->delflg = sms_del->delflg;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsmd_sms_send(struct lgsm_handle *lh, 
		const struct lgsm_sms *sms)   
{
	/* FIXME: only support PDU mode */
	struct gsmd_msg_hdr *gmh;
	struct gsmd_sms *gs;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_SEND, sizeof(*gs));
	if (!gmh)
		return -ENOMEM;
	gs = (struct gsmd_sms *) gmh->data;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsmd_sms_write(struct lgsm_handle *lh, 
		const struct lgsm_sms_write *sms_write)
{
	/* FIXME: only support PDU mode */
	struct gsmd_msg_hdr *gmh;
	struct gsmd_sms_write *gsw; 
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_WRITE, sizeof(*gsw));
	if (!gmh)
		return -ENOMEM;
	gsw = (struct gsmd_sms_write *) gmh->data;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int packing_7bit_character(char *src, char *dest)
{
	int i,j = 0;
	unsigned char ch1, ch2;
	char tmp[2];
	int shift = 0;
	
	*dest = '\0';

	for ( i=0; i<strlen(src); i++ ) {
		
		ch1 = src[i] & 0x7F;
		ch1 = ch1 >> shift;
		ch2 = src[(i+1)] & 0x7F;
		ch2 = ch2 << (7-shift); 

		ch1 = ch1 | ch2;
		
		j = strlen(dest);
 		sprintf(tmp, "%x", (ch1 >> 4));	
		dest[j++] = tmp[0];
		sprintf(tmp, "%x", (ch1 & 0x0F));
		dest[j++] = tmp[0];		
		dest[j++] = '\0';		
			
		shift++;
		
		if ( 7 == shift ) {
			shift = 0;
			i++;
		}
	}			
	
	return 0;
}

int unpacking_7bit_character(char *src, char *dest)
{
        unsigned char ch1, ch2 = '\0';
        int i, j;
        char buf[8];
        int shift = 0;

        *dest = '\0';

        for ( i=0; i<strlen(src); i+=2 ) {
                sprintf(buf, "%c%c", src[i], src[i+1]);
                ch1 = strtol(buf, NULL, 16);

                j = strlen(dest);
                dest[j++] = ((ch1 & (0x7F >> shift)) << shift) | ch2;
                dest[j++] = '\0';

                ch2 = ch1 >> (7-shift);

                shift++;
        }

        return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int packing_UCS2_80(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int unpacking_UCS2_80(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int packing_UCS2_81(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int unpacking_UCS2_81(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int packing_UCS2_82(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int unpacking_UCS2_82(char *src, char *dest)
{
	return 0;
}
