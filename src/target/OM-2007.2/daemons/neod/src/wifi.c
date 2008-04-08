/*
 *  Authored by Rob Bradford <rob@o-hand.com>
 *  Copyright (C) 2008 OpenMoko, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 */


#include "wifi.h"

gboolean
wifi_radio_is_on (const gchar *iface)
{
  struct iwreq wrq;
  int sock = 0; /* socket */

  /* Open socket to perform ioctl() on */
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (!sock)
  {
    g_warning ("Unable to open socket: %s", g_strerror (errno));
    return FALSE;
  }

  /* Clear our request and set the interface name */
  memset (&wrq, 0, sizeof (struct iwreq));
  strncpy ((char *)&wrq.ifr_name, iface, IFNAMSIZ);

  /* Feel the power, uhh, do the ioctl() */
  if (ioctl (sock, SIOCGIWTXPOW, &wrq) != 0)
  {
    g_warning ("Error performing ioctl: %s", g_strerror (errno));
    close (sock);
    return FALSE;
  }

  close (sock);

  return !wrq.u.txpower.disabled;
}

gboolean
wifi_radio_control (const gchar *iface, gboolean enable)
{
  struct iwreq wrq;
  int sock = 0; /* socket */

  /* Open socket to perform ioctl() on */
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (!sock)
  {
    g_warning ("Unable to open socket: %s", g_strerror (errno));
    return FALSE;
  }

  /* Clear our request and set the interface name */
  memset (&wrq, 0, sizeof (struct iwreq));

  strncpy ((char *)&wrq.ifr_name, iface, IFNAMSIZ);

  /* Feel the power, uhh, do the ioctl() */
  if (ioctl (sock, SIOCGIWTXPOW, &wrq) != 0)
  {
    g_warning ("Error performing ioctl: %s", g_strerror (errno));
    close (sock);
    return FALSE;
  }

  wrq.u.txpower.disabled = !enable;

  /* Feel the power, uhh, do the ioctl() */
  if (ioctl (sock, SIOCSIWTXPOW, &wrq) != 0)
  {
    g_warning ("Error performing ioctl: %s", g_strerror (errno));
    close (sock);
    return FALSE;
  }

  close (sock);

  return TRUE;
}
