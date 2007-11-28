/*  moko-gsmd-connection.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 */

#include "moko-gsmd-connection.h"
#include "moko-gsmd-marshal.h"

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include <libgsmd/voicecall.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define DEBUG_THIS_FILE
//#undef DEBUG_THIS_FILE

#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...) 
#endif

G_DEFINE_TYPE (MokoGsmdConnection, moko_gsmd_connection, G_TYPE_OBJECT)

#define GSMD_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_GSMD_CONNECTION, MokoGsmdConnectionPrivate))

#define MOKO_GSMD_CHECK_CONNECTION_GET_PRIV \
        g_return_if_fail( MOKO_IS_GSMD_CONNECTION( self ) ); \
        MokoGsmdConnectionPrivate* priv = GSMD_CONNECTION_GET_PRIVATE( self ); \
        if ( !priv->handle ) moko_gsmd_connection_init( self ); \
        g_return_if_fail( priv->handle );

GQuark
moko_gsmd_error_quark ()
{
  static GQuark quark = 0;

  if (quark == 0)
    quark = g_quark_from_static_string ("moko-gsmd-error");
  return quark;
}
#define MOKO_GSMD_ERROR moko_gsmd_error_quark ()

/* ugly temp. hack until libgsmd features a user_data pointer for its callbacks
 * Note that this effectively means you can only have one MokoGsmdConnection 
 * object per process (which should be ok anyway...) :M:
 */
static MokoGsmdConnection* moko_gsmd_connection_instance = 0;

typedef struct _MokoGsmdConnectionSource
{
    GSource source;
    GPollFD pollfd;
    struct lgsm_handle* handle;

} MokoGsmdConnectionSource;

typedef struct _MokoGsmdConnectionPrivate
{
    struct lgsm_handle* handle;
    MokoGsmdConnectionSource* source;

} MokoGsmdConnectionPrivate;

/* signals */
enum {
    SIGNAL_GSMD_EVT_IN_CALL        = 1,  /* Incoming call */
    SIGNAL_GSMD_EVT_IN_SMS         = 2,  /* Incoming SMS */
    SIGNAL_GSMD_EVT_IN_GPRS        = 3,  /* Network initiated GPRS */
    SIGNAL_GSMD_EVT_IN_CLIP        = 4,  /* Incoming CLIP */
    SIGNAL_GSMD_EVT_NETREG         = 5,  /* Network (un)registration event */
    SIGNAL_GSMD_EVT_SIGNAL         = 6,  /* Signal quality event */
    SIGNAL_GSMD_EVT_PIN            = 7,  /* Modem is waiting for some PIN/PUK */
    SIGNAL_GSMD_EVT_OUT_STATUS     = 8,  /* Outgoing call status */
    SIGNAL_GSMD_EVT_OUT_COLP       = 9,  /* Outgoing COLP */
    SIGNAL_GSMD_EVT_CALL_WAIT      = 10, /* Call Waiting */
    SIGNAL_GSMD_EVT_TIMEZONE       = 11, /* Timezone change */
    SIGNAL_GSMD_EVT_SUBSCRIPTIONS  = 12, /* To which events are we subscribed */
    SIGNAL_GSMD_EVT_CIPHER         = 13, /* Ciphering Information */
    SIGNAL_GSMD_EVT_IN_CBM         = 14, /* Incoming Cell Broadcast message */
    SIGNAL_GSMD_EVT_IN_DS          = 15, /* SMS Status Report */
    SIGNAL_GSMD_EVT_IN_ERROR       = 16, /* CME/CMS error */

    SIGNAL_GSMD_NET_CURRENT_OPERATOR = 100, /* Current Operator */

    SIGNAL_GSMD_CONNECTION_STATUS = 200, /* Status of connection to gsmd */

    LAST_SIGNAL,
};
static guint moko_gsmd_connection_signals[LAST_SIGNAL] = { 0 };

/* parent class pointer */
GObjectClass* parent_class = NULL;

/* forward declarations */
static gboolean
moko_gsmd_connection_try_connect(MokoGsmdConnection* self);
static int
_moko_gsmd_connection_eventhandler(struct lgsm_handle *lh, int evt_type, struct gsmd_evt_auxdata *aux);

