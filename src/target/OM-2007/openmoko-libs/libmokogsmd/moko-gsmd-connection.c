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
 *  Current Version: $Rev$ ($Date: 2006/12/21 18:03:04 $) [$Author: mickey $]
 */

#include "moko-gsmd-connection.h"

#include <libgsmd/libgsmd.h>

#undef DEBUG_THIS_FILE
#define DEBUG_THIS_FILE

#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoGsmdConnection, moko_gsmd_connection, G_TYPE_OBJECT);

#define GSMD_CONNECTION_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_GSMD_CONNECTION, MokoGsmdConnectionPrivate))

typedef struct _MokoGsmdConnectionPrivate
{
    struct lgsm_handle* handle;
    GPollFD fd;
} MokoGsmdConnectionPrivate;

/* parent class pointer */
GObjectClass* parent_class = NULL;

/* forward declarations */
/* ... */

static void
moko_gsmd_connection_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_gsmd_connection_parent_class)->dispose)
        G_OBJECT_CLASS (moko_gsmd_connection_parent_class)->dispose (object);
}

static void
moko_gsmd_connection_finalize(GObject* object)
{
    moko_debug( "finalize" );
    //TODO remove source from mainloop and cleanup w/ libgsmd
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
    moko_debug( "moko_gsmd_connection_init()" );
    MokoGsmdConnectionPrivate* priv = GSMD_CONNECTION_GET_PRIVATE(self);

    priv->handle = lgsm_init( LGSMD_DEVICE_GSMD );
    if ( !priv->handle )
    {
        g_warning( "libgsmd: can't connect to gsmd. You won't receive any events." );
    }
    else
    {
        moko_debug( "-- connected to gsmd (socketfd = %d)", lgsm_fd( priv->handle ) );
    }
}
