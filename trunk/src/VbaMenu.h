/*
 * menu.h
 *
 *  Created on: Oct 10, 2010
 *      Author: halsafar
 */

#ifndef MENU_H_
#define MENU_H_

#include <string>
using namespace::std;

#include "colors.h"

//lidbgfont
#define FONT_SIZE 1.0f

#define SETTING_DRAW_FPS 0
#define SETTING_CURRENT_SAVE_STATE_SLOT 1
#define SETTING_CHANGE_RESOLUTION 2
#define SETTING_KEEP_ASPECT_RATIO 3
#define SETTING_HW_TEXTURE_FILTER 4
#define SETTING_HW_OVERSCAN_AMOUNT 5
#define SETTING_CONTROL_STYLE 6
#define SETTING_SHADER 7
#define SETTING_GBABIOS 8
#define SETTING_PATH_SAVESTATES_DIRECTORY 9
#define SETTING_PATH_SRAM_DIRECTORY 10
#define SETTING_PATH_DEFAULT_ROM_DIRECTORY 11
#define SETTING_DEFAULT_ALL 12
#define SETTING_RSOUND_ENABLED 13
#define SETTING_RSOUND_SERVER_IP_ADDRESS 14


void MenuMainLoop(void);

void MenuStop();
bool MenuIsRunning();

#endif /* MENU_H_ */
