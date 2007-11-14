#ifndef _MOKO_DIALER_SMS_H
#define _MOKO_DIALER_SMS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_DIALER_TYPE_SMS moko_dialer_sms_get_type()

#define MOKO_DIALER_SMS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	MOKO_DIALER_TYPE_SMS, MokoDialerSMS))

#define MOKO_DIALER_SMS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
	MOKO_DIALER_TYPE_SMS, MokoDialerSMSClass))

#define MOKO_DIALER_IS_SMS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	MOKO_DIALER_TYPE_SMS))

#define MOKO_DIALER_IS_SMS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	MOKO_DIALER_TYPE_SMS))

#define MOKO_DIALER_SMS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
	MOKO_DIALER_TYPE_SMS, MokoDialerSMSClass))

#define PHONE_KIT_SMS_ERROR g_quark_from_static_string("phone-kit-sms")

typedef enum {
	PK_SMS_ERROR_NO_TOOLONG,
	PK_SMS_ERROR_MSG_TOOLONG
} PhoneKitSMSError;

typedef struct {
	GObject parent;
} MokoDialerSMS;

typedef struct {
	GObjectClass parent_class;
	
	void	(*sending)	(MokoDialerSMS *sms);
	void	(*sent)		(MokoDialerSMS *sms);
	void	(*rejected)	(MokoDialerSMS *sms, const gchar *message);
} MokoDialerSMSClass;

GType moko_dialer_sms_get_type (void);

MokoDialerSMS * moko_dialer_sms_new (void);

MokoDialerSMS * moko_dialer_sms_get_default (void);

gboolean moko_dialer_sms_send (MokoDialerSMS *sms, const gchar *number,
			       const gchar *message, GError **error);

void moko_dialer_sms_sending (MokoDialerSMS *sms);
void moko_dialer_sms_sent (MokoDialerSMS *sms);
void moko_dialer_sms_rejected (MokoDialerSMS *sms, const gchar *message);

G_END_DECLS

#endif /* _MOKO_DIALER_SMS_H */

