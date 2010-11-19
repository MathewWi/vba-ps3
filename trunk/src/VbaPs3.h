/*
 * VbaPs3.h
 *
 *  Created on: Nov 14, 2010
 *      Author: halsafar
 */

#ifndef VBAPS3_H_
#define VBAPS3_H_

#include "VbaImplementation.h"
#include "VbaGraphics.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/input/cellInput.h"

enum Emulator_Modes
{
	MODE_MENU,
	MODE_EMULATION,
	MODE_EXIT
};

enum Emulator_ControlStyle
{
	CONTROL_STYLE_ORIGINAL,
	CONTROL_STYLE_BETTER
};

void Emulator_RequestLoadROM(string rom, bool forceReload);
bool Emulator_IsROMLoaded();
void Emulator_SwitchMode(Emulator_Modes);
void Emulator_Shutdown();
void Emulator_StopROMRunning();
void Emulator_StartROMRunning();

bool Emulator_Snes9xInitialized();
bool Emulator_RomRunning();

void Emulator_GraphicsInit();

int Emulator_CurrentSaveStateSlot();
void Emulator_IncrementCurrentSaveStateSlot();
void Emulator_DecrementCurrentSaveStateSlot();
bool Emulator_InitSettings();
bool Emulator_SaveSettings();

extern Emulator_ControlStyle ControlStyle;

extern CellInputFacade* 		CellInput;
extern VbaGraphics* 			Graphics;

extern EmulatedSystem VbaEmulationSystem;


#endif /* VBAPS3_H_ */
