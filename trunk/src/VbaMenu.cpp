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

#include "utils/fex/Zip7_Extractor.h"

#include "VbaPs3.h"
#include "VbaMenu.h"
#include "VbaGraphics.h"
#include "VbaImplementation.h"
#include "VbaZipIo.h"

#include "cellframework/input/cellInput.h"
#include "cellframework/fileio/FileBrowser.h"
#include "cellframework/logger/Logger.h"

#include "conf/conffile.h"

#define USRDIR "/dev_hdd0/game/VBAM90000/USRDIR"
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

// if you add more settings to the screen, remember to change this value to the correct number
#define MAX_NO_OF_SETTINGS      12

#define NUM_ENTRY_PER_PAGE 24

// function pointer for menu render functions
typedef void (*curMenuPtr)();

// menu stack
std::stack<curMenuPtr> menuStack;

// is the menu running
bool menuRunning = false;

//
int16_t currently_selected_setting = 0;

// main file browser for rom browser
FileBrowser* browser = NULL;

// tmp file browser for everything else
FileBrowser* tmpBrowser = NULL;

VbaZipIo zipIo;


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

			Graphics->FlushDbgFont();
		}
	}
	Graphics->FlushDbgFont();
}

void do_biosChoice()
{
	if (tmpBrowser == NULL)
	{
		tmpBrowser = new FileBrowser("/\0");
	}
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
										"bin|gba|GBA|BIN");
			}
			else if (tmpBrowser->IsCurrentAFile())
			{
				path = tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name;

				// load shader
				Settings.GBABIOS.assign(path);
				LOG("Settings.GBABIOS: %s\n", Settings.GBABIOS.c_str());
				menuStack.pop();
			}
		}

		if (CellInput->WasButtonHeld(0, CTRL_TRIANGLE))
		{
			menuStack.pop();
		}
	}

	cellDbgFontPuts(0.05f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory/select BIOS");
	cellDbgFontPuts(0.05f, 0.92f, FONT_SIZE, PURPLE, "Triangle - return to settings");
	Graphics->FlushDbgFont();

	RenderBrowser(tmpBrowser);
}

void do_saveStatePathChoice()
{
	if (tmpBrowser == NULL)
	{
		tmpBrowser = new FileBrowser("/\0");
	}
	string path;

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(tmpBrowser);
		if (CellInput->WasButtonPressed(0,CTRL_SQUARE))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
				path = tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name + "/";
				Settings.PS3PathSaveStates = path;
				menuStack.pop();
			}
		}
		if (CellInput->WasButtonHeld(0, CTRL_TRIANGLE))
		{
			path = USRDIR;
			Settings.PS3PathSaveStates = path;
			menuStack.pop();
		}
		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
				tmpBrowser->PushDirectory(tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name, CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY, "empty");
			}
		}
	}
			
	cellDbgFontPuts(0.05f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory");
	cellDbgFontPuts(0.05f, 0.92f, FONT_SIZE, BLUE, "SQUARE - select directory as path for savestate files");
	cellDbgFontPuts(0.05f, 0.96f, FONT_SIZE, PURPLE, "Triangle - return to settings");
	Graphics->FlushDbgFont();
			
	RenderBrowser(tmpBrowser);
}

void do_sramPathChoice()
{
	if (tmpBrowser == NULL)
	{
		tmpBrowser = new FileBrowser("/\0");
	}
	string path;

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(tmpBrowser);
		if (CellInput->WasButtonPressed(0,CTRL_SQUARE))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
				path = tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name + "/";
				Settings.PS3PathSRAM = path;
				menuStack.pop();
			}
		}
		if (CellInput->WasButtonHeld(0, CTRL_TRIANGLE))
		{
			path = USRDIR;
			Settings.PS3PathSRAM = path;
			menuStack.pop();
		}
		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
				tmpBrowser->PushDirectory(tmpBrowser->GetCurrentDirectoryInfo().dir + "/" + tmpBrowser->GetCurrentEntry()->d_name, CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY, "empty");
			}
		}
	}
			
	cellDbgFontPuts(0.05f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory");
	cellDbgFontPuts(0.05f, 0.92f, FONT_SIZE, BLUE, "SQUARE - select directory as path for SRAM files");
	cellDbgFontPuts(0.05f, 0.96f, FONT_SIZE, PURPLE, "Triangle - return to settings");
	Graphics->FlushDbgFont();
			
	RenderBrowser(tmpBrowser);
}

