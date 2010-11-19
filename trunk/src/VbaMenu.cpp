/*
 * menu.cpp
 *
 *  Created on: Oct 10, 2010
 *      Author: halsafar
 */
#include <string.h>
#include <stack>

#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/pad.h>
#include <cell/dbgfont.h>
#include <sysutil/sysutil_sysparam.h>

#include "VbaPs3.h"
#include "VbaMenu.h"

#include "VbaGraphics.h"
#include "VbaImplementation.h"

#include "cellframework/input/cellInput.h"
#include "cellframework/fileio/FileBrowser.h"
#include "cellframework/logger/Logger.h"

#include "conf/conffile.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

// if you add more settings to the screen, remember to change this value to the correct number
#define MAX_NO_OF_SETTINGS      7

#define NUM_ENTRY_PER_PAGE 24

enum MENU_RESOLUTION_CHOICES
{
	RESOLUTION_480,
	RESOLUTION_720,
	RESOLUTION_1080
};

// function pointer for menu render functions
typedef void (*curMenuPtr)();

// menu stack
std::stack<curMenuPtr> menuStack;

// initial resolution
static uint8_t initial_resolution = 0;

// is the menu running
bool menuRunning = false;

//
int16_t currently_selected_setting = 0;

// main file browser for rom browser
FileBrowser* browser = NULL;

// tmp file browser for everything else
FileBrowser* tmpBrowser = NULL;


void MenuStop()
{
	menuRunning = false;
}


bool MenuIsRunning()
{
	return menuRunning;
}


void UpdateBrowser(FileBrowser* b)
{
	if (CellInput->WasButtonPressed(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0,CTRL_LSTICK))
	{
		b->IncrementEntry();
	}
	if (CellInput->WasButtonPressed(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0,CTRL_LSTICK))
	{
		b->DecrementEntry();
	}
	if (CellInput->WasButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
	{
		b->GotoEntry(MIN(b->GetCurrentEntryIndex()+5, b->GetCurrentDirectoryInfo().numEntries-1));
	}
	if (CellInput->WasButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
	{
		if (b->GetCurrentEntryIndex() <= 5)
		{
			b->GotoEntry(0);
		}
		else
		{
			b->GotoEntry(b->GetCurrentEntryIndex()-5);
		}
	}
	if (CellInput->WasButtonPressed(0,CTRL_R1))
	{
		b->GotoEntry(MIN(b->GetCurrentEntryIndex()+NUM_ENTRY_PER_PAGE, b->GetCurrentDirectoryInfo().numEntries-1));
	}
	if (CellInput->WasButtonPressed(0,CTRL_L1))
	{
		if (b->GetCurrentEntryIndex() <= NUM_ENTRY_PER_PAGE)
		{
			b->GotoEntry(0);
		}
		else
		{
			b->GotoEntry(b->GetCurrentEntryIndex()-NUM_ENTRY_PER_PAGE);
		}
	}

	if (CellInput->WasButtonPressed(0, CTRL_CIRCLE))
	{
		// don't let people back out past root
		if (b->DirectoryStackCount() > 1)
		{
			b->PopDirectory();
		}
	}
}


void RenderBrowser(FileBrowser* b)
{
	uint32_t file_count = b->GetCurrentDirectoryInfo().numEntries;
	int current_index = b->GetCurrentEntryIndex();

	if (file_count <= 0)
	{
		LOG("1: filecount <= 0");
	}
	else if (current_index > file_count)
	{
		LOG("2: current_index >= file_count");
	}
	else
	{
		int page_number = current_index / NUM_ENTRY_PER_PAGE;
		int page_base = page_number * NUM_ENTRY_PER_PAGE;
		float currentX = 0.05f;
		float currentY = 0.00f;
		float ySpacing = 0.035f;
		for (int i = page_base; i < file_count && i < page_base + NUM_ENTRY_PER_PAGE; ++i)
		{
			currentY = currentY + ySpacing;
			cellDbgFontPuts(currentX, currentY, FONT_SIZE,
							i == current_index ? RED : (*b)[i]->d_type == CELL_FS_TYPE_DIRECTORY ? GREEN : WHITE,
							(*b)[i]->d_name);

			// draw every 10
			if (i % 10 == 0)
			{
				Graphics->FlushDbgFont();
			}
		}
	}
	Graphics->FlushDbgFont();
}


void do_shaderChoice()
{
	string path;

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(tmpBrowser);

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
				tmpBrowser->PushDirectory(	tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name,
										CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY,
										"cg");
			}
			else if (tmpBrowser->IsCurrentAFile())
			{
				path = tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name;

				// load shader
				Graphics->LoadFragmentShader(path);
				Graphics->UpdateCgParams(160, 144, 160, 144);

				menuStack.pop();
			}
		}

		if (CellInput->WasButtonHeld(0, CTRL_TRIANGLE))
		{
			menuStack.pop();
		}
	}

	cellDbgFontPuts(0.05f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory/select shader");
	cellDbgFontPuts(0.05f, 0.92f, FONT_SIZE, PURPLE, "Triangle - return to settings");
	Graphics->FlushDbgFont();

	RenderBrowser(tmpBrowser);
}


