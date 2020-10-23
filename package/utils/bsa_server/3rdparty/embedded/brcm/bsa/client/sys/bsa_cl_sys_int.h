/*****************************************************************************
 **
 **  Name:           bsa_cl_sys_int.h
 **
 **  Description:    Internal API declaration
 **
 **  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_SYS_INT_H
#define BSA_CL_SYS_INT_H

/* For tBSA_STATUS */
#include "bsa_api.h"

/*******************************************************************************
 **
 ** Function         bsa_cl_ping
 **
 ** Description      function used to ping server (for test purposes)
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
extern tBSA_STATUS bsa_cl_ping(void);

/*******************************************************************************
 **
 ** Function         bsa_cl_kill_server
 **
 ** Description      Function used to kill the server
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
extern tBSA_STATUS bsa_cl_kill_server(void);


#endif /* BSA_CL_SYS_INT_H */
