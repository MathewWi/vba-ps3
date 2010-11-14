/*
 * Vba_Implementation.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */

#ifndef VBA_IMPLEMENTATION_CPP_
#define VBA_IMPLEMENTATION_CPP_

#include "VbaImplementation.h"

#include "vba/common/SoundDriver.h"

#include "cellframework/logger/Logger.h"

int RGB_LOW_BITS_MASK = 0;

u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift;
int systemGreenShift;
int systemBlueShift;
int systemColorDepth;
int systemDebug;
int systemVerbose;
int systemFrameSkip;
int systemSaveUpdateCounter;
int systemSpeed;

void (*dbgOutput)(const char *s, u32 addr);
void (*dbgSignal)(int sig,int number);


extern void log(const char *,...)
{

}


extern bool systemPauseOnFrame()
{

}


extern void systemGbPrint(uint8_t *,int,int,int,int)
{

}


extern void systemScreenCapture(int)
{

}


extern void systemDrawScreen()
{

}


// updates the joystick data
extern bool systemReadJoypads()
{

}


// return information about the given joystick, -1 for default joystick
extern uint32_t systemReadJoypad(int)
{

}


extern uint32_t systemGetClock()
{

}


extern void systemMessage(int, const char *, ...)
{

}
extern void systemSetTitle(const char *)
{

}
extern SoundDriver* systemSoundInit()
{

}
extern void systemOnWriteDataToSoundBuffer(const uint16_t * finalWave, int length)
{

}
extern void systemOnSoundShutdown()
{

}
extern void systemScreenMessage(const char *)
{

}
extern void systemUpdateMotionSensor()
{

}
extern int  systemGetSensorX()
{

}
extern int  systemGetSensorY()
{

}
extern bool systemCanChangeSoundQuality()
{

}
extern void systemShowSpeed(int)
{

}
extern void system10Frames(int)
{

}
extern void systemFrame()
{

}
extern void systemGbBorderOn()
{

}

extern void Sm60FPS_Init()
{

}
extern bool Sm60FPS_CanSkipFrame()
{

}
extern void Sm60FPS_Sleep()
{

}
extern void DbgMsg(const char *msg, ...)
{

}
extern void winlog(const char *,...)
{

}


#endif /* VBA_IMPLEMENTATION_CPP_ */
