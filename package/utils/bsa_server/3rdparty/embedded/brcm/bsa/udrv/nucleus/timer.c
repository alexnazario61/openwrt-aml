/*******************************************************************************
**  Name:       timer.c
**
**  Description:
**
**  This file contains the universal driver wrapper for the timer
**  drivers
**
**  Copyright (c) 2001-2004, WIDCOMM Inc., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
*******************************************************************************/

#include "bt_target.h"
#include "gki.h"
#include "utimer.h"

/*******************************************************************************
**
** Function           UTIMER_Init
**
** Description        Universal driver API timer initialization.  
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/

UDRV_API void    UTIMER_Init(void *p_cfg)
{
}

/*******************************************************************************
**
** Function           UTIMER_Start
**
** Description        Start the indicated timer with the given configuration
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/

UDRV_API void    UTIMER_Start(tUTIMER_ID timer, tUTIMER_CFG *p_cfg, tUTIMER_CBACK *p_cback)
{

}

/*******************************************************************************
**
** Function           UTIMER_Read
**
** Description        Read the count value of a timer
**
** Output Parameter   Number of microseconds elapsed since the timer was started
**
** Returns            Nothing
**
*******************************************************************************/

UDRV_API void    UTIMER_Read(tUTIMER_ID timer, tUTIMER_TIME *p_time)
{
}

/*******************************************************************************
**
** Function           UTIMER_Stop
**
** Description        Stop a timer
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/

UDRV_API void    UTIMER_Stop(tUTIMER_ID timer)
{

}

/*******************************************************************************
**
** Function           UTIMER_Feature
**
** Description        Check whether a feature of the timer API is supported.
**
** Output Parameter   None
**
** Returns            TRUE  if the feature is supported 
**                    FALSE if the feature is not supported
**
*******************************************************************************/

UDRV_API BOOLEAN UTIMER_Feature(tUTIMER_FEATURE feature)
{
    /* Only one timer supported */
    if (feature == UTIMER_FEAT_ID_1)
        return TRUE;

    return FALSE;
}

