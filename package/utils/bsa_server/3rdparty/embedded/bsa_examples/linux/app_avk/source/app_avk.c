/*****************************************************************************
 **
 **  Name:           app_avk.c
 **
 **  Description:    Bluetooth Manager application
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "bsa_api.h"
#include "bsa_avk_api.h"

#include "gki.h"
#include "uipc.h"

#include "app_xml_utils.h"
#include "app_disc.h"
#include "app_utils.h"
#include "app_wav.h"

#ifdef PCM_ALSA
#include "alsa/asoundlib.h"
#define APP_AVK_ASLA_DEV "default"
#endif

/*
 * Defines
 */

#define APP_XML_REM_DEVICES_FILE_PATH "./bt_devices.xml"

#define APP_AVK_SOUND_FILE "test_avk"

#ifndef BSA_AVK_SECURITY
#define BSA_AVK_SECURITY    BSA_SEC_AUTHORIZATION
#endif
#ifndef BSA_AVK_FEATURES
#define BSA_AVK_FEATURES    (BSA_AVK_FEAT_RCCT|BSA_AVK_FEAT_RCTG|BSA_AVK_FEAT_VENDOR|BSA_AVK_FEAT_METADATA | BSA_AVK_FEAT_BROWSE)
#endif

#ifndef BSA_AVK_DUMP_RX_DATA
#define BSA_AVK_DUMP_RX_DATA FALSE
#endif

/*
* Types
*/
typedef struct
{
    tBSA_STATUS last_open_status;
    tBSA_STATUS last_start_status;
    volatile BOOLEAN open_pending; /* Indicate that there is an open pending */
    BOOLEAN is_rc_open;
    tBSA_AVK_FEAT features;       /* features mask */
    UINT16 peer_version; /*Avrp version*/
    BOOLEAN is_open;
    BOOLEAN is_started;
    UINT16 sample_rate;
    UINT8 num_channel;
    UINT8 bit_per_sample;
    tBSA_AVK_CHNL channel;
    UINT8 rc_handle;
    tUIPC_CH_ID uipc_audio_channel;
    int fd;
#ifdef PCM_ALSA
    snd_pcm_t *alsa_handle;
#endif
    UINT8 label;
    BD_ADDR bda_connected;
} tAPP_AVK_CB;

/*
 * global variable
 */

tAPP_AVK_CB app_avk_cb;

#ifdef PCM_ALSA
static char *alsa_device = "default"; /* ALSA playback device */
#endif

/*
 * Local functions
 */
static void app_avk_close_wave_file(void);
static void app_avk_create_wave_file(void);
static void app_avk_uipc_cback(BT_HDR *p_msg);
/* Callback to application for connection events */
typedef void ( tAvkCallback)(tBSA_AVK_EVT event, tBSA_AVK_MSG *p_data);

static tAvkCallback *s_pCallback = NULL;
/*******************************************************************************
 **
 ** Function         app_avk_create_wave_file
 **
 ** Description     create a new wave file
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_create_wave_file(void)
{
    int file_index = 0;
    int fd;
    char filename[200];

#ifdef PCM_ALSA
    return;
#endif

    /* If a wav file is currently open => close it */
    if (app_avk_cb.fd != -1)
    {
        app_avk_close_wave_file();
    }

    do
    {
        snprintf(filename, sizeof(filename), "%s-%d.wav", APP_AVK_SOUND_FILE, file_index);
        filename[sizeof(filename)-1] = '\0';
        fd = app_wav_create_file(filename, O_EXCL);
        file_index++;
    } while (fd < 0);

    app_avk_cb.fd = fd;
}

/*******************************************************************************
 **
 ** Function         app_avk_close_wave_file
 **
 ** Description     proper header and close the wave file
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_close_wave_file(void)
{
    tAPP_WAV_FILE_FORMAT format;

#ifdef PCM_ALSA
    return;
#endif

    if (app_avk_cb.fd == -1)
    {
        printf("app_avk_close_wave_file: no file to close\n");
        return;
    }

    format.bits_per_sample = app_avk_cb.bit_per_sample;
    format.nb_channels = app_avk_cb.num_channel;
    format.sample_rate = app_avk_cb.sample_rate;

    app_wav_close_file(app_avk_cb.fd, &format);

    app_avk_cb.fd = -1;
}

/*******************************************************************************
**
** Function         app_avk_end
**
** Description      This function is used to close AV
**
** Returns          void
**
*******************************************************************************/
void app_avk_end(void)
{
    tBSA_AVK_DISABLE disable_param;
    tBSA_AVK_DEREGISTER deregister_param;

    /* deregister avk */
    BSA_AvkDeregisterInit(&deregister_param);
    deregister_param.channel = app_avk_cb.num_channel;
    BSA_AvkDeregister(&deregister_param);

    /* disable avk */
    BSA_AvkDisableInit(&disable_param);
    BSA_AvkDisable(&disable_param);
}


