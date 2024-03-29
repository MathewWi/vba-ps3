/*
 * VbaPs3.h
 *
 *  Created on: Nov 14, 2010
 *      Author: halsafar
 */

#ifndef VBAPS3_H_
#define VBAPS3_H_

#include <vector>
#include <deque>

#include "vba/Util.h"

#include "VbaImplementation.h"
#include "VbaGraphics.h"
#include "VbaAudio.h"

#include "cellframework/logger/Logger.h"
#include "cellframework/input/cellInput.h"

#define EMULATOR_VERSION "1.0"

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
	FILETYPE_BMP,
	FILETYPE_IMAGE_PREFS
};

enum
{
	MAP_BUTTONS_OPTION_SETTER,
	MAP_BUTTONS_OPTION_GETTER,
	MAP_BUTTONS_OPTION_DEFAULT
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

   	float GetFontSize();
	void ButtonMappingSettings(bool map_button_option_enum);

	string MakeFName(Emulator_FileTypes type);

	IMAGE_TYPE GetVbaCartType();

	void LoadImagePreferences();

	void OSKStart(const wchar_t* msg, const wchar_t* init);
	const char * OSKOutputString();

	void PushScreenMessage(string msg);

   static void RSound_Error();
   void ToggleSound();

	struct EmulatedSystem Vba;
private:
	int gbaRomSize;
	void EmulationLoop();
	void VbaGraphicsInit();

	int current_state_save;
	Emulator_Modes mode_switch;
	bool emulation_running;
	bool vba_loaded;
	bool load_settings;
	bool rom_loaded;
	string current_rom;
	IMAGE_TYPE cartridgeType;

	std::deque<string> _messages;
	float _messageTimer;
};

extern VbaPs3* App;
extern CellInputFacade* 		CellInput;
extern VbaGraphics* 			Graphics;

#endif /* VBAPS3_H_ */
