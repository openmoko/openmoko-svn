
#include "moko-dialer-sms.h"
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>
#include <libgsmd/misc.h>
#include <libgsmd/sms.h>
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <string.h>
#include <unistd.h>

#include "moko-dialer-sms-glue.h"

G_DEFINE_TYPE (MokoDialerSMS, moko_dialer_sms, G_TYPE_OBJECT)

#define SMS_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_DIALER_TYPE_SMS, MokoDialerSMSPrivate))

typedef struct _MokoDialerSMSPrivate MokoDialerSMSPrivate;

typedef struct {
	GSource source;
	GPollFD pollfd;
	struct lgsm_handle *handle;
} MokoDialerSMSSource;

struct _MokoDialerSMSPrivate {
	struct lgsm_handle *handle;
	JanaStore *note_store;
	JanaNote *last_msg;
	MokoDialerSMSSource *source;
};

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
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (object);
	
	g_source_destroy ((GSource *)priv->source);
	lgsm_exit (priv->handle);

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
}

static void
status_report_added_cb (JanaStoreView *view, GList *components, gchar *ref)
{
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (
		moko_dialer_sms_get_default ());

	for (; components; components = components->next) {
		gchar *compref;
		JanaComponent *comp = JANA_COMPONENT (components->data);
		
		compref = jana_component_get_custom_prop (
			comp, "X-PHONEKIT-SMS-REF");
		if (compref && (strcmp (compref, ref) == 0)) {
			jana_utils_component_remove_category (comp, "Sending");
			jana_utils_component_insert_category (comp, "Sent", 0);
			jana_store_modify_component (priv->note_store, comp);
			g_debug ("Setting message status to confirmed sent");
		}
		g_free (ref);
	}
}

static void
status_report_progress_cb (JanaStoreView *view, gint percent, gchar *ref)
{
	if (percent != 100) return;
	
	g_object_unref (view);
	g_free (ref);
}

static void
store_sms (MokoDialerSMS *dialer_sms, struct gsmd_sms_list *sms)
{
	gchar *message;

	MokoDialerSMSPrivate *priv = SMS_PRIVATE (dialer_sms);

	/* Ignore voicemail notifications */
	if (sms->payload.is_voicemail) return;

	/* TODO: Verify type of message for journal (sent/received) - 
	 *       Assuming received for now, as messages sent with phone-kit 
	 *       will be marked already.
	 */
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
		
		g_debug ("Moving message to journal:\n\"%s\"", message);
		
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
		
		g_free (message);
	}
}

static int 
gsmd_eventhandler (struct lgsm_handle *lh, int evt_type,
		   struct gsmd_evt_auxdata *aux)
{
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (
		moko_dialer_sms_get_default ());
	
	/* TODO: Handle events that aren't in-line */
	
	switch (evt_type) {
	    case GSMD_EVT_IN_SMS : /* Incoming SMS */
		/* TODO: Read UDH for multi-part messages */
		g_debug ("Received incoming SMS");
		if (aux->u.sms.inlined) {
			struct gsmd_sms_list * sms =
				(struct gsmd_sms_list *)aux->data;
			store_sms (moko_dialer_sms_get_default (), sms);
		} else {
			lgsm_sms_read (priv->handle, aux->u.sms.index);
		}
		break;
	    case GSMD_EVT_IN_DS : /* SMS status report */
		if (aux->u.ds.inlined) {
			struct gsmd_sms_list *sms =
				(struct gsmd_sms_list *) aux->data;
			
			/* TODO: I'm not entirely sure of the spec when if 
			 *       storing an unsent message means it failed?
			 */
			if (sms->payload.coding_scheme == LGSM_SMS_STO_SENT) {
				gchar *ref = g_strdup_printf ("%d", sms->index);
				JanaStoreView *view = jana_store_get_view (
					priv->note_store);
				g_debug ("Received sent SMS status report");
				jana_store_view_add_match (view,
					JANA_STORE_VIEW_CATEGORY, "Sending");
				g_signal_connect (view, "added", G_CALLBACK (
					status_report_added_cb), ref);
				g_signal_connect (view, "progress", G_CALLBACK (
					status_report_progress_cb), ref);
			}
		} else {
			g_warning ("Not an in-line event, unhandled");
		}
		break;
	    default :
		g_warning ("Unhandled gsmd event (%d)", evt_type);
	}
	