/* class definition */
static void
moko_gsmd_connection_dispose(GObject* object)
{
    moko_debug( "dispose" );
    MokoGsmdConnectionPrivate* priv;

    priv = GSMD_CONNECTION_GET_PRIVATE( MOKO_GSMD_CONNECTION( object ) );

    if (priv->source)
      g_source_destroy( (GSource*) priv->source );

    if (priv->handle)
      lgsm_exit( priv->handle );

    if (moko_gsmd_connection_instance)
      moko_gsmd_connection_instance = NULL;

    /* call parent destructor */
    if (G_OBJECT_CLASS (moko_gsmd_connection_parent_class)->dispose)
        G_OBJECT_CLASS (moko_gsmd_connection_parent_class)->dispose (object);
}

static void
moko_gsmd_connection_finalize(GObject* object)
{
    moko_debug( "finalize" );
    G_OBJECT_CLASS (moko_gsmd_connection_parent_class)->finalize (object);
}

static void
moko_gsmd_connection_class_init(MokoGsmdConnectionClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoGsmdConnectionPrivate));

    /* hook destruction */
    object_class->dispose = moko_gsmd_connection_dispose;
    object_class->finalize = moko_gsmd_connection_finalize;

    /* register signals */
    moko_gsmd_connection_signals[SIGNAL_GSMD_CONNECTION_STATUS] = g_signal_new
        ("gmsd-connection-status",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, gsmd_connection_status),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__BOOLEAN,
        G_TYPE_NONE,
        1,
        G_TYPE_BOOLEAN,
        NULL );

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_CALL] = g_signal_new
        ("incoming-call",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, incoming_call),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE,
        1,
        G_TYPE_INT,
        NULL );

    /* TODO add SIGNAL_GSMD_EVT_IN_SMS once libgsmd has it */
    /* TODO add SIGNAL_GSMD_EVT_IN_GPRS once libgsmd has it */

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_CLIP] = g_signal_new
        ("incoming-clip",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, incoming_clip),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE,
        1,
        G_TYPE_STRING,
        NULL);

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_NETREG] = g_signal_new
        ("network-registration",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, network_registration),
        NULL,
        NULL,
        moko_gsmd_marshal_VOID__INT_INT_INT,
        G_TYPE_NONE,
        3,
        G_TYPE_INT,
        G_TYPE_INT,
        G_TYPE_INT,
        NULL);

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_SIGNAL] = g_signal_new
        ("signal-strength-changed",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, signal_strength_changed),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE,
        1,
        G_TYPE_INT,
        NULL);

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_PIN] = g_signal_new
        ("pin-requested",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, pin_requested),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE,
        1,
        G_TYPE_INT,
        NULL);

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_OUT_STATUS] = g_signal_new
        ("call-progress",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, call_status_progress),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE,
        1,
        G_TYPE_INT,
        NULL );

    //TODO add SIGNAL_GSMD_EVT_OUT_COLP       = 9, /* Outgoing COLP */
    //TODO add SIGNAL_GSMD_EVT_CALL_WAIT      = 10, /* Call Waiting */
    //TODO add SIGNAL_GSMD_EVT_TIMEZONE       = 11, /* Timezone change */
    //TODO add SIGNAL_GSMD_EVT_SUBSCRIPTIONS  = 12, /* To which events are we subscribed to */
    //TODO add SIGNAL_GSMD_EVT_CIPHER         = 13, /* Chiphering Information */
    //TODO add SIGNAL_GSMD_EVT_IN_CBM         = 14, /* Incoming Cell Broadcast message */
    //TODO add SIGNAL_GSMD_EVT_IN_DS          = 15, /* SMS Status Report */

    moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_ERROR] = g_signal_new
        ("cme-cms-error",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, cme_cms_error ),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE,
        1,
        G_TYPE_INT,
        NULL );

    moko_gsmd_connection_signals[SIGNAL_GSMD_NET_CURRENT_OPERATOR] = g_signal_new
        ("network-current-operator",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        G_STRUCT_OFFSET (MokoGsmdConnectionClass, network_current_operator),
        NULL,
        NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE,
        1,
        G_TYPE_STRING,
        NULL);

    /* virtual methods */

    /* install properties */
}

