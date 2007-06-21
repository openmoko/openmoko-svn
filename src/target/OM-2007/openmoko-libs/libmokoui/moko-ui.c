#include "moko-ui.h"

void moko_ui_banner_show_text( gint timeout, const gchar* message, ... )
{
    va_list a;
    g_return_if_fail( timeout ); // don't allow permanent banners using the simple interface
    va_start( a, message );
    const gchar* string = g_strdup_vprintf( message, a );
    va_end( a );
    moko_banner_show_text( moko_banner_get_instance (), string, timeout );
    g_free( string );
}

