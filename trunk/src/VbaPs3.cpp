/*
 * VbaPs3.cpp
 *
 *  Created on: Nov 13, 2010
 *      Author: halsafar
 */
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

#define SYS_CONFIG_FILE "/dev_hdd0/game/VBAM90000/USRDIR/vba.conf"

SYS_PROCESS_PARAM(1001, 0x10000);

VbaGraphics* Graphics = NULL;
CellInputFacade* CellInput = NULL;
Audio::Stream<int32_t>* CellAudio = NULL;

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


void Emulator_Shutdown()
{
	// do any clean up... save stuff etc
	// ...
	if (rom_loaded)
	{

	}

	//add saving back of conf file
	Emulator_SaveSettings();

	// shutdown everything
	Graphics->DeinitDbgFont();
	Graphics->Deinit();

	if (CellInput)
		delete CellInput;

	if (CellAudio)
		delete CellAudio;

	if (Graphics)
		delete Graphics;

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

	if (currentconfig->Exists("FCEU::Controlstyle"))
	{
		Settings.FCEUControlstyle = currentconfig->GetInt("FCEU::Controlstyle");
	}
	else
	{
		Settings.FCEUControlstyle = CONTROL_STYLE_BETTER;
	}

	if (currentconfig->Exists("FCEU::Shader"))
	{
		Graphics->LoadFragmentShader(currentconfig->GetString("FCEU::Shader"));
	}
	else
	{
		Graphics->LoadFragmentShader(DEFAULT_SHADER_FILE);
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
	currentconfig->SetBool("PS3General::KeepAspect",Settings.PS3KeepAspect);
	currentconfig->SetBool("PS3General::Smooth", Settings.PS3Smooth);
	currentconfig->SetInt("FCEU::Controlstyle",Settings.FCEUControlstyle);
	currentconfig->SetString("FCEU::Shader",Graphics->GetFragmentShaderPath());
	return currentconfig->SaveTo(SYS_CONFIG_FILE);
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
		current_rom = rom;

		Emulator_CloseGame();

		if (utilIsGBImage(rom.c_str()))
		{
			gbCleanUp();

			if (!gbLoadRom(rom.c_str()))
			{
				LOG("FAILED to GB load rom...\n");
				Emulator_Shutdown();
			}

			VbaEmulationSystem = GBSystem;

            gbBorderOn = 0; // GB borders always off

            if(gbBorderOn)
            {
                    //srcWidth = 256;
                    //srcHeight = 224;
                    gbBorderLineSkip = 256;
                    gbBorderColumnSkip = 48;
                    gbBorderRowSkip = 40;
            }
            else
            {
                    //srcWidth = 160;
                    //srcHeight = 144;
                    gbBorderLineSkip = 160;
                    gbBorderColumnSkip = 0;
                    gbBorderRowSkip = 0;
            }

            //srcPitch = 324;

            // Port - init system (startime, palettes, etc) FIXME: startime should be set more accurately, fix once gb doesnt crash
            systemInit();

            //gbSetPalette(0);

            // VBA - init GB core
            soundSetSampleRate(44100);

            gbGetHardwareType();
            gbSoundReset();
            gbSoundSetDeclicking(true);
            gbReset();

            // PS3 - setup graphics for GB
        	Graphics->SetDimensions(160, 144, 160 * 4);

        	Rect r;
        	r.x = 0;
        	r.y = 0;
        	r.w = 160;
        	r.h = 144;
        	Graphics->SetRect(r);
        	Graphics->SetAspectRatio(SCREEN_4_3_ASPECT_RATIO);

        	Graphics->UpdateCgParams(160, 144, 160, 144);

        	soundInit();
		}
		else if (utilIsGBAImage(rom.c_str()))
		{
			if (!CPULoadRom(rom.c_str()))
			{
				LOG("FAILED to GBA load rom\n");
				Emulator_Shutdown();
			}

			VbaEmulationSystem = GBASystem;

            //srcWidth = 240;
            //srcHeight = 160;
            //srcPitch = 484;
            soundSetSampleRate(22050); //44100 / 2
            cpuSaveType = 0;

            CPUReset();
            soundReset();
		}
		else
		{
			LOG("Unsupported rom type!\n");
			Emulator_Shutdown();
		}

		// load battery ram
		// FIXME: load battery

		rom_loaded = true;
		LOG("Successfully loaded rom!\n");
	}
}


void Emulator_StopROMRunning()
{
	emulation_running = false;

	// load battery ram
	// FIXME: Load sram

}

void Emulator_StartROMRunning()
{
	Emulator_SwitchMode(MODE_EMULATION);
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

	LOG("Emulator_GraphicsInit->Setting Dimensions\n");
	Graphics->SetDimensions(256, 240, 256 * 4);

	Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 256;
	r.h = 240;
	Graphics->SetRect(r);
	Graphics->SetAspectRatio(SCREEN_4_3_ASPECT_RATIO);

	LOG("Emulator_GraphicsInit->InitDebug Font\n");
	Graphics->InitDbgFont();
}


void Input_SetVbaInput()
{

}


void UpdateInput()
{
	for (int i = 0; i < CellInput->NumberPadsConnected(); i++)
	{
		if (CellInput->UpdateDevice(i) != CELL_PAD_OK)
		{
			continue;
		}

		if (CellInput->IsButtonPressed(i, CTRL_L3) && CellInput->IsButtonPressed(i, CTRL_R3))
		{
			Emulator_StopROMRunning();
			Emulator_SwitchMode(MODE_MENU);
		}
	}
}


// FCEU quirk. 16-bit integer are embedded into a 32-bit int ...
static void Emulator_Convert_Samples(float * __restrict__ out, const int32_t * __restrict__ in, size_t frames)
{
   union {
      const int32_t *i32;
      const int16_t *i16;
   } u;
   u.i32 = in;

   for (size_t i = 0; i < frames * 2; i+=2)
   {
      out[i] = (float)u.i16[i+1] / 0x7FFF;
      out[i + 1] = (float)u.i16[i+1] / 0x7FFF;
   }
}


void Emulator_EnableSound()
{
	if(CellAudio)
	{
		delete CellAudio;
	}

	CellAudio = new Audio::AudioPort<int32_t>(1, AUDIO_INPUT_RATE);
	CellAudio->set_float_conv_func(Emulator_Convert_Samples);
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

	// init cell audio
	if (CellAudio == NULL)
	{
		//LOG("Initializing CellAudio!\n");
		//CellAudio = new Audio::AudioPort<int32_t>(1, AUDIO_INPUT_RATE);
		//CellAudio->set_float_conv_func(Emulator_Convert_Samples);
	}

	// set the shader cg params
	Graphics->UpdateCgParams(256, 240, 256, 240);

	// set fceu input
	Input_SetVbaInput();

	// FIXME: implement for real
	int fskip = 0;

	// Tell VBA we are emulating
	emulating = 1;

	// emulation loop
	emulation_running = true;
	while (emulation_running)
	{
		//LOG("EmulationLoopStep\n");
		UpdateInput();
		sys_timer_usleep(1);
		VbaEmulationSystem.emuMain(VbaEmulationSystem.emuCount);
		Graphics->Swap();

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

	Emulator_VbaInit();

	//needs to come first before graphics
	currentconfig = new ConfigFile();
	if(Emulator_InitSettings())
	{
		load_settings = false;
	}

    // allocate memory to store rom
    //nesrom = (unsigned char *)memalign(32,1024*1024*4); // 4 MB should be plenty

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
