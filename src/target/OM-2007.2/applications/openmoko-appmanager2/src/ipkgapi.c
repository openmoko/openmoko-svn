/*
 * @file ipkgapi.c
 * @brief Package management api based on libipkg.a.
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
#include <stdio.h>
#define __USE_XOPEN_EXTENDED
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <signal.h>
#include <errno.h>

#include <libipkg.h>
#include <ipkg_conf.h>
#include <pkg_hash.h>
#include "ipkgapi.h"
#include "ipkg_cmd.h"


/*
 * @brief Debug information.
 */
#ifdef DEBUG 
    #define DBG(...) (printf("%s:%d: ", __FUNCTION__, __LINE__), printf(__VA_ARGS__))
#else 
    #define DBG(...) 
#endif 


/*
 * @brief check whether ipkg state is changed.
 */
extern int ipkg_state_changed;

/*
 * @brief Golbal config structure.
 */
ipkg_conf_t global_conf;

/*
 * @brief Golbal config structure.
 */
int is_init_successful = 0;



/*
 * @brief IPKG function's return code.
 */
int nReturnCode = 0;
/*
 * @brief Error message.
 */
char errMsg[1024*5];

/*
 * @brief IPKG message callback function.
 */
extern ipkg_message_callback ipkg_cb_message;



/*
 * @brief IPKG message callback function.
 *
 * @param conf IPKG config structure pointer.
 * @param level IPKG message level,
 *              only IPKG_ERROR will be processed.
 * @param msg  IPKG message.
 *
 * @retval 0
 */
int def_ipkg_message_callback(ipkg_conf_t *conf, message_level_t level, char *msg)
{
    if (level == IPKG_ERROR){
        
        /* Set return code */
        nReturnCode = -1;
    	  /*
    	   * filter some messages
    	   */
    	  if ( strstr(msg, "removal of this package")!=NULL ||
    	  	   strstr(msg, "-force-removal-of-dependent-packages")!=NULL ||
    	  	   strstr(msg, "force_removal_of_dependent_packages")!=NULL ||
    	  	   strstr(msg, "ipkg.conf")!=NULL  )
    	      return 0;
    	  DBG(msg);
        strcat(errMsg, msg);
    } else {
      DBG(msg);
      printf ("IPKG <%d>: %s", level, msg);
    }
    return 0;
}


/*
 * @brief Get error message.
 *
 * @return Error message.
 */
char *get_error_msg()
{
    return errMsg;
}


/*
 * @brief Check whether IPKG status file can be accessed for writing.
 *
 * @param conf IPKG config structure pointer.
 *
 * @retval 0 access denied
 * @retval !=0 access failed
 */
int check_status_files(ipkg_conf_t *conf)
{
	  pkg_dest_list_elt_t *iter;
	  pkg_dest_t *dest;
	  
    for (iter = conf->pkg_dest_list.head; iter; iter = iter->next) {
	      dest = iter->data;
	      dest->status_file = fopen(dest->status_file_tmp_name, "w");
	      if (dest->status_file == NULL) {
	          ipkg_message(conf, IPKG_ERROR, "Can't open status file: %s for writing: %s\n",
		        dest->status_file_name, strerror(errno));
		        return errno;
	      }
    }
	  
    return 0;
}


/*
 * @brief Save IPKG status file and update file lists.
 *
 * @param conf IPKG config structure pointer.
 *
 * @return none
 */
static void write_status_files_if_changed(ipkg_conf_t *conf)
{
    if (ipkg_state_changed && !conf->noaction) {
	      DBG(" writing status file\n");
	      
	      if ( check_status_files(conf)!=0 )
	          return;
	      
	      ipkg_conf_write_status_files(conf);
	      pkg_write_changed_filelists(conf);
	      
    } else { 
	      printf("Nothing to be done\n");
    }
    
    return;
}


/*
 * @brief Check whether the package file is valid.
 *
 * @param conf IPKG config structure pointer.
 * @param package_filename Package file name with path.
 *
 * @retval 0 valid
 * @retval -1 invalid
 */