/*******************************************************************************
**
** Function         app_avk_handle_start
**
** Description      This function is the AVK start handler function
**
** Returns          void
**
*******************************************************************************/
static void app_avk_handle_start(tBSA_AVK_MSG *p_data)
{
    char *avk_format_display[12] = {"SBC", "M12", "M24", "??", "ATRAC", "PCM", "APT-X", "SEC"};
#ifdef PCM_ALSA
    int status;
    snd_pcm_format_t format;
#endif

    if (p_data->start.media_receiving.format < (sizeof(avk_format_display)/sizeof(avk_format_display[0])))
    {
        APP_INFO1("AVK start: format %s", avk_format_display[p_data->start.media_receiving.format]);
    }
    else
    {
        APP_INFO1("AVK start: format code (%u) unknown", p_data->start.media_receiving.format);
    }

    if (p_data->start.media_receiving.format == BSA_AVK_CODEC_PCM)
    {
        app_avk_create_wave_file();/* create and open a wave file */
        app_avk_cb.sample_rate = p_data->start.media_receiving.cfg.pcm.sampling_freq;
        app_avk_cb.num_channel = p_data->start.media_receiving.cfg.pcm.num_channel;
        app_avk_cb.bit_per_sample = p_data->start.media_receiving.cfg.pcm.bit_per_sample;
        printf("Sampling rate:%d, number of channel:%d bit per sample:%d\n",
            p_data->start.media_receiving.cfg.pcm.sampling_freq,
            p_data->start.media_receiving.cfg.pcm.num_channel,
            p_data->start.media_receiving.cfg.pcm.bit_per_sample);
#ifdef PCM_ALSA
        /* If ALSA PCM driver was already open => close it */
        if (app_avk_cb.alsa_handle != NULL)
        {
            snd_pcm_close(app_avk_cb.alsa_handle);
            app_avk_cb.alsa_handle = NULL;
        }

        /* Open ALSA driver */
        status = snd_pcm_open(&(app_avk_cb.alsa_handle), alsa_device,
            SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        if (status < 0)
        {
            APP_ERROR1("snd_pcm_open failed: %s", snd_strerror(status));
        }
        else
        {
            APP_DEBUG0("ALSA driver opened");
            if (app_avk_cb.bit_per_sample == 8)
                format = SND_PCM_FORMAT_U8;
            else
                format = SND_PCM_FORMAT_S16_LE;
            /* Configure ALSA driver with PCM parameters */
            status = snd_pcm_set_params(app_avk_cb.alsa_handle, format,
                SND_PCM_ACCESS_RW_INTERLEAVED, app_avk_cb.num_channel,
                app_avk_cb.sample_rate, 1, 500000);/* 0.5sec */
            if (status < 0)
            {
                APP_ERROR1("snd_pcm_set_params failed: %s", snd_strerror(status));
                exit(1);
            }
        }
#endif
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_cback
 **
 ** Description      This function is the AVK callback function
 **
 ** Returns          void
 **
 *******************************************************************************/
static void app_avk_cback(tBSA_AVK_EVT event, tBSA_AVK_MSG *p_data)
{
    switch (event)
    {
    case BSA_AVK_OPEN_EVT:
        APP_DEBUG1("BSA_AVK_OPEN_EVT status 0x%x", p_data->open.status);
        app_avk_cb.last_open_status = p_data->open.status;
        app_avk_cb.channel = p_data->open.channel;

        if (p_data->open.status == BSA_SUCCESS)
        {
            app_avk_cb.is_open = TRUE;
            bdcpy(app_avk_cb.bda_connected, p_data->open.bd_addr);
        }
        else
        {
            app_avk_cb.is_open = FALSE;
        }

        app_avk_cb.open_pending = FALSE;

        printf("AVK connected to device %02X:%02X:%02X:%02X:%02X:%02X\n",
            p_data->open.bd_addr[0], p_data->open.bd_addr[1],p_data->open.bd_addr[2],
            p_data->open.bd_addr[3], p_data->open.bd_addr[4],p_data->open.bd_addr[5]);
        break;
    case BSA_AVK_CLOSE_EVT:
        /* Close event, reason BTA_AVK_CLOSE_STR_CLOSED or BTA_AVK_CLOSE_CONN_LOSS*/
        APP_DEBUG1("BSA_AVK_CLOSE_EVT status 0x%x", p_data->close.status);
        app_avk_cb.is_open = FALSE;
        app_avk_cb.open_pending = FALSE;
        if (app_avk_cb.is_started == TRUE)
        {
            app_avk_cb.is_started = FALSE;
            app_avk_close_wave_file();
        }
        break;
    case BSA_AVK_START_EVT:
        APP_DEBUG1("BSA_AVK_START_EVT status 0x%x", p_data->start.status);
        app_avk_cb.last_start_status = p_data->start.status;
        app_avk_cb.is_started = TRUE;
        app_avk_handle_start(p_data);
        break;
    case BSA_AVK_STOP_EVT:
        APP_DEBUG0("BSA_AVK_STOP_EVT");
        app_avk_cb.is_started = FALSE;
        app_avk_close_wave_file();
#ifdef PCM_ALSA
        if ((app_avk_cb.alsa_handle) != NULL)
        {
            snd_pcm_close(app_avk_cb.alsa_handle);
            app_avk_cb.alsa_handle = NULL;
        }
#endif
        break;
    case BSA_AVK_RC_OPEN_EVT:
        APP_DEBUG1("BSA_AVK_RC_OPEN_EVT peer_feature=0x%x rc_handle=%d",
            p_data->rc_open.peer_features, p_data->rc_open.rc_handle);
        app_avk_cb.rc_handle = p_data->rc_open.rc_handle;
        app_avk_cb.features =  p_data->rc_open.peer_features;
        app_avk_cb.peer_version = p_data->rc_open.peer_version;
        app_avk_cb.is_rc_open = TRUE;
        break;
    case BSA_AVK_RC_CLOSE_EVT:
        APP_DEBUG0("BSA_AVK_RC_CLOSE_EVT");
        app_avk_cb.is_rc_open = FALSE;
        break;

    case BSA_AVK_REMOTE_RSP_EVT:
        APP_DEBUG0("BSA_AVK_REMOTE_RSP_EVT");
        break;
    case BSA_AVK_VENDOR_CMD_EVT:
        APP_DEBUG0("BSA_AVK_VENDOR_CMD_EVT");
        APP_DEBUG1(" code:0x%x", p_data->vendor_cmd.code);
        APP_DEBUG1(" company_id:0x%x", p_data->vendor_cmd.company_id);
        APP_DEBUG1(" label:0x%x", p_data->vendor_cmd.label);
        APP_DEBUG1(" len:0x%x", p_data->vendor_cmd.len);
#if (defined(BSA_AVK_DUMP_RX_DATA) && (BSA_AVK_DUMP_RX_DATA == TRUE))
        APP_DUMP("A2DP Data", p_data->vendor_cmd.data, p_data->vendor_cmd.len);
#endif
        break;
    case BSA_AVK_VENDOR_RSP_EVT:
        APP_DEBUG0("BSA_AVK_VENDOR_RSP_EVT");
        APP_DEBUG1(" code:0x%x", p_data->vendor_rsp.code);
        APP_DEBUG1(" company_id:0x%x", p_data->vendor_rsp.company_id);
        APP_DEBUG1(" label:0x%x", p_data->vendor_rsp.label);
        APP_DEBUG1(" len:0x%x", p_data->vendor_rsp.len);
#if (defined(BSA_AVK_DUMP_RX_DATA) && (BSA_AVK_DUMP_RX_DATA == TRUE))
        APP_DUMP("A2DP Data", p_data->vendor_rsp.data, p_data->vendor_rsp.len);
#endif
        break;

    case BSA_AVK_CP_INFO_EVT:
        APP_DEBUG0("BSA_AVK_CP_INFO_EVT");
        if(p_data->cp_info.id == BSA_AVK_CP_SCMS_T_ID)
        {
            switch(p_data->cp_info.info.scmst_flag)
            {
                case BSA_AVK_CP_SCMS_COPY_NEVER:
                    APP_INFO1(" content protection:0x%x - COPY NEVER", p_data->cp_info.info.scmst_flag);
                    break;
                case BSA_AVK_CP_SCMS_COPY_ONCE:
                    APP_INFO1(" content protection:0x%x - COPY ONCE", p_data->cp_info.info.scmst_flag);
                    break;
                case BSA_AVK_CP_SCMS_COPY_FREE:
                case (BSA_AVK_CP_SCMS_COPY_FREE+1):
                    APP_INFO1(" content protection:0x%x - COPY FREE", p_data->cp_info.info.scmst_flag);
                    break;
                default:
                    APP_ERROR1(" content protection:0x%x - UNKNOWN VALUE", p_data->cp_info.info.scmst_flag);
                    break;
            }
        }
        else
        {
            APP_INFO0("No content protection");
        }
        break;

    case BSA_AVK_REGISTER_NOTIFICATION_EVT:
    case BSA_AVK_LIST_PLAYER_APP_ATTR_EVT:
    case BSA_AVK_LIST_PLAYER_APP_VALUES_EVT:
    case BSA_AVK_SET_PLAYER_APP_VALUE_EVT:
    case BSA_AVK_GET_PLAYER_APP_VALUE_EVT:
    case BSA_AVK_GET_ELEM_ATTR_EVT:
    case BSA_AVK_GET_PLAY_STATUS_EVT:
    case BSA_AVK_SET_ADDRESSED_PLAYER_EVT:
    case BSA_AVK_SET_BROWSED_PLAYER_EVT:
    case BSA_AVK_GET_FOLDER_ITEMS_EVT:
    case BSA_AVK_CHANGE_PATH_EVT:
    case BSA_AVK_GET_ITEM_ATTR_EVT:
    case BSA_AVK_PLAY_ITEM_EVT:
    case BSA_AVK_ADD_TO_NOW_PLAYING_EVT:
        break;

    default:
        APP_ERROR1("Unsupported event %d", event);
        break;
    }

    /* forward the callback to registered applications */
    if(s_pCallback)
        s_pCallback(event, p_data);
}

/*******************************************************************************
 **
 ** Function         app_avk_open
 **
 ** Description      Example of function to open AVK connection
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_open(void)
{
    tBSA_STATUS status;
    int choice;
    BOOLEAN connect = FALSE;
    BD_ADDR bd_addr;
    UINT8 *p_name;
    tBSA_AVK_OPEN open_param;

    if (app_avk_cb.open_pending)
    {
        APP_ERROR0("already trying to connect");
        return;
    }

    printf("Bluetooth AVK Open menu:\n");
    printf("    0 Device from XML database (already paired)\n");
    printf("    1 Device found in last discovery\n");
    choice = app_get_choice("Select source");

    switch(choice)
    {
    case 0:
        /* Read the XML file which contains all the bonded devices */
        app_read_xml_remote_devices();

        app_xml_display_devices(app_xml_remote_devices_db, APP_NUM_ELEMENTS(app_xml_remote_devices_db));
        choice = app_get_choice("Select device");
        if ((choice >= 0) && (choice < APP_NUM_ELEMENTS(app_xml_remote_devices_db)))
        {
            if (app_xml_remote_devices_db[choice].in_use != FALSE)
            {
                bdcpy(bd_addr, app_xml_remote_devices_db[choice].bd_addr);
                p_name = app_xml_remote_devices_db[choice].name;
                connect = TRUE;
            }
            else
            {
                APP_ERROR0("Device entry not in use");
            }
        }
        else
        {
            APP_ERROR0("Unsupported device number");
        }
        break;
    case 1:
        app_disc_display_devices();
        choice = app_get_choice("Select device");
        if ((choice >= 0) && (choice < APP_NUM_ELEMENTS(app_discovery_cb.devs)))
        {
            if (app_discovery_cb.devs[choice].in_use != FALSE)
            {
                bdcpy(bd_addr, app_discovery_cb.devs[choice].device.bd_addr);
                p_name = app_discovery_cb.devs[choice].device.name;
                connect = TRUE;
            }
            else
            {
                APP_ERROR0("Device entry not in use");
            }
        }
        else
        {
            APP_ERROR0("Unsupported device number");
        }
        break;
    default:
        APP_ERROR0("Unsupported choice");
        break;
    }

    if (connect != FALSE)
    {
        /* Open AVK stream */
        printf("Connecting to AV device:%s \n", p_name);

        app_avk_cb.open_pending = TRUE;

        BSA_AvkOpenInit(&open_param);
        memcpy((char *) (open_param.bd_addr), bd_addr, sizeof(BD_ADDR));
        open_param.channel = BSA_AVK_CHNL_AUDIO;
        open_param.sec_mask = BSA_SEC_NONE;
        status = BSA_AvkOpen(&open_param);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("Unable to connect to device %02X:%02X:%02X:%02X:%02X:%02X with status %d",
                    open_param.bd_addr[0], open_param.bd_addr[1], open_param.bd_addr[2],
                    open_param.bd_addr[3], open_param.bd_addr[4], open_param.bd_addr[5], status);

            app_avk_cb.open_pending = FALSE;
        }
        else
        {
            /* this is an active wait for demo purpose */
            printf("waiting for AV connection to open\n");

            while (app_avk_cb.open_pending == TRUE);

            if (app_avk_cb.is_open == FALSE)

            {
                printf("failure opening AV connection status %d \n", app_avk_cb.last_open_status);
            }
            else
            {
                /* XML Database update should be done upon reception of AV OPEN event */
                APP_DEBUG1("Connected to AV device:%s", p_name);
                /* Read the Remote device xml file to have a fresh view */
                app_read_xml_remote_devices();

                /* Add AV service for this devices in XML database */
                app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr,
                    BSA_A2DP_SERVICE_MASK | BSA_AVRCP_SERVICE_MASK);

                app_xml_update_name_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db), bd_addr, p_name);

                /* Update database => write to disk */
                if (app_write_xml_remote_devices() < 0)
                {
                    APP_ERROR0("Failed to store remote devices database");
                }
            }
        }
    }

}

