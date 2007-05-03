/*
 *  RSS Reader, a simple RSS reader
 *
 *  A simple secondory/tertiary caching solution to limit the amount of data
 *  stored. This is meant to be pushed to the mokocore library.
 *
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "moko_cache.h"

#include <glib.h>
#include <glib/gstdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define CACHE_NAME ".moko-cache"

G_DEFINE_TYPE(MokoCache, moko_cache, G_TYPE_OBJECT)

#define MOKO_CACHE_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE((obj), MOKO_TYPE_CACHE, MokoCachePrivate))

typedef struct _MokoCachePrivate MokoCachePrivate;
struct _MokoCachePrivate {
    gint allowed_size;
};

/*
 * Create the dirs
 */
static void
moko_cache_create_dirs (gchar *cache_name)
{
    gchar *path = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), CACHE_NAME, NULL);
    if (g_file_test (path, G_FILE_TEST_EXISTS))
        g_mkdir (path, 0700);
    g_free (path);

    path = g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), CACHE_NAME, cache_name, NULL);
    if (g_file_test (path, G_FILE_TEST_EXISTS))
        g_mkdir (path, 0700);
    g_free (path);
    
}

/*
 * return a path to the cache directory
 */
static gchar*
moko_cache_create_path (gchar *cache_name, gchar *file_name)
{
    return g_build_path (G_DIR_SEPARATOR_S, g_get_home_dir (), CACHE_NAME, cache_name, file_name, NULL);
}

/*
 * The secret of this is to create a valid file name. This can be either a hash or
 * an escaped method
 */
static gchar*
object_name_to_file_name (gchar *file_name)
{
    gchar *result = g_strdup (file_name);
    const int l = strlen(result);
    for (int i = 0; i < l; ++i)
        if ( result[i] == '/' || result[i] == ':' || result[i] == '.' )
            result[i] = '_';

    return result;
}

static void
moko_cache_finalize (GObject *object)
{
    MokoCache *cache = MOKO_CACHE (object);
    g_free (cache->cache_name);
    cache->cache_name = NULL;

    G_OBJECT_CLASS(moko_cache_parent_class)->finalize (object);
}

static void
moko_cache_class_init (MokoCacheClass *klass)
{
    g_type_class_add_private (klass, sizeof (MokoCachePrivate));

    /*
     * virtual functions
     */
    G_OBJECT_CLASS(klass)->finalize = moko_cache_finalize;
}

static void
moko_cache_init (MokoCache *self)
{
    self->cache_name = NULL;
}

GObject*
moko_cache_new (gchar *name)
{
    MokoCache *cache = MOKO_CACHE(g_object_new (MOKO_TYPE_CACHE, NULL));
    cache->cache_name = g_strdup(name);


    return G_OBJECT(cache);
}

gint
moko_cache_get_allowed_size (MokoCache *self)
{
    return MOKO_CACHE_UNLIMITED;
}

gint
moko_cache_get_utilized_size(MokoCache *self)
{
    return 0;
}

gint
moko_cache_write_object (MokoCache *self, gchar *object_name, gchar *content, gsize size)
{
    int error = MOKO_CACHE_WRITE_SUCCESS;
    size = size == -1 ? strlen(content) : size;
    
    moko_cache_create_dirs (self->cache_name);
    gchar *file_name = object_name_to_file_name (object_name);
    gchar *path = moko_cache_create_path (self->cache_name, file_name);

    int fd = g_open (path, O_WRONLY|O_TRUNC, 0700);
    if ( fd < 0 ) {
        error = MOKO_CACHE_WRITE_UNKNOWN_ERROR;
        goto error_path;
    }

    if ( write (fd, content, size) < 0 ) {
        error = MOKO_CACHE_WRITE_UNKNOWN_ERROR;
        goto error_path;
    }

error_path:
    g_free (path);
    g_free (file_name);
    return error;
}

gchar*
moko_cache_read_object  (MokoCache *self, gchar *object_name, gsize *size)
{
    gchar *file_name = object_name_to_file_name (object_name);
    gchar *path = moko_cache_create_path (self->cache_name, file_name);

    gchar *result = NULL;
    g_file_get_contents (path, &result, size, NULL);
    if (result == NULL && *size == 0 )
        *size = -1;

    g_free (path);
    g_free (file_name);

    return result;
}