void do_pathChoice()
{

}


// void do_settings()
// // called from ROM menu by pressing the SELECT button
// // return to ROM menu by pressing the CIRCLE button
void do_settings()
{
	CellVideoOutState video_state;
	int current_resolution;
	if(initial_resolution == 0)
	{
		video_state = Graphics->GetVideoOutState();
		initial_resolution = video_state.displayMode.resolutionId;
	}

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_DOWN) | CellInput->WasAnalogPressedDown(0, CTRL_LSTICK))	// down to next setting
		{
			currently_selected_setting++;
			if (currently_selected_setting >= MAX_NO_OF_SETTINGS) currently_selected_setting = 0;
		}

		if (CellInput->WasButtonPressed(0, CTRL_UP) | CellInput->WasAnalogPressedUp(0, CTRL_LSTICK))	// up to previous setting
		{
				currently_selected_setting--;
				if (currently_selected_setting < 0) currently_selected_setting = MAX_NO_OF_SETTINGS-1;
		}

		if (CellInput->IsButtonPressed(0,CTRL_L2) && CellInput->IsButtonPressed(0,CTRL_R2))
		{
			// if a rom is loaded then resume it
			if (Emulator_IsROMLoaded())
			{
				MenuStop();

				Emulator_StartROMRunning();
			}

			return;
		}

		switch(currently_selected_setting)
		{
			case SETTING_CURRENT_SAVE_STATE_SLOT:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
					   Emulator_DecrementCurrentSaveStateSlot();
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
					   Emulator_IncrementCurrentSaveStateSlot();
				break;

			case SETTING_CHANGE_RESOLUTION:
			   current_resolution = Graphics->GetVideoOutState().displayMode.resolutionId;
			   if(CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
			   {
				   switch(initial_resolution)
				   {
					   case CELL_VIDEO_OUT_RESOLUTION_480:
						   break;
					  case CELL_VIDEO_OUT_RESOLUTION_720:
						  break;
					  case CELL_VIDEO_OUT_RESOLUTION_1080:
						  if (current_resolution == CELL_VIDEO_OUT_RESOLUTION_720)
						  {
							  Emulator_SaveSettings();
							  if (Graphics->ChangeResolution(CELL_VIDEO_OUT_RESOLUTION_1080, CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ) != CELL_OK)
							  {
								  Emulator_Shutdown();
							  }
							  Emulator_GraphicsInit();
							  Emulator_InitSettings();
						  }
						  if (current_resolution == CELL_VIDEO_OUT_RESOLUTION_480)
						  {
							  Emulator_SaveSettings();
							  if (Graphics->ChangeResolution(CELL_VIDEO_OUT_RESOLUTION_720, CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ) != CELL_OK)
							  {
								  Emulator_Shutdown();
							  }
							  Emulator_GraphicsInit();
							  Emulator_InitSettings();
						  }
						  break;
					  case CELL_VIDEO_OUT_RESOLUTION_576:
						  break;
					  case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
						  break;
					  case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
						  break;
					  case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
						  break;
					  case CELL_VIDEO_OUT_RESOLUTION_960x1080:
						  break;
				  }
			   }
			   if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
			   {
				   switch(initial_resolution)
				   {
					   case CELL_VIDEO_OUT_RESOLUTION_480:
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_720:
						   if (current_resolution == CELL_VIDEO_OUT_RESOLUTION_720)
						   {
							   Emulator_SaveSettings();
							   if (Graphics->ChangeResolution(CELL_VIDEO_OUT_RESOLUTION_480, CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ) != CELL_OK)
							   {
								   Emulator_Shutdown();
							   }
							  Emulator_GraphicsInit();
							  Emulator_InitSettings();
						   }
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_1080:
						   if (current_resolution == CELL_VIDEO_OUT_RESOLUTION_720)
						   {
							   Emulator_SaveSettings();
							   if (Graphics->ChangeResolution(CELL_VIDEO_OUT_RESOLUTION_480, CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ) != CELL_OK)
							   {
								   Emulator_Shutdown();
							   }
							  Emulator_GraphicsInit();
							  Emulator_InitSettings();
						   }
						   if (current_resolution == CELL_VIDEO_OUT_RESOLUTION_1080)
						   {
							   Emulator_SaveSettings();
							   if (Graphics->ChangeResolution(CELL_VIDEO_OUT_RESOLUTION_720, CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ) != CELL_OK)
							   {
								   Emulator_Shutdown();
							   }
							  Emulator_GraphicsInit();
							  Emulator_InitSettings();
						   }
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_576:
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
						   break;
					   case CELL_VIDEO_OUT_RESOLUTION_960x1080:
						   break;
				   }
			   }
			   break;
			case SETTING_KEEP_ASPECT_RATIO:
				cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &video_state);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				{
					Settings.PS3KeepAspect = !Settings.PS3KeepAspect;
					Graphics->SetAspectRatio(Settings.PS3KeepAspect ? SCREEN_4_3_ASPECT_RATIO : SCREEN_16_9_ASPECT_RATIO);
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3KeepAspect = true;
					Graphics->SetAspectRatio(SCREEN_4_3_ASPECT_RATIO);
				}
				break;
			case SETTING_HW_TEXTURE_FILTER:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				{
					Settings.PS3Smooth = !Settings.PS3Smooth;
					Graphics->SetSmooth(Settings.PS3Smooth);
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3Smooth = true;
					Graphics->SetSmooth(true);
				}
				break;
			case SETTING_CONTROL_STYLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				{
					Settings.FCEUControlstyle = (Emulator_ControlStyle)(((Settings.FCEUControlstyle) + 1) % 2);
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.FCEUControlstyle = CONTROL_STYLE_ORIGINAL;
				}
				break;
			case SETTING_SHADER:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					//tmpBrowser->Destroy();
					//tmpBrowser->PushDirectory("/\0", CELL_FS_TYPE_DIRECTORY | CELL_FS_TYPE_REGULAR, "cg");

					menuStack.push(do_shaderChoice);
				}
				break;
			case SETTING_DEFAULT_ALL:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK) | CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3KeepAspect = true;
					Settings.PS3Smooth = true;
					Graphics->SetAspectRatio(SCREEN_4_3_ASPECT_RATIO);
					Graphics->SetSmooth(true);
					Settings.FCEUControlstyle = CONTROL_STYLE_ORIGINAL;
				}
				break;
			default:
				break;
		} // end of switch

	}

	cellDbgFontPuts(0.05f, 0.05f, FONT_SIZE, currently_selected_setting == SETTING_CURRENT_SAVE_STATE_SLOT ? YELLOW : WHITE, "Current save state slot");
	cellDbgFontPrintf(0.5f, 0.05f, FONT_SIZE, Emulator_CurrentSaveStateSlot() == 0 ? GREEN : RED, "%d", Emulator_CurrentSaveStateSlot());

	cellDbgFontPuts(0.05f, 0.09f, FONT_SIZE, currently_selected_setting == SETTING_CHANGE_RESOLUTION ? YELLOW : WHITE, "Resolution");

	// get the video state again with new resolution
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &video_state);
	switch(video_state.displayMode.resolutionId)
	{
		case CELL_VIDEO_OUT_RESOLUTION_480:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "480");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_720:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "720");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1080:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_576:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "576");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "1600x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "1440x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "1280x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080:
			cellDbgFontPrintf(0.5f, 0.09f, FONT_SIZE, GREEN, "960x1080");
			break;
	}
	Graphics->FlushDbgFont();

	cellDbgFontPuts(0.05f, 0.13f, FONT_SIZE, currently_selected_setting == SETTING_KEEP_ASPECT_RATIO ? YELLOW : WHITE, "Aspect Ratio");
	cellDbgFontPrintf(0.5f, 0.13f, FONT_SIZE, Settings.PS3KeepAspect == true ? GREEN : RED, "%s", Settings.PS3KeepAspect == true ? "4:3" : "16:9");

	cellDbgFontPuts(0.05f, 0.17f, FONT_SIZE, currently_selected_setting == SETTING_HW_TEXTURE_FILTER ? YELLOW : WHITE, "Hardware Filtering");
	cellDbgFontPrintf(0.5f, 0.17f, FONT_SIZE, Settings.PS3Smooth == true ? GREEN : RED, "%s", Settings.PS3Smooth == true ? "Linear interpolation" : "Point filtering");

	cellDbgFontPuts(0.05f, 0.21f, FONT_SIZE, currently_selected_setting == SETTING_CONTROL_STYLE ? YELLOW : WHITE, "Control Style");
	cellDbgFontPrintf(0.5f, 0.21f, FONT_SIZE,
			Settings.FCEUControlstyle == CONTROL_STYLE_ORIGINAL ? GREEN : RED,
			"%s", Settings.FCEUControlstyle == CONTROL_STYLE_ORIGINAL ? "Original (X->B, O->A)" : "Better (X->A, []->B)");

	cellDbgFontPuts(0.05f, 0.25f, FONT_SIZE, currently_selected_setting == SETTING_SHADER ? YELLOW : WHITE, "Shader: ");
	cellDbgFontPrintf(0.5f, 0.25f, FONT_SIZE,
			GREEN,
			"%s", Graphics->GetFragmentShaderPath().substr(Graphics->GetFragmentShaderPath().find_last_of('/')).c_str());

	cellDbgFontPrintf(0.05f, 0.29f, FONT_SIZE, currently_selected_setting == SETTING_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");
	cellDbgFontPuts(0.01f, 0.88f, FONT_SIZE, YELLOW, "UP/DOWN - select, X/LEFT/RIGHT - change, START - default");
	cellDbgFontPuts(0.01f, 0.92f, FONT_SIZE, YELLOW, "CIRCLE - return to menu, L2+R2 - resume game");
	Graphics->FlushDbgFont();
}


