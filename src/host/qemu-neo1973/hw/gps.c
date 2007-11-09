/*
 * Very simple GPS chip simulation for QEMU.  This GPS communicates
 * through NMEA sentences, like the U-Blox ANTARIS 4 chipset.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Written by Andrzej Zaborowski.
 *
 * Tricky maths based roughly on "gpsfeed+" Tcl/Tk code by
 * Dimitrios C. Zachariadis.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * http://www.gnu.org/copyleft/gpl.html
 */
#include "vl.h"

#include <math.h>

#define GPS_MAX_SATS	32
#define GPS_DTOR(deg)	((double) deg * M_PI / 180.0 / 3600.0)
#define GPS_RTOD(rad)	((int) (rad * 180.0 / M_PI * 3600.0))
#define GPS_LLTOI(ll)	((int) (ll * 3600.0))

#define M_EPSILON	0.000001

struct gps_data_s {
    int enable;
    CharDriverState chr;
#define FIFO_LEN	4096
    int out_start;
    int out_len;
    char outfifo[FIFO_LEN];
    QEMUTimer *out_tm;
    int64_t baud_delay;

    enum {
        proto_nmea_15 = 0x15,
        proto_nmea_20 = 0x20,
        proto_nmea_23 = 0x23,
        proto_nmea_30 = 0x30,
        proto_ubx,
        proto_rtcm,
    } gps_proto;

    /* Dilution of precision */
    int pdop;
    /* Horizonal dilution of position */
    int hdop;
    /* Vertical dilution of precision */
    int vdop;
    /* Time in seconds since last DGPS update */
    int dgpsdt;
    /* DGPS station ID number */
    int dgpsid;
    /* Altitude, meters aboce mean sea level */
    int altitude;
    char altitude_unit;
    /* Height of geoid (mean sea level) above WGS94 ellipsoid */
    int gheight;
    char gheight_unit;
    /* Bearing */
    int bear;
    int dist;
    /* Ground speed in knots */
    int knots;
    /* Magnetic Variation */
    int magnvar;
    int magnew;
    int magnfix;

    struct gps_pos_s {
        int latitude;
        int longitude;
    } pos, pos_orig;
    void (*pos_update)(struct gps_data_s *s);
    int64_t vmtime_orig;
    int64_t vmtime_last;

    /* Number of satellites that were used for current solution */
    int nsat;
    /* Number of currently visible satellites */
    int maxsat;
    /* Satellite data */
    struct {
        int elevation;
        int azimuth;
        int cur_elevation;
        int cur_azimuth;
        int visible;
        int snr;
        int prn;
    } sats[GPS_MAX_SATS];
    /* Fix quality */
    enum {
        gps_fix_invalid = 0,
        gps_fix_gps,
        gps_fix_dgps,
        gps_fix_pps,
        gps_fix_real_time_kinematic,
        gps_fix_float_rtk,
        gps_fix_estimated_dead_reckon,
        gps_fix_manual_input,
        gps_fix_simulation,
    } fix_q;
    /* Fix type */
    enum {
        gps_type_none = 1,
        gps_type_2d,
        gps_type_3d,
    } fix_type;
    /* Fix timestamp */
    struct tm fix_time;
    char fix;

    int tz_hour;
    int tz_min;

    QEMUTimer *newdata_tm;
    int64_t rate;
};

static inline void gps_fifo_wake(struct gps_data_s *s)
{
    if (!s->enable || !s->out_len)
        return;

    if (qemu_chr_can_read(&s->chr) && s->chr.chr_read) {
        s->chr.chr_read(s->chr.handler_opaque,
                        s->outfifo + s->out_start ++, 1);
        s->out_len --;
        s->out_start &= FIFO_LEN - 1;
    }

    if (s->out_len)
        qemu_mod_timer(s->out_tm, qemu_get_clock(vm_clock) + s->baud_delay);
}

static void gps_write(struct gps_data_s *s, const char *line)
{
    int len, off;

    len = strlen(line);
    if (len + s->out_len > FIFO_LEN) {
        s->out_len = 0;
        return;
    }
    off = (s->out_start + s->out_len) & (FIFO_LEN - 1);
    if (off + len > FIFO_LEN) {
        memcpy(s->outfifo + off, line, FIFO_LEN - off);
        memcpy(s->outfifo, line + (FIFO_LEN - off), off + len - FIFO_LEN);
    } else
        memcpy(s->outfifo + off, line, len);
    s->out_len += len;
    gps_fifo_wake(s);
}