/*******************************************************************************
 **
 ** Function         app_avk_close
 **
 ** Description      Function to close AVK connection
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_close(void)
{
    tBSA_STATUS status;
    tBSA_AVK_CLOSE bsa_avk_close_param;

    /* Close AVK connection */
    BSA_AvkCloseInit(&bsa_avk_close_param);
    status = BSA_AvkClose(&bsa_avk_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Close AVK connection with status %d", status);
    }

}

/*******************************************************************************
**
** Function         app_avk_open_rc
**
** Description      Function to opens avrc controller connection. AVK should be open before opening rc.
**
** Returns          void
**
*******************************************************************************/
void app_avk_open_rc(void)
{
    tBSA_STATUS status;
    tBSA_AVK_OPEN bsa_avk_open_param;

    /* open avrc connection */
    BSA_AvkOpenRcInit(&bsa_avk_open_param);
    status = BSA_AvkOpenRc(&bsa_avk_open_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to open arvc connection with status %d", status);
    }

}

/*******************************************************************************
**
** Function         app_avk_close_rc
**
** Description      Function to closes avrc controller connection.
**
** Returns          void
**
*******************************************************************************/
void app_avk_close_rc()
{
    tBSA_STATUS status;
    tBSA_AVK_CLOSE bsa_avk_close_param;

    /* close avrc connection */
    BSA_AvkCloseRcInit(&bsa_avk_close_param);

    bsa_avk_close_param.handle = app_avk_cb.rc_handle;

    status = BSA_AvkCloseRc(&bsa_avk_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to close arvc connection with status %d", status);
    }

}

