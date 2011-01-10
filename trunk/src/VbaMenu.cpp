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
	#include "VbaInput.h"

#include "cellframework/input/cellInput.h"
#include "cellframework/fileio/FileBrowser.h"
#include "cellframework/logger/Logger.h"

#include "conf/conffile.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define NUM_ENTRY_PER_PAGE 22
#define NUM_ENTRY_PER_SETTINGS_PAGE 18

// function pointer for menu render functions
typedef void (*curMenuPtr)();

// menu stack
std::stack<curMenuPtr> menuStack;

// is the menu running
bool menuRunning = false;


// main file browser for rom browser
FileBrowser* browser = NULL;

// tmp file browser for everything else
FileBrowser* tmpBrowser = NULL;

int16_t currently_selected_setting =		FIRST_GENERAL_SETTING;
int16_t currently_selected_vba_setting =	FIRST_VBA_SETTING;
int16_t currently_selected_path_setting = 	FIRST_PATH_SETTING;
int16_t currently_selected_controller_setting = FIRST_CONTROLS_SETTING;

#define FILEBROWSER_DELAY	100000
#define SETTINGS_DELAY		150000	

VbaZipIo zipIo;


void MenuStop()
{
	menuRunning = false;
}


bool MenuIsRunning()
{
	return menuRunning;
}

void MenuResetControlStyle()
{
	if(Settings.ButtonCircle == BTN_A && Settings.ButtonCross == BTN_B)
	{
		Settings.ControlStyle = CONTROL_STYLE_ORIGINAL;
	}
}