static void gps_fix_reset(struct gps_data_s *s)
{
    s->fix = 'V';
    s->nsat = 0;
    s->fix_q = gps_fix_invalid;
    s->fix_type = gps_type_none;
}

static void gps_fix_acquire(struct gps_data_s *s)
{
    time_t now = time(0);

    gmtime_r(&now, &s->fix_time);
    s->nsat = MIN(s->maxsat, 4);
}

/* Set random initial satellite positions (elevations and azimuths).
 * These are gonna change as the satellites move in orbit.  */
static void gps_satellite_random(struct gps_data_s *s)
{
    int i;

    srand(time(0));
    for (i = 0; i < GPS_MAX_SATS; i ++) {
        s->sats[i].elevation = (rand() % 180) - 90;
        s->sats[i].azimuth = rand() % 360;
        s->sats[i].prn = 2 + i;
    }
}

/* Input parameters are in degrees.  */
static void gps_diopteusis(int lat0, int lon0, int lat1, int lon1,
                int *dist, int *bearing)
{
    double d, b, arc;
    double lat0f = GPS_DTOR(lat0);
    double lon0f = GPS_DTOR(lon0);
    double lat1f = GPS_DTOR(lat1);
    double lon1f = GPS_DTOR(lon1);

    arc = sin(lat0f) * sin(lat1f) +
            cos(lat0f) * cos(lat1f) * cos(lon0f - lon1f);

    /* Find the distance travelled in knots.  */
    d = acos(arc);
    if (!isnormal(d))
        d = 0.0;

    /* Find the bearing.  */

    /* Is lat0 at the pole or very near? */
    if (abs(lat0f) - 0.5 * M_PI < M_EPSILON)
        b = (lat0f > 0.0) ? M_PI : 0.0;
    else if (d > 0.0) {
        b = (sin(lat1f) - sin(lat0f) * cos(d)) / (cos(lat0f) * sin(d));
        if (b > 1.0)
            b = 1.0;
        else if (b < -1.0)
            b = -1.0;
        b = (abs(d) < M_EPSILON) ? 0.0 : acos(b);
    } else
        b = 0.0;

    *bearing = GPS_RTOD(b);
    if (lon0 > lon1)
        *bearing = 360 * 3600 - *bearing;
    *dist = GPS_LLTOI(d);
}

static inline void gps_latlon_split(int ll, const char *letters, char *hemi,
                int *deg, int *min, int *sec)
{
    if (ll > 0) {
        *hemi = letters[0];
        *deg = ll / 3600;
        *min = (ll * 1000 / 60) % (60 * 1000);
        *sec = ll % 60;
    } else {
        *hemi = letters[1];
        *deg = -ll / 3600;
        *min = (-ll * 1000 / 60) % (60 * 1000);
        *sec = -ll % 60;
    }
}

/* Update our position, find the distance D and the angle C,
 * update satellites status.  */
static void gps_makedata(struct gps_data_s *s)
{
    struct gps_pos_s pos0;
    int64_t now = qemu_get_clock(vm_clock);
    int elevation, i;
    int secs = (now - s->vmtime_orig) / ticks_per_sec;

    pos0 = s->pos;
    s->pos_update(s);

    /* Find distance (minutes) and bearing (degrees) */
    gps_diopteusis(pos0.latitude, pos0.longitude,
                    s->pos.latitude, s->pos.longitude, &s->dist, &s->bear);
    s->knots = muldiv64(s->dist * 10, ticks_per_sec, now - s->vmtime_last);
    s->vmtime_last = now;

    /* Count visible satellites */
    s->maxsat = 0;
    /* Do calculations for all our satellites */
    for (i = 0; i < GPS_MAX_SATS; i ++) {
        /* Set the elevation for each satellite */
        elevation = (int) (90.0 * sin(M_PI *
                                (10.0 * s->sats[i].elevation +
                                 secs) / 1800.0));
        /* Arbitrarily, we consider a satellite visible if
         * its elevation is > 30deg.  */
        s->sats[i].visible = (elevation > 30);
        s->maxsat += s->sats[i].visible;

        s->sats[i].cur_elevation = elevation;
        s->sats[i].cur_azimuth = (s->sats[i].azimuth + secs / 10) % 360;
        s->sats[i].snr = (elevation > 5) ? (int) (60.0 * sin(M_PI *
                                (elevation - 30.0) / 180.0)) : 0;
    }

    if (s->maxsat >= 3) {
        if (!s->nsat) {
            /* Record fix timestamp */
            gps_fix_acquire(s);
        }
        s->fix = 'A';
        s->fix_q = gps_fix_gps;
        s->fix_type = gps_type_3d;
    } else
        gps_fix_reset(s);
}

