/**********************************************************************
 * open all input nodes, listen for and show events
 * Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 * (C) 2007 OpenMoko Inc.
 * GPLv2
 **********************************************************************/

#include "input.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <glib-2.0/glib.h>

#define MAX_INPUT_NODE 9

int main( int argc, char** argv )
{
    fd_set fdset;
    int fd[MAX_INPUT_NODE];
    int highestfd = -1;
    int maxfd = -1;
    for ( int i = 0; i < MAX_INPUT_NODE+1; ++i )
    {
        gchar event[7] = "event\0\0";
        event[5] = 0x30 + i;
        gchar* filename = g_build_filename( "/dev/input", &event, NULL );
        g_debug( "opening '%s'", filename );
        int result = open( filename, O_RDONLY );
        if ( result != -1 )
        {
            fd[i] = result;
            maxfd++;
            g_debug( "%s open ok fd = %d", filename, result );
            if ( result > highestfd )
                highestfd = result;
        }
        else
        {
            g_debug( "can't open %s (%s)", filename, strerror(errno) );
            break;
        }
        g_free( filename );
    }
    if ( maxfd == -1 )
    {
        g_debug( "can't open ANY input events -- aborting." );
        return -1;
    }

    while ( TRUE )
    {
        FD_ZERO(&fdset);
        for ( int i = 0; i < maxfd+1; ++i )
            FD_SET( fd[i], &fdset );
        int result = select( highestfd+1, &fdset, NULL, NULL, NULL );
        g_debug( "select returned %d", result );
        if ( result == 1 )
        {
            struct input_event event;
            for( int i = 0; i < maxfd+1; ++i )
            {
                if ( FD_ISSET( fd[i], &fdset ) )
                {
                    int size = read( fd[i], &event, sizeof( struct input_event ) );
                    g_debug( "read %d bytes from fd %d", size, fd[i] );
                    g_debug( "input event = ( %0x, %0x, %0x )", event.type, event.code, event.value );
                }
            }
        }
    }

    return 0;
}