MokoGsmdConnection*
moko_gsmd_connection_new(void)
{
    return g_object_new(MOKO_TYPE_GSMD_CONNECTION, NULL);
}

static void
moko_gsmd_connection_init(MokoGsmdConnection* self)
{
    moko_debug( "moko_gsmd_connection_init" );
    moko_gsmd_connection_instance = self;

    g_timeout_add_seconds( 5, (GSourceFunc)moko_gsmd_connection_try_connect, self );
}

static gboolean 
_moko_gsmd_connection_source_prepare( GSource* self, 
                                      gint* timeout )
{
    moko_debug( "moko_gsmd_connection_source_prepare" );
    return FALSE;
}

static gboolean 
_moko_gsmd_connection_source_check( GSource* source )
{
    moko_debug( "moko_gsmd_connection_source_check" );
    MokoGsmdConnectionSource *self;

    self = (MokoGsmdConnectionSource*)source;
    return self->pollfd.revents & G_IO_IN;
}

static gboolean 
_moko_gsmd_connection_source_dispatch( GSource *source, 
                                       GSourceFunc callback, 
                                       gpointer data )
{
    moko_debug( "moko_gsmd_connection_source_dispatch" );
    char buf[1025];
    int size;
    MokoGsmdConnectionSource *self;

    self = (MokoGsmdConnectionSource*)source;
   
    size = read( self->pollfd.fd, &buf, sizeof( buf ) );
    if ( size < 0 )
    {
        g_warning( "moko_gsmd_connection_source_dispatch:%s %s", "read error from libgsmd:", strerror( errno ) );
    }
    else
    {
        if ( size == 0 ) /* EOF */
            return FALSE;

        lgsm_handle_packet( self->handle, buf, size );
    }
    return TRUE;
}

static int net_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
    printf( "=================== new code here ==========================\n");

    moko_debug( "moko_gsmd_connection_network_eventhandler type = %d", gmh->msg_subtype );
    MokoGsmdConnection* self = moko_gsmd_connection_instance;

    struct gsmd_msg_auxdata* aux = (struct gsmd_msg_auxdata*) ((void *) gmh + sizeof(*gmh));

    switch ( gmh->msg_subtype )
    {
        case GSMD_NETWORK_SIGQ_GET:
            g_signal_emit( G_OBJECT(self),
                           moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_SIGNAL],
                           0,
                           aux->u.signal.sigq.rssi );
            break;
        case GSMD_NETWORK_OPER_GET:
            g_signal_emit( G_OBJECT(self),
                           moko_gsmd_connection_signals[SIGNAL_GSMD_NET_CURRENT_OPERATOR],
                           0,
                           aux->u.current_operator.name );
            break;
    }
    printf( "=================== old code here ==========================\n");

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

    //printf( "a = %p, b = %p\n", (void*)aux->u.current_operator.name, oper );

    switch (gmh->msg_subtype)
    {
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

int
_moko_gsmd_connection_eventhandler (struct lgsm_handle *lh, int evt_type, struct gsmd_evt_auxdata *aux)
{
    moko_debug( "moko_gsmd_connection_eventhandler type = %d", evt_type );
    MokoGsmdConnection* self = moko_gsmd_connection_instance;

    switch(evt_type)
    {
        case GSMD_EVT_IN_CALL:
            g_signal_emit( G_OBJECT(self), 
                          moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_CALL],
                          0, 
                          aux->u.call.type ); 
            break;
        case GSMD_EVT_IN_SMS:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_SMS];*/
            break;
        case GSMD_EVT_IN_GPRS:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_GPRS]; */ 
            break;
        case GSMD_EVT_IN_CLIP:
            g_signal_emit( G_OBJECT(self), 
                          moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_CLIP],
                          0,
                          aux->u.clip.addr.number );
            break;
        case GSMD_EVT_NETREG:
            g_signal_emit( G_OBJECT(self), 
                          moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_NETREG],
                          0,
                          aux->u.netreg.state, 
                          aux->u.netreg.lac, 
                          aux->u.netreg.ci );
            break;
        case GSMD_EVT_SIGNAL:
            g_signal_emit( G_OBJECT(self), 
                           moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_SIGNAL],
                           0,
                           aux->u.signal.sigq.rssi );
            break;
        case GSMD_EVT_PIN:
            g_signal_emit( G_OBJECT(self), 
                           moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_PIN], 
                           0,
                           aux->u.pin.type );
            break;
        case GSMD_EVT_OUT_STATUS:
            g_signal_emit( G_OBJECT(self),
                moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_OUT_STATUS],
                0, 
                aux->u.call_status.prog );
            break;
        case GSMD_EVT_OUT_COLP:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_OUT_COLP]; */
            break;
        case GSMD_EVT_CALL_WAIT:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_CALL_WAIT]; */
            break;
        case GSMD_EVT_TIMEZONE:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_TIMEZONE]; */
            break;
        case GSMD_EVT_SUBSCRIPTIONS:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_SUBSCRIPTIONS]; */
            break;
        case GSMD_EVT_CIPHER:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_CIPHER]; */
            break;
        case GSMD_EVT_IN_CBM:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_IN_CBM]; */
            break;
        case GSMD_EVT_IN_DS:
            /* moko_gsmd_connection_signals[SIGNAL_GSMD_IN_DS]; */
            break;
        case GSMD_EVT_IN_ERROR:
            g_signal_emit( G_OBJECT(self), 
                           moko_gsmd_connection_signals[SIGNAL_GSMD_EVT_IN_ERROR],
                           0,
                           aux->u.cme_err.number );
            break;
        default:
            g_critical( "_moko_gsmd_connection_eventhandler: %s %d",
                        "unhandled event type =", evt_type );
    }
    return 0;
}