static void gps_append_chksum(char *sentence)
{
    uint8_t reg = 0;

    sentence ++;
    while (*sentence)
        reg ^= *(sentence ++);
    sprintf(sentence, "*%02X\n", reg);
}

static char sentence[1024];

/* RMC: Recommended Minimum sentence C */
static void gps_nmea_gprmc(struct gps_data_s *s)
{
    char lat_hemi, lon_hemi;
    int lat_deg, lon_deg, lat_min, lon_min, lat_sec, lon_sec;

    /* NMEA format:
     * $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
     * ANTARIS4 format:
     * $GPRMC,162254.00,A,3723.02837,N,12159.39853,W,0.820,188.36,110706,,,A*74
     */

    gps_latlon_split(s->pos.latitude, "NS",
                    &lat_hemi, &lat_deg, &lat_min, &lat_sec);
    gps_latlon_split(s->pos.longitude, "EW",
                    &lon_hemi, &lon_deg, &lon_min, &lon_sec);

    sprintf(sentence, "$GPRMC,%02i%02i%02i.00,%c,%02i%02i.%03i,%c,%03i%02i."
                    "%03i,%c,%03i.%i,%03i.%i,%02i%02i%02i,%03i.%i,%c,%c",
                    s->fix_time.tm_hour, s->fix_time.tm_min,
                    s->fix_time.tm_sec, s->fix, lat_deg, lat_min / 1000,
                    lat_min % 1000, lat_hemi, lon_deg, lon_min / 1000,
                    lon_min % 1000, lon_hemi, s->knots / 10, s->knots % 10,
                    s->bear / 10, s->bear % 10, s->fix_time.tm_mday,
                    s->fix_time.tm_mon, s->fix_time.tm_year % 100,
                    s->magnvar / 10, s->magnvar % 10, s->magnew, s->magnfix);
    gps_append_chksum(sentence);
    gps_write(s, sentence);
}

/* GSV: Satellites in View */
static void gps_nmea_gpgsv(struct gps_data_s *s)
{
    int lines = (s->maxsat + 3) >> 2;
    int line = 1;
    int count = 0;
    int i;

    /* NMEA format:
     * $GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
     * ...
     * ANTARIS4 format:
     * $GPGSV,4,1,14,25,15,175,30,14,80,041,,19,38,259,14,01,52,223,18*76
     * ...
     */
    for (i = 0; i < GPS_MAX_SATS; i ++)
        if (s->sats[i].visible) {
            if (count == 0)
                sprintf(sentence, "$GPGSV,%i,%i,%02i",
                                lines, line ++, s->maxsat);

            sprintf(sentence + strlen(sentence), ",%02i,%02i,%03i,%02i",
                            s->sats[i].prn, s->sats[i].cur_elevation,
                            s->sats[i].cur_azimuth, s->sats[i].snr);
            count ++;

            if (count == 4) {
                gps_append_chksum(sentence);
                gps_write(s, sentence);
                count = 0;
            }
        }
    if (count) {
        gps_append_chksum(sentence);
        gps_write(s, sentence);
    }
}

/* GGA: GPS Fix Data */
static void gps_nmea_gpgga(struct gps_data_s *s)
{
    char lat_hemi, lon_hemi;
    int lat_deg, lon_deg, lat_min, lon_min, lat_sec, lon_sec;

    /* NMEA format:
     * $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
     * ANTARIS4 format:
     * $GPGGA,162254.00,3723.02837,N,12159.39853,W,1,03,2.36,525.6,M,-25.6,M,,
     * *65
     *
     * The last two values are only valid when DGPS is in use and are:
     * the time in seconds since last DGPS update and the DGPS station ID.
     */

    gps_latlon_split(s->pos.latitude, "NS",
                    &lat_hemi, &lat_deg, &lat_min, &lat_sec);
    gps_latlon_split(s->pos.longitude, "EW",
                    &lon_hemi, &lon_deg, &lon_min, &lon_sec);

    sprintf(sentence, "$GPGGA,%02i%02i%02i.00,%02i%02i.%03i,%c,"
                    "%03i%02i.%03i,%c,%i,%02i,%i.%i,%i.%i,%c,%i.%i,%c,,",
                    s->fix_time.tm_hour, s->fix_time.tm_min,
                    s->fix_time.tm_sec, lat_deg, lat_min / 1000,
                    lat_min % 1000, lat_hemi, lon_deg, lon_min / 1000,
                    lon_min % 1000, lon_hemi, s->fix_q, s->maxsat,
                    s->hdop / 10, s->hdop % 10, s->altitude / 10,
                    s->altitude % 10, s->altitude_unit, s->gheight / 10,
                    s->gheight % 10, s->gheight_unit);
    gps_append_chksum(sentence);
    gps_write(s, sentence);
}

