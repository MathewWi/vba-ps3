***** VBA PS3 *********************************************************
***** Version 0.9.9 ******************************************************

This is an application for the PlayStation3 that makes it possible to play
Game Boy Classic / Game Boy Color / Game Boy Advance games on your jailbroken
PlayStation3. It is based on an up-to-date version of the popular PC emulator,
VBA-M.

=========================================
         Table of Contents
=========================================
1. Currently Implemented VBA-M functions.........................
2. Controls......................................................
     2.1.1 - Controls in ROM menu................................
     2.1.2 - Controls in Settings menu...........................
     2.1.3 - Controls in-game....................................
3. Settings......................................................
     3.1 - Settings..............................................
4. Currently known issues (as of 0.9.9)..........................
     4.1 - Resolution switching with FW 1.92 build...............
     4.2 - Sonic Advance 1/2 don't work..........................
5. Notes.........................................................
     5.1 - General notes.........................................
     5.2 - Performance notes.....................................
     5.3 - Performance w/ pixel shaders..........................
6. Planned improvements..........................................
7. For Developers................................................
     7.1 - Source Code Repository................................
          7.1.1 - Getting the latest VBA-PS3 revision............
     7.2  - Compilation Instructions.............................
          7.2.1 - For SDK 1.92...................................
          7.2.2 - For SDK 3.41...................................
	  7.2.3 - To make a debugging-friendly compile...........
     7.3 - CellFramework.........................................
          7.3.1 - Getting the latest Cellframework revision......
8. Credits......................................................
     


1. CURRENTLY IMPLEMENTED VBA-M FUNCTIONS
=========================================
* Saving/loading of SRAM
* Savestate loading/saving support
  - Savestate slot selectable in-game
  - Up to 10 saveslots

2. CONTROLS
===========
2.1.1 CONTROLS IN ROM MENU
==========================
Up				- Go up
Down				- Go down
Left				- Go back five file entries
Right				- Go forward five file entries
L1				- Go back one page
R1				- Go forward one page

Cross				- (If directory selected) enter directory/ (if ROM selected) start ROM
Triangle			- (If ROM selected) start ROM with multitap support
Circle				- (If not in root directory) Go back to previous directory
L2 + R2				- (If you previously exited a ROM) return to game
Select				- Go to settings menu (see 'CONTROLS IN SETTINGS MENU' section)

2.1.2 CONTROLS IN SETTINGS MENU
===============================
Up				- Go up one setting.
Down				- Go down one setting.
Left				- Change setting to the left.
Right				- Change setting to the right.

Circle				- Go back to ROM menu/Go back to previous Settings screen
Start				- Reset the setting back to the default value.
R1				- Go to the next Settings screen
L1				- Go to the previous Settings screen
L3 + R3				- Return back to game (if a ROM is loaded)

2.1.3 CONTROLS IN-GAME
======================
Nothing worth really explaining here - 

R3 + L3				- Press these two buttons together while in-game to go back to the ROM browser menu.

R3 + R2				- Save to currently selected save state slot
L3 + L2				- Load from currently selected save state slot

Right analog stick - Left	- Move current savestate slot one slot backwards
Right analog stick - Right	- Move current savestate slot one slot forward

To play a game with a USB controller as Player 1, start up your PS3 and rather than using the Sixaxis/DualShock3, plug in an USB port before
connecting the controller to the PS3 - your USB pad should then become Controller 1.

3. SETTINGS
===========

3.1 GENERAL SETTINGS
====================
Display framerate		- This will show the FPS (Frames Per Second) onscreen
Current save state slot		- Set the save state slot - this way, you can save multiple states and switch inbetween save states
Resolution			- Switch between resolutions - 480p mode, 576p mode, 720p mode, 1080p mode and more (depending on your monitor's
				supported resolutions). Press X to switch to the selected resolution. (*)