int check_ipk_file(ipkg_conf_t *conf, char *package_filename)
{
    FILE *deb_stream;
    char ar_magic[8];
    
    deb_stream = fopen(package_filename, "r");
    if (deb_stream==NULL) {
        ipkg_message(conf, IPKG_INFO, 
                     "Not local file,install from internet!\n", 
		                 package_filename );
    	  return 0;
    }
    
    fread(ar_magic, 1, 8, deb_stream);
    
    if ( strncmp(ar_magic,"!<arch>",7) == 0 || 
         strncmp(ar_magic, "\037\213", 2) == 0 )
        return 0;
        
    ipkg_message(conf, IPKG_ERROR, 
                 "Invalid ipk file: %s.\n", 
		             package_filename );
    return -1;

    
}


/*
 * @brief Signal handler.
 *
 * @param sig SIGNAL.
 *
 * @return none
 */
static void sigint_handler(int sig)
{
     signal(sig, SIG_DFL);
     DBG("ipkg: interrupted. writing out status database\n");
     write_status_files_if_changed(&global_conf);
     exit(128 + sig);
}



/*
 * @brief Free package list in head of list.
 *
 * @param head Package list head.
 *
 * @return none
 */
void free_pkg_list(PKG_LIST_HEAD *head)
{
    IPK_PACKAGE *prev;
    IPK_PACKAGE *current;
    
    current = head->pkg_list;
    
    while(current!=NULL) {
        prev = current;
        current = current->next;
        if (prev->name)
            free(prev->name);
        if (prev->version)
            free(prev->version);
        if (prev->section)
            free(prev->section);
        if (prev->size)
            free(prev->size);
        if (prev->depends)
            free(prev->depends);
        if (prev->description)
            free(prev->description);
        if (prev->maintainer)
            free(prev->maintainer);
            
        free(prev);
    }
    
    return;
}


/*
 * @brief Free package detail info.
 *
 * @param info Package detail info pointer.
 *
 * @return none
 */
void free_pkg_detail_info(PACKAGE_DETAIL_INFO *info)
{
    if (info->name)
        free(info->name);
    if (info->version)
        free(info->version);
    if (info->depends)
        free(info->depends);
    if (info->description)
        free(info->description);
    if (info->filename)
        free(info->filename);
    if (info->installed_size)
        free(info->installed_size);
    if (info->size)
        free(info->size);
    if (info->section)
        free(info->section);
    if (info->maintainer)
        free(info->maintainer);
    
    free(info);
    return;
}


/*
 * @brief Set ipkg installation directory.
 *
 * @param info Package detail info pointer.
 *
 * @retval 0 success
 * @retval 1 failure.
 */
static int ipkg_conf_set_default_dest(ipkg_conf_t *conf, const char *default_dest_name)
{
    pkg_dest_list_elt_t *iter;
    pkg_dest_t *dest;

    for (iter = conf->pkg_dest_list.head; iter; iter = iter->next) {
	      dest = iter->data;
	      if (strcmp(dest->name, default_dest_name) == 0) {
            conf->default_dest = dest;
            conf->restrict_to_default_dest = 1;
            return 0;
        }
    }
    
    sprintf(errMsg, "ERROR: Unknown dest name: `%s'\n", default_dest_name);
    return 1;
    
}

/*
 * @brief IPKG initialize.
 *
 * @param noreadfeedsfile 
 *        Set to 1 means not reading feeds file.
 *        Set to 0 means reading feeds file and this will take longer time to init.
 *
 * @retval 0 success
 * @retval !=0 failure.
 */
int ipkg_initialize(int noreadfeedsfile)
{
    int err;
    args_t args;
	
    memset(&global_conf, 0 ,sizeof(global_conf));
    memset(&args, 0 ,sizeof(args));
	
    args_init (&args);
    args.noreadfeedsfile = noreadfeedsfile;
    err = ipkg_conf_init (&global_conf, &args);
    if (err)
    {
        return err;
    }
    args_deinit (&args);

    is_init_successful = 1;
    return 0;
}

