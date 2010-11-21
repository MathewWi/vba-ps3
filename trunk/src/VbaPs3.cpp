/*
 * VbaPs3.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */
#include <string>
#include <sstream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/timer.h>
#include <sys/return_code.h>
#include <sys/process.h>
#include <sys/spu_initialize.h>
#include <sysutil/sysutil_sysparam.h>

#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/pad.h>

#include "vba/Util.h"
#include "vba/gba/GBA.h"
#include "vba/gba/Sound.h"
#include "vba/gb/gb.h"
#include "vba/gb/gbSound.h"
#include "vba/gb/gbGlobals.h"

#include "conf/conffile.h"

#include "VbaPs3.h"
#include "VbaMenu.h"

#include "cellframework/utility/OSKUtil.h"

#define SYS_CONFIG_FILE "/dev_hdd0/game/VBAM90000/USRDIR/vba.conf"

SYS_PROCESS_PARAM(1001, 0x10000);

VbaGraphics* Graphics = NULL;
CellInputFacade* CellInput = NULL;
OSKUtil* oskutil = NULL;

ConfigFile	*currentconfig = NULL;

// VBA MUST
int emulating = 0;

//define struct
struct SSettings Settings;

// current save slot
int current_state_save = 0;

// mode the main loop is in
Emulator_Modes mode_switch = MODE_MENU;

// is a ROM running
bool emulation_running;

// is fceu loaded
bool vba_loaded = false;

// needs settings loaded
bool load_settings = true;

// current rom loaded
bool rom_loaded = false;

// current rom being emulated
string current_rom;

// cart type
IMAGE_TYPE cartridgeType = IMAGE_UNKNOWN;


struct EmulatedSystem VbaEmulationSystem =
{
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        false,
        0
};


bool Emulator_Initialized()
{
	return vba_loaded;
}


bool Emulator_IsROMLoaded()
{
	return rom_loaded;
}


bool Emulator_ROMRunning()
{
	return emulation_running;
}


void Emulator_SwitchMode(Emulator_Modes m)
{
	mode_switch = m;
}

void Emulator_OSKStart(const wchar_t* msg, const wchar_t* init)
{
	oskutil->Start(msg,init);
}

const char * Emulator_OSKOutputString()
{
	return oskutil->OutputString();
}


void Emulator_Shutdown()
{
	// do any clean up... save stuff etc
	// ...
	if (rom_loaded)
	{
		VbaEmulationSystem.emuWriteBattery(Emulator_MakeFName(FILETYPE_BATTERY).c_str());
	}

	//add saving back of conf file
	Emulator_SaveSettings();

	// shutdown everything
	Graphics->DeinitDbgFont();
	Graphics->Deinit();

	if (CellInput)
		delete CellInput;

	// VBA - shutdown sound, release CEllAudio thread
	soundShutdown();

	if (Graphics)
		delete Graphics;

	if (oskutil)
		delete oskutil;

	LOG_CLOSE();

	cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_IO);
	cellSysutilUnregisterCallback(0);

	// force exit
	exit(0);
}

static bool try_load_config_file (const char *fname, ConfigFile &conf)
{
	LOG("try_load_config_file(%s)\n", fname);
	FILE * fp;

	fp = fopen(fname, "r");
	if (fp)
	{
		fprintf(stdout, "Reading config file %s.\n", fname);
		conf.LoadFile(new fReader(fp));
		fclose(fp);
	}

	return (false);
}