/* GSA: Satellite Status */
static void gps_nmea_gpgsa(struct gps_data_s *s)
{
    int i, count = 0;

    /* NMEA format:
     * $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
     * ANTARIS4 format:
     * $GPGSA,A,2,25,01,22,,,,,,,,,,2.56,2.36,1.00*02
     */

    sprintf(sentence, "$GPGSA,%c,%i", 'A', s->fix_type);
    for (i = 0; i < GPS_MAX_SATS; i ++)
        if (s->sats[i].visible) {
            sprintf(sentence + strlen(sentence), ",%02i", s->sats[i].prn);
            count ++;
            if (count >= s->nsat)
                break;
        }
    for (i = count; i < 12; i ++)
        strcat(sentence, ",");
    sprintf(sentence + strlen(sentence), ",%i.%i,%i.%i,%i.%i",
                    s->pdop / 10, s->pdop % 10, s->hdop / 10,
                    s->hdop % 10, s->vdop / 10, s->vdop % 10);

    gps_append_chksum(sentence);
    gps_write(s, sentence);
}

/* GLL: Geographic position, Latitude and Longitude */
static void gps_nmea_gpgll(struct gps_data_s *s)
{
    char lat_hemi, lon_hemi;
    int lat_deg, lon_deg, lat_min, lon_min, lat_sec, lon_sec;

    /* NMEA format:
     * $GPGLL,4916.45,N,12311.12,W,225444,A,*31
     * ANTARIS4 format:
     * $GPGLL,3723.02837,N,12159.39853,W,162254.00,A,A*7C
     */

    gps_latlon_split(s->pos.latitude, "NS",
                    &lat_hemi, &lat_deg, &lat_min, &lat_sec);
    gps_latlon_split(s->pos.longitude, "EW",
                    &lon_hemi, &lon_deg, &lon_min, &lon_sec);

    sprintf(sentence, "$GPGLL,%02i%02i.%03i,%c,%03i%02i.%03i,%c,"
                    "%02i%02i%02i.00,%c,%c",
                    lat_deg, lat_min / 1000, lat_min % 1000, lat_hemi,
                    lon_deg, lon_min / 1000, lon_min % 1000, lon_hemi,
                    s->fix_time.tm_hour, s->fix_time.tm_min,
                    s->fix_time.tm_sec, s->fix, 'A');
    gps_append_chksum(sentence);
    gps_write(s, sentence);
}

/* ZDA: Date and Time */
static void gps_nmea_gpzda(struct gps_data_s *s)
{
    struct tm tm;
    time_t now = time(0);

    /* NMEA format:
     * $GPZDA,201530.00,04,07,2002,00,00*6E
     * ANTARIS4 format:
     * $GPZDA,162254.00,11,07,2006,00,00*63
     */

    gmtime_r(&now, &tm);
    sprintf(sentence, "$GPZDA,%02i%02i%02i.00,%02i,%02i,%04i,%02i,%02i",
                    tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday,
                    tm.tm_mon, tm.tm_year, s->tz_hour, s->tz_min);
    gps_append_chksum(sentence);
    gps_write(s, sentence);
}

/* VTG: Vector track (Velocity made good) and speed over the ground */
static void gps_nmea_gpvtg(struct gps_data_s *s)
{
    /* NMEA format:
     * $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*33
     * ANTARIS4 format:
     * $GPVTG,188.36,T,,M,0.820,N,1.519,K,A*3F
     */

    sprintf(sentence, "$GPZDA,%03i.%i,%c,%03i.%i,%c,%03i.%i,%c,%03i.%i,%c,%c",
                    s->bear / 10, s->bear % 10, 'T', 34, 4, 'M',
                    s->knots / 10, s->knots % 10, 'N',
                    s->knots / 5, (s->knots * 2) % 10, 'K', 'A');
    gps_append_chksum(sentence);
    gps_write(s, sentence);
}

static void gps_nmea_send(void *opaque)
{
    struct gps_data_s *s = (struct gps_data_s *) opaque;

    qemu_mod_timer(s->newdata_tm, qemu_get_clock(vm_clock) + s->rate);

    gps_makedata(s);

    /* Send a report */
    gps_nmea_gprmc(s);
    gps_nmea_gpvtg(s);
    gps_nmea_gpgga(s);
    gps_nmea_gpgsa(s);
    gps_nmea_gpgsv(s);
    gps_nmea_gpgll(s);
    gps_nmea_gpzda(s);
}

