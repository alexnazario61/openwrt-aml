/*****************************************************************************
** 
**  Name    upio_linux.c
** 
**  Description
**  This file contains the universal driver wrapper for the BTE-QC pio
**  drivers
**
**  Copyright (c) 2001-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/
#include "bt_target.h"
#include "gki.h"
#include "upio.h"

#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
#include "fcntl.h"
#include "errno.h"
#ifndef HCILP_BT_POWER_GPIO
#define HCILP_BT_POWER_GPIO UPIO_GENERAL3
#endif
#endif

/*******************************************************************************
**  UPIO Driver functions
*******************************************************************************/
#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
#include <stdio.h>
static int rfkill_id = -1;
static char *rfkill_state_path = NULL;

#define UPIO_BT_POWER_OFF 0
#define UPIO_BT_POWER_ON  1
#endif

#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
#include <unistd.h>
#define BLUESLEEP_PROC_BT_WAKE      "/proc/bluetooth/sleep/btwake"
#define BLUESLEEP_PROC_HOST_WAKE    "/proc/bluetooth/sleep/hostwake"
#define BLUESLEEP_PROC_PROTO        "/proc/bluetooth/sleep/proto"
#define BLUESLEEP_PROC_ASLEEP       "/proc/bluetooth/sleep/asleep"
#endif

#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
/*****************************************************************************
**
** Function         UPIO_set_bluesleep_proto
**
** Description
**      Enable or disable bluesleep.
**      This function is typically called when LPM is enabled or diabled
**
** Returns          nothing
**
*****************************************************************************/
void UPIO_set_bluesleep_proto(BOOLEAN bStart) {
    int fd = 0 ;
    char buf = '0';
    int size;

    APPL_TRACE_API1("UPIO_set_bluesleep_proto : %d ", bStart);

    fd = open(BLUESLEEP_PROC_PROTO, O_RDWR);
    if(fd < 0){
        DRV_TRACE_ERROR1("Fail to open %s \n", BLUESLEEP_PROC_PROTO);
        return;
    }
    if(bStart) buf = '1';
    size = write (fd, &buf, 1);
    if( size != 1){
        DRV_TRACE_ERROR1("Fail to write %s \n", BLUESLEEP_PROC_PROTO);
    }
    close(fd);

    return;
}
#endif