/*
 * @brief IPKG uninitialize.
 *
 */
void ipkg_uninitialize()
{
    is_init_successful = 0;
    ipkg_conf_deinit (&global_conf);
}


/*
 * @brief Get installed package list.
 *
 * @param pkg_list_head pkg_list_head->length indicates the number of installed packages, 
 *                      pkg_list_head->pkg_list indicates installed package list.
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_list_installed_cmd(PKG_LIST_HEAD *pkg_list_head)
{
    int i;
    pkg_vec_t *available;
    pkg_t *pkg;
    char *version_str;
    
    IPK_PACKAGE *current = NULL;
    IPK_PACKAGE *tmp = NULL;

    if (is_init_successful!=1) {
        sprintf(errMsg, "ipkg uninitialized!\n" );
        return -1;
    }
    
    nReturnCode = 0;
    
    ipkg_cb_message = def_ipkg_message_callback;
    memset(errMsg, 0, sizeof(errMsg));
    
    available = pkg_vec_alloc();
    pkg_hash_fetch_all_installed(&global_conf.pkg_hash, available);
    for (i=0; i < available->len; i++) {
    	
	      pkg = available->pkgs[i];
	      
        tmp = (IPK_PACKAGE *)malloc(sizeof(IPK_PACKAGE));
	      if (tmp==NULL) {
	          pkg_vec_free(available);
            free_pkg_list(pkg_list_head);
	          sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	          return -1;
	      }
        memset(tmp, 0, sizeof(IPK_PACKAGE));

        version_str = pkg_version_str_alloc(pkg);
          	      
        tmp->name         = strdup(pkg->name);
        tmp->version      = strdup(version_str);
        tmp->state_status = pkg->state_status;
        if (pkg->section)
            tmp->section  = strdup(pkg->section);
        if (pkg->size)
            tmp->size     = strdup(pkg->size);
        
        tmp->next     = NULL;
        
        /* Get packages that depent on it */
        /*
        {
            int j;
            int dep_str_len = 0;
            char *dep_pkg = NULL;
            for (j=0; j<pkg->depends_count; j++)
                dep_str_len += strlen(pkg->depends_str[j]) + 1;
         
            if (dep_str_len!=0) {
                dep_pkg = (char *)malloc(dep_str_len+1);
	              if (dep_pkg==NULL) {
	                  pkg_vec_free(available);
                    free(version_str);
                    free(tmp->name);
                    free(tmp->version);
                    free(tmp);
                    free_pkg_list(pkg_list_head);
	                  sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	                  return -1;
	              }
                memset(dep_pkg, 0, dep_str_len+1);
                for (j=0; j<pkg->depends_count; j++) {
                    strcat(dep_pkg, pkg->depends_str[j]);
                    strcat(dep_pkg, " ");
                }
            }

            DBG("%-*.*s - %-*.*s - %-*.*s \n", 
                   10, 10, pkg->name, 10, 10, version_str, 30, 30, dep_pkg );

            if (dep_pkg) {
                tmp->depends = strdup(dep_pkg);
	              free(dep_pkg);
	          }
        }
        */
        
        free(version_str);
	      
	      
	      if (current==NULL) {
            current = pkg_list_head->pkg_list = tmp;
        } else {
            current->next = tmp;
            current = tmp;
        }
	    
	    
    }

    pkg_list_head->length = available->len;
    
    pkg_vec_free(available);
    return nReturnCode;
}