void do_shaderChoice()
{
	if (tmpBrowser == NULL)
	{
		tmpBrowser = new FileBrowser("/dev_hdd0/game/VBAM90000/USRDIR/shaders/\0");
	}
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
				Graphics->UpdateCgParams();

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



// void do_settings()
// // called from ROM menu by pressing the SELECT button
// // return to ROM menu by pressing the CIRCLE button
void do_settings()
{
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
			if (App->IsROMLoaded())
			{
				MenuStop();

				App->StartROMRunning();
			}

			return;
		}

		switch(currently_selected_setting)
		{
		case SETTING_DRAW_FPS:
			if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
				Settings.DrawFps = !Settings.DrawFps;
			if(CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				Settings.DrawFps = !Settings.DrawFps;
			break;

			case SETTING_CURRENT_SAVE_STATE_SLOT:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
					   App->DecrementStateSlot();
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
					   App->IncrementStateSlot();
				break;

			case SETTING_CHANGE_RESOLUTION:
				/* FIXME: COMMENTED OUT FOR NOW
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
				{
					Graphics->NextResolution();
				}
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
				{
					Graphics->PreviousResolution();
				}
				if(CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					Graphics->SwitchResolution();
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Graphics->SwitchResolution(Graphics->GetInitialResolution());
				}
				*/
			   break;
			case SETTING_KEEP_ASPECT_RATIO:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				{
					Settings.PS3KeepAspect = !Settings.PS3KeepAspect;
					//Graphics->SetAspectRatio(Settings.PS3KeepAspect ? SCREEN_4_3_ASPECT_RATIO : SCREEN_16_9_ASPECT_RATIO);
               Graphics->SetStretched(!Settings.PS3KeepAspect);
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
			case SETTING_HW_OVERSCAN_AMOUNT:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if(Settings.PS3OverscanAmount > -40)
					{
						Settings.PS3OverscanAmount--;
						Settings.PS3OverscanEnabled = true;
						Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
					}
					if(Settings.PS3OverscanAmount == 0)
					{
						Settings.PS3OverscanEnabled = false;
						Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if((Settings.PS3OverscanAmount < 40))
					{
						Settings.PS3OverscanAmount++;
						Settings.PS3OverscanEnabled = true;
						Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
					}
					if(Settings.PS3OverscanAmount == 0)
					{
						Settings.PS3OverscanEnabled = false;
						Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3OverscanAmount = 0;
					Settings.PS3OverscanEnabled = false;
					Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
				}
				break;
			case SETTING_CONTROL_STYLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				{
					Settings.ControlStyle = (Emulator_ControlStyle)(((Settings.ControlStyle) + 1) % 2);
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.ControlStyle = CONTROL_STYLE_ORIGINAL;
				}
				break;
			case SETTING_SHADER:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					//tmpBrowser->Destroy();
					//tmpBrowser->PushDirectory("/\0", CELL_FS_TYPE_DIRECTORY | CELL_FS_TYPE_REGULAR, "cg");

					menuStack.push(do_shaderChoice);
					tmpBrowser = NULL;
				}
				break;
			case SETTING_GBABIOS:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_biosChoice);
					tmpBrowser = NULL;
				}
				break;
			case SETTING_PATH_SAVESTATES_DIRECTORY:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_saveStatePathChoice);
					tmpBrowser = NULL;
				}
				if (CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3PathSaveStates = USRDIR;
				}
				break;
			case SETTING_PATH_SRAM_DIRECTORY:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_sramPathChoice);
					tmpBrowser = NULL;
				}
				if (CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3PathSRAM = USRDIR;
				}
				break;
			case SETTING_DEFAULT_ALL:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK) | CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.PS3KeepAspect = true;
					Settings.PS3Smooth = true;
					Settings.PS3OverscanAmount = 0;
					Settings.PS3OverscanEnabled = false;
					Settings.RSoundEnabled = false;
					Settings.RSoundServerIPAddress = "0.0.0.0";
					Settings.GBABIOS.clear();
					Graphics->SetAspectRatio(SCREEN_4_3_ASPECT_RATIO);
					Graphics->SetSmooth(Settings.PS3Smooth);
					Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
					Settings.ControlStyle = CONTROL_STYLE_ORIGINAL;
					Settings.PS3PALTemporalMode60Hz = false;
					Settings.PS3PathSRAM = USRDIR;
					Settings.PS3PathSaveStates = USRDIR;
					//FIXME: For when resolution switching works
					/*
					if(Graphics->CheckResolution(CELL_VIDEO_OUT_RESOLUTION_576))
					{
						Graphics->SetPAL60Hz(Settings.PS3PALTemporalMode60Hz);
						Graphics->SwitchResolution();
					}
					*/
				}
				break;
			default:
				break;
		} // end of switch

	}

	float yPos = 0.05;
	float ySpacing = 0.04;

	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_DRAW_FPS ? YELLOW : WHITE, "Draw Framerate");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.DrawFps ? RED : GREEN, "%d", Settings.DrawFps ? "Enabled" : "Disabled");

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_CURRENT_SAVE_STATE_SLOT ? YELLOW : WHITE, "Current save state slot");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, App->CurrentSaveStateSlot() == 0 ? GREEN : RED, "%d", App->CurrentSaveStateSlot());

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_CHANGE_RESOLUTION ? YELLOW : WHITE, "Resolution");

	switch(Graphics->GetCurrentResolution())
	{
		case CELL_VIDEO_OUT_RESOLUTION_480:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_480 ? GREEN : RED, "480p (59.94Hz)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_720:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_720 ? GREEN : RED, "720p (59.94Hz)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1080 ? GREEN : RED, "1080p (59.94Hz)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_576:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_576 ? GREEN : RED, "576p (50Hz)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1600x1080 ? GREEN : RED, "1600x1080");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1440x1080 ? GREEN : RED, "1440x1080");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1280x1080 ? GREEN : RED, "1280x1080");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_960x1080 ? GREEN : RED, "960x1080");
			Graphics->FlushDbgFont();
			break;
	}
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_KEEP_ASPECT_RATIO ? YELLOW : WHITE, "Aspect Ratio");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.PS3KeepAspect == true ? GREEN : RED, "%s", Settings.PS3KeepAspect == true ? "4:3" : "16:9");

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_HW_TEXTURE_FILTER ? YELLOW : WHITE, "Hardware Filtering");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.PS3Smooth == true ? GREEN : RED, "%s", Settings.PS3Smooth == true ? "Linear interpolation" : "Point filtering");

	yPos += ySpacing;
	cellDbgFontPuts		(0.05f,	yPos,	FONT_SIZE,	currently_selected_setting == SETTING_HW_OVERSCAN_AMOUNT ? YELLOW : WHITE,	"Overscan");
	cellDbgFontPrintf	(0.5f,	yPos,	FONT_SIZE,	Settings.PS3OverscanAmount == 0 ? GREEN : RED, "%f", (float)Settings.PS3OverscanAmount/100);

	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_CONTROL_STYLE ? YELLOW : WHITE, "Control Style");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE,
			Settings.ControlStyle == CONTROL_STYLE_ORIGINAL ? GREEN : RED,
			"%s", Settings.ControlStyle == CONTROL_STYLE_ORIGINAL ? "Original (X->B, O->A)" : "Better (X->A, []->B)");

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_SHADER ? YELLOW : WHITE, "Shader: ");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE,
			GREEN,
			"%s", Graphics->GetFragmentShaderPath().substr(Graphics->GetFragmentShaderPath().find_last_of('/')).c_str());

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_GBABIOS ? YELLOW : WHITE, "Use GBA BIOS: ");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.GBABIOS.empty() ? GREEN : RED, Settings.GBABIOS.empty() ? "NO" : "YES");

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_PATH_SAVESTATES_DIRECTORY ? YELLOW : WHITE, "Savestate Directory");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.PS3PathSaveStates.c_str() == USRDIR ? GREEN : RED, Settings.PS3PathSaveStates.c_str());
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_PATH_SRAM_DIRECTORY ? YELLOW : WHITE, "SRAM Directory");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.PS3PathSRAM.c_str() == USRDIR ? GREEN : RED, Settings.PS3PathSRAM.c_str());
	Graphics->FlushDbgFont();


	yPos += ySpacing;
	cellDbgFontPrintf(0.05f, yPos, FONT_SIZE, currently_selected_setting == SETTING_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");


	cellDbgFontPuts(0.01f, 0.88f, FONT_SIZE, YELLOW, "UP/DOWN - select, X/LEFT/RIGHT - change, START - default");
	cellDbgFontPuts(0.01f, 0.92f, FONT_SIZE, YELLOW, "CIRCLE - return to menu, L2+R2 - resume game");
	Graphics->FlushDbgFont();
}