bool Emulator_InitSettings()
{
	LOG("Emulator_InitSettings()\n");

	if (currentconfig == NULL)
	{
		currentconfig = new ConfigFile();
	}

	memset((&Settings), 0, (sizeof(Settings)));

	currentconfig->Clear();

	#ifdef SYS_CONFIG_FILE
		try_load_config_file(SYS_CONFIG_FILE, *currentconfig);
	#endif

	//PS3 - General settings
	if (currentconfig->Exists("PS3General::KeepAspect"))
	{
		Settings.PS3KeepAspect		=	currentconfig->GetBool("PS3General::KeepAspect");
	}
	else
	{
		Settings.PS3KeepAspect		=	true;
	}
	Graphics->SetAspectRatio(Settings.PS3KeepAspect ? SCREEN_4_3_ASPECT_RATIO : SCREEN_16_9_ASPECT_RATIO);

	if (currentconfig->Exists("PS3General::Smooth"))
	{
		Settings.PS3Smooth		=	currentconfig->GetBool("PS3General::Smooth");
	}
	else
	{
		Settings.PS3Smooth		=	false;
	}
	Graphics->SetSmooth(Settings.PS3Smooth);

	if (currentconfig->Exists("PS3General::OverscanEnabled"))
	{
		Settings.PS3OverscanEnabled	= currentconfig->GetBool("PS3General::OverscanEnabled");
	}
	else
	{
		Settings.PS3OverscanEnabled	= false;
	}
	if (currentconfig->Exists("PS3General::OverscanAmount"))
	{
		Settings.PS3OverscanAmount	= currentconfig->GetInt("PS3General::OverscanAmount");
	}
	else
	{
		Settings.PS3OverscanAmount	= 0;
	}
	Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);

	if (currentconfig->Exists("VBA::Controlstyle"))
	{
		Settings.ControlStyle = currentconfig->GetInt("VBA::Controlstyle");
	}
	else
	{
		Settings.ControlStyle = CONTROL_STYLE_BETTER;
	}

	if (currentconfig->Exists("VBA::Shader"))
	{
		Graphics->LoadFragmentShader(currentconfig->GetString("VBA::Shader"));
	}
	else
	{
		Graphics->LoadFragmentShader(DEFAULT_SHADER_FILE);
	}
	if (currentconfig->Exists("PS3General::PS3PALTemporalMode60Hz"))
	{
		Settings.PS3PALTemporalMode60Hz = currentconfig->GetBool("PS3General::PS3PALTemporalMode60Hz");
		Graphics->SetPAL60Hz(Settings.PS3PALTemporalMode60Hz);
	}
	else
	{
		Settings.PS3PALTemporalMode60Hz = false;
		Graphics->SetPAL60Hz(Settings.PS3PALTemporalMode60Hz);
	}
	//RSound Settings
	if(currentconfig->Exists("RSound::RSoundEnabled"))
	{
		Settings.RSoundEnabled		= currentconfig->GetBool("RSound::RSoundEnabled");
	}
	else
	{
		Settings.RSoundEnabled		= false;
	}
	if(currentconfig->Exists("RSound::RSoundServerIPAddress"))
	{
		Settings.RSoundServerIPAddress	= currentconfig->GetString("RSound::RSoundServerIPAddress");
	}
	else
	{
		Settings.RSoundServerIPAddress = "0.0.0.0";
	}
	// PS3 Path Settings
	/*
	if (currentconfig->Exists("PS3Paths::PathSaveStates"))
	{
		Settings.PS3PathSaveStates		= currentconfig->GetString("PS3Paths::PathSaveStates");
	}
	else
	{
		Settings.PS3PathSaveStates		= USRDIR;
	}

	if (currentconfig->Exists("PS3Paths::PathSRAM"))
	{
		Settings.PS3PathSRAM		= currentconfig->GetString("PS3Paths::PathSRAM");
	}

	if (currentconfig->Exists("PS3Paths::PathScreenshots"))
	{
		Settings.PS3PathScreenshots		= currentconfig->GetString("PS3Paths::PathScreenshots");
	}

	if (currentconfig->Exists("PS3Paths::PathROMDirectory"))
	{
		Settings.PS3PathROMDirectory		= currentconfig->GetString("PS3Paths::PathROMDirectory");
	}
	 */

	LOG("SUCCESS - Emulator_InitSettings()\n");
	return true;
}

bool Emulator_SaveSettings()
{
	if (currentconfig != NULL)
	{
		currentconfig->SetBool("PS3General::KeepAspect",Settings.PS3KeepAspect);
		currentconfig->SetBool("PS3General::Smooth", Settings.PS3Smooth);
		currentconfig->SetBool("PS3General::OverscanEnabled", Settings.PS3OverscanEnabled);
		currentconfig->SetInt("PS3General::OverscanAmount",Settings.PS3OverscanAmount);
		currentconfig->SetBool("PS3General::PS3PALTemporalMode60Hz",Settings.PS3PALTemporalMode60Hz);
		currentconfig->SetInt("VBA::Controlstyle",Settings.ControlStyle);
		currentconfig->SetString("VBA::Shader",Graphics->GetFragmentShaderPath());
		currentconfig->SetString("RSound::RSoundServerIPAddress",Settings.RSoundServerIPAddress);
		currentconfig->SetBool("RSound::RSoundEnabled",Settings.RSoundEnabled);
		return currentconfig->SaveTo(SYS_CONFIG_FILE);
	}
}


