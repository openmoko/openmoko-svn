/*
 * omp_spiff_c - Enhanced C interface for libSpiff
 *
 * Copyright (C) 2007, Ed Schouten / Xiph.Org Foundation
 * All rights reserved.
 *
 * Modified by Soeren Apel (abraxa) for OpenMoko to include XSPF
 * features not found in the simple C bindings
 *
 * Redistribution  and use in source and binary forms, with or without
 * modification,  are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions   of  source  code  must  retain  the   above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer.
 *
 *     * Redistributions  in  binary  form must  reproduce  the  above
 *       copyright  notice, this list of conditions and the  following
 *       disclaimer   in  the  documentation  and/or  other  materials
 *       provided with the distribution.
 *
 *     * Neither  the name of the Xiph.Org Foundation nor the names of
 *       its  contributors may be used to endorse or promote  products
 *       derived  from  this software without specific  prior  written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT  NOT
 * LIMITED  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS
 * FOR  A  PARTICULAR  PURPOSE ARE DISCLAIMED. IN NO EVENT  SHALL  THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL,    SPECIAL,   EXEMPLARY,   OR   CONSEQUENTIAL   DAMAGES
 * (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Ed Schouten <ed@fxq.nl>
 */

/**
 * @file omp_spiff_c.h
 * Enhanced libSpiff C bindings
 */

#ifndef OMP_SPIFF_C_H
#define OMP_SPIFF_C_H

/// Linked list for values inside tracks or lists with strings
typedef struct _omp_spiff_mvalue
{
	/// Value of the current list entry
	char *value;

	/// Pointer to next object in the list
	struct _omp_spiff_mvalue *next;

} omp_spiff_mvalue;


/// Single track in an XSPF list
typedef struct _omp_spiff_track
{
	/// Track's creator (= artist)
	char *creator;

	/// Track's display name
	char *title;

	/// Album or collection of origin
	char *album;
	
	/// Track duration in milliseconds
	int duration;

	/// Track number
	int tracknum;

	/// Track's file locations
	omp_spiff_mvalue *locations;

	/// Unique track identifiers
	omp_spiff_mvalue *identifiers;

	/// Preliminary title flag, title will be overwritten on incoming tag data (non-XSPF feature)
	int title_is_preliminary;

	/// Pointer to next track
	struct _omp_spiff_track *next;

} omp_spiff_track;


/// Parsed XSPF file
typedef struct _omp_spiff_list
{
	/// Playlist's license
	char *license;

	/// Playlist's file location
	char *location;

	/// Playlist's unique indentifier
	char *identifier;

	/// Linked list of tracks inside the playlist
	omp_spiff_track *tracks;

} omp_spiff_list;


/// Easy interface for walking through tracks
#define SPIFF_LIST_FOREACH_TRACK(l,t) \
    for ((t) = (l)->tracks; (t) != NULL; (t) = (t)->next)

/// Easy interface for walking through locations
#define SPIFF_TRACK_FOREACH_LOCATION(t,l) \
    for ((l) = (t)->locations; (l) != NULL; (l) = (l)->next)

/// Easy interface for walking through identifiers
#define SPIFF_TRACK_FOREACH_IDENTIFIER(t,i) \
    for ((i) = (t)->identifiers; (i) != NULL; (i) = (i)->next)

omp_spiff_list *omp_spiff_parse(const char *filename);
omp_spiff_list *omp_spiff_new(void);
void omp_spiff_free(omp_spiff_list *list);
int omp_spiff_write(omp_spiff_list *list, const char *filename);

void omp_spiff_setvalue(char **str, const char *nstr);

omp_spiff_mvalue *omp_spiff_new_mvalue_before(
    omp_spiff_mvalue **mvalue);

omp_spiff_track *omp_spiff_new_track_before(
    omp_spiff_track **track);

#endif /* !OMP_SPIFF_C_H */