Aspect Ratio			- Switch between aspect ratios 'Scaled' (4:3) and 'Stretched' (widescreen/16:9).
Hardware Filtering		- Switch between Linear interpolation (Bilinear filtering) and Point filtering.
Overscan			- Set this to a certain value so that no part of the screen is cutoff on your television/monitor. Which value to set varies
				depending on your monitor/TV
Control Style			- Button mapping of the Game Boy controls.
					- Better: maps A button to Cross and the B button to Square
					- Original: maps the B button to Cross and the A button to Circle
Selected Shader			- Select a shader - the default shader is '/stock.cg'. Choose between 2XSaI, Bloom shader, curved CRT shader, HQ2x,
				Super2xSaI, SuperEagle, and more
Use GBA BIOS			- Select a GBA BIOS from the filesystem - upon pressing Cross, you can select a GBA BIOS. Note that Game Boy Classic ROMs
				will not work when a GBA BIOS has been selected. You can disable the GBA BIOS by pressing 'Start' on this setting.
				points to the server that will be running an RSound server application.
Savestate Directory		- Set the default savestate directory - all your savestates will be saved here and loaded from this location. Is set to
				USRDIR by default.
SRAM Directory			- Set the default SRAM directory - all your save data files (SRAM/EEPROM/flash) will be saved here and loaded from this
				location. Is set to USRDIR by default.
Startup ROM Directory		- Set the default ROM path to be used. The emulator will use this as the 'root' directory inside the ROM menu. Is set to '/'
				by default.
DEFAULT				- Set all of the general settings back to their default values

* - Might cause some problems on firmware FW 1.92 if you resolution switch too many times.

4. CURRENTLY KNOWN ISSUES (AS OF BUILD 0.9.9)
==========================================

4.1 RESOLUTION SWITCHING WITH 1.92 BUILD
========================================
1) Resolution switching with the 1.92 build may cause the program to crash after switching too many times. This issue will be hopefully resolved in
the future. The 3.41 build is unaffected by this.

4.1 SONIC ADVANCE 1/2 DON'T WORK
================================
2) Sonic Advance 1/2 currently don't work with VBA. This is a known issue and will be looked at shortly.

5. NOTES
========