/*
 * @brief Get package list which can be updated.
 *
 * @param pkg_list_head pkg_list_head->length indicates the number of installed packages, 
 *                      pkg_list_head->pkg_list indicates installed package list.
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_list_updated_cmd(PKG_LIST_HEAD *pkg_list_head)
{
    sprintf(errMsg, "Not implemented!\n");
    return -1;
}





/*
 * @brief Get available package list.
 *
 * @param pkg_list_head pkg_list_head->length indicates the number of installed packages, 
 *                      pkg_list_head->pkg_list indicates installed package list.
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_list_available_cmd(PKG_LIST_HEAD *pkg_list_head)
{
    int i;
    pkg_vec_t *available;
    pkg_t *pkg;
    char *version_str;
    char desc_short[IPKG_LIST_DESCRIPTION_LENGTH];
    char *newline;

    
    IPK_PACKAGE *current = NULL;
    IPK_PACKAGE *tmp = NULL;

    if (is_init_successful!=1) {
        sprintf(errMsg, "ipkg uninitialized!\n" );
        return -1;
    }

    nReturnCode = 0;

    ipkg_cb_message = def_ipkg_message_callback;
    memset(errMsg, 0, sizeof(errMsg));
    
    available = pkg_vec_alloc();
    pkg_hash_fetch_available(&global_conf.pkg_hash, available);
    for (i=0; i < available->len; i++) {
    	
	      pkg = available->pkgs[i];

        tmp = (IPK_PACKAGE *)malloc(sizeof(IPK_PACKAGE));
	      if (tmp==NULL) {
	          pkg_vec_free(available);
            free_pkg_list(pkg_list_head);
	          sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	          return -1;
	      }
        memset(tmp, 0, sizeof(IPK_PACKAGE));

        if (pkg->description) {
            strncpy(desc_short, pkg->description, IPKG_LIST_DESCRIPTION_LENGTH);
        } else {
            desc_short[0] = '\0';
        }
        desc_short[IPKG_LIST_DESCRIPTION_LENGTH - 1] = '\0';
        newline = strchr(desc_short, '\n');
        if (newline) {
            *newline = '\0';
        }


        version_str = pkg_version_str_alloc(pkg);
          	      
        tmp->name         = strdup(pkg->name);
        tmp->version      = strdup(version_str);
        tmp->state_status = pkg->state_status;
        if (pkg->section)
            tmp->section  = strdup(pkg->section);
        if (pkg->size)
            tmp->size     = strdup(pkg->size);
                
        tmp->next     = NULL;
        
        /* Get packages that depent on it */
        {
            int j;
            int dep_str_len = 0;
            char *dep_pkg = NULL;
            for (j=0; j<pkg->depends_count; j++)
                dep_str_len += strlen(pkg->depends_str[j]) + 1;
         
            if (dep_str_len!=0) {
                dep_pkg = (char *)malloc(dep_str_len+1);
	              if (dep_pkg==NULL) {
	                  pkg_vec_free(available);
                    free(version_str);
                    free(tmp->name);
                    free(tmp->version);
                    free(tmp);
                    free_pkg_list(pkg_list_head);
	                  sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	                  return -1;
	              }
                memset(dep_pkg, 0, dep_str_len+1);
                for (j=0; j<pkg->depends_count; j++) {
                    strcat(dep_pkg, pkg->depends_str[j]);
                    strcat(dep_pkg, ",");
                }
            }
            if (dep_pkg)
                dep_pkg[strlen(dep_pkg)-1] = 0;
            
            DBG("%-*.*s - %-*.*s - %-*.*s\n", 
                   10, 10, pkg->name, 10, 10, version_str, 30, 30, dep_pkg );

            if (dep_pkg) {
                tmp->depends = strdup(dep_pkg);
	              free(dep_pkg);
	          }
        }
        
        free(version_str);
        tmp->description = strdup(desc_short);
        if (pkg->maintainer)
            tmp->maintainer     = strdup(pkg->maintainer);
	      
	      
	      if (current==NULL) {
            current = pkg_list_head->pkg_list = tmp;
        } else {
            current->next = tmp;
            current = tmp;
        }
	    
	    
    }

    pkg_list_head->length = available->len;
    
    pkg_vec_free(available);
    return nReturnCode;
}




/*
 * @brief Get package detail information.
 *
 * @param pkg_name Package name.
 * @param query_status PKG_INSTALLED or PKG_AVAILABLE.
 *
 * @return A pointer to PACKAGE_DETAIL_INFO.
 *         NULL for failure.
 */