void do_ZipMenu()
{
	int current_index = zipIo.GetCurrentEntryIndex();
	int file_count = zipIo.GetCurrentEntryCount();

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0,CTRL_LSTICK))
		{
			current_index++;
			if (current_index >= file_count)
			{
				current_index = 0;
			}
		}
		if (CellInput->WasButtonPressed(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0,CTRL_LSTICK))
		{
			current_index--;
			if (current_index < 0)
			{
				current_index = file_count -1;
			}
		}
		if (CellInput->WasButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
		{
			current_index = MIN(current_index+5, file_count-1);
		}
		if (CellInput->WasButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
		{
			if (current_index <= 5)
			{
				current_index = 0;
			}
			else
			{
				current_index -= 5;
			}
		}
		if (CellInput->WasButtonPressed(0,CTRL_R1))
		{
			current_index = MIN(current_index+NUM_ENTRY_PER_PAGE, file_count-1);
		}
		if (CellInput->WasButtonPressed(0,CTRL_L1))
		{
			if (current_index <= NUM_ENTRY_PER_PAGE)
			{
				current_index = 0;
			}
			else
			{
				current_index -= NUM_ENTRY_PER_PAGE;
			}
		}

		if (CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			// don't let people back out past root
			zipIo.PopDir();

			if (zipIo.GetDirStackCount() <= 0)
			{
				menuStack.pop();
				return;
			}
		}

		if (CellInput->WasButtonPressed(0, CTRL_CROSS))
		{
			if (zipIo[current_index].type == ZIPIO_TYPE_DIR)
			{
				LOG_DBG("ZipIO: trying to push directory\n");
				zipIo.PushDir(zipIo[current_index].name);
			}
			else if (zipIo[current_index].type == ZIPIO_TYPE_FILE)
			{
				string rom_path = "/dev_hdd0/" + zipIo.GetCurrentEntry().name;
				LOG_DBG("ZipIO: try and load rom now: %s\n", rom_path.c_str());

				uint8_t* data = zipIo.GetEntryData();
				FILE *file = fopen(rom_path.c_str(), "wb");
				LOG("ZipIO: writing size: %d\n", zipIo.GetCurrentEntrySize());
				fwrite(data, 1, zipIo.GetCurrentEntrySize(), file);
				fclose(file);
				LOG_DBG("ZipIO: stored hack tmpfile\n");

				free(data);

				App->LoadROM(rom_path, true);
				{
					menuStack.pop();
					MenuStop();

					// switch emulator to emulate mode
					App->StartROMRunning();
				}

				remove(rom_path.c_str());
			}
		}
	}

	zipIo.SetCurrentEntryPosition(current_index);

	// render it
	int page_number = current_index / NUM_ENTRY_PER_PAGE;
	int page_base = page_number * NUM_ENTRY_PER_PAGE;
	float currentX = 0.05f;
	float currentY = 0.00f;
	float ySpacing = 0.035f;
	for (int i = page_base; i < file_count && i < page_base + NUM_ENTRY_PER_PAGE; ++i)
	{
		// error!
		if (i >= zipIo.GetCurrentEntryCount())
		{
			continue;
		}

		currentY = currentY + ySpacing;
		cellDbgFontPuts(currentX, currentY, FONT_SIZE,
						i == current_index ? RED : zipIo[i].type == ZIPIO_TYPE_DIR ? GREEN : WHITE,
						zipIo[i].name.c_str());

		Graphics->FlushDbgFont();
	}

	cellDbgFontPuts(0.05f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory/load game");
	cellDbgFontPuts(0.05f, 0.92f, FONT_SIZE, PURPLE, "L2 + R2 - return to game");
	cellDbgFontPuts(0.5f, 0.92f, FONT_SIZE, BLUE, "O - Back a directory");
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
										"gb|gbc|gba|GBA|GB|GBC|7z");
			}
			else if (browser->IsCurrentAFile())
			{
				//load game (standard controls), go back to main loop
				rom_path = browser->GetCurrentDirectoryInfo().dir + "/" + browser->GetCurrentEntry()->d_name;

				if (FileBrowser::GetExtension(rom_path).compare("7z") == 0)
				{
					zipIo.Open(rom_path);

					menuStack.push(do_ZipMenu);
				}
				else
				{
					MenuStop();

					// switch emulator to emulate mode
					App->StartROMRunning();

					//FIXME: 1x dirty const char* to char* casts... menu sucks.
					App->LoadROM((char*)rom_path.c_str(), true);

					return;
				}
			}
		}
		if (CellInput->IsButtonPressed(0,CTRL_L2) && CellInput->IsButtonPressed(0,CTRL_R2))
		{
			// if a rom is loaded then resume it
			if (App->IsROMLoaded())
			{
				MenuStop();

				App->StartROMRunning();
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



