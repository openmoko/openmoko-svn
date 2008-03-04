#ifndef _LIBGSMD_PBOOK_H
#define _LIBGSMD_PBOOK_H

#include <libgsmd/libgsmd.h>

/* Phonebook */

/* Chapter 8.11 */
enum lgsm_pbook_type {
	LGSM_PB_ME_DIALLED		= 1,
	LGSM_PB_SIM_EMERGENCY		= 2,
	LGSM_PB_SIM_FIXDIAL		= 3,
	LGSM_PB_SIM_DIALLED		= 4,
	LGSM_PB_ME_MISSED		= 5,
	LGSM_PB_ME_PHONEBOOK		= 6,
	LGSM_PB_COMB_PHONEBOOK		= 7,
	LGSM_PB_SIM_OWN_NUMBERS		= 8,
	LGSM_PB_ME_RECEIVED		= 9,
	LGSM_PB_SIM_PHONEBOOK		= 10,
	LGSM_PB_TA_PHONEBOOK		= 11,
};

/* Refer to GSM 07.07 subclause 8.14 */
enum lgsm_pb_addr_type {	
	LGSM_PB_ATYPE_INTL		= 145,
	LGSM_PB_ATYPE_OTHE		= 129,
};

/* Refer to GSM 07.07 subclause 8.12 */
struct lgsm_phonebook_readrg {
	int index1;
	int index2;
};

/* Refer to GSM 07.07 subclause 8.14 */
/* FIXME: the nlength and tlength depend on SIM, use +CPBR=? to get */ 
#define	LGSM_PB_NUMB_MAXLEN	44
#define LGSM_PB_TEXT_MAXLEN	14
struct lgsm_phonebook {
	int index;
	char numb[LGSM_PB_NUMB_MAXLEN+1];
	enum lgsm_pb_addr_type type;
	char text[LGSM_PB_TEXT_MAXLEN+1];
};

/* Refer to GSM 07.07 subclause 8.13 */
/* FIXME: the tlength depends on SIM, use +CPBR=? to get */ 
struct lgsm_phonebook_find {	
	char findtext[LGSM_PB_TEXT_MAXLEN+1];
};

struct lgsm_pb_entry {
	struct lgsm_pb_entry	*next;
	enum lgsm_pbook_type 	type;
	u_int32_t 		index;
	char 			text[LGSM_PB_TEXT_MAXLEN+1];
};

/* List of supported phonebook memory storage */
extern int lgsm_pb_list_storage(struct lgsm_handle *lh);

/* Select phonebook memory storage */
extern int lgsm_pb_set_storage(struct lgsm_handle *lh, char *storage);

/* Find phonebook entires which alphanumeric filed start 
 * with string <findtext> */
extern int lgsm_pb_find_entry(struct lgsm_handle *lh, 
		const struct lgsm_phonebook_find *pb_find);

/* Read phonebook entry in location number index */
extern int lgsm_pb_read_entry(struct lgsm_handle *lh, int index);

/* Read phonebook entries in location number range */
extern int lgsm_pb_read_entries(struct lgsm_handle *lh,
		const struct lgsm_phonebook_readrg *pb_readrg);

/* Delete phonebook entry in location index */
extern int lgsm_pb_del_entry(struct lgsm_handle *lh, int index);

/* Write phonebook entry in location */
extern int lgsm_pb_write_entry(struct lgsm_handle *lh,
		const struct lgsm_phonebook *pb);

/* Get the location range/nlength/tlength supported */
extern int lgsm_pb_get_support(struct lgsm_handle *lh);

#endif