/*******************************************************************************
**
** Function        app_avk_start
**
** Description     Example of function to start an av stream
**
** Returns         void
**
*******************************************************************************/
void app_avk_start(void)
{
    tBSA_STATUS status;
    tBSA_AVK_START req;

    /* Close AVK connection */
    BSA_AvkStartInit(&req);
    status = BSA_AvkStart(&req);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to start AVK connection with status %d", status);
    }
}

/*******************************************************************************
**
** Function        app_avk_start
**
** Description     Example of function to stop an av stream
**
** Returns         void
**
*******************************************************************************/
void app_avk_stop(BOOLEAN suspend)
{
    tBSA_STATUS status;
    tBSA_AVK_STOP req;

    /* Close AVK connection */
    BSA_AvkStopInit(&req);
    req.pause = suspend; /* suspend or local stop*/

    status = BSA_AvkStop(&req);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to stop AVK connection with status %d", status);
    }
}

/*******************************************************************************
**
** Function        app_avk_register
**
** Description     Example of function to register an avk endpoint
**
** Returns         void
**
*******************************************************************************/
int app_avk_register(void)
{
    tBSA_STATUS status;
    tBSA_AVK_REGISTER bsa_avk_register_param;

    /* register an audio AVK end point */
    BSA_AvkRegisterInit(&bsa_avk_register_param);
    bsa_avk_register_param.channel = BSA_AVK_CHNL_AUDIO;
    bsa_avk_register_param.media_sup_format.audio.pcm_supported = TRUE;
    bsa_avk_register_param.media_sup_format.audio.sec_supported = TRUE;
    strncpy(bsa_avk_register_param.service_name, "BSA Audio Service",
        BSA_AVK_SERVICE_NAME_LEN_MAX);

    status = BSA_AvkRegister(&bsa_avk_register_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to register an AV sink with status %d", status);
        return -1;
    }
    /* Save UIPC channel */
    app_avk_cb.uipc_audio_channel = bsa_avk_register_param.uipc_channel;

    /* open the UIPC channel to receive the pcm */
    if (UIPC_Open(bsa_avk_register_param.uipc_channel, app_avk_uipc_cback) == FALSE)
    {
        APP_ERROR0("Unable to open UIPC channel");
        return -1;
    }

    app_avk_cb.fd = -1;

    return 0;

}

/*******************************************************************************
 **
 ** Function        app_avk_deregister
 **
 ** Description     Example of function to deregister an avk endpoint
 **
 ** Returns         void
 **
 *******************************************************************************/
void app_avk_deregister(void)
{
    tBSA_STATUS status;
    tBSA_AVK_DEREGISTER req;

    /* register an audio AVK end point */
    BSA_AvkDeregisterInit(&req);
    status = BSA_AvkDeregister(&req);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to deregister an AV sink with status %d", status);
    }

    app_avk_close_wave_file();

    /* close the UIPC channel receiving the pcm */
    UIPC_Close(app_avk_cb.uipc_audio_channel);
    app_avk_cb.uipc_audio_channel = UIPC_CH_ID_BAD;

}

