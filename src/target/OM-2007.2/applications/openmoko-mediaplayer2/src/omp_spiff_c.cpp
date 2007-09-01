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
 * @file omp_spiff_c.cpp
 * Enhanced libSpiff C bindings
 */

#include <spiff/SpiffIndentFormatter.h>
#include <spiff/SpiffProps.h>
#include <spiff/SpiffPropsWriter.h>
#include <spiff/SpiffReader.h>
#include <spiff/SpiffReaderCallback.h>
#include <spiff/SpiffTrack.h>
#include <spiff/SpiffTrackWriter.h>
#include <spiff/SpiffWriter.h>

extern "C"
{
	#include "omp_spiff_c.h"
}

/*****************************************************************************/
/*** Private C++ interface                                                 ***/
/*****************************************************************************/

using namespace Spiff;

/// Spiff list reading callback that stores data in a specific C-style Spiff list
class SpiffCReaderCallback : public SpiffReaderCallback
{
	private:
		/// The C-style Spiff list the tracks should be appended to
		omp_spiff_list *list;

		/// Pointer to the `next' field in the last inserted item for improving append speed
		omp_spiff_track **newtrack;

		/// Callback which adds tracks to the list
		void addTrack(SpiffTrack *track);

		/// Callback which sets properties in the list
		void setProps(SpiffProps *props);

	public:
		/// Creates the callback interface for filling an omp_spiff_list
		SpiffCReaderCallback(omp_spiff_list *list);

		/// Finalizes the list
		virtual ~SpiffCReaderCallback();
};

SpiffCReaderCallback::SpiffCReaderCallback(omp_spiff_list *list)
{
	this->list = list;
	newtrack = &list->tracks;
}

SpiffCReaderCallback::~SpiffCReaderCallback()
{
	// Null-terminate the tracks list
	*newtrack = NULL;
}

void
SpiffCReaderCallback::addTrack(SpiffTrack *track)
{
	omp_spiff_mvalue **newmv;
	char *str;

	// Append new item to the track list
	*newtrack = new omp_spiff_track;

	(*newtrack)->creator = track->stealCreator();
	(*newtrack)->title = track->stealTitle();
	(*newtrack)->album = track->stealAlbum();
	(*newtrack)->duration = track->getDuration();
	(*newtrack)->tracknum = track->getTrackNum();

	// Locations
	newmv = &(*newtrack)->locations;
	while ((str = track->stealFirstLocation()) != NULL) {
		*newmv = new omp_spiff_mvalue;
		(*newmv)->value = str;

		// On to the next location
		newmv = &(*newmv)->next;
	}
	*newmv = NULL;

	// Identifiers
	newmv = &(*newtrack)->identifiers;
	while ((str = track->stealFirstIdentifier()) != NULL) {
		*newmv = new omp_spiff_mvalue;
		(*newmv)->value = str;

		// On to the next identifier
		newmv = &(*newmv)->next;
	}
	*newmv = NULL;

	// Clean up and move on to the next track
	delete track;
	newtrack = &(*newtrack)->next;
}

void
SpiffCReaderCallback::setProps(SpiffProps *props)
{
	list->license = props->stealLicense();
	list->location = props->stealLocation();
	list->identifier = props->stealIdentifier();

	delete props;
}

/*****************************************************************************/
/*** Private C functions                                                   ***/
/*****************************************************************************/

/**
 * Deallocates all objects in an omp_spiff_mvalue linked list
 */
static void
omp_spiff_mvalue_free(omp_spiff_mvalue *mv)
{
	omp_spiff_mvalue *nmv;

	for (; mv != NULL; mv = nmv) {
		// Back-up pointer
		nmv = mv->next;
		delete mv->value;
		delete mv;
	}
}

/*****************************************************************************/
/*** Public C interface                                                    ***/
/*****************************************************************************/

/**
 * Parses an XSPF file by filename
 */
extern "C" omp_spiff_list *
omp_spiff_parse(const char *filename)
{
	SpiffReader read;
	omp_spiff_list *ret;

	// Allocate empty playlist
	ret = new omp_spiff_list;

	// Fill the list with parser results
	SpiffCReaderCallback readcb(ret);

	if (read.parseFile(filename, &readcb) == SPIFF_READER_SUCCESS)
	{
		return ret;

	} else {

		delete ret;
		return NULL;
	}
}