void UpdateBrowser(FileBrowser* b)
{
	if (CellInput->WasButtonHeld(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0,CTRL_LSTICK))
	{
		if(b->GetCurrentEntryIndex() < b->GetCurrentDirectoryInfo().numEntries-1)
		{
			b->IncrementEntry();
			if(CellInput->IsButtonPressed(0,CTRL_DOWN))
			{
				sys_timer_usleep(FILEBROWSER_DELAY);
			}
		}
	}
	if (CellInput->WasButtonHeld(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0,CTRL_LSTICK))
	{
		if(b->GetCurrentEntryIndex() > 0)
		{
			b->DecrementEntry();
			if(CellInput->IsButtonPressed(0,CTRL_UP))
			{
				sys_timer_usleep(FILEBROWSER_DELAY);
			}
		}
	}
	if (CellInput->WasButtonHeld(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
	{
		b->GotoEntry(MIN(b->GetCurrentEntryIndex()+5, b->GetCurrentDirectoryInfo().numEntries-1));
		if(CellInput->IsButtonPressed(0,CTRL_RIGHT))
		{
			sys_timer_usleep(FILEBROWSER_DELAY);
		}
	}
	if (CellInput->WasButtonHeld(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
	{
		if (b->GetCurrentEntryIndex() <= 5)
		{
			b->GotoEntry(0);
			if(CellInput->IsButtonPressed(0,CTRL_LEFT))
			{
				sys_timer_usleep(FILEBROWSER_DELAY);
			}
		}
		else
		{
			b->GotoEntry(b->GetCurrentEntryIndex()-5);
			if(CellInput->IsButtonPressed(0,CTRL_LEFT))
			{
				sys_timer_usleep(FILEBROWSER_DELAY);
			}
		}
	}
	if (CellInput->WasButtonHeld(0,CTRL_R1))
	{
		b->GotoEntry(MIN(b->GetCurrentEntryIndex()+NUM_ENTRY_PER_PAGE, b->GetCurrentDirectoryInfo().numEntries-1));
		if(CellInput->IsButtonPressed(0,CTRL_R1))
		{
			sys_timer_usleep(FILEBROWSER_DELAY);
		}
	}
	if (CellInput->WasButtonHeld(0,CTRL_L1))
	{
		if (b->GetCurrentEntryIndex() <= NUM_ENTRY_PER_PAGE)
		{
			b->GotoEntry(0);
		}
		else
		{
			b->GotoEntry(b->GetCurrentEntryIndex()-NUM_ENTRY_PER_PAGE);
		}
		if(CellInput->IsButtonPressed(0,CTRL_L1))
		{
			sys_timer_usleep(FILEBROWSER_DELAY);
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
		float currentX = 0.09f;
		float currentY = 0.09f;
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

	cellDbgFontPrintf(0.09f, 0.09f, FONT_SIZE, YELLOW, "PATH: %s", tmpBrowser->GetCurrentDirectoryInfo().dir.c_str());
	cellDbgFontPuts	(0.09f,	0.05f,	FONT_SIZE,	RED,	"GBA BIOS SELECTION");
	cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,"X - Enter directory             /\\ - return to settings");
	//cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,"SQUARE - Select directory as path");
	cellDbgFontPrintf(0.09f, 0.89f, 0.86f, LIGHTBLUE, "%s",
	"INFO - Browse to a directory and select the GBA BIOS ROM by pressing CROSS button.");
	Graphics->FlushDbgFont();
			
	RenderBrowser(tmpBrowser);
}

void do_pathChoice()
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
				switch(currently_selected_setting)
				{
					case SETTING_PATH_SAVESTATES_DIRECTORY:
						Settings.PS3PathSaveStates = path;
						break;
					case SETTING_PATH_SRAM_DIRECTORY:
						Settings.PS3PathSRAM = path;
						break;
					case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
						Settings.PS3PathROMDirectory = path;
						break;
				}
				menuStack.pop();
			}
		}
		if (CellInput->WasButtonHeld(0, CTRL_TRIANGLE))
		{
			path = EMULATOR_PATH_STATES;
			switch(currently_selected_setting)
			{
				case SETTING_PATH_SAVESTATES_DIRECTORY:
					Settings.PS3PathSaveStates = path;
					break;
				case SETTING_PATH_SRAM_DIRECTORY:
					Settings.PS3PathSRAM = path;
					break;
				case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
					Settings.PS3PathROMDirectory = path;
					break;
			}
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
			
	cellDbgFontPrintf(0.09f, 0.09f, FONT_SIZE, YELLOW, "PATH: %s", tmpBrowser->GetCurrentDirectoryInfo().dir.c_str());
	cellDbgFontPuts	(0.09f,	0.05f,	FONT_SIZE,	RED,	"DIRECTORY SELECTION");
	cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,"X - Enter directory             /\\ - return to settings");
	//cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,"SQUARE - Select directory as path");
	cellDbgFontPrintf(0.09f, 0.89f, 0.86f, LIGHTBLUE, "%s",
	"INFO - Browse to a directory and assign it as the path by pressing SQUARE button.");
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

	cellDbgFontPrintf(0.09f, 0.09f, FONT_SIZE, YELLOW, "PATH: %s", tmpBrowser->GetCurrentDirectoryInfo().dir.c_str());
	cellDbgFontPuts	(0.09f,	0.05f,	FONT_SIZE,	RED,	"SHADER SELECTION");
	cellDbgFontPuts(0.09f, 0.92f, FONT_SIZE, YELLOW,
	"X - Select shader               /\\ - return to settings");
	cellDbgFontPrintf(0.09f, 0.89f, 0.86f, LIGHTBLUE, "%s",
	"INFO - Select a shader from the menu by pressing the X button. ");
	Graphics->FlushDbgFont();

	RenderBrowser(tmpBrowser);
}

void DisplayHelpMessage(int currentsetting)
{
	switch(currentsetting)
	{
		case SETTING_CURRENT_SAVE_STATE_SLOT:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Set the current savestate slot (can also be configured ingame).\n");
			break;
		case SETTING_PATH_SAVESTATES_DIRECTORY:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Set the default path where all the savestate files will be saved.\n");
			break;
		case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Set the default ROM startup directory. NOTE: You will have to\nrestart the emulator for this change to have any effect.\n");
			break;
		case SETTING_PATH_SRAM_DIRECTORY:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Set the default SRAM (SaveRAM) directory path.\n");
			break;
		case SETTING_SHADER:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Select a pixel shader. NOTE: Some shaders might be too slow at 1080p.\nIf you experience any slowdown, try another shader.");
			break;
		case SETTING_VBA_GBABIOS:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", Settings.GBABIOS.empty() ? "INFO - GBA BIOS has been disabled. If a game doesn't work, try selecting a BIOS\nwith this option (press X to select)" : "INFO - GBA BIOS has been selected. Some games might require this.\nNOTE: GB Classic games will not work with a GBA BIOS enabled.");
			break;
		case SETTING_VBA_CONTROL_STYLE:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Map the face buttons of the Game Boy (Classic/Color/Advance) differently\ndepending on personal preference.");
			break;
		case SETTING_VBA_FPS:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", Settings.DisplayFrameRate ? "INFO - Display Framerate is set to 'Enabled' - an FPS counter is displayed onscreen." : "INFO - Display Framerate is set to 'Disabled'.");
			break;
		case SETTING_CHANGE_RESOLUTION:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", "INFO - Change the display resolution - press X to confirm.");
			#ifndef PS3_SDK_3_41
				cellDbgFontPrintf(0.09f, 0.86f, 0.86f, RED, "%s", "WARNING - This setting might not work correctly on 1.92 FW.");
			#endif
			break;
/*
		case SETTING_PAL60_MODE:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", Settings.PS3PALTemporalMode60Hz ? "INFO - PAL 60Hz mode is enabled - 60Hz NTSC games will run correctly at 576p PAL\nresolution. NOTE: This is configured on-the-fly." : "INFO - PAL 60Hz mode disabled - 50Hz PAL games will run correctly at 576p PAL\nresolution. NOTE: This is configured on-the-fly.");
			break;
*/
		case SETTING_HW_TEXTURE_FILTER:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", Settings.PS3Smooth ? "INFO - Hardware filtering is set to 'Bilinear filtering'." : "INFO - Hardware filtering is set to 'Point filtering' - most shaders\nlook much better on this setting.");
			break;
		case SETTING_KEEP_ASPECT_RATIO:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", Settings.PS3KeepAspect ? "INFO - Aspect ratio is set to 'Scaled' - screen will have black borders\nleft and right on widescreen TVs/monitors." : "INFO - Aspect ratio is set to 'Stretched' - widescreen TVs/monitors will have\na full stretched picture.");
			break;
		case SETTING_RSOUND_ENABLED:
			cellDbgFontPrintf(0.09f, 0.83f, 0.86f, LIGHTBLUE, "%s", Settings.RSoundEnabled ? "INFO - Sound is set to 'RSound' - the sound will be streamed over the network\nto the RSound audio server." : "INFO - Sound is set to 'Normal' - normal audio output will be used.");
			break;
		case SETTING_RSOUND_SERVER_IP_ADDRESS:
			cellDbgFontPuts(0.09f, 0.83f, 0.86f, LIGHTBLUE, "INFO - Enter the IP address of the RSound audio server. IP address must be\nan IPv4 32-bits address, eg: '192.168.0.7'.");
			break;
/*
		case SETTING_FONT_SIZE:
			cellDbgFontPuts(0.09f, 0.83f, 0.86f, LIGHTBLUE, "INFO - Increase or decrease the font size in the menu.");
			break;
*/
		case SETTING_HW_OVERSCAN_AMOUNT:
			cellDbgFontPuts(0.09f, 0.83f, 0.86f, LIGHTBLUE, "INFO - Adjust or decrease overscan. Set this to higher than 0.000\nif the screen doesn't fit on your TV/monitor.");
			break;
		case SETTING_DEFAULT_ALL:
			cellDbgFontPuts(0.09f, 0.83f, 0.86f, LIGHTBLUE, "INFO - Set all 'General' settings back to their default values.");
			break;
		case SETTING_VBA_DEFAULT_ALL:
			cellDbgFontPuts(0.09f, 0.83f, 0.86f, LIGHTBLUE, "INFO - Set all 'VBA' settings back to their default values.");
			break;
		case SETTING_PATH_DEFAULT_ALL:
			cellDbgFontPuts(0.09f, 0.83f, 0.86f, LIGHTBLUE, "INFO - Set all 'Path' settings back to their default values.");
			break;
	}
}

char * Input_PrintMappedButton(int mappedbutton)
{
	switch(mappedbutton)
	{
		case BTN_A:
			return "Button A";
			break;
		case BTN_B:
			return "Button B";
			break;
		case BTN_L:
			return "L Button";
			break;
		case BTN_R:
			return "R Button";
			break;
		case BTN_SELECT:
			return "Button Select";
			break;
		case BTN_START:
			return "Button Start";
			break;
		case BTN_LEFT:
			return "D-Pad Left";
			break;
		case BTN_RIGHT:
			return "D-Pad Right";
			break;
		case BTN_UP:
			return "D-Pad Up";
			break;
		case BTN_DOWN:
			return "D-Pad Down";
			break;
		case BTN_QUICKSAVE:
			return "Save State";
			break;
		case BTN_QUICKLOAD:
			return "Load State";
			break;
		case BTN_INCREMENTSAVE:
			return "Increment state position";
			break;
		case BTN_DECREMENTSAVE:
			return "Decrement state position";
			break;
		case BTN_EXITTOMENU:
			return "Exit to menu";
			break;
		case BTN_NONE:
			return "None";
			break;
		case BTN_FASTFORWARD:
			return "Fast forward";
			break;
		default:
			return "Unknown";
			break;

	}
}

//bool next: true is next, false is previous
int Input_GetAdjacentButtonmap(int buttonmap, bool next)
{
	switch(buttonmap)
	{
		case BTN_UP:
			return next ? BTN_DOWN : BTN_NONE;
			break;
		case BTN_DOWN:
			return next ? BTN_LEFT : BTN_UP;
			break;
		case BTN_LEFT:
			return next ? BTN_RIGHT : BTN_DOWN;
			break;
		case BTN_RIGHT:
			return next ?  BTN_A : BTN_LEFT;
			break;
		case BTN_A:
			return next ? BTN_B : BTN_RIGHT;
			break;
		case BTN_B:
			return next ? BTN_L : BTN_A;
			break;
		case BTN_L:
			return next ? BTN_R : BTN_B;
			break;
		case BTN_R:
			return next ? BTN_SELECT : BTN_L;
			break;
		case BTN_SELECT:
			return next ? BTN_START : BTN_R;
			break;
		case BTN_START:
			return next ? BTN_QUICKSAVE : BTN_SELECT;
			break;
		case BTN_QUICKSAVE:
			return next ? BTN_QUICKLOAD : BTN_START;
			break;
		case BTN_QUICKLOAD:
			return next ? BTN_EXITTOMENU : BTN_QUICKSAVE;
			break;
		case BTN_EXITTOMENU:
			return next ? BTN_DECREMENTSAVE : BTN_QUICKLOAD;
			break;
		case BTN_DECREMENTSAVE:
			return next ? BTN_INCREMENTSAVE : BTN_EXITTOMENU;
			break;
		case BTN_INCREMENTSAVE:
			return next ? BTN_FASTFORWARD : BTN_DECREMENTSAVE;
			break;
		case BTN_FASTFORWARD:
			return next ? BTN_NONE : BTN_INCREMENTSAVE;
			break;
		case BTN_NONE:
			return next ? BTN_UP : BTN_FASTFORWARD;
			break;
		default:
			return BTN_NONE;
			break;
	}
}

void Input_MapButton(int* buttonmap, bool next, int defaultbutton)
{
	if(defaultbutton == NULL)
	{
		*buttonmap = Input_GetAdjacentButtonmap(*buttonmap, next);
	}
	else
	{
		*buttonmap = defaultbutton;
	}
	//FIXME: Do something with this, or remove it
	/*
	if(*buttonmap == (BTN_LEFT | BTN_RIGHT | BTN_DOWN | BTN_UP | BTN_A | BTN_B | BTN_START | BTN_SELECT))
	{
	}
	*/
	MenuResetControlStyle();
}

void do_controls_settings()
{
	if(CellInput->UpdateDevice(0) == CELL_OK)
	{
			// back to ROM menu if CIRCLE is pressed
			if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
			{
				menuStack.pop();
				return;
			}

			if (CellInput->WasButtonHeld(0, CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))	// down to next setting
			{
				currently_selected_controller_setting++;
				if (currently_selected_controller_setting >= MAX_NO_OF_CONTROLS_SETTINGS)
				{
					currently_selected_controller_setting = FIRST_CONTROLS_SETTING;
				}
				if(CellInput->IsButtonPressed(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))
				{
					sys_timer_usleep(SETTINGS_DELAY);
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

			if (CellInput->WasButtonHeld(0, CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))	// up to previous setting
			{
					currently_selected_controller_setting--;
					if (currently_selected_controller_setting < FIRST_CONTROLS_SETTING)
					{
						currently_selected_controller_setting = MAX_NO_OF_CONTROLS_SETTINGS-1;
					}
					if(CellInput->IsButtonPressed(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))
					{
						sys_timer_usleep(SETTINGS_DELAY);
					}
			}
					switch(currently_selected_controller_setting)
					{
						case SETTING_CONTROLS_DPAD_UP:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.DPad_Up,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.DPad_Up,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.DPad_Up,true,BTN_UP);
						}
							break;
						case SETTING_CONTROLS_DPAD_DOWN:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.DPad_Down,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.DPad_Down,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.DPad_Down,true,BTN_DOWN);
						}
							break;
						case SETTING_CONTROLS_DPAD_LEFT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.DPad_Left,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.DPad_Left,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.DPad_Left,true,BTN_LEFT);
						}
							break;
						case SETTING_CONTROLS_DPAD_RIGHT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.DPad_Right,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.DPad_Right,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.DPad_Right,true,BTN_RIGHT);
						}
							break;
						case SETTING_CONTROLS_BUTTON_CIRCLE:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonCircle,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonCircle,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonCircle,true,BTN_A);
						}
							break;
						case SETTING_CONTROLS_BUTTON_CROSS:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonCross,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonCross,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonCross,true,BTN_B);
						}
							break;
						case SETTING_CONTROLS_BUTTON_TRIANGLE:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonTriangle,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonTriangle,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonTriangle,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_SQUARE:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonSquare,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonSquare,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonSquare,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_SELECT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonSelect,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonSelect,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonSelect,true,BTN_SELECT);
						}
							break;
						case SETTING_CONTROLS_BUTTON_START:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonStart,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonStart,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonStart,true,BTN_START);
						}
							break;
						case SETTING_CONTROLS_BUTTON_L1:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL1,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL1,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL1,true,BTN_L);
						}
							break;
						case SETTING_CONTROLS_BUTTON_L2:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_R2:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR2,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR2,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR2,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_L3:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL3,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL3,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL3,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_R3:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR3,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR3,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR3,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_R1:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR1,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR1,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR1,true,BTN_R);
						}
							break;
						case SETTING_CONTROLS_BUTTON_L2_BUTTON_L3:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonL3,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonL3,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonL3,true,BTN_NONE);
						}
							break;
						case SETTING_CONTROLS_BUTTON_L2_BUTTON_R2:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonR2,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonR2,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonR2,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_L2_BUTTON_R3:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonR3,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonR3,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_ButtonR3,true,BTN_QUICKLOAD);
						}
						break;
						case SETTING_CONTROLS_BUTTON_R2_BUTTON_R3:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR2_ButtonR3,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR2_ButtonR3,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR2_ButtonR3,true,BTN_QUICKSAVE);
						}
							break;
						case SETTING_CONTROLS_ANALOG_R_UP:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.AnalogR_Up,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.AnalogR_Up,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.AnalogR_Up,true,BTN_NONE);
						}
						if(CellInput->WasButtonPressed(0, CTRL_SELECT))
						{
							Settings.AnalogR_Up_Type = !Settings.AnalogR_Up_Type;
						}
						break;
						case SETTING_CONTROLS_ANALOG_R_DOWN:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.AnalogR_Down,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.AnalogR_Down,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.AnalogR_Down,true,BTN_NONE);
						}
						if(CellInput->WasButtonPressed(0, CTRL_SELECT))
						{
							Settings.AnalogR_Down_Type = !Settings.AnalogR_Down_Type;
						}
						break;
						case SETTING_CONTROLS_ANALOG_R_LEFT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.AnalogR_Left,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.AnalogR_Left,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.AnalogR_Left,true,BTN_DECREMENTSAVE);
						}
						if(CellInput->WasButtonPressed(0, CTRL_SELECT))
						{
							Settings.AnalogR_Left_Type = !Settings.AnalogR_Left_Type;
						}
						break;
						case SETTING_CONTROLS_ANALOG_R_RIGHT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.AnalogR_Right,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.AnalogR_Right,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.AnalogR_Right,true,BTN_INCREMENTSAVE);
						}
						if(CellInput->WasButtonPressed(0, CTRL_SELECT))
						{
							Settings.AnalogR_Right_Type = !Settings.AnalogR_Right_Type;
						}
						break;
						case SETTING_CONTROLS_BUTTON_L2_ANALOG_R_RIGHT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Right,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Right,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Right,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_L2_ANALOG_R_LEFT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Left,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Left,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Left,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_L2_ANALOG_R_UP:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Up,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Up,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Up,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_L2_ANALOG_R_DOWN:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Down,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Down,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Down,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_R2_ANALOG_R_RIGHT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Right,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Right,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonL2_AnalogR_Right,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_R2_ANALOG_R_LEFT:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Left,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Left,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Left,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_R2_ANALOG_R_UP:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Up,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Up,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Up,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_R2_ANALOG_R_DOWN:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Down,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Down,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR2_AnalogR_Down,true,BTN_NONE);
						}
						break;
						case SETTING_CONTROLS_BUTTON_R3_BUTTON_L3:
						if(CellInput->WasButtonHeld(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK))
						{
							Input_MapButton(&Settings.ButtonR3_ButtonL3,false,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->WasButtonHeld(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Input_MapButton(&Settings.ButtonR3_ButtonL3,true,NULL);
							if(CellInput->IsButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
							{
								sys_timer_usleep(SETTINGS_DELAY);
							}
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Input_MapButton(&Settings.ButtonR3_ButtonL3,true,BTN_EXITTOMENU);
						}
						break;
						case SETTING_CONTROLS_DEFAULT_ALL:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->IsButtonPressed(0, CTRL_START))
						{
							App->ButtonMappingSettings(MAP_BUTTONS_OPTION_DEFAULT);
						}
							break;
				default:
					break;
			} // end of switch
	}

	float yPos = 0.09;
	float ySpacing = 0.04;

	cellDbgFontPuts		(0.09f,	0.05f,	FONT_SIZE,	GREEN,	"GENERAL");
	cellDbgFontPuts		(0.27f,	0.05f,	FONT_SIZE,	GREEN,	"VBA");
	cellDbgFontPuts		(0.45f,	0.05f,	FONT_SIZE,	GREEN,	"PATH");
	cellDbgFontPuts		(0.63f, 0.05f,	FONT_SIZE,	RED,	"CONTROLS"); 
	Graphics->FlushDbgFont();