PACKAGE_DETAIL_INFO *ipkg_get_pkg_detail_info(char *pkg_name, pkg_query_status_t query_status)
{
    int i,done;
    pkg_vec_t *available;
    pkg_t *pkg;
    char desc_short[IPKG_LIST_DESCRIPTION_LENGTH];
    char *newline;
    
    PACKAGE_DETAIL_INFO *info = NULL;

    if (pkg_name==NULL)
    	return NULL;

    if (is_init_successful!=1) {
        sprintf(errMsg, "ipkg uninitialized!\n" );
        return NULL;
    }


    nReturnCode = 0;
    
    ipkg_cb_message = def_ipkg_message_callback;
    memset(errMsg, 0, sizeof(errMsg));
    
    done = 0;
    
    available = pkg_vec_alloc();
    if (query_status==PKG_INSTALLED)
        pkg_hash_fetch_all_installed(&global_conf.pkg_hash, available);
    else
        pkg_hash_fetch_available(&global_conf.pkg_hash, available);
    for (i=0; i < available->len; i++) {
    
	      pkg = available->pkgs[i];
	      if (pkg_name && fnmatch(pkg_name, pkg->name, 0))
	          continue;
	      
        if (pkg->description) {
            strncpy(desc_short, pkg->description, IPKG_LIST_DESCRIPTION_LENGTH);
        } else {
            desc_short[0] = '\0';
        }
        desc_short[IPKG_LIST_DESCRIPTION_LENGTH - 1] = '\0';
        newline = strchr(desc_short, '\n');
        if (newline) {
            *newline = '\0';
        }


	      info = (PACKAGE_DETAIL_INFO *)malloc(sizeof(PACKAGE_DETAIL_INFO));
	      if (info==NULL) {
	          pkg_vec_free(available);
	          sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	          return NULL;
	      }
	      memset(info, 0, sizeof(PACKAGE_DETAIL_INFO));
	      

        /* Get packages that depent on it */
        {
            int j;
            int dep_str_len = 0;
            char *dep_pkg = NULL;
            for (j=0; j<pkg->depends_count; j++)
                dep_str_len += strlen(pkg->depends_str[j]) + 1;
         
            if (dep_str_len!=0) {
                dep_pkg = (char *)malloc(dep_str_len+1);
	              if (dep_pkg==NULL) {
	                  pkg_vec_free(available);
                    free(info);
	                  sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	                  return NULL;
	              }
                memset(dep_pkg, 0, dep_str_len+1);
                for (j=0; j<pkg->depends_count; j++) {
                    strcat(dep_pkg, pkg->depends_str[j]);
                    strcat(dep_pkg, " ");
                }
            }
            if (dep_pkg) {
                info->depends = strdup(dep_pkg);
	              free(dep_pkg);
	          }
        }
	      
	      info->name    = strdup(pkg->name);
	      info->version = pkg_version_str_alloc(pkg);
	      info->description = strdup(desc_short);

        if (pkg->section)
            info->section        = strdup(pkg->section);
        if (pkg->size)
            info->size           = strdup(pkg->size);
        if (pkg->filename)
            info->filename       = strdup(pkg->filename);
        if (pkg->installed_size)
            info->installed_size = strdup(pkg->installed_size);
        if (pkg->maintainer)
            info->maintainer     = strdup(pkg->maintainer);

	      
	      done = 1;
	      break;
    }

    pkg_vec_free(available);
    
    if (done==1&&nReturnCode==0)
        return info;
    else
        return NULL;
    
}




/*
 * @brief Get package by name and dest.
 *
 * @param hash Package hash table.
 * @param pkg_name Package name.
 * @param dest Package dest directory
 *
 * @retval pkg_t Package
 * @retval NULL Not found.
 */
pkg_t *pkg_hash_fetch_available_by_name_dest(hash_table_t *hash, const char *pkg_name, pkg_dest_t *dest)
{
    pkg_vec_t * vec;
    register int i;

    if(!(vec = pkg_vec_fetch_by_name(hash, pkg_name))) {
        return NULL;
    }
    
    for(i = 0; i < vec->len; i++)
        if(vec->pkgs[i]->dest == dest) {
            return vec->pkgs[i];
    }
    return NULL;
}

