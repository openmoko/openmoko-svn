
#include "moko-dialer-sms.h"
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include <libgsmd/sms.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>

#include "moko-dialer-sms-glue.h"

G_DEFINE_TYPE (MokoDialerSMS, moko_dialer_sms, G_TYPE_OBJECT)

#define SMS_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_DIALER_TYPE_SMS, MokoDialerSMSPrivate))

typedef struct _MokoDialerSMSPrivate MokoDialerSMSPrivate;

struct _MokoDialerSMSPrivate {
	struct lgsm_handle *handle;
	JanaStore *note_store;
};

enum {
  SENDING,
  SENT,
  REJECTED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
moko_dialer_sms_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_dialer_sms_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_dialer_sms_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (moko_dialer_sms_parent_class)->dispose)
		G_OBJECT_CLASS (moko_dialer_sms_parent_class)->dispose (object);
}

static void
moko_dialer_sms_finalize (GObject *object)
{
	G_OBJECT_CLASS (moko_dialer_sms_parent_class)->finalize (object);
}

static void
moko_dialer_sms_class_init (MokoDialerSMSClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoDialerSMSPrivate));

	object_class->get_property = moko_dialer_sms_get_property;
	object_class->set_property = moko_dialer_sms_set_property;
	object_class->dispose = moko_dialer_sms_dispose;
	object_class->finalize = moko_dialer_sms_finalize;
	
	signals[SENDING] = g_signal_new ("sending", 
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (MokoDialerSMSClass, sending),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	signals[SENT] = g_signal_new ("sent", 
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (MokoDialerSMSClass, sent),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	signals[REJECTED] = g_signal_new ("rejected", 
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (MokoDialerSMSClass, rejected),
		NULL, NULL,
		g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 1, G_TYPE_STRING);
}

MokoDialerSMS *static_self;

static int 
gsmd_eventhandler (struct lgsm_handle *lh, int evt_type,
		   struct gsmd_evt_auxdata *aux)
{
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (static_self);
	
	switch (evt_type) {
	    case GSMD_EVT_IN_SMS : /* Incoming SMS */
		if (aux->u.sms.inlined) {
			gchar *message;

			struct gsmd_sms_list * sms =
				(struct gsmd_sms_list *)aux->data;

			/* Ignore voicemail notifications */
			if (sms->payload.is_voicemail) break;

			message = NULL;
			switch (sms->payload.coding_scheme) {
			    case ALPHABET_DEFAULT :
				message = g_malloc (GSMD_SMS_DATA_MAXLEN);
				unpacking_7bit_character (
					&sms->payload, message);
				break;
			    case ALPHABET_8BIT :
				/* TODO: Verify: Is this encoding just UTF-8? */
				message = g_strdup (sms->payload.data);
				break;
			    case ALPHABET_UCS2 :
				message = g_utf16_to_utf8 ((const gunichar2 *)
					sms->payload.data, sms->payload.length,
					NULL, NULL, NULL);
				break;
			}
			
			/* Store message in the journal */
			if (message) {
				struct lgsm_sms_delete sms_del;
				gchar *author, *recipient;
				JanaNote *note = jana_ecal_note_new ();
				
				author = g_strconcat (((sms->addr.type &
					__GSMD_TOA_TON_MASK) ==
					GSMD_TOA_TON_INTERNATIONAL) ? "+" : "",
					sms->addr.number, NULL);
				jana_note_set_author (note, author);
				g_free (author);
				
				/* TODO: Normalise number necessary? */
				recipient = g_strdup_printf ("%d",
					lgsm_get_subscriber_num (priv->handle));
				jana_note_set_recipient (note, recipient);
				g_free (recipient);
				
				jana_note_set_body (note, message);
				
				/* TODO: Set creation time from SMS timestamp */
				
				/* Add SMS to store */
				jana_store_add_component (priv->note_store,
					JANA_COMPONENT (note));
				
				/* Delete SMS from internal storage */
				sms_del.index = sms->index;
				sms_del.delflg = LGSM_SMS_DELFLG_INDEX;
				lgsm_sms_delete (priv->handle, &sms_del);
			}
		} else {
		}
		break;
	    case GSMD_EVT_IN_DS : /* SMS status report */
		break;
	    default :
		g_warning ("Unhandled gsmd event (%d)", evt_type);
	}
	
	return 0;
}

static void
moko_dialer_sms_init (MokoDialerSMS *self)
{
	static gboolean first_init = TRUE;
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (self);
	
	/* We can only have one of these objects per process, as the gsmd 
	 * event handling callback does not allow for custom data...
	 */
	if (!first_init)
		g_error ("MokoDialerSMS already created in this process");
	first_init = FALSE;
	static_self = self;
	
	/* Get the note store */
	priv->note_store = jana_ecal_store_new (JANA_COMPONENT_NOTE);
	
	/* Initialise gsmd and connect event handler */
	if (!(priv->handle = lgsm_init (LGSMD_DEVICE_GSMD))) {
		g_warning ("Failed to connect to gsmd, signals won't work");
	} else {
		lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_SMS,
			gsmd_eventhandler);
		lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_DS,
			gsmd_eventhandler);
	}
}

MokoDialerSMS*
moko_dialer_sms_new (void)
{
	return g_object_new (MOKO_DIALER_TYPE_SMS, NULL);
}

gboolean
moko_dialer_sms_send (MokoDialerSMS *sms, const gchar *number,
		      const gchar *message, gboolean ascii, GError **error)
{
}

void
moko_dialer_sms_sending (MokoDialerSMS *sms)
{
	g_signal_emit (sms, signals[SENDING], 0);
}

void
moko_dialer_sms_sent (MokoDialerSMS *sms)
{
	g_signal_emit (sms, signals[SENT], 0);
}

void
moko_dialer_sms_rejected (MokoDialerSMS *sms, const gchar *message)
{
	g_signal_emit (sms, signals[REJECTED], 0, message);
}