//PAGE 1
if((currently_selected_controller_setting-FIRST_CONTROLS_SETTING) < NUM_ENTRY_PER_SETTINGS_PAGE)
{
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_DPAD_UP ? YELLOW : WHITE,	"D-Pad Up");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.DPad_Up == BTN_UP ? GREEN : ORANGE, Input_PrintMappedButton(Settings.DPad_Up));

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_DPAD_DOWN ? YELLOW : WHITE,	"D-Pad Down");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.DPad_Down == BTN_DOWN ? GREEN : ORANGE, Input_PrintMappedButton(Settings.DPad_Down));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_DPAD_LEFT ? YELLOW : WHITE,	"D-Pad Left");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.DPad_Left == BTN_LEFT ? GREEN : ORANGE, Input_PrintMappedButton(Settings.DPad_Left));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_DPAD_RIGHT ? YELLOW : WHITE,	"D-Pad Right");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.DPad_Right == BTN_RIGHT ? GREEN : ORANGE, Input_PrintMappedButton(Settings.DPad_Right));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_CIRCLE ? YELLOW : WHITE,	"Circle button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonCircle == BTN_A ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonCircle));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_CROSS ? YELLOW : WHITE,	"Cross button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonCross == BTN_B ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonCross));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_TRIANGLE ? YELLOW : WHITE,	"Triangle button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonTriangle == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonTriangle));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_SQUARE ? YELLOW : WHITE,	"Square button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonSquare == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonSquare));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_SELECT ? YELLOW : WHITE,	"Select button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonSelect == BTN_SELECT ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonSelect));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_START ? YELLOW : WHITE,	"Start button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonStart == BTN_START ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonStart));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L1 ? YELLOW : WHITE,	"L1 button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL1 == BTN_L ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL1));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R1 ? YELLOW : WHITE,	"R1 button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR1 == BTN_R ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR1));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2 ? YELLOW : WHITE,	"L2 button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2 == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R2 ? YELLOW : WHITE,	"R2 button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR2 == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR2));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L3 ? YELLOW : WHITE,	"L3 button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL3 == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL3));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R3 ? YELLOW : WHITE,	"R3 button");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR3 == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR3));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_BUTTON_L3 ? YELLOW : WHITE,	"Button combo: L2 & L3");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_ButtonL3 == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_ButtonL3));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_BUTTON_R2 ? YELLOW : WHITE,	"Button combo: L2 & R2");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_ButtonR2 == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_ButtonR2));
	Graphics->FlushDbgFont();

}