5.1 GENERAL NOTES
=================
* For people running this on HDTVs complaining about input lag:
	- Turn off all post-processing filters you may have running - on Sony Bravia HDTVs, display Motion Flow (this also causes input lags
	with most games in general, not just this SNES emu. If your HDTV has a 'Game' mode or something of the sort, select that as well.

5.2 PERFORMANCE NOTES
=====================
This emulator has been optimized to run most games at full-speed (at any resolution - 480p/720p/1080p).

Some of the games that are guaranteed to run at fullspeed (without frameskipping) are games like:

Advance Wars
Astro Boy
Castlevania - Circle Of The Moon
Castlevania - Harmony of Dissonance
F-Zero: Maximum Velocity
Final Fantasy Tactics Advance
Golden Sun
Kuru Kuru Kururin
Legend of Zelda: A Link To The Past
Legend of Zelda: The Minish Cap
Mario Kart; Super Circuit
Mega Man Battle Network
Metroid Fusion
Metroid Zero Mission
Mother 3
Super Mario Advance
Super Mario Advance 2
Super Mario Advance 3
Sword of Mana
Tactics Ogre - The Knight of Lodis
Wario Land 4
Wario Ware Inc

and so on.

As ever, your mileage may vary. Please notify us of games that won't run 


5.3 PERFORMANCE NOTES - SHADERS
===============================
Here is the performance of the various fragment/pixel shaders:

SHADER      576p      480p      720p      960x1080      1280x1080      1440x1080                   1600x1080            1080p
=====================================================================================================================================
Stock       60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps 
2xSaI       60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Bloom       60fps     60fps     60fps     60fps         60fps          60fps (*)                   not fullspeed (**)   not fullspeed
Blur        60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
CRT         60fps     60fps     60fps     60fps         60fps          not fullspeed (**)          not fullspeed (**)   not fullspeed
HQ2x        60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Lanzcos12   60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Lanzcos16   60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
McGreen     60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Quad_Interp 60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Scale2xPlus 60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Scanlines   60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Sharpen     60fps     60fps     60fps     60fps         60fps          60fps                       60fps                60fps
Super2xSaI  60fps     60fps     60fps     60fps         60fps          60fps                       not fullspeed        not fullspeed
SuperEagle  60fps     60fps     60fps     60fps         60fps          60fps                       60fps                not fullspeed

*  - Might fluctuate with certain games that are CPU-intensive
** - Is fullspeed at 4:3 but not at 16:9

6. PLANNED IMPROVEMENTS
=======================
* Get Sonic Advance 1/2 to work
* 7z archive support

7. FOR DEVELOPERS
==================

7.1 SOURCE CODE REPOSITORY
===========================
This release corresponds (roughly) with revision = 9c5e75c77b

Source code repository is here:

https://code.google.com/p/vba-ps3/

7.1.1 GETTING LATEST VBA PS3 REVISION
=====================================
To be able to check out the latest revision, you must have Mercurial installed on your system. Type in the following:

hg clone https://vba-ps3.googlecode.com/hg/ vba-ps3

7.2 COMPILATION INSTRUCTIONS
===============================

7.2.1 FOR SDK 1.92
=====================
1 - Edit the Makefile and take out '-DPS3_SDK_3_41' from both lines:

PPU_CXXFLAGS	+=	-DGEKKO -DPS3_SDK_3_41 -DPSGL -DPATH_MAX=1024
PPU_CFLAGS		+=	-DGEKKO -DPS3_SDK_3_41 -DPSGL -DPATH_MAX=1024		

So it becomes like this:

PPU_CXXFLAGS	+=	-DGEKKO -DPSGL -DPATH_MAX=1024
PPU_CFLAGS		+=	-DGEKKO -DPSGL -DPATH_MAX=1024		

3 - Do 'make clean && make && make pkg'

7.2.2 FOR SDK 3.41
=====================

1 - Do 'make clean && make && make pkg'

7.2.3 TO MAKE A DEBUGGING-FRIENDLY COMPILE
=============================================
We provide a netlogger in case you want to have some way of debugging SNES9x PS3 remotely.

1 - Edit the Makefile and comment out the following lines below the comment 'debugging':

#PPU_CXXFLAGS 	+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.101\" -DPS3_DEBUG_PORT=9002
#PPU_CFLAGS	+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.101\" -DPS3_DEBUG_PORT=9002

The IP address needs to be changed to the IP address of the host machine that will be running netcat.

2 - Do 'make clean && make compile'.

3 - Install VBA PS3 on your PS3, start up netcat on your PC with the following command:

netcat -l -p 9002

4 - Start up VBA PS3. If all went well, you will see debugging messages on your PC with the netcat application.

7.3 CELLFRAMEWORK
===================
VBA PS3 is an implementation of 'Cellframework', which is written by the same developers as a rudimentary framework
for PS3 app development. It provides more-or-less complete classes for graphics, input, audio, and network.

Cellframework is a subrepository of VBA PS3. To update the subrepository, from the commandline, go to the folder and type 'hg up'.

Source code repository for Cellframework is here:

https://code.google.com/p/cellframework/)

7.3.1 GETTING LATEST CELLFRAMEWORK REVISION
=============================================
To be able to check out the latest revision, you must have Mercurial installed on your system. Type in the following:

hg clone https://cellframework.googlecode.com/hg/ cellframework

8. CREDITS
===========
Lantus  -	Optimizations from VBA 360 0.03
Grandy  -	Special Thanks/Testing
Orioto	-	PIC1.PNG (http://orioto.deviantart.com/art/Bird-Chase-Time-147870358)
