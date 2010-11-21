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
#include "vba/gba/RTC.h"
#include "vba/gba/agbprint.h"
#include "vba/gb/gb.h"
#include "vba/gb/gbSound.h"
#include "vba/gb/gbGlobals.h"

#include "conf/conffile.h"

#include "VbaPs3.h"
#include "VbaMenu.h"

#include "cellframework/utility/OSKUtil.h"

#define SYS_CONFIG_FILE "/dev_hdd0/game/VBAM90000/USRDIR/vba.conf"
SYS_PROCESS_PARAM(1001, 0x40000);


VbaPs3* App = NULL;
VbaGraphics* Graphics = NULL;
CellInputFacade* CellInput = NULL;
OSKUtil* oskutil = NULL;
ConfigFile	*currentconfig = NULL;

// VBA MUST
int emulating = 0;

//define struct
struct SSettings Settings;

VbaPs3::VbaPs3()
{
	vba_loaded = false;
	load_settings = true;
	rom_loaded = false;
	cartridgeType = IMAGE_UNKNOWN;
	mode_switch = MODE_MENU;
	current_state_save = 0;
	memset(&Vba, 0, sizeof(Vba));

	Graphics = new VbaGraphics();
	App->GraphicsInit();

	CellInput = new CellInputFacade();
	CellInput->Init();

	oskutil = new OSKUtil();

	//load settings
	currentconfig = new ConfigFile();
	if(App->InitSettings())
	{
		load_settings = false;
	}
}


VbaPs3::~VbaPs3()
{

}


bool VbaPs3::IsInitialized()
{
	return vba_loaded;
}


bool VbaPs3::IsROMLoaded()
{
	return rom_loaded;
}


bool VbaPs3::IsROMRunning()
{
	return emulation_running;
}


void VbaPs3::SwitchMode(Emulator_Modes m)
{
	mode_switch = m;
}


void VbaPs3::OSKStart(const wchar_t* msg, const wchar_t* init)
{
	oskutil->Start(msg,init);
}


const char * VbaPs3::OSKOutputString()
{
	return oskutil->OutputString();
}


void VbaPs3::Shutdown()
{
	// do any clean up... save stuff etc
	// ...
	if (rom_loaded)
	{
		Vba.emuWriteBattery(this->MakeFName(FILETYPE_BATTERY).c_str());
	}

	//add saving back of conf file
	VbaPs3::SaveSettings();

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

	// force exit --
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


bool VbaPs3::InitSettings()
{
	LOG("VbaPs3::InitSettings()\n");

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

	LOG("SUCCESS - VbaPs3::InitSettings()\n");
	return true;
}


bool VbaPs3::SaveSettings()
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

	return false;
}


int VbaPs3::CurrentSaveStateSlot()
{
	return current_state_save;
}


void VbaPs3::DecrementStateSlot()
{
	current_state_save = (current_state_save-1);
	if (current_state_save < 0) current_state_save = 9;
}


void VbaPs3::IncrementStateSlot()
{
	current_state_save = (current_state_save+1) % 9;
}


int VbaPs3::CloseROM()
{
    if(!rom_loaded) {
        return(1);
    }

    // FIXME: implement this!

    return 0;
}


void VbaPs3::LoadROM(string filename, bool forceReload)
{
	if (!rom_loaded || forceReload || current_rom.empty() || current_rom.compare(filename) != 0)
	{
		current_rom = filename;

		this->CloseROM();

		if (utilIsGBImage(filename.c_str()))
		{
			// FIXME: reconsider where we call this. IT MUST BE before loading/initing VBA
	        systemInit();

			if (!gbLoadRom(filename.c_str()))
			{
				LOG("FAILED to GB load rom...\n");
				this->Shutdown();
			}

			cartridgeType = IMAGE_GB;
			Vba = GBSystem;
		}
		else if (utilIsGBAImage(filename.c_str()))
		{
			// FIXME: reconsider where we call this. IT MUST BE before loading/initing VBA
	        systemInit();

			if (!CPULoadRom(filename.c_str()))
			{
				LOG("FAILED to GBA load rom\n");
				this->Shutdown();
			}

			cartridgeType = IMAGE_GBA;
			Vba = GBASystem;
		}
		else
		{
			cartridgeType = IMAGE_UNKNOWN;

			LOG("Unsupported rom type!\n");
			this->Shutdown();
		}

		// force a vba reload now
		vba_loaded = false;

		rom_loaded = true;
		LOG("Successfully loaded rom!\n");
	}
}


void VbaPs3::StopROMRunning()
{
	// app
	emulation_running = false;

	// vba - doing this breaks VBA, do not uncomment this!
	//emulating = 0;
}


void VbaPs3::StartROMRunning()
{
	this->SwitchMode(MODE_EMULATION);
}


IMAGE_TYPE VbaPs3::GetVbaCartType()
{
	return cartridgeType;
}