//PAGE 2
if((currently_selected_controller_setting >= SETTING_CONTROLS_BUTTON_L2_BUTTON_R3))
{
	yPos = 0.09;

	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_BUTTON_R3 ? YELLOW : WHITE,	"Button combo: L2 & R3");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_ButtonR3 == BTN_QUICKLOAD ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_ButtonR3));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_ANALOG_R_RIGHT ? YELLOW : WHITE,	"Button combo: L2 & RStick Right");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_AnalogR_Right == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_AnalogR_Right));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_ANALOG_R_LEFT ? YELLOW : WHITE,	"Button combo: L2 & RStick Left");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_AnalogR_Left == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_AnalogR_Left));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_ANALOG_R_UP ? YELLOW : WHITE,	"Button combo: L2 & RStick Up");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_AnalogR_Up == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_AnalogR_Up));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_L2_ANALOG_R_DOWN ? YELLOW : WHITE,	"Button combo: L2 & RStick Down");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonL2_AnalogR_Down == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonL2_AnalogR_Down));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R2_ANALOG_R_RIGHT ? YELLOW : WHITE,	"Button combo: R2 & RStick Right");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR2_AnalogR_Right == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR2_AnalogR_Right));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R2_ANALOG_R_LEFT ? YELLOW : WHITE,	"Button combo: R2 & RStick Left");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR2_AnalogR_Left == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR2_AnalogR_Left));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R2_ANALOG_R_UP ? YELLOW : WHITE,	"Button combo: R2 & RStick Up");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR2_AnalogR_Up == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR2_AnalogR_Up));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R2_ANALOG_R_DOWN ? YELLOW : WHITE,	"Button combo: R2 & RStick Down");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR2_AnalogR_Down == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR2_AnalogR_Down));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R2_BUTTON_R3 ? YELLOW : WHITE,	"Button combo: R2 & R3");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR2_ButtonR3 == BTN_QUICKSAVE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR2_ButtonR3));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_BUTTON_R3_BUTTON_L3 ? YELLOW : WHITE,	"Button combo: R3 & L3");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.ButtonR3_ButtonL3 == BTN_EXITTOMENU ? GREEN : ORANGE, Input_PrintMappedButton(Settings.ButtonR3_ButtonL3));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPrintf		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_ANALOG_R_UP ? YELLOW : WHITE,	"Right Stick - Up %s", Settings.AnalogR_Up_Type ? "(IsPressed)" : "(WasPressed)");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.AnalogR_Up == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.AnalogR_Up));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPrintf		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_ANALOG_R_DOWN ? YELLOW : WHITE,	"Right Stick - Down %s", Settings.AnalogR_Down_Type ? "(IsPressed)" : "(WasPressed)");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.AnalogR_Down == BTN_NONE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.AnalogR_Down));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPrintf		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_ANALOG_R_LEFT ? YELLOW : WHITE,	"Right Stick - Left %s", Settings.AnalogR_Left_Type ? "(IsPressed)" : "(WasPressed)");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.AnalogR_Left == BTN_DECREMENTSAVE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.AnalogR_Left));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPrintf		(0.09f,	yPos,	1.00f,	currently_selected_controller_setting == SETTING_CONTROLS_ANALOG_R_RIGHT ? YELLOW : WHITE,	"Right Stick - Right %s", Settings.AnalogR_Right_Type ? "(IsPressed)" : "(WasPressed)");
	cellDbgFontPuts		(0.5f,	yPos,	1.00f,	Settings.AnalogR_Right == BTN_INCREMENTSAVE ? GREEN : ORANGE, Input_PrintMappedButton(Settings.AnalogR_Right));
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, 1.00f, currently_selected_controller_setting == SETTING_CONTROLS_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");

}

	DisplayHelpMessage(currently_selected_controller_setting);

	cellDbgFontPuts(0.09f, 0.89f, 1.00f, YELLOW,
	"UP/DOWN - select  L2+R2 - resume game   X/LEFT/RIGHT - change");
	cellDbgFontPuts(0.09f, 0.93f, 1.00f, YELLOW,
	"START - default   L1/CIRCLE - go back");
	Graphics->FlushDbgFont();
}