	return 0;
}

static int
sms_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	MokoDialerSMS *sms = moko_dialer_sms_get_default ();
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (sms);

	/* Store sent messages */
	if ((gmh->msg_subtype == GSMD_SMS_SEND) && priv->last_msg) {
		int *result = (int *) ((void *) gmh + sizeof(*gmh));
		gchar *uid = jana_component_get_uid (
			JANA_COMPONENT (priv->last_msg));
		
		if (*result >= 0) {
			gchar *ref = g_strdup_printf ("%d", *result);
			jana_component_set_custom_prop (
				JANA_COMPONENT (priv->last_msg),
				"X-PHONEKIT-SMS-REF", ref);
			g_free (ref);
			g_debug ("Sent message accepted");
		} else {
			g_debug ("Sent message rejected");
			jana_utils_component_remove_category (
				JANA_COMPONENT(priv->last_msg), "Sending");
			jana_utils_component_insert_category (
				JANA_COMPONENT(priv->last_msg), "Rejected", 0);
			/* TODO: Add error codes? 42 = congestion? */
		}
		jana_store_modify_component (priv->note_store,
			JANA_COMPONENT (priv->last_msg));
		
		g_free (uid);
		g_object_unref (priv->last_msg);
		priv->last_msg = NULL;
	} else if ((gmh->msg_subtype == GSMD_SMS_LIST) ||
		   (gmh->msg_subtype == GSMD_SMS_READ)) {
		struct gsmd_sms_list *sms_list = (struct gsmd_sms_list *)
			((void *) gmh + sizeof(*gmh));

		g_debug ("Storing message on SIM");
		store_sms (sms, sms_list);
	}
	
	return 0;
}

static gboolean 
connection_source_prepare (GSource* self, gint* timeout)
{
    return FALSE;
}

static gboolean 
connection_source_check (GSource* source)
{
	MokoDialerSMSSource *self = (MokoDialerSMSSource *)source;
	return self->pollfd.revents & G_IO_IN;
}

static gboolean 
connection_source_dispatch (GSource *source, GSourceFunc callback,
			    gpointer data)
{
	char buf[1025];
	int size;

	MokoDialerSMSSource *self = (MokoDialerSMSSource *)source;

	size = read (self->pollfd.fd, &buf, sizeof(buf));
	if (size < 0) {
		g_warning ("moko_gsmd_connection_source_dispatch:%s %s",
			"read error from libgsmd:", strerror (errno));
	} else {
		if (size == 0) /* EOF */
			return FALSE;
		lgsm_handle_packet (self->handle, buf, size);
	}
	
	return TRUE;
}

static void
opened_cb (JanaStore *store, MokoDialerSMS *self)
{
	static GSourceFuncs funcs = {
		connection_source_prepare,
		connection_source_check,
		connection_source_dispatch,
		NULL,
	};
	
	MokoDialerSMSPrivate *priv = SMS_PRIVATE (self);
	
	g_debug ("Journal opened, connecting to gsmd");

	/* Initialise gsmd and connect event handler */
	if (!(priv->handle = lgsm_init (LGSMD_DEVICE_GSMD))) {
		g_error ("Failed to connect to gsmd");
	} else {
		g_debug ("Connected to gsmd");
		lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_SMS,
			gsmd_eventhandler);
		lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_DS,
			gsmd_eventhandler);
	}
	
	/* Connect SMS message handler (to get sent message references) */
	lgsm_register_handler (priv->handle, GSMD_MSG_SMS, &sms_msghandler);
	
	/* List all messages to move to journal */
	lgsm_sms_list (priv->handle, GSMD_SMS_ALL);
	
	/* Start polling for events */
	priv->source = (MokoDialerSMSSource *)
		g_source_new (&funcs, sizeof (MokoDialerSMSSource));
	priv->source->handle = priv->handle;
	priv->source->pollfd.fd = lgsm_fd (priv->handle);
	priv->source->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
	priv->source->pollfd.revents = 0;
	g_source_add_poll ((GSource*)priv->source, &priv->source->pollfd);
	g_source_attach ((GSource*)priv->source, NULL);
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
	
	/* Get the note store */
	priv->note_store = jana_ecal_store_new (JANA_COMPONENT_NOTE);
	g_signal_connect (priv->note_store, "opened",
		G_CALLBACK (opened_cb), self);
	jana_store_open (priv->note_store);
}