/* this is the handler for receiving passthrough responses */
static int
pt_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
    char *payload = (char *)gmh + sizeof(*gmh);
    g_debug("PASSTHROUGH RESPONSE = '%s'", payload);
    return 0;
}

static gboolean
moko_gsmd_connection_try_connect(MokoGsmdConnection* self)
{
    moko_debug( "moko_gsmd_connection_try_reconnect" );
    MokoGsmdConnectionPrivate* priv = GSMD_CONNECTION_GET_PRIVATE(self);

    priv->handle = lgsm_init( LGSMD_DEVICE_GSMD );
    if ( !priv->handle )
    {
        priv->handle = lgsm_init( LGSMD_DEVICE_GSMD );

        if ( !priv->handle )
        {
            g_warning( "libgsmd: %s",
                       "can't connect to gsmd. You won't receive any events." );

            g_signal_emit( G_OBJECT(self),
                           moko_gsmd_connection_signals[SIGNAL_GSMD_CONNECTION_STATUS],
                           0,
                           FALSE );
            return TRUE; // can't connect, please call me again
        }
    }
    else
    {
        moko_debug( "-- connected to gsmd (socketfd = %d)",
                    lgsm_fd( priv->handle ) );
    }

    static GSourceFuncs funcs = {
        _moko_gsmd_connection_source_prepare,
        _moko_gsmd_connection_source_check,
        _moko_gsmd_connection_source_dispatch,
        NULL,
    };

    priv->source = (MokoGsmdConnectionSource*) g_source_new( &funcs,
                    sizeof( MokoGsmdConnectionSource) );
    priv->source->handle = priv->handle;
    priv->source->pollfd.fd = lgsm_fd( priv->handle );
    priv->source->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
    priv->source->pollfd.revents = 0;
    g_source_add_poll( (GSource*) priv->source, &priv->source->pollfd );
    g_source_attach( (GSource*) priv->source, NULL );

    int rc = 0;
    for ( int i = GSMD_EVT_IN_CALL; i < __NUM_GSMD_EVT; ++i )
    {
        rc |= lgsm_evt_handler_register( priv->handle, i,
                                         _moko_gsmd_connection_eventhandler );
        moko_debug( "-- registered for event %d, return code %d", i, rc );
    }

    lgsm_register_handler( priv->handle, GSMD_MSG_PASSTHROUGH, &pt_msghandler);
    lgsm_register_handler( priv->handle, GSMD_MSG_NETWORK, &net_msghandler);

    g_signal_emit( G_OBJECT(self),
                   moko_gsmd_connection_signals[SIGNAL_GSMD_CONNECTION_STATUS],
                   0,
                   TRUE );

    return FALSE; // connection established, don't call again
}

