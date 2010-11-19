/*
 * Vba_Implementation.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */
#include "VbaImplementation.h"

#include "vba/gba/Sound.h"
#include "vba/common/SoundDriver.h"
#include "vba/gba/Globals.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/utility/utils.h"

#include "conf/conffile.h"

#include "VbaPs3.h"
#include "VbaAudio.h"


#define VBA_BUTTON_A            1
#define VBA_BUTTON_B            2
#define VBA_BUTTON_SELECT       4
#define VBA_BUTTON_START        8
#define VBA_RIGHT                       16
#define VBA_LEFT                        32
#define VBA_UP                          64
#define VBA_DOWN                        128
#define VBA_BUTTON_R            256
#define VBA_BUTTON_L            512
#define VBA_SPEED                       1024
#define VBA_CAPTURE                     2048


// VBA - must define these
int RGB_LOW_BITS_MASK = 0;

u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift = 0;
int systemBlueShift = 0;
int systemGreenShift = 0;
int systemColorDepth = 16;
int systemDebug = 0;
int systemVerbose = 1;
int systemFrameSkip = 0;
int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
int systemSpeed = 0;

void (*dbgOutput)(const char *s, u32 addr);
void (*dbgSignal)(int sig,int number);

// start time of the system
uint32_t startTime = 0;

extern void log(const char * fmt,...)
{
	LOG("VbaLog:\n\t");

	va_list args;
	va_start(args,fmt);
	LOG(fmt, args);
	va_end(args);

	LOG("\n");
}


// shinhalsafar: Vba should force this... there is some setup.
bool systemInit()
{
	LOG_DBG("systemInit()\n");

	startTime = sys_time_get_system_time();

    int i;
    // Build GBPalette
    for( i = 0; i < 24; )
    {
            systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
            systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
            systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
            systemGbPalette[i++] = 0;
    }

    // Set palette etc - Fixed to RGB565
    systemColorDepth = 16;

    systemRedShift    = 19;
    systemGreenShift  = 11;
    systemBlueShift   = 3;

    //systemRedShift = 11;
    //systemGreenShift = 6;
    //systemBlueShift = 0;

    RGB_LOW_BITS_MASK = 0x00010101;

    for(i = 0; i < 0x10000; i++)
    {
		systemColorMap16[i] =
				((i & 0x1f) << systemRedShift) |
				(((i & 0x3e0) >> 5) << systemGreenShift) |
				(((i & 0x7c00) >> 10) << systemBlueShift);
    }

    for (int i = 0; i < 0x10000; i++)
    {
      systemColorMap32[i] = (((i & 0x1f) << systemRedShift)
                             | (((i & 0x3e0) >> 5) << systemGreenShift)
                             | (((i & 0x7c00) >> 10) << systemBlueShift));
    }

	return true;
}


bool systemPauseOnFrame()
{
	LOG_DBG("systemPauseOnFrame()\n");

	return false;
}


void systemGbPrint(u8 *data,int pages,int feed,int palette, int contrast)
{
	LOG_DBG("systemGbPrint(params not included)\n");
}


void systemScreenCapture(int a)
{
	LOG_DBG("systemScreenCapture(%d)\n", a);
}


void systemDrawScreen()
{
	//LOG_DBG("systemDrawScreen()\n");
	Graphics->Draw(pix);
}


// updates the joystick data
bool systemReadJoypads()
{
	//LOG_DBG("systemReadJoypads()\n");

	return true;
}


