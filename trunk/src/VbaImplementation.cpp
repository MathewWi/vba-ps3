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
#include "vba/Util.h"
#include "vba/gb/gb.h"
#include "vba/gb/gbGlobals.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/utility/utils.h"

#include "conf/conffile.h"

#include "VbaPs3.h"
#include "VbaAudio.h"


#define VBA_BUTTON_A            1
#define VBA_BUTTON_B            2
#define VBA_BUTTON_SELECT       4
#define VBA_BUTTON_START        8
#define VBA_RIGHT               16
#define VBA_LEFT                32
#define VBA_UP                  64
#define VBA_DOWN                128
#define VBA_BUTTON_R            256
#define VBA_BUTTON_L            512
#define VBA_SPEED               1024
#define VBA_CAPTURE             2048


// VBA - must define these
int RGB_LOW_BITS_MASK = 0;

u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift = 0;
int systemBlueShift = 0;
int systemGreenShift = 0;
int systemColorDepth = 32;
int systemDebug = 0;
int systemVerbose = 0;
int systemFrameSkip = 0;
int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
int systemSpeed = 0;

void (*dbgOutput)(const char *s, u32 addr);
void (*dbgSignal)(int sig,int number);

// start time of the system
uint64_t startTime = 0;

uint32_t renderedFrames = 0;

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

	startTime = get_usec();
	renderedFrames = 0;

    // Build GBPalette
    int i = 0;
    for( i = 0; i < 24; )
    {
            systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
            systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
            systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
            systemGbPalette[i++] = 0;
    }

    systemColorDepth = 32;

    // GL_RGBA
    //systemRedShift    = 27;
    //systemGreenShift  = 19;
    //systemBlueShift   = 11;

    // GL_ARGB
    systemRedShift    = 19;
    systemGreenShift  = 11;
    systemBlueShift   = 3;

    // FIXME: add systemColorDepth = 16 shifts one day

    // VBA - used by the cpu filters only, not needed really
    //RGB_LOW_BITS_MASK = 0x00010101;

    utilUpdateSystemColorMaps(App->GetVbaCartType() == IMAGE_GBA && gbColorOption == 1);

	return true;
}


__attribute__ ((__always_inline__)) void *SystemMalloc(size_t size)
{
	return malloc(size);
}

__attribute__ ((__always_inline__)) extern void *SystemCalloc(size_t nelem, size_t elsize)
{
	return calloc(nelem, elsize);
}

__attribute__ ((__always_inline__)) extern void *SystemRealloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}


// VBA - TELLS IF IT SHOULD PAUSE THIS FRAME
__attribute__ ((__always_inline__)) bool systemPauseOnFrame()
{
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


__attribute__ ((__always_inline__)) void systemDrawScreen()
{
	//LOG_DBG("systemDrawScreen()\n");
	Graphics->Draw(pix);

	++renderedFrames;
}


// updates the joystick data
__attribute__ ((__always_inline__)) bool systemReadJoypads()
{
	//LOG_DBG("systemReadJoypads()\n");

	return true;
}


// return information about the given joystick, -1 for default joystick
__attribute__ ((__always_inline__)) uint32_t systemReadJoypad(int pad)
{
	//LOG_DBG("systemReadJoypad(%d)\n", pad);

	u32 J = 0;
	if (pad == -1) pad = 0;

	int i = pad;
	if (CellInput->UpdateDevice(i) != CELL_PAD_OK)
	{
		return J;
	}

	if (Settings.ControlStyle == CONTROL_STYLE_BETTER)
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

	// state shift
	if (CellInput->WasAnalogPressedLeft(i, CTRL_RSTICK))
	{
		App->DecrementStateSlot();
	}
	if (CellInput->WasAnalogPressedRight(i, CTRL_RSTICK))
	{
		App->IncrementStateSlot();
	}

	// state save
	if (CellInput->WasButtonHeld(i, CTRL_R2) && CellInput->WasButtonHeld(i, CTRL_R3))
	{

		App->Vba.emuWriteState(App->MakeFName(FILETYPE_STATE).c_str());
	}

	// state load
	if (CellInput->WasButtonHeld(i, CTRL_L2) && CellInput->WasButtonHeld(i, CTRL_L3))
	{
		App->Vba.emuReadState(App->MakeFName(FILETYPE_STATE).c_str());
	}

	// return to menu
	if (CellInput->IsButtonPressed(i, CTRL_L3) && CellInput->IsButtonPressed(i, CTRL_R3))
	{
		App->Vba.emuWriteBattery(App->MakeFName(FILETYPE_BATTERY).c_str());

		App->StopROMRunning();
		App->SwitchMode(MODE_MENU);
	}

	return J;
}


// Returns msecs since startup.
uint32_t systemGetClock()
{
   //LOG_DBG("systemGetClock()\n");
   uint64_t now = get_usec();
   return (now - startTime) / 1000;
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

	return true;
}


void systemShowSpeed(int speed)
{
	systemSpeed = speed;
	//LOG_DBG("systemShowSpeed: %3d%%(%d, %d fps)\n", systemSpeed, systemFrameSkip, renderedFrames );
	renderedFrames = 0;
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