/*******************************************************************************
**
** Function        app_avk_uipc_cback
**
** Description     uipc audio call back function.
**
** Parameters      pointer on a buffer containing PCM sample with a BT_HDR header
**
** Returns          void
**
*******************************************************************************/
static void app_avk_uipc_cback(BT_HDR *p_msg)
{
#ifdef PCM_ALSA
    snd_pcm_sframes_t alsa_frames;
    snd_pcm_sframes_t alsa_frames_to_send;
#endif
    UINT8 *p_buffer;
    int dummy;

    if (p_msg == NULL)
    {
        return;
    }

    p_buffer = ((UINT8 *)(p_msg + 1)) + p_msg->offset;

    if (app_avk_cb.fd != -1)
    {
        dummy = write(app_avk_cb.fd, p_buffer, p_msg->len);
        (void)dummy;
    }

#if (defined(BSA_AVK_DUMP_RX_DATA) && (BSA_AVK_DUMP_RX_DATA == TRUE))
    APP_DUMP("A2DP Data", p_buffer, p_msg->len);
#endif

#ifdef PCM_ALSA
    if (app_avk_cb.alsa_handle != NULL)
    {
        /* Compute number of PCM samples (contained in p_msg->len bytes) */
        alsa_frames_to_send = p_msg->len / app_avk_cb.num_channel;
        if (app_avk_cb.bit_per_sample == 16)
            alsa_frames_to_send /= 2;

        alsa_frames = snd_pcm_writei(app_avk_cb.alsa_handle, p_buffer, alsa_frames_to_send);
        if (alsa_frames < 0)
        {
            alsa_frames = snd_pcm_recover(app_avk_cb.alsa_handle, alsa_frames, 0);
        }
        if (alsa_frames < 0)
        {
/*            printf("app_avk_uipc_cback snd_pcm_writei failed:%s\n", snd_strerror(alsa_frames)); */
        }
        if (alsa_frames > 0 && alsa_frames < alsa_frames_to_send)
            printf("app_avk_uipc_cback Short write (expected %li, wrote %li)\n",
            (long) alsa_frames_to_send, alsa_frames);
    }
#endif
    GKI_freebuf(p_msg);
}

/*******************************************************************************
**
** Function         app_avk_init
**
** Description      Init Manager application
**
** Parameters       Application callback (if null, default will be used)
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
int app_avk_init(tAvkCallback pcb /* = NULL */)
{
    tBSA_AVK_ENABLE bsa_avk_enable_param;
    tBSA_STATUS status;

    /* register callback */
    s_pCallback = pcb;

    /* Initialize the control structure */
    memset(&app_avk_cb, 0, sizeof(app_avk_cb));
    app_avk_cb.uipc_audio_channel = UIPC_CH_ID_BAD;

    app_avk_cb.features = BSA_AVK_FEATURES;
    app_avk_cb.fd = -1;
    /* get hold on the AVK resource, synchronous mode */
    BSA_AvkEnableInit(&bsa_avk_enable_param);

    bsa_avk_enable_param.sec_mask = BSA_AVK_SECURITY;
    bsa_avk_enable_param.features = BSA_AVK_FEATURES;
    bsa_avk_enable_param.p_cback = app_avk_cback;

    status = BSA_AvkEnable(&bsa_avk_enable_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR0("Unable to enable AVK service");
        return -1;
    }

    return BSA_SUCCESS;

}

/*******************************************************************************
**
** Function         app_avk_rc_send_command
**
** Description      Example of Send a RemoteControl command
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_send_command(UINT8 key_state, UINT8 command)
{
    int status;
    tBSA_AVK_REM_CMD bsa_avk_rc_cmd;

    /* Send remote control command */
    status = BSA_AvkRemoteCmdInit(&bsa_avk_rc_cmd);
    bsa_avk_rc_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_rc_cmd.key_state = (tBSA_AVK_STATE)key_state;
    bsa_avk_rc_cmd.rc_id = (tBSA_AVK_RC)command;
    bsa_avk_rc_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */

    status = BSA_AvkRemoteCmd(&bsa_avk_rc_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send AV RC Command %d", status);
    }
}

