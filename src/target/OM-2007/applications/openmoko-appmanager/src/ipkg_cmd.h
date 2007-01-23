/**
 * @file ipkg_cmd.h
 * @brief The funtions in this header file is defined in libipkg.a .
 * @author Ken Zhao
 * @date 2006-07-27
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#ifndef _FIC_IPKG_CMD_H
#define _FIC_IPKG_CMD_H
#include <ipkg.h>


/**
 * @brief See libipkg.a document.
 */
extern int ipkg_remove_pkg(ipkg_conf_t *conf, pkg_t *pkg,int message);

/**
 * @brief See libipkg.a document.
 */
extern int ipkg_prepare_url_for_install(ipkg_conf_t *conf, const char *url, char **namep);

/**
 * @brief See libipkg.a document.
 */
extern ipkg_error_t ipkg_install_by_name(ipkg_conf_t *conf, const char *pkg_name);

/**
 * @brief See libipkg.a document.
 */
extern ipkg_error_t ipkg_install_multi_by_name(ipkg_conf_t *conf, const char *pkg_name);

/**
 * @brief See libipkg.a document.
 */
extern int ipkg_configure_packages(ipkg_conf_t *conf, char *pkg_name);



#endif

