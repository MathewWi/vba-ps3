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

#define SETTING_CURRENT_SAVE_STATE_SLOT 0
#define SETTING_CHANGE_RESOLUTION 1
#define SETTING_KEEP_ASPECT_RATIO 2
#define SETTING_HW_TEXTURE_FILTER 3
#define SETTING_HW_OVERSCAN_AMOUNT 4
#define SETTING_CONTROL_STYLE 5
#define SETTING_SHADER 6
#define SETTING_DEFAULT_ALL 7

void MenuMainLoop(void);

void MenuStop();
bool MenuIsRunning();

#endif /* MENU_H_ */
