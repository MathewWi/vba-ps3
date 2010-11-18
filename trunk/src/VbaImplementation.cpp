/*
 * Vba_Implementation.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */
#include "VbaImplementation.h"

#include "vba/common/SoundDriver.h"
#include "vba/gba/Globals.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/utility/utils.h"

#include "VbaPs3.h"
#include "VbaAudio.h"


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
    systemRedShift = 11;
    systemGreenShift = 6;
    systemBlueShift = 0;
    for(i = 0; i < 0x10000; i++)
    {
            systemColorMap16[i] =
                    ((i & 0x1f) << systemRedShift) |
                    (((i & 0x3e0) >> 5) << systemGreenShift) |
                    (((i & 0x7c00) >> 10) << systemBlueShift);
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
	LOG_DBG("systemDrawScreen()\n");
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
	//LOG_DBG("systemReadJoypad(%d)\n", pad);

	return 0;
}


uint32_t systemGetClock()
{
	LOG_DBG("systemGetClock()\n");

	uint32_t time = get_usec() * 1000;
	LOG("TIME: %u", time);
	return time;
    //uint32_t now = sys_time_get_system_time();
    //return now - startTime;
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
	LOG_DBG("system10Frames(%d)\n", rate);
}


 void systemFrame()
{
	 LOG_DBG("systemFrame()\n");
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

