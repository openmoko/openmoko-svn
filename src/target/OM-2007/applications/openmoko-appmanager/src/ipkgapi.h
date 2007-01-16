/**
 * @file ipkgapi.h
 * @brief Package management api header file.
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
#ifndef _FIC_IPKG_API_H
#define _FIC_IPKG_API_H

#include <libipkg.h>
#include <ipkg_conf.h>
#include <pkg_hash.h>

/**
 * @brief IPKG package structure.
 */
typedef struct ipk_package {
  int  mark;                        /* package mark */
  char *name;                       /* package name */
  char *version;                    /* package version */
  char *section;                    /* package section */
  char *size;                       /* package size */
  char *depends;                    /* dependency package */
  char *description;                /* package description */
  char *maintainer;                 /* package maintainer */
  pkg_state_status_t state_status;  /* package status defined in libipkg/pkg.h */
  struct ipk_package *next;         /* pointer to next package */
}IPK_PACKAGE;

/**
 * @brief IPKG package list head structure.
 */
typedef struct pkg_list_head {
  int length;                   /* the number of installed packages */
  IPK_PACKAGE *pkg_list;        /* package list head pointer */
}PKG_LIST_HEAD;

/**
 * @brief IPKG package detail information structure.
 */
typedef struct package_detail_info {
  char *name;                       /* package name */
  char *version;                    /* package version */
  char *depends;                    /* dependency package */
  char *description;                /* package description */
  char *section;                    /* package section */
  char *size;                       /* package size */
  char *filename;                   /* package file name */
  char *installed_size;             /* package installed size */
  char *maintainer;                 /* package maintainer */
  pkg_state_status_t state_status;  /* package status */
}PACKAGE_DETAIL_INFO;


/**
 * @brief IPKG package query status.
 */
enum pkg_query_status
{
    PKG_INSTALLED = 1,
    PKG_AVAILABLE
};
typedef enum pkg_query_status pkg_query_status_t; /* IPKG package query status */


int ipkg_initialize(int noreadfeedsfile);
void ipkg_uninitialize();

int ipkg_list_available_cmd(PKG_LIST_HEAD *pkg_list_head);
int ipkg_list_installed_cmd(PKG_LIST_HEAD *pkg_list_head);
int ipkg_list_updated_cmd(PKG_LIST_HEAD *pkg_list_head);

PACKAGE_DETAIL_INFO *ipkg_get_pkg_detail_info(char *pkg_name, pkg_query_status_t query_status);

int ipkg_install_cmd(char *pkg_name, char *dest_name, char **pkg_real_name);
int ipkg_remove_cmd(char *pkg_name);
int ipkg_search_cmd(char *pkg_name, PKG_LIST_HEAD *pkg_list_head);

void free_pkg_list(PKG_LIST_HEAD *head);
void free_pkg_detail_info(PACKAGE_DETAIL_INFO *info);

char *get_error_msg();

#endif