/**
 * Creates a new empty XSPF playlist
 */
extern "C" omp_spiff_list *
omp_spiff_new(void)
{
	omp_spiff_list *ret;

	ret = new omp_spiff_list;

	ret->license = NULL;
	ret->location = NULL;
	ret->identifier = NULL;
	ret->tracks = NULL;

	return ret;
}

/**
 * Frees the parser results
 */
extern "C" void
omp_spiff_free(omp_spiff_list *list)
{
	omp_spiff_track *tr, *ntr;

	delete list->license;
	delete list->location;
	delete list->identifier;

	for (tr = list->tracks; tr != NULL; tr = ntr)
	{
		// Back-up pointer
		ntr = tr->next;

		delete tr->creator;
		delete tr->title;
		delete tr->album;

		omp_spiff_mvalue_free(tr->locations);
		omp_spiff_mvalue_free(tr->identifiers);

		delete tr;
	}

	delete list;
}

/**
 * Writes the XSPF playlist to a file
 */
extern "C" int
omp_spiff_write(omp_spiff_list *list, const char *filename)
{
	omp_spiff_track *strack;
	omp_spiff_mvalue *smvalue;

	SpiffIndentFormatter formatter(-2);
	SpiffProps props;

	// Playlist properties
	props.lendLicense(list->license);
	props.lendLocation(list->location);
	props.lendIdentifier(list->identifier);

	SpiffPropsWriter pwriter(&props);
	SpiffTrackWriter twriter;
	SpiffWriter writer(1, formatter, pwriter);

	SPIFF_LIST_FOREACH_TRACK(list, strack)
	{
		// Tracks
		SpiffTrack track;

		track.lendCreator(strack->creator);
		track.lendTitle(strack->title);
		track.lendAlbum(strack->album);
		track.setDuration(strack->duration);
		track.setTrackNum(strack->tracknum);

		// Track locations and identifiers
		SPIFF_TRACK_FOREACH_LOCATION(strack,smvalue)
			track.lendAppendLocation(smvalue->value);

		SPIFF_TRACK_FOREACH_IDENTIFIER(strack, smvalue)
			track.lendAppendIdentifier(smvalue->value);

		twriter.setTrack(&track);
		writer.addTrack(twriter);
	}

	return writer.writeFile(filename);
}

/**
 * @brief Sets or overwrites a value in the omp_spiff_list, omp_spiff_track or
 * @brief omp_spiff_mvalue structures
 * @note Passing NULL will unset the string.
 */
extern "C" void
omp_spiff_setvalue(char **str, const char *nstr)
{
	// Delete old string
	delete *str;

	if (nstr == NULL) {
		// Unset value
		*str = NULL;

	} else {

		// Copy value
		size_t len;
		len = strlen(nstr)+1;
		*str = new char[len];
		strcpy(*str, nstr);
	}
}

/**
 * @brief Inserts a new mvalue to the linked list before the specified one
 * @note This routine can also be used to insert a new mvalue to
 *       the end of the list (or an empty list) by passing the address
 *       of the mvalue list or the next field in the last object.
 */
extern "C" omp_spiff_mvalue *
omp_spiff_new_mvalue_before(omp_spiff_mvalue **mvalue)
{
	omp_spiff_mvalue *ret;

	ret = new omp_spiff_mvalue;
	ret->value = NULL;
	ret->next = *mvalue;
	*mvalue = ret;

	return ret;
}

/**
 * @brief Inserts a new track to the linked list before the specified one
 * @note This routine can also be used to insert a new track to
 *       the end of the list (or an empty list) by passing the address
 *       of the track list or the next field in the last object.
 */
extern "C" omp_spiff_track *
omp_spiff_new_track_before(omp_spiff_track **track)
{
	omp_spiff_track *ret;

	ret = new omp_spiff_track;
	ret->creator = NULL;
	ret->title = NULL;
	ret->album = NULL;
	ret->duration = -1;
	ret->tracknum = -1;
	ret->locations = NULL;
	ret->identifiers = NULL;
	ret->title_is_preliminary = 0;
	ret->next = *track;
	*track = ret;

	return ret;
}