// return information about the given joystick, -1 for default joystick
uint32_t systemReadJoypad(int pad)
{
	u32 J = 0;

	LOG_DBG("systemReadJoypad(%d)\n", pad);
	if (pad == -1) pad = 0;

	int i = pad;
	if (CellInput->UpdateDevice(i) != CELL_PAD_OK)
	{
		return J;
	}

	if (Settings.FCEUControlstyle == CONTROL_STYLE_BETTER)
	{
		if (CellInput->IsButtonPressed(i, CTRL_CIRCLE))
		{
			J = VBA_BUTTON_B;
		}
		if (CellInput->IsButtonPressed(i, CTRL_CROSS))
		{
			J |= VBA_BUTTON_A;
		}
	}
	else
	{
		if (CellInput->IsButtonPressed(i, CTRL_CIRCLE))
		{
			J = VBA_BUTTON_A;
		}
		if (CellInput->IsButtonPressed(i, CTRL_CROSS))
		{
			J |= VBA_BUTTON_B;
		}
	}

	if (CellInput->IsButtonPressed(i, CTRL_TRIANGLE))
	{
		J |= VBA_BUTTON_A;
	}
	if (CellInput->IsButtonPressed(i, CTRL_SQUARE))
	{
		J |= VBA_BUTTON_B;
	}

	if (CellInput->IsButtonPressed(i, CTRL_L1))
	{
		J |= VBA_BUTTON_L;
	}
	if (CellInput->IsButtonPressed(i, CTRL_R1))
	{
		J |= VBA_BUTTON_R;
	}

	if (CellInput->IsButtonPressed(i, CTRL_UP) | CellInput->IsAnalogPressedUp(i, CTRL_LSTICK))
	{
		J |= VBA_UP;
	}
	if (CellInput->IsButtonPressed(i, CTRL_DOWN) | CellInput->IsAnalogPressedDown(i, CTRL_LSTICK))
	{
		J |= VBA_DOWN;
	}
	if (CellInput->IsButtonPressed(i, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(i, CTRL_LSTICK))
	{
		J |= VBA_LEFT;
	}
	if (CellInput->IsButtonPressed(i, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(i, CTRL_LSTICK))
	{
		J |= VBA_RIGHT;
	}

	if (CellInput->IsButtonPressed(i, CTRL_START))
	{
		J |= VBA_BUTTON_START;
	}
	if (CellInput->IsButtonPressed(i, CTRL_SELECT))
	{
		J |= VBA_BUTTON_SELECT;
	}

	if (CellInput->IsButtonPressed(i, CTRL_R2))
	{
		J |= VBA_SPEED;
	}

	if (CellInput->IsButtonPressed(i, CTRL_L3) && CellInput->IsButtonPressed(i, CTRL_R3))
	{
		Emulator_StopROMRunning();
		Emulator_SwitchMode(MODE_MENU);
	}

	return J;
}


uint32_t systemGetClock()
{
	LOG_DBG("systemGetClock()\n");

	/*uint32_t time = get_usec() * 1000;
	LOG("TIME: %u", time);
	return time;*/

    uint32_t now = sys_time_get_system_time();
    return now - startTime;

    //return diff_usec(start, now) / 1000;
}


void systemMessage(int id, const char * fmt, ...)
{
	LOG_DBG("systemMessage(%d)\n\t", id);

	va_list args;
	va_start(args,fmt);
	LOG(fmt, args);
	va_end(args);

	LOG("\n");
}


void systemSetTitle(const char * title)
{
	LOG_DBG("systemSetTitle(%s)\n", title);
}


SoundDriver* systemSoundInit()
{
	LOG_DBG("systemSoundInit()\n");

	// VBA - shutdown sound
	soundShutdown();

	// Port - return audio interface for VBA
	return new VbaAudio();
}


void systemOnWriteDataToSoundBuffer(const uint16_t * finalWave, int length)
{

}


void systemOnSoundShutdown()
{

}


void systemScreenMessage(const char *)
{

}


void systemUpdateMotionSensor()
{

}


int  systemGetSensorX()
{
	return 0;
}


int  systemGetSensorY()
{
	return 0;
}


bool systemCanChangeSoundQuality()
{
	LOG_DBG("systemCanChangeSoundQuality()\n");

	return false;
}


void systemShowSpeed(int speed)
{
	LOG_DBG("systemShowSpeed(%d)\n", speed);
}


void system10Frames(int rate)
{
	//LOG_DBG("system10Frames(%d)\n", rate);
	systemFrameSkip = 0;
}


 void systemFrame()
{
	 //LOG_DBG("systemFrame()\n");
}


void systemGbBorderOn()
{
	LOG_DBG("systemGbBorderOn()\n");
}


 void Sm60FPS_Init()
{
	 LOG_DBG("Sm60FPS_Init()\n");
}


bool Sm60FPS_CanSkipFrame()
{
	LOG_DBG("Sm60FPS_CanSkipFrame()\n");

	return false;
}


void Sm60FPS_Sleep()
{
	LOG_DBG("Sm60FPS_Sleep()\n");
}


void DbgMsg(const char *msg, ...)
{

}


void winlog(const char *,...)
{

}