MokoDialerSMS*
moko_dialer_sms_new (void)
{
	return g_object_new (MOKO_DIALER_TYPE_SMS, NULL);
}

MokoDialerSMS*
moko_dialer_sms_get_default (void)
{
	static MokoDialerSMS *sms = NULL;

	if (sms) return sms;

	sms = moko_dialer_sms_new ();

	return sms;
}

gboolean
moko_dialer_sms_send (MokoDialerSMS *self, const gchar *number,
		      const gchar *message, gchar **uid, GError **error)
{
	MokoDialerSMSPrivate *priv;
	struct lgsm_sms sms;
	gint msg_length, c;
	gboolean ascii;
	JanaNote *note;
	gchar *author;
	
	g_assert (self && number && message);

	priv = SMS_PRIVATE (self);
	
	/* Ask for delivery report */
	sms.ask_ds = 1;
	
	/* Set destination number */
	if (strlen (number) > GSMD_ADDR_MAXLEN + 1) {
		*error = g_error_new (PHONE_KIT_SMS_ERROR,
			PK_SMS_ERROR_NO_TOOLONG, "Number too long");
		return FALSE;
	} else {
		strcpy (sms.addr, number);
	}
	
	/* Set message */
	/* Check if the text is ascii (and pack in 7 bits if so) */
	ascii = TRUE;
	for (c = 0; message[c] != '\0'; c++) {
		if (((guint8)message[c]) > 0x7F) {
			ascii = FALSE;
			break;
		}
	}
	
	/* TODO: Multi-part messages using UDH */
	msg_length = strlen (message);
	if ((ascii && (msg_length > 160)) || (msg_length > 140)) {
			*error = g_error_new (PHONE_KIT_SMS_ERROR,
				PK_SMS_ERROR_MSG_TOOLONG, "Message too long");
			return FALSE;
	}
	if (ascii) {
		packing_7bit_character (message, &sms);
	} else {
		sms.alpha = ALPHABET_8BIT;
		sms.length = strlen (message);
		strcpy ((gchar *)sms.data, message);
	}
	
	/* Send message */
	lgsm_sms_send (priv->handle, &sms);
	
	/* Store sent message in journal */
	note = jana_ecal_note_new ();
	jana_note_set_recipient (note, number);
	
	/* TODO: Normalise number necessary? */
	author = g_strdup_printf ("%d",
		lgsm_get_subscriber_num (priv->handle));
	jana_note_set_author (note, author);
	g_free (author);
	
	jana_note_set_body (note, message);
	jana_component_set_categories (JANA_COMPONENT (note),
		(const gchar *[]){ "Sending", NULL});
	
	jana_store_add_component (priv->note_store,
		JANA_COMPONENT (note));
	if (uid) *uid = jana_component_get_uid (JANA_COMPONENT (note));
	
	if (priv->last_msg) {
		g_warning ("Confirmation not received for last sent SMS, "
			"delivery report will be lost.");
		g_object_unref (priv->last_msg);
		priv->last_msg = NULL;
	}
	priv->last_msg = note;
	
	return TRUE;
}
