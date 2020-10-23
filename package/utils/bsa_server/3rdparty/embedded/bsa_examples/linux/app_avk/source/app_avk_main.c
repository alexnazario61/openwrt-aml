/*****************************************************************************
 **
 **  Name:           app_avk_main.c
 **
 **  Description:    Bluetooth Manager application
 **
 **  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "bsa_api.h"

#include "gki.h"
#include "uipc.h"

#include "app_avk.h"
#include "bsa_avk_api.h"
#include "app_xml_param.h"
#include "app_mgt.h"
#include "app_disc.h"
#include "app_utils.h"

/* Menu items */
enum
{
    APP_AVK_MENU_DISCOVERY = 1,
    APP_AVK_MENU_REGISTER,
    APP_AVK_MENU_DEREGISTER,
    APP_AVK_MENU_OPEN,
    APP_AVK_MENU_CLOSE,
    APP_AVK_MENU_START,
    APP_AVK_MENU_STOP,
    APP_AVK_MENU_PLAY_START,
    APP_AVK_MENU_PLAY_STOP,
    APP_AVK_MENU_PLAY_PAUSE,
    APP_AVK_MENU_PLAY_NEXT_TRACK,
    APP_AVK_MENU_PLAY_PREVIOUS_TRACK,
    APP_AVK_MENU_RC_CMD,
    APP_AVK_MENU_QUIT = 99
};


/*******************************************************************************
 **
 ** Function         app_avk_display_main_menu
 **
 ** Description      This is the main menu
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_display_main_menu(void)
{
    printf("\nBluetooth AVK Main menu:\n");

    printf("    %d  => Start Discovery\n", APP_AVK_MENU_DISCOVERY);
    printf("    %d  => AVK Register\n", APP_AVK_MENU_REGISTER);
    printf("    %d  => AVK DeRegister\n", APP_AVK_MENU_DEREGISTER);
    printf("    %d  => AVK Open\n", APP_AVK_MENU_OPEN);
    printf("    %d  => AVK Close\n", APP_AVK_MENU_CLOSE);
    printf("    %d  => AVK Start\n", APP_AVK_MENU_START);
    printf("    %d  => AVK Stop\n", APP_AVK_MENU_STOP);
    printf("    %d  => AVK Start steam play\n", APP_AVK_MENU_PLAY_START);
    printf("    %d  => AVK Stop stream play\n", APP_AVK_MENU_PLAY_STOP);
    printf("    %d  => AVK Pause stream play\n", APP_AVK_MENU_PLAY_PAUSE);
    printf("    %d  => AVK Play next track\n", APP_AVK_MENU_PLAY_NEXT_TRACK);
    printf("    %d  => AVK Play previous track\n", APP_AVK_MENU_PLAY_PREVIOUS_TRACK);
    printf("    %d  => AVRC Command\n", APP_AVK_MENU_RC_CMD);
    printf("    %d  => Quit\n", APP_AVK_MENU_QUIT);
}


/*******************************************************************************
 **
 ** Function         app_avk_mgt_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          TRUE if the event was completely handle, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN app_avk_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_STATUS_EVT:
        APP_DEBUG0("BSA_MGT_STATUS_EVT");
        if (p_data->status.enable)
        {
            APP_DEBUG0("Bluetooth restarted => re-initialize the application");
            app_avk_init(NULL);
        }
        break;

    case BSA_MGT_DISCONNECT_EVT:
        APP_DEBUG1("BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
        /* Connection with the Server lost => Just exit the application */
        exit(-1);
        break;

    default:
        break;
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         main
 **
 ** Description      This is the main function to connect to AVK. It is assumed that an other process handle security.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int main(int argc, char **argv)
{
    int choice;

    /* Open connection to BSA Server */
    app_mgt_init();
    if (app_mgt_open(NULL, app_avk_mgt_callback) < 0)
    {
        APP_ERROR0("Unable to connect to server");
        return -1;
    }

    /* Init XML state machine */
    app_xml_init();

    app_avk_init(NULL);

    do
    {
        app_avk_display_main_menu();

        choice = app_get_choice("Select action");

        switch (choice)
        {

        case APP_AVK_MENU_DISCOVERY:
            /* Example to perform Device discovery (in blocking mode) */
            app_disc_start_regular(NULL);
            break;

        case APP_AVK_MENU_REGISTER:
            /* Example to register AVK connection end point*/
            app_avk_register();
            break;
        case APP_AVK_MENU_DEREGISTER:
            /* Example to de-register an AVK end point */
            app_avk_deregister();
            break;
        case APP_AVK_MENU_OPEN:
            /* Example to Open AVK connection (connect device)*/
            app_avk_open();
            break;

        case APP_AVK_MENU_CLOSE:
            /* Example to Close AVK connection (disconnect device)*/
            app_avk_close();
            break;

        case APP_AVK_MENU_START:
            /* Example to start AVK stream */
            app_avk_start();
            break;

        case APP_AVK_MENU_STOP:
            /* Example to stop AVK stream */
            app_avk_stop(TRUE);
            break;

        case APP_AVK_MENU_PLAY_START:
            /* Example to start stream play */
            app_avk_play_start();
            break;

        case APP_AVK_MENU_PLAY_STOP:
            /* Example to stop stream play */
            app_avk_play_stop();
            break;  

        case APP_AVK_MENU_PLAY_PAUSE:
            /* Example to pause stream play */
            app_avk_play_pause();
            break;

        case APP_AVK_MENU_PLAY_NEXT_TRACK:
            /* Example to play next track */
             app_avk_play_next_track();
            break;

        case APP_AVK_MENU_PLAY_PREVIOUS_TRACK:
            /* Example to play previous track */
            app_avk_play_previous_track();
            break;  

        case APP_AVK_MENU_RC_CMD:
            /* Example to send AVRC cmd */
            app_avk_rc_cmd();
            break;          

        case APP_AVK_MENU_QUIT:
            printf("main: Bye Bye\n");
            break;

        default:
            printf("main: Unknown choice:%d\n", choice);
            break;
        }
    } while (choice != 99); /* While user don't exit application */

    /* Terminate the profile */
    app_avk_end();


    /* Close BSA before exiting (to release resources) */
    app_mgt_close();

    return 0;
}
