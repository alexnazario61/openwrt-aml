#ifndef __BRCM_LINUX_IOCTL_H__
#define __BRCM_LINUX_IOCTL_H__

//
// IOCTL definitions (shared among all user mode applications, do not modify)
//
#define IOCTL_BTWUSB_GET_STATS            0x1001
#define IOCTL_BTWUSB_CLEAR_STATS          0x1002
#define IOCTL_BTWUSB_PUT                  0x1003
#define IOCTL_BTWUSB_PUT_CMD              0x1004
#define IOCTL_BTWUSB_GET                  0x1005
#define IOCTL_BTWUSB_GET_EVENT            0x1006
#define IOCTL_BTWUSB_PUT_VOICE            0x1007
#define IOCTL_BTWUSB_GET_VOICE            0x1008
#define IOCTL_BTWUSB_ADD_VOICE_CHANNEL    0x1009
#define IOCTL_BTWUSB_REMOVE_VOICE_CHANNEL 0x100a
#define IOCTL_BTWUSB_DEV_RESET            0x100b

typedef struct
{
    unsigned short sco_handle;
    char burst;
} tUSB_SCO_CONTROL;

#endif  /* __BRCM_LINUX_IOCTL_H__  */