int Emulator_CurrentSaveStateSlot()
{
	return current_state_save;
}


void Emulator_DecrementCurrentSaveStateSlot()
{
	current_state_save = (current_state_save-1);
	if (current_state_save < 0) current_state_save = 9;
	//FIXME: add state select here
}


void Emulator_IncrementCurrentSaveStateSlot()
{
	current_state_save = (current_state_save+1) % 9;
	//FIXME: add state select here
}


int Emulator_CloseGame()
{
    if(!rom_loaded) {
        return(0);
    }

}


void Emulator_RequestLoadROM(string rom, bool forceReload)
{
	if (!rom_loaded || forceReload || current_rom.empty() || current_rom.compare(rom) != 0)
	{
		int srcWidth = 0;
		int srcHeight = 0;
		//int srcPitch = 0;

		current_rom = rom;

		Emulator_CloseGame();

		if (utilIsGBImage(rom.c_str()))
		{
	        // Port - init system (startime, palettes, etc) FIXME: startime should be set more accurately, fix once gb doesnt crash
	        systemInit();

			gbCleanUp();

			if (!gbLoadRom(rom.c_str()))
			{
				LOG("FAILED to GB load rom...\n");
				Emulator_Shutdown();
			}

			cartridgeType = IMAGE_GB;
			VbaEmulationSystem = GBSystem;

			// FIXME: make this an option that is toggable, implement systemGbBorderOn
            gbBorderOn = 0;
            if(gbBorderOn)
            {
                    srcWidth = 256;
                    srcHeight = 224;
                    gbBorderLineSkip = 256;
                    gbBorderColumnSkip = 48;
                    gbBorderRowSkip = 40;
            }
            else
            {
                    srcWidth = 160;
                    srcHeight = 144;
                    gbBorderLineSkip = 160;
                    gbBorderColumnSkip = 0;
                    gbBorderRowSkip = 0;
            }

            //srcPitch = 324;

            // VBA - init GB core and sound core
            soundSetSampleRate(44100);

            gbGetHardwareType();
            gbSoundReset();
            gbSoundSetDeclicking(false);
            gbReset();

        	soundInit();
		}
		else if (utilIsGBAImage(rom.c_str()))
		{
	        // Port - init system (startime, palettes, etc) FIXME: startime should be set more accurately, fix once gb doesnt crash
	        systemInit();

			if (!CPULoadRom(rom.c_str()))
			{
				LOG("FAILED to GBA load rom\n");
				Emulator_Shutdown();
			}

			cartridgeType = IMAGE_GBA;
			VbaEmulationSystem = GBASystem;

            srcWidth = 240;
            srcHeight = 160;
            //srcPitch = 484;

            soundSetSampleRate(44100); //22050

            cpuSaveType = 0;

            CPUInit("", false);

            CPUReset();

            soundReset();
        	soundInit();
		}
		else
		{
			cartridgeType = IMAGE_UNKNOWN;

			LOG("Unsupported rom type!\n");
			Emulator_Shutdown();
		}

		// ROM SUCCESSFULLY LOADED AT THIS POINT

    	// PORT - init graphics for this rom
    	Graphics->SetDimensions(srcWidth, srcHeight, (srcWidth)*4);

    	Rect r;
    	r.x = 0;
    	r.y = 0;
    	r.w = srcWidth;
    	r.h = srcHeight;
    	Graphics->SetRect(r);

    	Graphics->UpdateCgParams(srcWidth, srcHeight, srcWidth, srcHeight);

		rom_loaded = true;
		LOG("Successfully loaded rom!\n");
	}
}


void Emulator_StopROMRunning()
{
	// app
	emulation_running = false;

	// vba
	//emulating = 0;
}

void Emulator_StartROMRunning()
{
	Emulator_SwitchMode(MODE_EMULATION);
}


IMAGE_TYPE Emulator_GetVbaCartType()
{
	return cartridgeType;
}


void Emulator_VbaInit()
{
	LOG("Emulator_VbaInit()\n");

    vba_loaded = true;
}


