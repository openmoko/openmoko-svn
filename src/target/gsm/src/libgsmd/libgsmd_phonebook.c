#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include <libgsmd/phonebook.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"


int lgsm_pb_list_storage(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_LIST_STORAGE);
}

int lgsm_pb_set_storage(struct lgsm_handle *lh, char *storage)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_SET_STORAGE, 3);

	if (!gmh)
		return -ENOMEM;

	strncpy((char*)gmh->data, storage, 2);
	gmh->data[2] = '\0';

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_pb_find_entry(struct lgsm_handle *lh, 
		const struct lgsm_phonebook_find *pb_find)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_phonebook_find *gpf;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_FIND, sizeof(*gpf));
	if (!gmh)
		return -ENOMEM;
	gpf = (struct gsmd_phonebook_find *)gmh->data;
	memcpy(gpf->findtext, pb_find->findtext, sizeof(gpf->findtext));
	gpf->findtext[sizeof(gpf->findtext)-1] = '\0';

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}


int lgsm_pb_read_entry(struct lgsm_handle *lh, int index)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_READ, sizeof(int));
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

int lgsm_pb_read_entries(struct lgsm_handle *lh,
		const struct lgsm_phonebook_readrg *pb_readrg)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_phonebook_readrg *gpr;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_READRG, sizeof(*gpr));
	if (!gmh)
		return -ENOMEM;
	gpr = (struct gsmd_phonebook_readrg *) gmh->data;
	gpr->index1 = pb_readrg->index1;
	gpr->index2 = pb_readrg->index2;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_pb_del_entry(struct lgsm_handle *lh, int index)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_DELETE, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *)(gmh->data) = index;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_pb_write_entry(struct lgsm_handle *lh,
		const struct lgsm_phonebook *pb)
{
	/* FIXME: only support alphabet now */
	struct gsmd_msg_hdr *gmh;
	struct gsmd_phonebook *gp;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_WRITE, sizeof(*gp));
	if (!gmh)
		return -ENOMEM;
	gp = (struct gsmd_phonebook *) gmh->data;
	gp->index = pb->index;
	memcpy(gp->numb, pb->numb, sizeof(gp->numb));
	gp->numb[sizeof(gp->numb)-1] = '\0';
	gp->type = pb->type;
	memcpy(gp->text, pb->text, sizeof(gp->text));
	gp->text[sizeof(gp->text)-1] = '\0';

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_pb_get_support(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_PHONEBOOK, GSMD_PHONEBOOK_GET_SUPPORT);
}

int lgsm_pb_retrieve_readrg(struct lgsm_handle *lh, int num)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_RETRIEVE_READRG, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *)(gmh->data) = num;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_pb_retrieve_find(struct lgsm_handle *lh, int num)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_PHONEBOOK,
			GSMD_PHONEBOOK_RETRIEVE_FIND, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *)(gmh->data) = num;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_get_imsi(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_PHONEBOOK, GSMD_PHONEBOOK_GET_IMSI);
}

