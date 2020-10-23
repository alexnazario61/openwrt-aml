/*****************************************************************************
**
**  Name    upio.c
**
**  Description
**  This file contains the universal driver wrapper for the PIO
**  drivers
**
**  Copyright (c) 2001-2004, WIDCOMM Inc., All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
*****************************************************************************/
#include "bt_target.h"
#include "gki.h"
#include "upio.h"


/*******************************************************************************
**  UPIO Driver functions
*******************************************************************************/

/*****************************************************************************
**
** Function         UPIO_Init
**
** Description
**      Initialize the GPIO service.
**      This function is typically called once upon system startup.
**
** Returns          nothing
**
*****************************************************************************/
UDRV_API void UPIO_Init(void *p_cfg)
{
}

/*****************************************************************************
**
** Function         UPIO_Set
**
** Description
**      This function sets one or more GPIO devices to the given state.
**      Multiple GPIOs of the same type can be masked together to set more
**      than one GPIO. This function can only be used on types UPIO_LED and
**      UPIO_GENERAL.
**
** Input Parameters:
**      type    The type of device.
**      pio     Indicates the particular GPIOs.
**      state   The desired state.
**
** Output Parameter:
**      None.
**
** Returns:
**      None.
**
*****************************************************************************/
UDRV_API void UPIO_Set(tUPIO_TYPE type, tUPIO pio, tUPIO_STATE state)
{
}



/*****************************************************************************
**
** Function         UPIO_Read
**
** Description
**      Read the state of a GPIO. This function can be used for any type of
**      device. Parameter pio can only indicate a single GPIO; multiple GPIOs
**      cannot be masked together.
**
** Input Parameters:
**      Type: The type of device.
**      pio:    Indicates the particular GUPIO.
**
** Output Parameter:
**      None.
**
** Returns:
**      State of GPIO (UPIO_ON or UPIO_OFF).
**
*****************************************************************************/
UDRV_API tUPIO_STATE UPIO_Read(tUPIO_TYPE type, tUPIO pio)
{
    /* if not a valid pio return OFF */
    return UPIO_OFF;

}



/*****************************************************************************
**
** Function         UPIO_Config
**
** Description      - Configure GPIOs of type UPIO_GENERAL as inputs or outputs
**                  - Configure GPIOs to be polled or interrupt driven
**
**
** Output Parameter:
**      None.
**
** Returns:
**      None.
**
*****************************************************************************/
UDRV_API void UPIO_Config(tUPIO_TYPE type, tUPIO pio, tUPIO_CONFIG config, tUPIO_CBACK *cback)
{

}

