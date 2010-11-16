/*
 * Vba_Implementation.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */


#include "VbaImplementation.h"

#include "vba/common/SoundDriver.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/utility/utils.h"

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
	startTime = sys_time_get_system_time();
}


bool systemPauseOnFrame()
{
	return false;
}


void systemGbPrint(uint8_t *,int,int,int,int)
{

}


void systemScreenCapture(int)
{

}


void systemDrawScreen()
{

}


// updates the joystick data
bool systemReadJoypads()
{

}


// return information about the given joystick, -1 for default joystick
uint32_t systemReadJoypad(int)
{

}


uint32_t systemGetClock()
{
    uint32_t now = sys_time_get_system_time();
    return now - startTime;
    //return diff_usec(start, now) / 1000;
}


void systemMessage(int id, const char * fmt, ...)
{
	LOG("systemMessage(%d)\n\t", id);

	va_list args;
	va_start(args,fmt);
	LOG(fmt, args);
	va_end(args);

	LOG("\n");
}


void systemSetTitle(const char *)
{

}


SoundDriver* systemSoundInit()
{
	return NULL;
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
	return false;
}


void systemShowSpeed(int)
{

}


void system10Frames(int)
{

}


 void systemFrame()
{

}


void systemGbBorderOn()
{

}


 void Sm60FPS_Init()
{

}


bool Sm60FPS_CanSkipFrame()
{

}


void Sm60FPS_Sleep()
{

}


void DbgMsg(const char *msg, ...)
{

}


void winlog(const char *,...)
{

}

