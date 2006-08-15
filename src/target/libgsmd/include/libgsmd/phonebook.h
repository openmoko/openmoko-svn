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

/* Get a bitmask of supported phonebook types */
extern int lgsm_pb_get_types(struct lgsm_handle *lh, u_int32 *typemask);

/* Get a range of supported indexes in given phonebook type, Chapter 8.12 */
extern int lgsm_pb_get_range(struct lgsm_handle *lh,
			     enum lgsm_pbook_type type,
			     u_int32_t *from, u_int32_t *to,
			     u_int32_t *nlength, *u_int32_t tlength);

#define LGSM_PB_TEXT_MAXLEN	31

struct lgsm_pb_entry {
	struct lgsm_pb_entry	*next;
	enum lgsm_pbook_type 	type;
	u_int32_t 		index;
	char 			text[LGSM_PB_TEXT_MAXLEN+1];
};

/* Get a specific phonebook entry  and store it to 'pb'
 * pb' is caller-allocated */
extern int lgsm_pb_get_entry(struct lgsm_handle *lh,
			     struct lgsm_pb_entry *pb);

/* Store a specific phonebook entry 'pb' into phone */
extern int lgsm_pb_set_entry(struct lgsm_handle *lh,
			     struct lgsm_pb_entry *pb);


#endif