int32_t VbaPs3::VbaInit()
{
	LOG("VbaPs3::VbaInit()\n");

	int srcWidth = 0;
	int srcHeight = 0;
	//int srcPitch = 0;

	if (cartridgeType == IMAGE_GB)
	{
		//soundShutdown();

        systemInit();

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
	else if (cartridgeType == IMAGE_GBA)
	{
		LOG("1:\n");
		//soundShutdown();

		LOG("2:\n");
        systemInit();

        srcWidth = 240;
        srcHeight = 160;
        //srcPitch = 484;

        LOG("3:\n");
        soundSetSampleRate(44100); //22050

        // vba - automatic type
        cpuSaveType = 0;

        // vba - set default flash size
        flashSetSize(0x10000); // 64K

        // vba - real time clock
        rtcEnable(false);

        // vba - we explicitly do not want debug messages
        agbPrintEnable(false);

        // vba - default to this
        mirroringEnable = false;
        doMirroring(mirroringEnable);

        // FIXME: take a moment to learn vba's per image settings interface
        // ie.  apparently Pokemon games need rtc enabled or anything that is based on time of day.

        LOG("4:\n");
        CPUInit(NULL, false);

        LOG("5:\n");
        CPUReset();

        LOG("6:\n");
        soundReset();

        LOG("7:\n");
    	soundInit();
	}
	else
	{
		LOG("Bad cart type!\n");
		this->Shutdown(); //FIXME: be more graceful
	}

	// ROM SUCCESSFULLY LOADED AT THIS POINT

	// PORT - init graphics for this rom
	LOG("8:\n");
	Graphics->SetDimensions(srcWidth, srcHeight, (srcWidth)*4);

	Rect r;
	r.x = 0;
	r.y = 0;
	r.w = srcWidth;
	r.h = srcHeight;

	LOG("9:\n");
	Graphics->SetRect(r);

	LOG("10:\n");
	Graphics->UpdateCgParams(srcWidth, srcHeight, srcWidth, srcHeight);

    vba_loaded = true;

    LOG("SUCCESS - VbaPs3::VbaInit()\n");
    return 0;
}


int32_t VbaPs3::GraphicsInit()
{
	LOG("VbaPs3::GraphicsInit()\n");

	if (Graphics == NULL)
	{
		Graphics = new VbaGraphics();
	}

	Graphics->Init();

	if (Graphics->InitCg() != CELL_OK)
	{
		LOG("Failed to InitCg: %d\n", __LINE__);
		this->Shutdown();
	}

	LOG("VbaPs3::GraphicsInit->InitDebug Font\n");
	Graphics->InitDbgFont();

	return 0;
}


string VbaPs3::MakeFName(Emulator_FileTypes type)
{
	LOG_DBG("VbaPs3::MakeFName(%s, %d)\n", current_rom.c_str(), type);

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


void VbaPs3::EmulationLoop()
{
	LOG("VbaPs3::EmulationLoop()\n");

	if (!vba_loaded)
	{
		if (this->VbaInit() != 0)
		{
			LOG("VBA INIT FAILED");
			this->SwitchMode(MODE_MENU);
			return;
		}
	}

	if (!rom_loaded)
	{
		LOG("No Rom Loaded!\n");
		this->SwitchMode(MODE_MENU);
		return;
	}

	// Load the battery of the rom
	Vba.emuReadBattery(this->MakeFName(FILETYPE_BATTERY).c_str());

	// FIXME: implement for real
	int fskip = 0;

	// Tell VBA we are emulating
	emulating = 1;

	// emulation loop
	emulation_running = true;
	while (emulation_running)
	{
		Vba.emuMain(Vba.emuCount);

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


int VbaPs3::MainLoop()
{
	// main loop
	while(1)
	{
		switch(mode_switch)
		{
			case MODE_MENU:
				MenuMainLoop();
				break;
			case MODE_EMULATION:
				this->EmulationLoop();
				break;
			case MODE_EXIT:
				this->Shutdown();
				return 0;
		}
	}
}


//
// SONY PS3 - XMB EXIT CALLBACK
//
void sysutil_exit_callback (uint64_t status, uint64_t param, void *userdata) {
	(void) param;
	(void) userdata;

	switch (status) {
		case CELL_SYSUTIL_REQUEST_EXITGAME:
			MenuStop();
			App->StopROMRunning();
			App->SwitchMode(MODE_EXIT);
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


//
// ENTRY POINT
//
int main()
{
	// Initialize 6 SPUs but reserve 1 SPU as a raw SPU for PSGL
	sys_spu_initialize(6, 1);

	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);

	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);

	LOG_INIT();
	LOG("LOGGER LOADED!\n");

	int ret = 0;
	App = new VbaPs3();
	ret = App->MainLoop();

	if (App != NULL)
	{
		delete App;
	}

	return ret;
}
