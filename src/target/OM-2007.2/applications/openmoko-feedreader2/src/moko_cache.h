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
#ifndef OPENMOKO_RSS_MOKO_CACHE_H
#define OPENMOKO_RSS_MOKO_CACHE_H


#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_CACHE             (moko_cache_get_type())
#define MOKO_CACHE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), MOKO_TYPE_CACHE, MokoCache))
#define MOKO_CACHE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),  MOKO_TYPE_CACHE, MokoCacheClass))
#define MOKO_IS_CACHE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), MOKO_TYPE_CACHE))
#define MOKO_IS_CACHE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),  MOKO_TYPE_CACHE))
#define MOKO_CACHE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),  MOKO_TYPE_CACHE, MokoCacheClass))

typedef struct _MokoCache MokoCache;
typedef struct _MokoCacheClass MokoCacheClass;

struct _MokoCache {
    GObject parent;

    gchar *cache_name;
};

struct _MokoCacheClass {
    GObjectClass parent;
};

enum MokoCacheAllowedSize {
    MOKO_CACHE_UNLIMITED = 0 
};

enum MokoCacheWriteResult {
    MOKO_CACHE_WRITE_SUCCESS,
    MOKO_CACHE_WRITE_OUT_OF_SPACE,
    MOKO_CACHE_WRITE_LIMIT_EXCEEDED,
    MOKO_CACHE_WRITE_UNKNOWN_ERROR
};

GType         moko_cache_get_type (void);
GObject*      moko_cache_new (gchar *cache_policy_name);
gint          moko_cache_get_allowed_size (MokoCache *self);
gint          moko_cache_get_utilized_size(MokoCache *self);

gint          moko_cache_write_object (MokoCache *self, gchar *object_name, gchar *content, gsize size, GError** error);
gchar*        moko_cache_read_object  (MokoCache *self, gchar *object_name, gsize *size);

/**
 * \fn moko_cache_new(gchar *cache_policy_name)
 * @param cache_policy_name Name of the policy, e.g. "openmoko-feedreader2"
 * 
 * The policy defines the storage path and the amount of storage available
 */

/**
 * \fn moko_cache_get_allowed_size (MokoCache* self)
 *
 * @return This method returns the number of bytes this object
 * may safe. As a special case MOKO_CACHE_UNLIMITED might be returned.
 */

/**
 * \fn moko_cache_get_utilized_size (MokoCache *self)
 *
 * @return Return the size currently used.
 */

/**
 * \fn moko_cache_write_object (MokoCache *self, gchar *object_name, gchar *content, guint size, GError** error);
 * @param object_name The name of the object. E.g. http://www.heise.de/atom.xml
 * @param content     The actual content to be written to the cache
 * @param size        The size of the content. If it is -1 strlen will be used to determine the size
 * @param error       The error containing a nice string.
 * @return one of the MokoCacheWriteResult
 */

/**
 * \fn moko_cache_read_object (MokoCache *self, gchar *object_name, gint *size)
 * @param object_name The name of the object to be retrieved
 * @param size        Out parameter. The size of the object will be returned
 *
 * @return NULL in case of error, otherwise a pointer to the object. You are responsible
 * to delete it! size can be -1 in case of error or >= 0.
 */

G_END_DECLS

#endif
