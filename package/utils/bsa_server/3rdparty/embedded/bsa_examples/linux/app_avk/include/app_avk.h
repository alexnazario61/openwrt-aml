/*****************************************************************************
**
**  Name:           app_avk.h
**
**  Description:    Bluetooth Audio/Video Streaming application
**
**  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef APP_AVK_H_
#define APP_AVK_H_
#include "data_types.h"

/* Callback to application for connection events */
typedef void ( tAvkCallback)(tBSA_AVK_EVT event, tBSA_AVK_MSG *p_data);

/*******************************************************************************
**
** Function         app_avk_init
**
** Description      Init AVK application
**
** Parameters       Application callback (if null, default will be used)
**
** Returns          0 if successful, error code otherwise
**
*******************************************************************************/
int app_avk_init(tAvkCallback pcb /* = NULL */);

/*******************************************************************************
**
** Function         app_avk_end
**
** Description      This function is used to close application
**
** Returns          void
**
*******************************************************************************/
void app_avk_end(void);

/*******************************************************************************
**
** Function         app_avk_register
**
** Description      Register a new AV sink point
**
** Returns          0 succees
**
*******************************************************************************/
int app_avk_register(void);

/*******************************************************************************
**
** Function         app_avk_deregister
**
** Description      DeRegister an AV sink point
**
** Returns          void
**
*******************************************************************************/
void app_avk_deregister(void);

/*******************************************************************************
**
** Function         app_avk_open
**
** Description      Function to open AV connection
**
** Returns          void
**
*******************************************************************************/
void app_avk_open(void);

/*******************************************************************************
**
** Function         app_avk_close
**
** Description      Function to close AVK connection
**
** Returns          void
**
*******************************************************************************/
void app_avk_close(void);

/*******************************************************************************
**
** Function         app_avk_open_rc
**
** Description      Function to opens avrc controller connection. AVK should be open before opening rc.
**
** Returns          void
**
*******************************************************************************/
void app_avk_open_rc(void);

/*******************************************************************************
**
** Function         app_avk_close_rc
**
** Description      Function to closes avrc controller connection.
**
** Returns          void
**
*******************************************************************************/
void app_avk_close_rc(void);

/*******************************************************************************
**
** Function         app_avk_start
**
** Description      Resume playing
**
** Returns          void
**
*******************************************************************************/
void app_avk_start(void);

/*******************************************************************************
**
** Function         app_avk_stop
**
** Description      Stop AVK stream
**
** Returns          void
**
*******************************************************************************/
void app_avk_stop(BOOLEAN suspend);


/*******************************************************************************
**
** Function         app_avk_cancel
**
** Description      cancel connection command
**
** Returns          void
**
*******************************************************************************/
void app_avk_cancel();

/*******************************************************************************
**
** Function         app_avk_play_start
**
** Description      Example of start steam play
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_start(void);

/*******************************************************************************
**
** Function         app_avk_play_stop
**
** Description      Example of stop steam play
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_stop(void);

/*******************************************************************************
**
** Function         app_avk_play_pause
**
** Description      Example of pause steam pause
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_pause(void);

/*******************************************************************************
**
** Function         app_avk_play_next_track
**
** Description      Example of play next track
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_next_track(void);

/*******************************************************************************
**
** Function         app_avk_play_previous_track
**
** Description      Example of play previous track
**
** Returns          void
**
*******************************************************************************/
void app_avk_play_previous_track(void);

/*******************************************************************************
**
** Function         app_avk_rc_cmd
**
** Description      Example of function to open AVK connection
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_cmd(void);

/*******************************************************************************
**
** Function         app_avk_rc_send_command
**
** Description      Example of Send a RemoteControl command
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_send_command(UINT8 key_state, UINT8 command);

/*******************************************************************************
**
** Function         app_avk_rc_send_click
**
** Description      Send press and release states of a command
**
** Returns          void
**
*******************************************************************************/
void app_avk_rc_send_click(UINT8 command);

/*******************************************************************************
**
** Function         app_avk_send_get_element_attributes_cmd
**
** Description      Example of function to send Vendor Dependent Command
**
** Returns          void
**
*******************************************************************************/
void app_avk_send_get_element_attributes_cmd(void);

/*******************************************************************************
 **
 ** Function         app_avk_rc_get_element_attr_command
 **
 ** Description      Example of function to send get_element_attr Command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_element_attr_command(void);

/*******************************************************************************
 **
 ** Function         app_avk_rc_list_player_attr_command
 **
 ** Description      Example of function to send list_player_attr Command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_list_player_attr_command(void);

/*******************************************************************************
 **
 ** Function         app_avk_rc_list_player_attr_value_command
 **
 ** Description      Example of function to send list_player_attr value Command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_list_player_attr_value_command(UINT8 attr);

/*******************************************************************************
 **
 ** Function         app_avk_rc_get_player_value_command
 **
 ** Description      Example of get_player_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_player_value_command(UINT8 *attrs, UINT8 num_attr);

/*******************************************************************************
 **
 ** Function         app_avk_rc_set_player_value_command
 **
 ** Description      Example of set_player_value_command command
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_player_value_command(UINT8 num_attr, UINT8 *attr_ids, UINT8 * attr_val);

/*******************************************************************************
 **
 ** Function         app_avk_rc_get_play_status_command
 **
 ** Description      Example of get_play_status
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_play_status_command();

/*******************************************************************************
 **
 ** Function         app_avk_rc_set_browsed_player_command
 **
 ** Description      Example of set_browsed_player
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_browsed_player_command(UINT16  player_id);

/*******************************************************************************
 **
 ** Function         app_avk_rc_set_addressed_player_command
 **
 ** Description      Example of set_addressed_player
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_set_addressed_player_command(UINT16  player_id);


/*******************************************************************************
 **
 ** Function         app_avk_rc_change_path_command
 **
 ** Description      Example of change_path
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_change_path_command(UINT16   uid_counter, UINT8  direction, tAVRC_UID folder_uid);


/*******************************************************************************
 **
 ** Function         app_avk_rc_get_folder_items
 **
 ** Description      Example of get_folder_items
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_folder_items(UINT8  scope, UINT32  start_item, UINT32  end_item);

/*******************************************************************************
 **
 ** Function         app_avk_rc_get_items_attr
 **
 ** Description      Example of get_items_attr
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_get_items_attr(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter);

/*******************************************************************************
 **
 ** Function         app_avk_rc_play_item
 **
 ** Description      Example of play_item
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_play_item(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter);


/*******************************************************************************
 **
 ** Function         app_avk_rc_add_to_play
 **
 ** Description      Example of add_to_play
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_avk_rc_add_to_play(UINT8  scope, tAVRC_UID  uid, UINT16  uid_counter);


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
BOOLEAN avk_is_open(BD_ADDR *bda);


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
BOOLEAN avk_is_rc_open();

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
BOOLEAN avk_is_open_pending();


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
void avk_set_open_pending(BOOLEAN bopenpend);

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
tBSA_STATUS avk_last_open_status();

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
BOOLEAN avk_is_started();

#endif /* APP_AVK_H_ */