static void gps_reset(struct gps_data_s *s)
{
    s->out_len = 0;
    gps_fix_reset(s);
}

void gps_enable(CharDriverState *chr, int enable)
{
    struct gps_data_s *s = (struct gps_data_s *) chr->opaque;

    s->enable = enable;
    if (!enable)
        qemu_del_timer(s->newdata_tm);
    else {
        gps_reset(s);
        qemu_mod_timer(s->newdata_tm,
                        qemu_get_clock(vm_clock) + 3 * ticks_per_sec);
    }
}

static int gps_read(struct CharDriverState *chr, const uint8_t *buf, int len)
{
    /* We don't handle any input atm.  */
    return len;
}

static void gps_out_tick(void *opaque)
{
    gps_fifo_wake((struct gps_data_s *) opaque);
}

static int gps_ioctl(struct CharDriverState *chr, int cmd, void *arg)
{
    QEMUSerialSetParams *ssp;
    struct gps_data_s *s = (struct gps_data_s *) chr->opaque;
    switch (cmd) {
    case CHR_IOCTL_SERIAL_SET_PARAMS:
        ssp = (QEMUSerialSetParams *) arg;
        s->baud_delay = ticks_per_sec / ssp->speed;
        break;

    default:
        return -ENOTSUP;
    }
    return 0;
}

static void gps_run_circles(struct gps_data_s *s)
{
    double angle_velocity = M_PI / 10;
    double radius = 0.1; /* Degrees */
    double t = (double) (qemu_get_clock(vm_clock) - s->vmtime_orig) /
            ticks_per_sec;

    s->pos.latitude = s->pos_orig.latitude + (int)
            (3600.0 * radius * sin(t * angle_velocity));
    s->pos.longitude = s->pos_orig.longitude + (int)
            (3600.0 * radius * cos(t * angle_velocity));
}

static void gps_position_load(struct gps_data_s *s)
{
    int fd;
    uint8_t buffer[4096];
    char *line, *end;
    int ret, len;
    double val;

    /* default */
    s->pos.latitude = GPS_LLTOI(52.143721);
    s->pos.longitude = GPS_LLTOI(21.063452);

    fd = open("openmoko/position", O_RDONLY);
    if (fd == -1)
        return;
    for (len = 0; len < sizeof(buffer) - 1; len += MAX(ret, 0)) {
        ret = read(fd, buffer + len, sizeof(buffer) - 1 - len);
        if (ret == 0)
            break;
        if (ret < 0 && errno != EINTR)
            return;
    }
    close(fd);
    for (line = buffer; line < (char *) buffer + len; line = end + 1) {
        end = memchr(line, '\n', len - (line - (char *) buffer)) ?:
                (buffer + len);
        *end = 0;
        if (sscanf(line, "Latitude: %lf", &val) > 0)
            s->pos.latitude = GPS_LLTOI(val);
        if (sscanf(line, "Longitude: %lf", &val) > 0)
            s->pos.longitude = GPS_LLTOI(val);
    }
}

CharDriverState *gps_antaris_serial_init(void)
{
    struct gps_data_s *s = (struct gps_data_s *)
            qemu_mallocz(sizeof(struct gps_data_s));
    time_t now = time(0);
    s->chr.opaque = s;
    s->chr.chr_write = gps_read;
    s->chr.chr_ioctl = gps_ioctl;
    s->out_tm = qemu_new_timer(vm_clock, gps_out_tick, s);
    s->baud_delay = ticks_per_sec / 9600;

    s->gps_proto = proto_nmea_20;
    s->rate = ticks_per_sec * 4;
    s->newdata_tm = qemu_new_timer(vm_clock, gps_nmea_send, s);

    gps_position_load(s);
    gps_satellite_random(s);
    gps_fix_acquire(s);
    gps_fix_reset(s);
    s->tz_hour = 0;
    s->tz_min = 0;
    s->magnvar = 50;
    s->magnew = 'E';
    s->magnfix = 'A';
    s->vdop = 12;
    s->pdop = 34;
    s->hdop = 56;
    s->altitude = 2345;
    s->altitude_unit = 'M';
    s->gheight = 345;
    s->gheight_unit = 'M';
    s->pos_orig.latitude = s->pos.latitude;
    s->pos_orig.longitude = s->pos.longitude;

    s->vmtime_last = s->vmtime_orig = qemu_get_clock(vm_clock);
    s->pos_update = gps_run_circles;
    s->pos_update(s);

    gmtime_r(&now, &s->fix_time);

    return &s->chr;
}