void do_path_settings()
{
	if(CellInput->UpdateDevice(0) == CELL_OK)
	{
			// back to ROM menu if CIRCLE is pressed
			if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
			{
				menuStack.pop();
				return;
			}

			if (CellInput->WasButtonPressed(0, CTRL_R1))
			{
				menuStack.push(do_controls_settings);
				return;
			}

			if (CellInput->WasButtonHeld(0, CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))	// down to next setting
			{
				currently_selected_path_setting++;
				if (currently_selected_path_setting >= MAX_NO_OF_PATH_SETTINGS)
				{
					currently_selected_path_setting = FIRST_PATH_SETTING;
				}

				if(CellInput->IsButtonPressed(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))
				{
					sys_timer_usleep(SETTINGS_DELAY);
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

			if (CellInput->WasButtonHeld(0, CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))	// up to previous setting
			{
					currently_selected_path_setting--;
					if (currently_selected_path_setting < FIRST_PATH_SETTING)
					{
						currently_selected_path_setting = MAX_NO_OF_PATH_SETTINGS-1;
					}
					if(CellInput->IsButtonPressed(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))
					{
						sys_timer_usleep(SETTINGS_DELAY);
					}
			}

					switch(currently_selected_path_setting)
					{
						case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							menuStack.push(do_pathChoice);
							tmpBrowser = NULL;
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Settings.PS3PathROMDirectory = "/";
						}
							break;
						case SETTING_PATH_SAVESTATES_DIRECTORY:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							menuStack.push(do_pathChoice);
							tmpBrowser = NULL;
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Settings.PS3PathSaveStates = EMULATOR_PATH_STATES;
						}
							break;
						case SETTING_PATH_SRAM_DIRECTORY:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							menuStack.push(do_pathChoice);
							tmpBrowser = NULL;
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Settings.PS3PathSRAM = EMULATOR_PATH_STATES;
						}
							break;
						case SETTING_PATH_DEFAULT_ALL:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->IsButtonPressed(0, CTRL_START))
						{
							Settings.PS3PathROMDirectory = "/";
							Settings.PS3PathSaveStates = EMULATOR_PATH_STATES;
							Settings.PS3PathSRAM = EMULATOR_PATH_STATES;
						}
							break;
				default:
					break;
			} // end of switch
	}

	float yPos = 0.09;
	float ySpacing = 0.04;

	cellDbgFontPuts		(0.09f,	0.05f,	FONT_SIZE,	GREEN,	"GENERAL");
	cellDbgFontPuts		(0.27f,	0.05f,	FONT_SIZE,	GREEN,	"VBA");
	cellDbgFontPuts		(0.45f,	0.05f,	FONT_SIZE,	RED,	"PATH");
	cellDbgFontPuts		(0.63f, 0.05f,	FONT_SIZE, GREEN,	"CONTROLS"); 
	Graphics->FlushDbgFont();

	cellDbgFontPuts		(0.09f,	yPos,	FONT_SIZE,	currently_selected_path_setting == SETTING_PATH_DEFAULT_ROM_DIRECTORY ? YELLOW : WHITE,	"Startup ROM Directory");
	cellDbgFontPuts		(0.5f,	yPos,	FONT_SIZE,	!(strcmp(Settings.PS3PathROMDirectory.c_str(),"/")) ? GREEN : ORANGE, Settings.PS3PathROMDirectory.c_str());

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	FONT_SIZE,	currently_selected_path_setting == SETTING_PATH_SAVESTATES_DIRECTORY ? YELLOW : WHITE,	"Savestate Directory");
	cellDbgFontPuts		(0.5f,	yPos,	FONT_SIZE,	!(strcmp(Settings.PS3PathSaveStates.c_str(),EMULATOR_PATH_STATES)) ? GREEN : ORANGE, Settings.PS3PathSaveStates.c_str());
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	FONT_SIZE,	currently_selected_path_setting == SETTING_PATH_SRAM_DIRECTORY ? YELLOW : WHITE,	"SRAM Directory");
	cellDbgFontPuts		(0.5f,	yPos,	FONT_SIZE,	!(strcmp(Settings.PS3PathSRAM.c_str(),EMULATOR_PATH_STATES)) ? GREEN : ORANGE, Settings.PS3PathSRAM.c_str());
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, FONT_SIZE, currently_selected_path_setting == SETTING_PATH_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");

	DisplayHelpMessage(currently_selected_path_setting);

	cellDbgFontPuts(0.09f, 0.89f, FONT_SIZE, YELLOW,
	"UP/DOWN - select  L2+R2 - resume game   X/LEFT/RIGHT - change");
	cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,
	"START - default   L1/CIRCLE - go back   R1 - go forward");
	Graphics->FlushDbgFont();
}