/*******************************************************************************
**
** Function         app_avk_rc_send_click
**
** Description      Send press and release states of a command
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_send_click(UINT8 command)
{
    app_avk_rc_send_command(BSA_AV_STATE_PRESS, command);
    GKI_delay(300);
    app_avk_rc_send_command(BSA_AV_STATE_RELEASE, command);
}


/*******************************************************************************
**
** Function         app_avk_play_start
**
** Description      Example of start steam play
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_start(void)
{
    if ((app_avk_cb.features & BTA_AVK_FEAT_RCTG) && app_avk_cb.is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_PLAY);
    }
    else
    {
        app_avk_start();
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_play_stop
 **
 ** Description      Example of stop steam play
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_stop(void)
{
    if ((app_avk_cb.features & BTA_AVK_FEAT_RCTG) && app_avk_cb.is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_STOP);
    }
    else
    {
        app_avk_stop(TRUE);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_play_pause
 **
 ** Description      Example of pause steam pause
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_pause(void)
{
    if ((app_avk_cb.features & BTA_AVK_FEAT_RCTG) && app_avk_cb.is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_PAUSE);
    }
    else
    {
        app_avk_stop(TRUE);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_play_next_track
 **
 ** Description      Example of play next track
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_next_track(void)
{
    if ((app_avk_cb.features & BTA_AVK_FEAT_RCTG) && app_avk_cb.is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_FORWARD);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCTG %d, is rc open %d",
            (app_avk_cb.features & BTA_AVK_FEAT_RCTG), app_avk_cb.is_rc_open);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_play_previous_track
 **
 ** Description      Example of play previous track
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_play_previous_track(void)
{
    if ((app_avk_cb.features & BTA_AVK_FEAT_RCTG) && app_avk_cb.is_rc_open)
    {
        app_avk_rc_send_click(BSA_AVK_RC_BACKWARD);
    }
    else
    {
        APP_ERROR1("Unable to send AVRC command, is support RCCT %d, is rc open %d",
            (app_avk_cb.features & BTA_AVK_FEAT_RCTG), app_avk_cb.is_rc_open);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_cmd
 **
 ** Description      Example of function to open AVK connection
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_cmd(void)
{
    int choice;

    do
    {
        printf("Bluetooth AVK AVRC CMD menu:\n");
        printf("    0 play\n");
        printf("    1 stop\n");
        printf("    2 pause\n");
        printf("    3 forward\n");
        printf("    4 backward\n");
        printf("    5 rewind key_press\n");
        printf("    6 rewind key_release\n");
        printf("    7 fast forward key_press\n");
        printf("    8 fast forward key_release\n");

        printf("    99 exit\n");

        choice = app_get_choice("Select source");

        switch(choice)
        {
            case 0:
                app_avk_rc_send_click(BSA_AVK_RC_PLAY);
                break;

            case 1:
                app_avk_rc_send_click(BSA_AVK_RC_STOP);
                break;

            case 2:
                app_avk_rc_send_click(BSA_AVK_RC_PAUSE);
                break;

            case 3:
                app_avk_rc_send_click(BSA_AVK_RC_FORWARD);
                break;

            case 4:
                app_avk_rc_send_click(BSA_AVK_RC_BACKWARD);
                break;

            case 5:
                app_avk_rc_send_command(BSA_AV_STATE_PRESS, BSA_AVK_RC_REWIND);
                break;

            case 6:
                app_avk_rc_send_command(BSA_AV_STATE_RELEASE, BSA_AVK_RC_REWIND);
                break;

            case 7:
                app_avk_rc_send_command(BSA_AV_STATE_PRESS, BSA_AVK_RC_FAST_FOR);
                break;

            case 8:
                app_avk_rc_send_command(BSA_AV_STATE_RELEASE, BSA_AVK_RC_FAST_FOR);
                break;

            default:
                APP_ERROR0("Unsupported choice");
            break;
        }
    } while (choice != 99);
}


/*******************************************************************************
 **
 ** Function         app_avk_send_get_element_attributes_cmd
 **
 ** Description      Example of function to send Vendor Dependent Command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_send_get_element_attributes_cmd(void)
{
    int status;
    tBSA_AVK_VEN_CMD bsa_avk_vendor_cmd;

    /* Send remote control command */
    status = BSA_AvkVendorCmdInit(&bsa_avk_vendor_cmd);

    bsa_avk_vendor_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_vendor_cmd.ctype = BSA_AVK_CMD_STATUS;
    bsa_avk_vendor_cmd.data[0] = BSA_AVK_RC_VD_GET_ELEMENT_ATTR;
    bsa_avk_vendor_cmd.data[1] = 0; /* reserved & packet type */
    bsa_avk_vendor_cmd.data[2] = 0; /* length high*/
    bsa_avk_vendor_cmd.data[3] = 0x11; /* length low*/

    /* data 4 ~ 11 are 0, which means "identifier 0x0 PLAYING" */

    bsa_avk_vendor_cmd.data[12] = 2; /* num of attributes */

    /* data 13 ~ 16 are 0x1, which means "attribute ID 1 : Title of media" */
    bsa_avk_vendor_cmd.data[16] = 0x1;

    /* data 17 ~ 20 are 0x7, which means "attribute ID 2 : Playing Time" */
    bsa_avk_vendor_cmd.data[20] = 0x7;

    bsa_avk_vendor_cmd.length = 21;
    bsa_avk_vendor_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */

    status = BSA_AvkVendorCmd(&bsa_avk_vendor_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send AV Vendor Command %d", status);
    }
}

/*******************************************************************************
**
** Function         avk_is_open_pending
**
** Description      Check if AVK Open is pending
**
** Parameters
**
** Returns          TRUE if open is pending, FALSE otherwise
**
*******************************************************************************/
BOOLEAN avk_is_open_pending()
{
    return app_avk_cb.open_pending;
}

/*******************************************************************************
**
** Function         avk_set_open_pending
**
** Description      Set AVK open pending
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
void avk_set_open_pending(BOOLEAN bopenpend)
{
    app_avk_cb.open_pending = bopenpend;
}

/*******************************************************************************
**
** Function         avk_is_open
**
** Description      Check if AVK is open
**
** Parameters
**
** Returns          TRUE if AVK is open, FALSE otherwise. Return BDA of the connected device
**
*******************************************************************************/
BOOLEAN avk_is_open(BD_ADDR *bda)
{
    BOOLEAN bopen = app_avk_cb.is_open;
    if(bopen)
    {
        bdcpy(bda, app_avk_cb.bda_connected);
    }

    return bopen;
}

/*******************************************************************************
**
** Function         avk_is_rc_open
**
** Description      Check if AVRC is open
**
** Parameters
**
** Returns          TRUE if AVRC is open, FALSE otherwise.
**
*******************************************************************************/
BOOLEAN avk_is_rc_open()
{
    return app_avk_cb.is_rc_open;
}

/*******************************************************************************
**
** Function         avk_last_open_status
**
** Description      Get the last AVK open status
**
** Parameters
**
** Returns          open status
**
*******************************************************************************/
tBSA_STATUS avk_last_open_status()
{
    return app_avk_cb.last_open_status;
}

/*******************************************************************************
**
** Function         avk_is_started
**
** Description      Check if AVK is started stream
**
** Parameters
**
** Returns          TRUE if AVK is started stream, FALSE otherwise.
**
*******************************************************************************/
BOOLEAN avk_is_started()
{
    return app_avk_cb.is_started;
}

