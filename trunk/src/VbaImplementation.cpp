/*
 * Vba_Implementation.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */
#include "VbaImplementation.h"

#include <string>
#include <sstream>

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
#include "VbaInput.h"

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


__attribute__ ((__always_inline__)) void * SystemMemCpy( void * destination, const void * source, size_t num )
{
	return memcpy(destination, source, num);
}


__attribute__ ((__always_inline__)) void * SystemMemSet ( void * ptr, int value, size_t num )
{
	return memset(ptr, value, num);
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

	//++renderedFrames;
}


// updates the joystick data
__attribute__ ((__always_inline__)) bool systemReadJoypads()
{
	//LOG_DBG("systemReadJoypads()\n");

	return true;
}

__attribute__ ((__always_inline__)) void special_actions(uint32_t specialbuttonmap)
{
	std::stringstream ss;
	switch(specialbuttonmap)
	{
		case BTN_EXITTOMENU:
			App->Vba.emuWriteBattery(App->MakeFName(FILETYPE_BATTERY).c_str());

			App->StopROMRunning();
			App->SwitchMode(MODE_MENU);
			break;
		case BTN_DECREMENTSAVE:
			App->DecrementStateSlot();
			ss << "Select state slot: " << App->CurrentSaveStateSlot();
			App->PushScreenMessage(ss.str());
			break;
		case BTN_INCREMENTSAVE:
			App->IncrementStateSlot();
			ss << "Select state slot: " << App->CurrentSaveStateSlot();
			App->PushScreenMessage(ss.str());
			break;
		case BTN_QUICKSAVE:
			if (App->Vba.emuWriteState(App->MakeFName(FILETYPE_STATE).c_str()))
			{
				ss << "State Saved - slot " << App->CurrentSaveStateSlot();
				App->PushScreenMessage(ss.str());
			}
			break;
		case BTN_QUICKLOAD:
			if (App->Vba.emuReadState(App->MakeFName(FILETYPE_STATE).c_str()))
			{
				ss << "State Loaded - slot " << App->CurrentSaveStateSlot();
				App->PushScreenMessage(ss.str());
			}
			break;
		default:
			break;
		}
}
__attribute__ ((__always_inline__)) bool  is_special_button_mapping(uint32_t specialbuttonmap)
{
	uint32_t buttonmapreturn;

	if (specialbuttonmap >= BTN_QUICKLOAD)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// return information about the given joystick, -1 for default joystick
__attribute__ ((__always_inline__)) uint32_t systemReadJoypad(int pad)
{
	u32 J = 0;
	//LOG_DBG("systemReadJoypad(%d)\n", pad);

	if (pad == -1) pad = 0;

	int i = pad;
	if (CellInput->UpdateDevice(i) != CELL_PAD_OK)
	{
		return J;
	}
    		if (CellInput->IsButtonPressed(i, CTRL_UP) | CellInput->IsAnalogPressedUp(i, CTRL_LSTICK))
		{
			J |= Settings.DPad_Up;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_DOWN) | CellInput->IsAnalogPressedDown(i, CTRL_LSTICK))
		{
			J |= Settings.DPad_Down;
		}
    		if (CellInput->IsButtonPressed(i,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(i, CTRL_LSTICK))
		{
			J |= Settings.DPad_Left;
		}
    		else if (CellInput->IsButtonPressed(i,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(i, CTRL_LSTICK))
		{
			J |= Settings.DPad_Right;
		}
    		if (CellInput->IsButtonPressed(i,CTRL_SQUARE))
		{
			J |= Settings.ButtonSquare;
		}
		if (CellInput->IsButtonPressed(i,CTRL_CROSS))
		{
			J |= Settings.ButtonCross;
		}
    		if (CellInput->IsButtonPressed(i,CTRL_CIRCLE))
		{
			J |= Settings.ButtonCircle;
		}
    		if (CellInput->IsButtonPressed(i,CTRL_TRIANGLE))
		{
			J |= Settings.ButtonTriangle;
		}
    		if (CellInput->IsButtonPressed(i,CTRL_START))
		{
			J |= Settings.ButtonStart;
		}
    		if (CellInput->IsButtonPressed(i,CTRL_SELECT))
		{
			J |= Settings.ButtonSelect;
		}
		if (CellInput->IsButtonPressed(i,CTRL_R1))
		{
			J |= Settings.ButtonR1;
		}
		if (CellInput->IsButtonPressed(i,CTRL_L1))
		{
			J |= Settings.ButtonL1;
		}


		//Button combos - L2
		if ((CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasButtonPressed(i,CTRL_R3)))
		{
			J |= Settings.ButtonL2_ButtonR3;
		}
		if ((CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasButtonPressed(i,CTRL_R2) && !(CellInput->WasButtonPressed(i,CTRL_R3))))
		{
			J |= Settings.ButtonL2_ButtonR2;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasAnalogPressedRight(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonL2_AnalogR_Right;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasAnalogPressedLeft(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonL2_AnalogR_Left;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasAnalogPressedUp(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonL2_AnalogR_Up;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasAnalogPressedDown(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonL2_AnalogR_Down;
		}
		else if ((CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->WasButtonPressed(i,CTRL_L3)))
		{
			J |= Settings.ButtonL2_ButtonL3;
		}
		if (CellInput->IsButtonPressed(i,CTRL_L2) && !(CellInput->IsButtonPressed(i,CTRL_L3)) && !(CellInput->IsButtonPressed(i,CTRL_R3)))
		{
			J |= Settings.ButtonL2;
		}

		//Button combos - L3 + R3
		if (CellInput->IsButtonPressed(i,CTRL_L3) && CellInput->IsButtonPressed(i,CTRL_R3) && !(CellInput->IsButtonPressed(i,CTRL_L2)) && !(CellInput->IsButtonPressed(i,CTRL_R2)))
		{
			J |= Settings.ButtonR3_ButtonL3;
		}
		if (CellInput->IsButtonPressed(i,CTRL_L3) && !(CellInput->IsButtonPressed(i,CTRL_L2)) && !(CellInput->IsButtonPressed(i,CTRL_R2)) && !(CellInput->IsButtonPressed(i,CTRL_R3)))
		{
			J |= Settings.ButtonL3;
		}
		if (CellInput->IsButtonPressed(i,CTRL_R3) && !(CellInput->IsButtonPressed(i,CTRL_L2)) && !(CellInput->IsButtonPressed(i,CTRL_R2)) && !(CellInput->IsButtonPressed(i,CTRL_L3)))
		{
			J |= Settings.ButtonR3;
		}

		//Button combos - R2
		if ((CellInput->IsButtonPressed(i,CTRL_R2) && CellInput->WasButtonPressed(i,CTRL_R3)))    
		{
			J |= Settings.ButtonR2_ButtonR3;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_R2) && CellInput->WasAnalogPressedRight(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonR2_AnalogR_Right;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_R2) && CellInput->WasAnalogPressedLeft(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonR2_AnalogR_Left;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_R2) && CellInput->WasAnalogPressedUp(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonR2_AnalogR_Up;
		}
		else if (CellInput->IsButtonPressed(i,CTRL_R2) && CellInput->WasAnalogPressedDown(i,CTRL_RSTICK))
		{
			J |= Settings.ButtonR2_AnalogR_Down;
		}
		else if (Settings.AnalogR_Down_Type ? CellInput->IsAnalogPressedDown(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)) : CellInput->WasAnalogPressedDown(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)))
		{
			J |= Settings.AnalogR_Down;
		}
		else if (Settings.AnalogR_Up_Type ? CellInput->IsAnalogPressedUp(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)) : CellInput->WasAnalogPressedUp(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)))
		{
			J |= Settings.AnalogR_Up;
		}
		else if (Settings.AnalogR_Left_Type ? CellInput->IsAnalogPressedLeft(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)) : CellInput->WasAnalogPressedLeft(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)))
		{
			J |= Settings.AnalogR_Left;
		}
		else if (Settings.AnalogR_Right_Type ? CellInput->IsAnalogPressedRight(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)) : CellInput->WasAnalogPressedRight(i,CTRL_RSTICK) && !(CellInput->IsButtonPressed(i,CTRL_L2)))
		{
			J |= Settings.AnalogR_Right;
		}
		if (CellInput->IsButtonPressed(i,CTRL_R2) && !(CellInput->IsButtonPressed(i, CTRL_L2)) && !(CellInput->WasButtonPressed(i,CTRL_R3)) && !(CellInput->WasButtonPressed(i,CTRL_L3)))
		{
			J |= Settings.ButtonR2;
		}

	//LOG_DBG("J:  %d\n", J);
	if(!is_special_button_mapping(J))
	{
		return J;
	}
	else
	{
		special_actions(J);
	}
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
#ifdef CELL_DEBUG
	LOG("systemMessage(%d)\n\t", id);

	va_list args;
	va_start(args,fmt);
	LOG(fmt, args);
	va_end(args);

	LOG("\n");
#endif

	// FIXME: really implement this...
	App->PushScreenMessage(fmt);
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
   VbaAudio *driver = new VbaAudio();

   if (Settings.RSoundEnabled)
   {
      driver->enable_network(Settings.RSoundServerIPAddress);
   }

	return driver;
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
	//renderedFrames = 0;
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

