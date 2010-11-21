/*
 * VbaPs3.h
 *
 *  Created on: Nov 14, 2010
 *      Author: halsafar
 */

#ifndef VBAPS3_H_
#define VBAPS3_H_

#include "vba/Util.h"

#include "VbaImplementation.h"
#include "VbaGraphics.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/input/cellInput.h"

#define EMULATOR_PATH_STATES "/dev_hdd0/game/VBAM90000/USRDIR/"

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

enum Emulator_FileTypes
{
	FILETYPE_STATE,
	FILETYPE_BATTERY,
	FILETYPE_PNG,
	FILETYPE_BMP
};

class VbaPs3
{
public:
	VbaPs3();
	~VbaPs3();

	int MainLoop();
	void Shutdown();

	void SwitchMode(Emulator_Modes);

	void LoadROM(string rom, bool forceReload);

	void StopROMRunning();
	void StartROMRunning();
	int CloseROM();

	bool IsInitialized();
	bool IsROMRunning();
	bool IsROMLoaded();

	int32_t VbaInit();
	int32_t GraphicsInit();

	bool InitSettings();
	bool SaveSettings();

	int CurrentSaveStateSlot();
	void IncrementStateSlot();
	void DecrementStateSlot();

	string MakeFName(Emulator_FileTypes type);

	IMAGE_TYPE GetVbaCartType();

	void OSKStart(const wchar_t* msg, const wchar_t* init);
	const char * OSKOutputString();

	struct EmulatedSystem Vba;
private:
	void EmulationLoop();

	int current_state_save;
	Emulator_Modes mode_switch;
	bool emulation_running;
	bool vba_loaded;
	bool load_settings;
	bool rom_loaded;
	string current_rom;
	IMAGE_TYPE cartridgeType;
};

extern VbaPs3* App;
extern CellInputFacade* 		CellInput;
extern VbaGraphics* 			Graphics;

#endif /* VBAPS3_H_ */