/*******************************************************************************
**
** Function         app_avk_cancel
**
** Description      Example of cancel connection command
**
** Returns          void
**
*******************************************************************************/
void app_avk_cancel()
{
    int status;
    tBSA_AVK_CANCEL_CMD bsa_avk_cancel_cmd;

    /* Send remote control command */
    status = BSA_AvkCancelCmdInit(&bsa_avk_cancel_cmd);

    status = BSA_AvkCancelCmd(&bsa_avk_cancel_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_cancel_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_element_attr_command
 **
 ** Description      Example of get_element_attr_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_element_attr_command()
{
    int status;
    tBSA_AVK_GET_ELEMENT_ATTR bsa_avk_get_elem_attr_cmd;

    UINT8 attrs[]  = {AVRC_MEDIA_ATTR_ID_TITLE,
                      AVRC_MEDIA_ATTR_ID_ARTIST,
                      AVRC_MEDIA_ATTR_ID_ALBUM,
                      AVRC_MEDIA_ATTR_ID_TRACK_NUM,
                      AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                      AVRC_MEDIA_ATTR_ID_GENRE,
                      AVRC_MEDIA_ATTR_ID_PLAYING_TIME};
    UINT8 num_attr  = 7;

    /* Send command */
    status = BSA_AvkGetElementAttrCmdInit(&bsa_avk_get_elem_attr_cmd);
    bsa_avk_get_elem_attr_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_get_elem_attr_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */

    bsa_avk_get_elem_attr_cmd.num_attr = num_attr;
    memcpy(bsa_avk_get_elem_attr_cmd.attrs, attrs, sizeof(attrs));

    status = BSA_AvkGetElementAttrCmd(&bsa_avk_get_elem_attr_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_element_attr_command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_list_player_attr_command
 **
 ** Description      Example of app_avk_rc_list_player_attr_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_list_player_attr_command()
{
    int status;
    tBSA_AVK_LIST_PLAYER_ATTR bsa_avk_list_player_attr_cmd;

    /* Send command */
    status = BSA_AvkListPlayerAttrCmdInit(&bsa_avk_list_player_attr_cmd);
    bsa_avk_list_player_attr_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_list_player_attr_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */

    status = BSA_AvkListPlayerAttrCmd(&bsa_avk_list_player_attr_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_list_player_attr_command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_list_player_attr_value_command
 **
 ** Description      Example of app_avk_rc_list_player_attr_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_list_player_attr_value_command(UINT8 attr)
{
    int status;
    tBSA_AVK_LIST_PLAYER_VALUES bsa_avk_list_player_attr_cmd;

    /* Send command */
    status = BSA_AvkListPlayerValuesCmdInit(&bsa_avk_list_player_attr_cmd);
    bsa_avk_list_player_attr_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_list_player_attr_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */
    bsa_avk_list_player_attr_cmd.attr = attr;

    status = BSA_AvkListPlayerValuesCmd(&bsa_avk_list_player_attr_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_list_player_attr_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_player_value_command
 **
 ** Description      Example of get_player_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_player_value_command(UINT8 *attrs, UINT8 num_attr)
{
    int status;
    tBSA_AVK_GET_PLAYER_VALUE bsa_avk_get_player_val_cmd;

    /* Send command */
    status = BSA_AvkGetPlayerValueCmdInit(&bsa_avk_get_player_val_cmd);
    bsa_avk_get_player_val_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_get_player_val_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */
    bsa_avk_get_player_val_cmd.num_attr = num_attr;

    memcpy(bsa_avk_get_player_val_cmd.attrs, attrs, sizeof(UINT8) * num_attr);

    status = BSA_AvkGetPlayerValueCmd(&bsa_avk_get_player_val_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_player_value_command %d", status);
    }
}



/*******************************************************************************
 **
 ** Function         app_avk_rc_set_player_value_command
 **
 ** Description      Example of set_player_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_player_value_command(UINT8 num_attr, UINT8 *attr_ids, UINT8 * attr_val)
{
    int status;
    tBSA_AVK_SET_PLAYER_VALUE bsa_avk_set_player_val_cmd;

    /* Send command */
    status = BSA_AvkSetPlayerValueCmdInit(&bsa_avk_set_player_val_cmd);
    bsa_avk_set_player_val_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_set_player_val_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */
    bsa_avk_set_player_val_cmd.num_attr = num_attr;

    memcpy(bsa_avk_set_player_val_cmd.player_attr, attr_ids, sizeof(UINT8) * num_attr);
    memcpy(bsa_avk_set_player_val_cmd.player_value, attr_val, sizeof(UINT8) * num_attr);

    status = BSA_AvkSetPlayerValueCmd(&bsa_avk_set_player_val_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_set_player_value_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_play_status_command
 **
 ** Description      Example of get_play_status
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_play_status_command()
{
    int status;
    tBSA_AVK_GET_PLAY_STATUS bsa_avk_play_status_cmd;

    /* Send command */
    status = BSA_AvkGetPlayStatusCmdInit(&bsa_avk_play_status_cmd);
    bsa_avk_play_status_cmd.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_play_status_cmd.label = app_avk_cb.label++; /* Just used to distinguish commands */

    status = BSA_AvkGetPlayStatusCmd(&bsa_avk_play_status_cmd);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_play_status_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_set_browsed_player_command
 **
 ** Description      Example of set_browsed_player
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_browsed_player_command(UINT16  player_id)
{
    int status;
    tBSA_AVK_SET_BROWSED_PLAYER bsa_avk_set_browsed_player;

    /* Send command */
    status = BSA_AvkSetBrowsedPlayerCmdInit(&bsa_avk_set_browsed_player);
    bsa_avk_set_browsed_player.player_id = player_id;
    bsa_avk_set_browsed_player.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_set_browsed_player.label = app_avk_cb.label++; /* Just used to distinguish commands */

    status = BSA_AvkSetBrowsedPlayerCmd(&bsa_avk_set_browsed_player);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_set_browsed_player_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_set_addressed_player_command
 **
 ** Description      Example of set_addressed_player
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_addressed_player_command(UINT16  player_id)
{
    int status;
    tBSA_AVK_SET_ADDR_PLAYER bsa_avk_set_player;

    /* Send command */
    status = BSA_AvkSetAddressedPlayerCmdInit(&bsa_avk_set_player);
    bsa_avk_set_player.player_id = player_id;
    bsa_avk_set_player.rc_handle = app_avk_cb.rc_handle;
    bsa_avk_set_player.label = app_avk_cb.label++; /* Just used to distinguish commands */

    status = BSA_AvkSetAddressedPlayerCmd(&bsa_avk_set_player);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_set_addressed_player_command %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_change_path_command
 **
 ** Description      Example of change_path
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_change_path_command(UINT16   uid_counter, UINT8  direction, tAVRC_UID folder_uid)
{
    int status;
    tBSA_AVK_CHG_PATH bsa_change_path;

    /* Send command */
    status = BSA_AvkChangePathCmdInit(&bsa_change_path);

    bsa_change_path.rc_handle = app_avk_cb.rc_handle;
    bsa_change_path.label = app_avk_cb.label++; /* Just used to distinguish commands */

    bsa_change_path.uid_counter = uid_counter;
    bsa_change_path.direction = direction;
    memcpy(bsa_change_path.folder_uid, folder_uid, sizeof(tAVRC_UID));

    status = BSA_AvkChangePathCmd(&bsa_change_path);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_change_path_command %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_folder_items
 **
 ** Description      Example of get_folder_items
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_folder_items(UINT8  scope, UINT32  start_item, UINT32  end_item)
{
    int status;
    tBSA_AVK_GET_FOLDER_ITEMS bsa_get_folder_items;

    UINT32 attrs[]  = {AVRC_MEDIA_ATTR_ID_TITLE,
                      AVRC_MEDIA_ATTR_ID_ARTIST,
                      AVRC_MEDIA_ATTR_ID_ALBUM,
                      AVRC_MEDIA_ATTR_ID_TRACK_NUM,
                      AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                      AVRC_MEDIA_ATTR_ID_GENRE,
                      AVRC_MEDIA_ATTR_ID_PLAYING_TIME};
    UINT8 num_attr  = 7;

    /* Send command */
    status = BSA_AvkGetFolderItemsCmdInit(&bsa_get_folder_items);

    bsa_get_folder_items.rc_handle = app_avk_cb.rc_handle;
    bsa_get_folder_items.label = app_avk_cb.label++; /* Just used to distinguish commands */

    bsa_get_folder_items.scope = scope;
    bsa_get_folder_items.start_item = start_item;
    bsa_get_folder_items.end_item = end_item;

    if(AVRC_SCOPE_PLAYER_LIST != scope)
    {
        bsa_get_folder_items.attr_count = num_attr;
        memcpy(bsa_get_folder_items.attrs, attrs, sizeof(attrs));
    }


    status = BSA_AvkGetFolderItemsCmd(&bsa_get_folder_items);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_folder_items %d", status);
    }
}

/*******************************************************************************
 **
 ** Function         app_avk_rc_get_items_attr
 **
 ** Description      Example of get_items_attr
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_items_attr(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter)
{
    int status;
    tBSA_AVK_GET_ITEMS_ATTR bsa_get_items_attr;

    UINT8 attrs[]  = {AVRC_MEDIA_ATTR_ID_TITLE,
                      AVRC_MEDIA_ATTR_ID_ARTIST,
                      AVRC_MEDIA_ATTR_ID_ALBUM,
                      AVRC_MEDIA_ATTR_ID_TRACK_NUM,
                      AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                      AVRC_MEDIA_ATTR_ID_GENRE,
                      AVRC_MEDIA_ATTR_ID_PLAYING_TIME};
    UINT8 num_attr  = 7;

    /* Send command */
    status = BSA_AvkGetItemsAttrCmdInit(&bsa_get_items_attr);

    bsa_get_items_attr.rc_handle = app_avk_cb.rc_handle;
    bsa_get_items_attr.label = app_avk_cb.label++; /* Just used to distinguish commands */

    bsa_get_items_attr.scope = scope;
    memcpy(bsa_get_items_attr.uid, uid, sizeof(tAVRC_UID));
    bsa_get_items_attr.uid_counter = uid_counter;
    bsa_get_items_attr.attr_count = num_attr;
    memcpy(bsa_get_items_attr.attrs, attrs, sizeof(attrs));

    status = BSA_AvkGetItemsAttrCmd(&bsa_get_items_attr);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_get_items_attr %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_play_item
 **
 ** Description      Example of play_item
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_play_item(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter)
{
    int status;
    tBSA_AVK_PLAY_ITEM bsa_play_item;


    /* Send command */
    status = BSA_AvkPlayItemCmdInit(&bsa_play_item);

    bsa_play_item.rc_handle = app_avk_cb.rc_handle;
    bsa_play_item.label = app_avk_cb.label++; /* Just used to distinguish commands */

    bsa_play_item.scope = scope;
    memcpy(bsa_play_item.uid, uid, sizeof(tAVRC_UID));
    bsa_play_item.uid_counter = uid_counter;

    status = BSA_AvkPlayItemCmd(&bsa_play_item);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_play_item %d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_avk_rc_add_to_play
 **
 ** Description      Example of add_to_play
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_add_to_play(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter)
{
    int status;
    tBSA_AVK_ADD_TO_PLAY bsa_add_play;


    /* Send command */
    status = BSA_AvkAddToPlayCmdInit(&bsa_add_play);

    bsa_add_play.rc_handle = app_avk_cb.rc_handle;
    bsa_add_play.label = app_avk_cb.label++; /* Just used to distinguish commands */

    bsa_add_play.scope = scope;
    memcpy(bsa_add_play.uid, uid, sizeof(tAVRC_UID));
    bsa_add_play.uid_counter = uid_counter;

    status = BSA_AvkAddToPlayCmd(&bsa_add_play);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send app_avk_rc_add_to_play %d", status);
    }
}
