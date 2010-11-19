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

//setting constants
/*
#define SETTING_DISPLAY_FRAMERATE 0
#define SETTING_SOUND_INPUT_RATE 1
#define SETTING_TRANSPARENCY 2
#define SETTING_SKIP_FRAMES 3
#define SETTING_DISABLE_GRAPHIC_WINDOWS 4
#define SETTING_DISPLAY_PRESSED_KEYS 5
#define SETTING_FORCE_NTSC 6
#define SETTING_PAL_TIMING 7
#define SETTING_SHUTDOWN_MASTER 8
*/

#define SETTING_CURRENT_SAVE_STATE_SLOT 0
#define SETTING_CHANGE_RESOLUTION 1
#define SETTING_KEEP_ASPECT_RATIO 2
#define SETTING_HW_TEXTURE_FILTER 3
#define SETTING_CONTROL_STYLE 4
#define SETTING_SHADER 5
#define SETTING_DEFAULT_ALL 6

void MenuMainLoop(void);

void MenuStop();
bool MenuIsRunning();


#endif /* MENU_H_ */
