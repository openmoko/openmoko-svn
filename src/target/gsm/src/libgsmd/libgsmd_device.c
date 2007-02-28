/* libgsmd phone related functions
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 


#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include "lgsm_internals.h"

int lgsm_get_info(struct lgsm_handle *lh,
			 lgsm_info_type type,
			 char *ret_string, unsigned int* len)
{
		switch (type)
		{
		case LGSM_INFO_TYPE_MANUF:
			return lgsm_passthrough(lh,"AT+CGMI",ret_string,len);	
			break;
		case LGSM_INFO_TYPE_MODEL:
			return lgsm_passthrough(lh,"AT+CGMM",ret_string,len);	
			break;
		case LGSM_INFO_TYPE_REVISION:
			return lgsm_passthrough(lh,"AT+CGMR",ret_string,len);	
			break;
		case LGSM_INFO_TYPE_SERIAL:
		       return lgsm_passthrough(lh,"AT+CGSN",ret_string,len);	
			break;
		case LGSM_INFO_TYPE_IMSI:
		       return lgsm_passthrough(lh,"AT+CIMI",ret_string,len);	
			break;
		case LGSM_INFO_TYPE_NONE:
		default:
			return -EINVAL;
			
		}
			return -EINVAL;	
}