void Emulator_GraphicsInit()
{
	LOG("Emulator_GraphicsInit()\n");

	if (Graphics == NULL)
	{
		Graphics = new VbaGraphics();
	}

	Graphics->Init();

	if (Graphics->InitCg() != CELL_OK)
	{
		LOG("Failed to InitCg: %d\n", __LINE__);
		Emulator_Shutdown();
	}

	LOG("Emulator_GraphicsInit->InitDebug Font\n");
	Graphics->InitDbgFont();
}


string Emulator_MakeFName(Emulator_FileTypes type)
{
	LOG_DBG("Emulator_MakeFName(%s, %d)\n", current_rom.c_str(), type);

	// strip out the filename from the path
	string fn = current_rom.substr(0, current_rom.find_last_of('.'));
	fn = fn.substr(fn.find_last_of('/')+1);

	std::stringstream ss;
	switch (type)
	{
		case FILETYPE_STATE:
			ss << EMULATOR_PATH_STATES << fn << current_state_save << ".sgm";
			fn = ss.str();
			break;
		case FILETYPE_BATTERY:
			ss << EMULATOR_PATH_STATES << fn << ".sav";
			fn = ss.str();
			break;
		case FILETYPE_PNG:
			ss << EMULATOR_PATH_STATES << fn << ".png";
			fn = ss.str();
			break;
		case FILETYPE_BMP:
			ss << EMULATOR_PATH_STATES << fn << ".bmp";
			fn = ss.str();
			break;
		default:
			break;
	}

	LOG("fn: %s\n", fn.c_str());
	return fn;
}


void UpdateInput()
{

}


void EmulationLoop()
{
	LOG("EmulationLoop()\n");

	if (!vba_loaded)
		Emulator_VbaInit();

	if (!rom_loaded)
	{
		LOG("No Rom Loaded!\n");
		Emulator_SwitchMode(MODE_MENU);
		return;
	}

	// Load the battery of the rom
	VbaEmulationSystem.emuReadBattery(Emulator_MakeFName(FILETYPE_BATTERY).c_str());

	// FIXME: implement for real
	int fskip = 0;

	// Tell VBA we are emulating
	emulating = 1;

	// emulation loop
	emulation_running = true;
	while (emulation_running)
	{
		VbaEmulationSystem.emuMain(VbaEmulationSystem.emuCount);

		//Graphics->Swap();

#ifdef EMUDEBUG
		if (CellConsole_IsInitialized())
		{
			cellConsolePoll();
		}
#endif

		cellSysutilCheckCallback();
	}
}


void sysutil_exit_callback (uint64_t status, uint64_t param, void *userdata) {
	(void) param;
	(void) userdata;

	switch (status) {
		case CELL_SYSUTIL_REQUEST_EXITGAME:
			MenuStop();
			Emulator_StopROMRunning();
			mode_switch = MODE_EXIT;
			break;
		case CELL_SYSUTIL_DRAWING_BEGIN:
		case CELL_SYSUTIL_DRAWING_END:
			break;
		case CELL_SYSUTIL_OSKDIALOG_LOADED:
			break;
		case CELL_SYSUTIL_OSKDIALOG_FINISHED:
			oskutil->Stop();
			break;
		case CELL_SYSUTIL_OSKDIALOG_UNLOADED:
			oskutil->Close();
			break;
	}
}

int main()
{
	// Initialize 6 SPUs but reserve 1 SPU as a raw SPU for PSGL
	sys_spu_initialize(6, 1);

	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);

	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);

	LOG_INIT();
	LOG("LOGGER LOADED!\n");

	Graphics = new VbaGraphics();
	Emulator_GraphicsInit();

	CellInput = new CellInputFacade();
	CellInput->Init();

	oskutil = new OSKUtil();

	Emulator_VbaInit();

	//load settings
	currentconfig = new ConfigFile();
	if(Emulator_InitSettings())
	{
		load_settings = false;
	}

	// main loop
	while(1)
	{
		switch(mode_switch)
		{
			case MODE_MENU:
				MenuMainLoop();
				break;
			case MODE_EMULATION:
				EmulationLoop();
				break;
			case MODE_EXIT:
				Emulator_Shutdown();
				return 0;
		}
	}

	return 0;
}
