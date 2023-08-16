/* $Header: /home/harrison/c/tcgmsg/ipcv4.0/RCS/defglobals.h,v 1.1 91/12/06 17:26:22 harrison Exp Locker: harrison $ */

#ifndef SNDRCVP
#include "sndrcvP.h"
#endif

/* Actual definition of these globals ... need this once in
   any executable ... included by cluster.c */

/*********************************************************
  Global information and structures ... all begin with SR_
  ********************************************************/

long SR_n_clus;                   /* No. of clusters */
long SR_n_proc;                   /* No. of processes excluding dummy
				     master process */

long SR_clus_id;                  /* Logical id of current cluster */
long SR_proc_id;                  /* Logical id of current process */

long SR_debug;                    /* flag for debug output */

long SR_exit_on_error;            /* flag to exit on error */
long SR_error;                    /* flag indicating error has been called
                                     with SR_exit_on_error == FALSE */

long SR_nchild;                   /* no. of forked processes */
long SR_pids[MAX_SLAVE];          /* pids of forked processes */


/* This is used to store info from the PROCGRP file about each
   cluster of processes */

struct cluster_info_struct SR_clus_info[MAX_CLUSTER];

struct process_info_struct SR_proc_info[MAX_PROCESS];