/*
 * @brief Get package by name.
 *
 * @param hash Package hash table.
 * @param pkg_name Package name.
 *
 * @retval pkg_t Package
 * @retval NULL Not found.
 */
pkg_t *pkg_hash_fetch_available_by_name(hash_table_t *hash,	const char *pkg_name)
{
    pkg_vec_t * vec;
    register int i;

    if(!(vec = pkg_vec_fetch_by_name(hash, pkg_name))){
        return NULL;
    } 

    for(i = 0; i < vec->len; i++)
        return vec->pkgs[i];
    
    return NULL;
}


/*
 * @brief Remove specified package.
 *
 * @param pkg_name Package name.  
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_remove_cmd(char *pkg_name) {

    int a,done;
    pkg_t *pkg;
    pkg_t *pkg_to_remove;
    pkg_vec_t *available;
    
    nReturnCode = 0;
    if (pkg_name==NULL)
    	return -1;

    if (is_init_successful!=1) {
        sprintf(errMsg, "ipkg uninitialized!\n" );
        return -1;
    }

    
    signal(SIGINT, sigint_handler);
    
    done = 0;
    
    ipkg_cb_message = def_ipkg_message_callback;
    memset(errMsg, 0, sizeof(errMsg));
    
    available = pkg_vec_alloc();
    pkg_info_preinstall_check(&global_conf);
    
    pkg_hash_fetch_all_installed(&global_conf.pkg_hash, available);
    for (a=0; a < available->len; a++) {
        pkg = available->pkgs[a];
        if (pkg_name && fnmatch(pkg_name, pkg->name, 0)) {
            continue;
        }
        if (global_conf.restrict_to_default_dest) {
            pkg_to_remove = pkg_hash_fetch_available_by_name_dest(&global_conf.pkg_hash,
    		                                                          pkg->name,
    		                                                          global_conf.default_dest);
        } else {
            pkg_to_remove = pkg_hash_fetch_available_by_name(&global_conf.pkg_hash, pkg->name );
        }

        if (pkg == NULL) {
            DBG("Package %s is not installed.\n", pkg->name);
            continue;
        }
        if (pkg->state_status == SS_NOT_INSTALLED) {    /* Added the control, so every already removed package could be skipped */
            DBG("Package seems to be %s not installed (STATUS = NOT_INSTALLED).\n", pkg->name);
            continue;
        }
        // pkg_to_remove->is_processing = 1;
        ipkg_remove_pkg(&global_conf, pkg_to_remove, 0);
        done = 1;
    }
    
    pkg_vec_free(available);
     

    if ( done == 0 ) {
        strcpy(errMsg, "No packages removed.\n");
        return 1403;
    }

    write_status_files_if_changed(&global_conf);
    
    return nReturnCode;
    
}


/*
 * @brief Install package.
 *
 * @param pkg_name Package file name with path.
 * @param dest_name Ipkg installation dest name.
 * @param pkg_real_name Output package name.
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_install_cmd(char *pkg_name, char *dest_name, char **pkg_real_name)
{
    char *arg;
    int  err = 0;
    nReturnCode = 0;
    
    if (pkg_name==NULL)
    	return -1;

    if (is_init_successful!=1) {
        sprintf(errMsg, "ipkg uninitialized!\n" );
        return -1;
    }

    signal(SIGINT, sigint_handler);

    ipkg_cb_message = def_ipkg_message_callback;
    memset(errMsg, 0, sizeof(errMsg));

    /*
     * check ipk package file
     */
    if (check_ipk_file(&global_conf, pkg_name)) {
        return -1;
    }

    if( ipkg_conf_set_default_dest(&global_conf, dest_name) ) {
        return -1;
    }

    /*
     * Now scan through package names and install
     */
    arg = pkg_name;
    err = ipkg_prepare_url_for_install(&global_conf, arg, &pkg_name);
    if (err != EINVAL && err != 0)
        return err;
    pkg_info_preinstall_check(&global_conf);

    *pkg_real_name = strdup(pkg_name);
    arg = pkg_name;
    if ( global_conf.multiple_providers ) {
        err = ipkg_install_multi_by_name(&global_conf, arg);
    }
    else {
        err = ipkg_install_by_name(&global_conf, arg);
    }
    if ( err == IPKG_PKG_HAS_NO_CANDIDATE ) {
        ipkg_message(&global_conf, IPKG_ERROR,
 	                   "Cannot find package %s.\n"
                     "Check the spelling or perhaps run 'ipkg update'.\n",
                     arg);
    }

    /* recheck to verify that all dependences are satisfied */
    /* if (0) ipkg_satisfy_all_dependences(global_conf); */

    ipkg_configure_packages(&global_conf, NULL);

    write_status_files_if_changed(&global_conf);

    if ( err==0 ) 
        return nReturnCode;
    else
        return err;
}