#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
static int init_rfkill() {
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;

    for (id = 0;; id++)
    {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0)
        {
            DRV_TRACE_DEBUG3("UPIO init_rfkill : open(%s) failed: %s (%d)\n", path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 9 && memcmp(buf, "bluetooth", 9) == 0)
        {
            rfkill_id = id;
            break;
        }
    }

    asprintf(&rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", rfkill_id);
    return 0;
}

static int check_bluetooth_power() {
    int sz;
    int fd = -1;
    int ret = -1;
    char buffer;

    if (rfkill_id == -1) {
        if (init_rfkill()) goto out;
    }

    fd = open(rfkill_state_path, O_RDONLY);
    if (fd < 0) {
        DRV_TRACE_DEBUG3("UPIO - check_bluetooth_power : open(%s) failed: %s (%d)", rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    sz = read(fd, &buffer, 1);
    if (sz != 1) {
        DRV_TRACE_DEBUG3("UPIO - check_bluetooth_power : read(%s) failed: %s (%d)", rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }

    switch (buffer) {
        case '1':
            ret = UPIO_BT_POWER_ON;
            break;
        case '0':
            ret = UPIO_BT_POWER_OFF;
            break;
    }

out:
    if (fd >= 0) close(fd);
        return ret;
}

static int set_bluetooth_power(int on) {
    int sz;
    int fd = -1;
    int ret = -1;
    char buffer = '0';

    switch(on)
    {
    case UPIO_BT_POWER_OFF:
        buffer = '0';
#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
        UPIO_set_bluesleep_proto(FALSE);
#endif
        break;
    case UPIO_BT_POWER_ON:
        buffer = '1';
        break;
    }

    if (rfkill_id == -1) {
        if (init_rfkill()) goto out;
    }

    fd = open(rfkill_state_path, O_WRONLY);
    if (fd < 0) {
        DRV_TRACE_DEBUG3("UPIO - set_bluetooth_power : open(%s) for write failed: %s (%d)",
                                 rfkill_state_path,strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        DRV_TRACE_DEBUG3("UPIO - set_bluetooth_power : write(%s) failed: %s (%d)", rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
       return ret;
}

int bt_is_enabled() {
    int ret = -1;

    ret = check_bluetooth_power();

    if (ret == -1 || ret == 0)
        ret = 0;
    else
        ret = 1;
    return ret;
}
#endif

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
#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
        /* Make sure we have Power to the BT Chip */
        UPIO_Set(UPIO_GENERAL, HCILP_BT_POWER_GPIO, UPIO_ON);
#endif
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
#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
        int rc;
#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
        char buf;
        int size = 0;
        static tUPIO_STATE prev_state = UPIO_TOGGLE;
        int fd = -1;
#endif

        DRV_TRACE_DEBUG3("UPIO_Set: type: [%d] , pio: [%d], state [%d] \n",type, pio, state);
        switch (type)
        {
        case UPIO_GENERAL:
            switch (pio)
            {
            case HCILP_BT_POWER_GPIO:
                DRV_TRACE_DEBUG1("UPIO_Set: HCILP_BT_POWER_GPIO set to state [%d]\n", state);

                if (state == UPIO_OFF)
                    rc = set_bluetooth_power(UPIO_BT_POWER_OFF);
                else
                    rc = set_bluetooth_power(UPIO_BT_POWER_ON);
                DRV_TRACE_DEBUG2("UPIO_Set: Set BT Power to state [%d] , rc = [%d] \n", state, rc );
                break;
#if (defined(HCILP_INCLUDED) && HCILP_INCLUDED == TRUE)
#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
            case HCILP_BT_WAKE_GPIO:

                if(prev_state == state)
                {
                    DRV_TRACE_DEBUG1("Setting to the same value [%d] \n", state);
                    return;
                }
                prev_state = state;

                fd = open(BLUESLEEP_PROC_BT_WAKE, O_WRONLY);
                if(fd < 0)
                {
                    DRV_TRACE_ERROR1("Fail to open %s \n", BLUESLEEP_PROC_BT_WAKE);
                    return;
                }
                buf = state+'0';
                size = write(fd, &buf, 1);
                if(size != 1)
                {
                    DRV_TRACE_ERROR1("Fail to set : HCILP_BT_WAKE_GPIO size [%d]\n", size);
                }
                close(fd);
                break;
#endif
#endif
            }
            break;
        }
#endif
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
**      Type:   The type of device.
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
#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
        int rc = 0;
        tUPIO_STATE ret = UPIO_OFF;
#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
        char buf;
        int size = 0;
        int fd = -1;
#endif

        switch (type)
        {
        case UPIO_GENERAL:
            switch (pio)
            {
            case HCILP_BT_POWER_GPIO:
                DRV_TRACE_DEBUG0("UPIO_Read: HCILP_BT_POWER_GPIO \n");
                rc = bt_is_enabled();
                ret = (rc == UPIO_BT_POWER_OFF) ? UPIO_OFF : UPIO_ON;
                DRV_TRACE_DEBUG2("UPIO_Read: HCILP_BT_POWER_GPIO rc [%d], ret [%d]\n", rc, ret );
                break;

#if (defined(HCILP_INCLUDED) && HCILP_INCLUDED == TRUE)
#if defined (BSA_SLEEP_PROC_IF) && (BSA_SLEEP_PROC_IF == TRUE)
            case HCILP_BT_WAKE_GPIO:
                fd = open(BLUESLEEP_PROC_BT_WAKE, O_RDONLY);
                if(fd < 0)
                {
                    DRV_TRACE_ERROR1("Fail to open %s \n", BLUESLEEP_PROC_BT_WAKE);
                    break;
                }

                size = read(fd, &buf, 1);
                close(fd);

                if(size != 1)
                {
                    DRV_TRACE_ERROR0("Fail to read: HCILP_BT_WAKE_GPIO");
                    break;
                }
                ret = (buf == '0') ? UPIO_OFF : UPIO_ON;
                break;

            case HCILP_HOST_WAKE_GPIO:
                fd = open(BLUESLEEP_PROC_HOST_WAKE, O_RDONLY);
                if(fd < 0)
                {
                    DRV_TRACE_ERROR1("Fail to open %s \n", BLUESLEEP_PROC_HOST_WAKE);
                    break;
                }
                size = read(fd, &buf, 1);
                close(fd);

                if(size != 1)
                {
                    DRV_TRACE_ERROR0("Fail to read: HCILP_HOST_WAKE_GPIO");
                    break;
                }
                ret = (buf == '0') ? UPIO_OFF : UPIO_ON;
                break;
#endif
#endif
            }
            break;
        }

        return ret;
#else
    return 0;
#endif

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
#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)

    switch (type)
     {
     case UPIO_GENERAL:
         switch (pio)
         {
         case HCILP_BT_POWER_GPIO:
             DRV_TRACE_DEBUG0("UPIO_Config: HCILP_BT_POWER_GPIO \n");
             break;
         }
         break;
     }
#endif
}