void do_ROMMenu ()
{
	string rom_path;

	// sort directory file entries - courtesy of cmonkey69
	// start sorting from first entry after '..' and sort file_count - 2 items (indexing starts at zero, don't sort '.' and '..')
	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(browser);

		if (CellInput->WasButtonPressed(0,CTRL_SELECT))
		{
			menuStack.push(do_settings);
		}

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(browser->IsCurrentADirectory())
			{
				browser->PushDirectory(	browser->GetCurrentDirectoryInfo().dir + "/" + browser->GetCurrentEntry()->d_name,
										CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY,
										"gb|gba");
			}
			else if (browser->IsCurrentAFile())
			{
				//load game (standard controls), go back to main loop
				rom_path = browser->GetCurrentDirectoryInfo().dir + "/" + browser->GetCurrentEntry()->d_name;

				MenuStop();

				// switch emulator to emulate mode
				Emulator_StartROMRunning();

				//FIXME: 1x dirty const char* to char* casts... menu sucks.
				Emulator_RequestLoadROM((char*)rom_path.c_str(), true);

				return;
			}
		}
		if (CellInput->IsButtonPressed(0,CTRL_L2) && CellInput->IsButtonPressed(0,CTRL_R2))
		{
			// if a rom is loaded then resume it
			if (Emulator_IsROMLoaded())
			{
				MenuStop();

				Emulator_StartROMRunning();
			}

			return;
		}
	}

	cellDbgFontPuts(0.05f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory/load game");
	cellDbgFontPuts(0.05f, 0.92f, FONT_SIZE, PURPLE, "L2 + R2 - return to game");
	cellDbgFontPuts(0.5f, 0.92f, FONT_SIZE, BLUE, "SELECT - settings screen");
	Graphics->FlushDbgFont();

	RenderBrowser(browser);
}


void MenuMainLoop()
{
	// create file browser if null
	if (browser == NULL)
	{
		browser = new FileBrowser("/\0");
	}

	if (tmpBrowser == NULL)
	{
		tmpBrowser = new FileBrowser("/dev_hdd0/game/FCEU90000/USRDIR/shaders/\0");
	}

	// FIXME: could always just return to last menu item... don't pop on resume kinda thing
	if (menuStack.empty())
	{
		menuStack.push(do_ROMMenu);
	}

	// menu loop
	menuRunning = true;
	while (!menuStack.empty() && menuRunning)
	{
		Graphics->Clear();

		menuStack.top()();

		Graphics->Swap();

		cellSysutilCheckCallback();

#ifdef EMUDEBUG
		if (CellConsole_IsInitialized())
		{
			cellConsolePoll();
		}
#endif
	}
}