/* public API */
void 
moko_gsmd_connection_set_antenna_power(MokoGsmdConnection* self, gboolean on, GError **error)
{
    MokoGsmdConnectionPrivate* priv;
    gint result;

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION ( self ) );
    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    if (!priv->handle)
    {
        g_set_error (error, MOKO_GSMD_ERROR, MOKO_GSMD_ERROR_CONNECT, "Error connecting to gsmd");
        return;
    }

    result = lgsm_phone_power( priv->handle, on ? 1 : 0 );

    if (result == -1)
    {
         g_set_error (error, MOKO_GSMD_ERROR, MOKO_GSMD_ERROR_POWER, "Error setting antenna power");
    }
}

void
moko_gsmd_connection_send_pin(MokoGsmdConnection* self, const gchar* pin)
{
    MokoGsmdConnectionPrivate* priv;

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION ( self ) );
    g_return_if_fail( pin );
    g_return_if_fail( strlen( pin ) >= 4 );
    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_if_fail( priv->handle );
    /*
     * FIXME lgsm_pin_auth is not yet implemented, so we call lgsm_pin 
     * directly...
     */
    /*lgsm_pin_auth( priv->handle, pin );*/
    lgsm_pin( priv->handle, 1, pin, NULL);
}

void
moko_gsmd_connection_network_register(MokoGsmdConnection* self)
{
    MokoGsmdConnectionPrivate* priv;

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION ( self ) );
    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_if_fail( priv->handle );

    lgsm_netreg_register( priv->handle, "" );
}

int
moko_gsmd_connection_get_network_status(MokoGsmdConnection* self)
{
    MokoGsmdConnectionPrivate* priv;
    enum lgsm_netreg_state state;

    g_return_val_if_fail (MOKO_IS_GSMD_CONNECTION ( self ), -1);
    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_val_if_fail (priv->handle, -1);

    lgsm_get_netreg_state (priv->handle, &state);

    return state;
}

void
moko_gsmd_connection_voice_accept(MokoGsmdConnection* self)
{
    MokoGsmdConnectionPrivate* priv;

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION ( self ) );
    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_if_fail( priv->handle );

    lgsm_voice_in_accept( priv->handle );
}

void
moko_gsmd_connection_voice_hangup(MokoGsmdConnection* self)
{
    MokoGsmdConnectionPrivate* priv;

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION ( self ) );
    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_if_fail( priv->handle );

    lgsm_voice_hangup( priv->handle );
}

void
moko_gsmd_connection_voice_dial(MokoGsmdConnection* self, const gchar* number)
{
    MokoGsmdConnectionPrivate* priv;
    struct lgsm_addr addr;   

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION (self) );
    g_return_if_fail( number );
    g_return_if_fail( strlen( number ) > 2 );

    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_if_fail( priv->handle );


    addr.type = 129; /* ??? */
    g_stpcpy( &addr.addr[0], number );
    lgsm_voice_out_init( priv->handle, &addr );
}

void
moko_gsmd_connection_voice_dtmf(MokoGsmdConnection* self, const gchar number)
{
    MokoGsmdConnectionPrivate* priv;

    g_return_if_fail ( MOKO_IS_GSMD_CONNECTION (self) );

    priv  = GSMD_CONNECTION_GET_PRIVATE( self );

    g_return_if_fail( priv->handle );

    lgsm_voice_dtmf( priv->handle, number );
}

void
moko_gsmd_connection_trigger_signal_strength_event(MokoGsmdConnection* self)
{
    MOKO_GSMD_CHECK_CONNECTION_GET_PRIV
    lgsm_signal_quality( priv->handle );
}

void
moko_gsmd_connection_trigger_current_operator_event(MokoGsmdConnection* self)
{
    MOKO_GSMD_CHECK_CONNECTION_GET_PRIV
    lgsm_oper_get( priv->handle );
}