/*
 * @brief Search specified package.
 *
 * @param pkg_name Package name.
 * @param pkg_list_head pkg_list_head->length indicates the number of installed packages, 
 *                      pkg_list_head->pkg_list indicates installed package list.
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_search_cmd(char *pkg_name, PKG_LIST_HEAD *pkg_list_head)
{
    int i,done;
    pkg_vec_t *available;
    pkg_t *pkg;
    char *version_str;
    char *pattern;
    int find = 0;
    
    IPK_PACKAGE *current = NULL;
    IPK_PACKAGE *tmp = NULL;
    
    if (pkg_name==NULL)
    	return -1;

    if (is_init_successful!=1) {
        sprintf(errMsg, "ipkg uninitialized!\n" );
        return -1;
    }
    
    
    pattern = (char *)malloc(strlen(pkg_name)+3);
    if (pattern==NULL) {
	      sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	      return -1;
	  }
	  sprintf(pattern, "*%s*", pkg_name);
    
    nReturnCode = 0;
    
    ipkg_cb_message = def_ipkg_message_callback;
    memset(errMsg, 0, sizeof(errMsg));
    
    done = 0;
    
    available = pkg_vec_alloc();
    pkg_hash_fetch_available(&global_conf.pkg_hash, available);
    for (i=0; i < available->len; i++) {
    
	      pkg = available->pkgs[i];
	      if (fnmatch(pattern, pkg->name, 0))
	          continue;
	      

        tmp = (IPK_PACKAGE *)malloc(sizeof(IPK_PACKAGE));
	      if (tmp==NULL) {
	          pkg_vec_free(available);
	          free(pattern);
            free_pkg_list(pkg_list_head);
	          sprintf(errMsg, "%s:%d: out of memory\n", __FUNCTION__,  __LINE__);
	          return -1;
	      }
        memset(tmp, 0, sizeof(IPK_PACKAGE));

        version_str = pkg_version_str_alloc(pkg);
          	      
        tmp->name         = strdup(pkg->name);
        tmp->version      = strdup(version_str);
        tmp->state_status = pkg->state_status;
        if (pkg->section)
            tmp->section  = strdup(pkg->section);
        if (pkg->size)
            tmp->size     = strdup(pkg->size);
        
        tmp->next     = NULL;
       
        free(version_str);
	      
	      
	      if (current==NULL) {
            current = pkg_list_head->pkg_list = tmp;
        } else {
            current->next = tmp;
            current = tmp;
        }

	      find++;
    }

    pkg_list_head->length = find;
    
    
    pkg_vec_free(available);
    free(pattern);
    return nReturnCode;
    
}



/*
 * @brief Upgrade specified package.
 *
 * @param pkg_name Package name.
 *
 * @retval 0 success
 * @retval !=0 failure.
 *         Call get_error_msg to get error message.
 */
int ipkg_upgrade_cmd(char *pkg_name)
{
    sprintf(errMsg, "Not implemented!\n");
    return -1;
}

/*
 * @brief Update the package list
 *
 */
int ipkg_update_cmd ()
{
  args_t args;
  
  memset (&args, 0, sizeof (args));
  
  args_init (&args);
  
  return ipkg_lists_update (&args);
}