void do_vba_settings()
{
	if(CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
			// back to ROM menu if CIRCLE is pressed
			if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
			{
				menuStack.pop();
				return;
			}

			if (CellInput->WasButtonPressed(0, CTRL_R1))
			{
				menuStack.push(do_path_settings);
				return;
			}

			if (CellInput->WasButtonHeld(0, CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))	// down to next setting
			{
				currently_selected_vba_setting++;
				if (currently_selected_vba_setting >= MAX_NO_OF_VBA_SETTINGS)
				{
					currently_selected_vba_setting = FIRST_VBA_SETTING;
				}
				if(CellInput->IsButtonPressed(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))
				{
					sys_timer_usleep(SETTINGS_DELAY);
				}
			}
			if (CellInput->WasButtonHeld(0, CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))	// up to previous setting
			{
					currently_selected_vba_setting--;
					if (currently_selected_vba_setting < FIRST_VBA_SETTING)
					{
						currently_selected_vba_setting = MAX_NO_OF_VBA_SETTINGS-1;
					}
					if(CellInput->IsButtonPressed(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))
					{
						sys_timer_usleep(SETTINGS_DELAY);
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
			
			switch(currently_selected_vba_setting)
			{
					// display framerate on/off
					case SETTING_VBA_FPS:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
						{
							Settings.DisplayFrameRate = !Settings.DisplayFrameRate;
						}
						if(CellInput->IsButtonPressed(0, CTRL_START))
						{
							Settings.DisplayFrameRate = false;
						}
						break;
			case SETTING_VBA_GBABIOS:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_biosChoice);
					tmpBrowser = NULL;
				}
				if (CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.GBABIOS.clear();
				}
				break;
			case SETTING_VBA_CONTROL_STYLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) || CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
				{
					Settings.ControlStyle = (Emulator_ControlStyle)(((Settings.ControlStyle) + 1) % 2);
					if(Settings.ControlStyle == CONTROL_STYLE_ORIGINAL)
					{
						Settings.ButtonCircle = BTN_A;
						Settings.ButtonCross = BTN_B;
					}
					else
					{
						Settings.ButtonSquare = BTN_B;
						Settings.ButtonCross = BTN_A;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Settings.ControlStyle = CONTROL_STYLE_ORIGINAL;
					Settings.ButtonCircle = BTN_A;
					Settings.ButtonCross = BTN_B;
				}
				break;
					case SETTING_VBA_DEFAULT_ALL:
						if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0,CTRL_LSTICK) | CellInput->IsButtonPressed(0, CTRL_START) | CellInput->WasButtonPressed(0, CTRL_CROSS))
					{
						Settings.DisplayFrameRate = false;
						Settings.ControlStyle = CONTROL_STYLE_ORIGINAL;
						Settings.ButtonCircle = BTN_A;
						Settings.ButtonCross = BTN_B;
						Settings.GBABIOS.clear();
					}
					break;
				default:
					break;
			} // end of switch
	}

	cellDbgFontPuts		(0.09f,	0.05f,	FONT_SIZE,	GREEN,	"GENERAL");
	cellDbgFontPuts		(0.27f,	0.05f,	FONT_SIZE,	RED,	"VBA");
	cellDbgFontPuts		(0.45f,	0.05f,	FONT_SIZE,	GREEN,	"PATH");
	cellDbgFontPuts		(0.63f, 0.05f,	FONT_SIZE, GREEN,	"CONTROLS"); 
	Graphics->FlushDbgFont();

	float yPos = 0.09;
	float ySpacing = 0.04;

	cellDbgFontPuts		(0.09f,	yPos,	FONT_SIZE,	currently_selected_vba_setting == SETTING_VBA_FPS ? YELLOW : WHITE,	"Display Framerate");
	cellDbgFontPuts		(0.5f,	yPos,	FONT_SIZE,	Settings.DisplayFrameRate == false ? GREEN : ORANGE, Settings.DisplayFrameRate == true ? "ON" : "OFF");

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_vba_setting == SETTING_VBA_GBABIOS ? YELLOW : WHITE, "Use GBA BIOS: ");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.GBABIOS.empty() ? GREEN : ORANGE, Settings.GBABIOS.empty() ? "NO" : "YES");

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_vba_setting == SETTING_VBA_CONTROL_STYLE ? YELLOW : WHITE, "Control Style");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE,
			Settings.ControlStyle == CONTROL_STYLE_ORIGINAL ? GREEN : ORANGE,
			"%s", Settings.ControlStyle == CONTROL_STYLE_ORIGINAL ? "Original (X->B, O->A)" : "Better (X->A, []->B)");

	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, FONT_SIZE, currently_selected_vba_setting == SETTING_VBA_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");
	Graphics->FlushDbgFont();

	DisplayHelpMessage(currently_selected_vba_setting);

	cellDbgFontPuts(0.09f, 0.89f, FONT_SIZE, YELLOW,
	"UP/DOWN - select  L2+R2 - resume game   X/LEFT/RIGHT - change");
	cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,
	"START - default   L1/CIRCLE - go back   R1 - go forward");
	Graphics->FlushDbgFont();
}

