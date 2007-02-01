/*  moko-dialer-declares.h
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#ifndef _MOKO_DIALER_DECLARES_H_
#define _MOKO_DIALER_DECLARES_H_
#define MOKO_DIALER_MAX_TIME_STATUS_LEN (128)   ///<TO HOLD THE STRINGS LIKE "CALL...(00:00:30)
#define MOKO_DIALER_MAX_NUMBER_LEN	(64)    ///<TO HOLD THE PHONE NUMBERS
#define MOKO_DIALER_MAX_DISP_NAME_LEN (20)      ///<THE MAXIMUM LENGTH OF THE DISPLAYED CONTACT NAME
#define MOKO_DIALER_MAX_PATH_LEN (128)  ///<THE MAX PATH LEN
//MAXDISPNAMENUM MUST >=1 & <=9!

#define MOKO_DIALER_MIN_SENSATIVE_LEN (1)       ///<only when user inputs at least MINSENSATIVELEN, should we start to search.
#define MOKO_DIALER_MAX_STATUS_ICONS (3)        ///<THE NUMBERS OF THE ICONS WHEN OUTGOING CALL IS ON
#define MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH ("unkown.png")    ///<THE DEFAULT PERSON IMAGE
#define MOKO_DIALER_MAX_TIPS (3)

#endif
