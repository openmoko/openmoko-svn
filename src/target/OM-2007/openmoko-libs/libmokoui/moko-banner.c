/*  moko-banner.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
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
 *  Current Version: $Rev$ ($Date: 2007/04/23 22:27:36 $) [$Author: mickey $]
 */

#include "moko-banner.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoBanner, moko_banner, G_TYPE_OBJECT)

#define BANNER_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_BANNER, MokoBannerPrivate))

typedef struct _MokoBannerPrivate
{
} MokoBannerPrivate;

/* parent class pointer */
static GObjectClass* parent_class = NULL;

/* signals */
/* ... */

/* forward declarations */
/* ... */

static void
moko_banner_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_banner_parent_class)->dispose)
        G_OBJECT_CLASS (moko_banner_parent_class)->dispose (object);
}

static void
moko_banner_finalize(GObject* object)
{
    G_OBJECT_CLASS (moko_banner_parent_class)->finalize (object);
}

static void
moko_banner_class_init(MokoBannerClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoBannerPrivate));

    /* hook destruction */
    object_class->dispose = moko_banner_dispose;
    object_class->finalize = moko_banner_finalize;

    /* register signals */

    /* virtual methods */

    /* install properties */

    /* initialize libxosd and configure defaults */
    klass->osd = xosd_create( 2 );
    if ( !klass->osd )
        g_warning( "Could not create libxosd contect (%s). On Screen Display won't be available", xosd_error );
    else
    {
        xosd_set_font( klass->osd, "-bitstream-bitstream vera sans-*-r-*-*-*-200-*-*" );
        xosd_set_colour( klass->osd, "orange" ); // x11 should gain a moko-orange...
        xosd_set_outline_offset( klass->osd, 2 );
        xosd_set_outline_colour( klass->osd, "black" );
        xosd_set_shadow_offset( klass->osd, 5 );
        xosd_set_pos( klass->osd, XOSD_middle );
        xosd_set_vertical_offset( klass->osd, -100 );
        xosd_set_align( klass->osd, XOSD_center );
        xosd_set_timeout( klass->osd, 2 );
    }
}

MokoBanner*
moko_banner_new(void)
{
    return g_object_new(MOKO_TYPE_BANNER, NULL);
}

static void
moko_banner_init(MokoBanner* self)
{
    /* Populate your instance here */
}

void moko_banner_show_text(MokoBanner* self, const gchar* text, gint timeout)
{
    MokoBannerClass* klass = MOKO_BANNER_GET_CLASS( self );
    g_return_if_fail( klass->osd );
    if ( timeout )
        xosd_set_timeout( klass->osd, timeout );
    xosd_display( klass->osd, 0, XOSD_string, text );
}

void moko_banner_hide(MokoBanner* self)
{
    MokoBannerClass* klass = MOKO_BANNER_GET_CLASS( self );
    g_return_if_fail( klass->osd );
    xosd_hide( klass->osd );
}