void do_general_settings()
{
	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_CIRCLE) || CellInput->WasButtonPressed(0, CTRL_L1))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_R1))
		{
			menuStack.push(do_vba_settings);
			return;
		}

		if (CellInput->WasButtonHeld(0, CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))	// down to next setting
		{
			currently_selected_setting++;
			if (currently_selected_setting >= MAX_NO_OF_SETTINGS)
			{
				currently_selected_setting = FIRST_GENERAL_SETTING;
			}
			if(CellInput->IsButtonPressed(0,CTRL_DOWN) | CellInput->IsAnalogPressedDown(0, CTRL_LSTICK))
			{
				sys_timer_usleep(SETTINGS_DELAY);
			}
		}

		if (CellInput->WasButtonHeld(0, CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))	// up to previous setting
		{
				currently_selected_setting--;
				if (currently_selected_setting < FIRST_GENERAL_SETTING)
				{
					currently_selected_setting = MAX_NO_OF_SETTINGS-1;
				}
				if(CellInput->IsButtonPressed(0,CTRL_UP) | CellInput->IsAnalogPressedUp(0, CTRL_LSTICK))
				{
					sys_timer_usleep(SETTINGS_DELAY);
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

		switch(currently_selected_setting)
		{
		case SETTING_CURRENT_SAVE_STATE_SLOT:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) || CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK))
					   App->DecrementStateSlot();
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) || CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
					   App->IncrementStateSlot();
				break;

			case SETTING_CHANGE_RESOLUTION:
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
					Graphics->SwitchResolution(Graphics->GetCurrentResolution(), Settings.PS3PALTemporalMode60Hz);
				}
				if(CellInput->IsButtonPressed(0, CTRL_START))
				{
					Graphics->SwitchResolution(Graphics->GetInitialResolution(), Settings.PS3PALTemporalMode60Hz);
				}
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

         case SETTING_RSOUND_ENABLED:
            if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
            {
               Settings.RSoundEnabled = !Settings.RSoundEnabled;
               App->ToggleSound();
            }
            if(CellInput->IsButtonPressed(0, CTRL_START))
            {
               Settings.RSoundEnabled = false;
               App->ToggleSound();
            }
            break;
         case SETTING_RSOUND_SERVER_IP_ADDRESS:
            if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->WasAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->WasAnalogPressedRight(0,CTRL_LSTICK))
            {
               App->OSKStart(L"Enter the IP address for the RSound Server. Example (below):", L"192.168.1.1");
               Settings.RSoundServerIPAddress = App->OSKOutputString();
            }
            if(CellInput->IsButtonPressed(0, CTRL_START))
            {
               Settings.RSoundServerIPAddress = "0.0.0.0";
            }
            break;

			case SETTING_SHADER:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_shaderChoice);
					tmpBrowser = NULL;
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
					Graphics->SetAspectRatio(SCREEN_4_3_ASPECT_RATIO);
					Graphics->SetSmooth(Settings.PS3Smooth);
					Graphics->SetOverscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100);
					Settings.ControlStyle = CONTROL_STYLE_ORIGINAL;
					Settings.PS3PALTemporalMode60Hz = false;
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

	float yPos = 0.09;
	float ySpacing = 0.04;

	cellDbgFontPuts		(0.09f,	0.05f,	FONT_SIZE,	RED,	"GENERAL");
	cellDbgFontPuts		(0.27f,	0.05f,	FONT_SIZE,	GREEN,	"VBA");
	cellDbgFontPuts		(0.45f,	0.05f,	FONT_SIZE,	GREEN,	"PATH");
	cellDbgFontPuts		(0.63f, 0.05f,	FONT_SIZE, GREEN,	"CONTROLS"); 

	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_setting == SETTING_CURRENT_SAVE_STATE_SLOT ? YELLOW : WHITE, "Current save state slot");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, App->CurrentSaveStateSlot() == 0 ? GREEN : ORANGE, "%d", App->CurrentSaveStateSlot());

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_setting == SETTING_CHANGE_RESOLUTION ? YELLOW : WHITE, "Resolution");

	switch(Graphics->GetCurrentResolution())
	{
		case CELL_VIDEO_OUT_RESOLUTION_480:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_480 ? GREEN : ORANGE, "720x480 (480p)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_720:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_720 ? GREEN : ORANGE, "1280x720 (720p)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1080 ? GREEN : ORANGE, "1920x1080 (1080p)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_576:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_576 ? GREEN : ORANGE, "720x576 (576p)");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1600x1080 ? GREEN : ORANGE, "1600x1080");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1440x1080 ? GREEN : ORANGE, "1440x1080");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1280x1080 ? GREEN : ORANGE, "1280x1080");
			Graphics->FlushDbgFont();
			break;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080:
			cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_960x1080 ? GREEN : ORANGE, "960x1080");
			Graphics->FlushDbgFont();
			break;
	}
	Graphics->FlushDbgFont();

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_setting == SETTING_SHADER ? YELLOW : WHITE, "Selected Shader: ");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE,
			GREEN,
			"%s", Graphics->GetFragmentShaderPath().substr(Graphics->GetFragmentShaderPath().find_last_of('/')).c_str());

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_setting == SETTING_KEEP_ASPECT_RATIO ? YELLOW : WHITE, "Aspect Ratio");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.PS3KeepAspect == true ? GREEN : ORANGE, "%s", Settings.PS3KeepAspect == true ? "Scaled" : "Stretched");

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, FONT_SIZE, currently_selected_setting == SETTING_HW_TEXTURE_FILTER ? YELLOW : WHITE, "Hardware Filtering");
	cellDbgFontPrintf(0.5f, yPos, FONT_SIZE, Settings.PS3Smooth == true ? GREEN : ORANGE, "%s", Settings.PS3Smooth == true ? "Linear interpolation" : "Point filtering");

	yPos += ySpacing;
	cellDbgFontPuts		(0.09f,	yPos,	FONT_SIZE,	currently_selected_setting == SETTING_HW_OVERSCAN_AMOUNT ? YELLOW : WHITE,	"Overscan");
	cellDbgFontPrintf	(0.5f,	yPos,	FONT_SIZE,	Settings.PS3OverscanAmount == 0 ? GREEN : ORANGE, "%f", (float)Settings.PS3OverscanAmount/100);

   yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, App->GetFontSize(), currently_selected_setting == SETTING_RSOUND_ENABLED ? YELLOW : WHITE, "Sound");
	cellDbgFontPuts(0.5f, yPos, App->GetFontSize(), Settings.RSoundEnabled == false ? GREEN : ORANGE, Settings.RSoundEnabled == true ? "RSound" : "Normal");

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, App->GetFontSize(), currently_selected_setting == SETTING_RSOUND_SERVER_IP_ADDRESS ? YELLOW : WHITE, "RSound Server IP Address");
	cellDbgFontPuts(0.5f, yPos, App->GetFontSize(), strcmp(Settings.RSoundServerIPAddress,"0.0.0.0") ? ORANGE : GREEN, Settings.RSoundServerIPAddress);


	Graphics->FlushDbgFont();




	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, FONT_SIZE, currently_selected_setting == SETTING_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");

	DisplayHelpMessage(currently_selected_setting);

	cellDbgFontPuts(0.09f, 0.89f, FONT_SIZE, YELLOW,
	"UP/DOWN - select  L2+R2 - resume game   X/LEFT/RIGHT - change");
	cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,
	"START - default   L1/CIRCLE - go back   R1 - go forward");
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

				const void* data = NULL;
				int size = zipIo.GetEntryData(&data);
				if (size > 0 && data != NULL)
				{
					FILE *file = fopen(rom_path.c_str(), "wb");
					LOG_DBG("ZipIO: writing file %s \t size: %d\n", rom_path.c_str(), size);
					fwrite(data, size, 1, file);
					fclose(file);
					LOG_DBG("ZipIO: stored hack tmpfile\n");

					//free(data);

					App->LoadROM(rom_path, true);
					{
						menuStack.pop();
						MenuStop();

						// switch emulator to emulate mode
						App->StartROMRunning();
					}

					remove(rom_path.c_str());
				}
				else
				{
					LOG_DBG("ZipIO: No data returned! size: (%d)\n", size);
				}
			}
		}
	}

	zipIo.SetCurrentEntryPosition(current_index);

	// render it
	int page_number = current_index / NUM_ENTRY_PER_PAGE;
	int page_base = page_number * NUM_ENTRY_PER_PAGE;
	float currentX = 0.09f;
	float currentY = 0.09f;
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

	cellDbgFontPuts(0.09f, 0.88f, FONT_SIZE, YELLOW, "X - enter directory/load game");
	cellDbgFontPuts(0.09f, 0.92f, FONT_SIZE, PURPLE, "L2 + R2 - return to game");
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
			menuStack.push(do_general_settings);
		}

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(browser->IsCurrentADirectory())
			{
				browser->PushDirectory(	browser->GetCurrentDirectoryInfo().dir + "/" + browser->GetCurrentEntry()->d_name,
										CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY,
										"gb|gbc|gba|GBA|GB|GBC|7z|zip");
			}
			else if (browser->IsCurrentAFile())
			{
				//load game (standard controls), go back to main loop
				rom_path = browser->GetCurrentDirectoryInfo().dir + "/" + browser->GetCurrentEntry()->d_name;

				if (FileBrowser::GetExtension(rom_path).compare("7z") == 0 ||
					FileBrowser::GetExtension(rom_path).compare("zip") == 0)
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

	if(browser->IsCurrentADirectory())
	{
		if(!strcmp(browser->GetCurrentEntry()->d_name,"app_home") || !strcmp(browser->GetCurrentEntry()->d_name,"host_root"))
		{
			cellDbgFontPrintf(0.09f, 0.90f, 0.91f, RED, "%s","WARNING - Do not open this directory, or you might have to restart!");
		}
		else if(!strcmp(browser->GetCurrentEntry()->d_name,".."))
		{
			cellDbgFontPrintf(0.09f, 0.90f, 0.91f, LIGHTBLUE, "%s","INFO - Press X to go back to the previous directory.");
		}
		else
		{
			cellDbgFontPrintf(0.09f, 0.90f, 0.91f, LIGHTBLUE, "%s","INFO - Press X to enter the directory.");
		}
	}
	if(browser->IsCurrentAFile())
	{
		cellDbgFontPrintf(0.09f, 0.90f, 0.91f, LIGHTBLUE, "%s", "INFO - Press X to load the game. ");
	}
	cellDbgFontPuts	(0.09f,	0.05f,	FONT_SIZE,	RED,	"FILE BROWSER");
	cellDbgFontPrintf(0.7f, 0.05f, 0.82f, WHITE, "VBA PS3 v%s", EMULATOR_VERSION);
	cellDbgFontPrintf(0.09f, 0.09f, FONT_SIZE, YELLOW, "PATH: %s", browser->GetCurrentDirectoryInfo().dir.c_str());
	cellDbgFontPuts(0.09f, 0.93f, FONT_SIZE, YELLOW,
	"L2 + R2 - resume game           SELECT - Settings screen");
	Graphics->FlushDbgFont();

	RenderBrowser(browser);
}

void MenuMainLoop()
{
	// create file browser if null
	if (browser == NULL)
	{
		browser = new FileBrowser(Settings.PS3PathROMDirectory);
		browser->SetEntryWrap(false);
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



