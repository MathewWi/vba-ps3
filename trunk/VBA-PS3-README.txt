================================================================================
			Visual Boy Advance PS3
			Version 1.0
================================================================================

This is an application for the PlayStation3 that makes it possible to play
Game Boy Classic / Game Boy Color / Game Boy Advance games on your jailbroken
PlayStation3. It is based on an up-to-date version of the popular PC emulator,
VBA-M.

================================================================================
			Table of Contents
================================================================================
0. Requirements........................................................... [A00]
1. Currently Implemented VBA-M functions.................................. [A01]
2. Controls............................................................... [B01]
     2.1.1 - Controls in ROM menu......................................... [B02]
     2.1.2 - Controls in Settings menu.................................... [B03]
     2.1.3 - Controls in-game............................................. [B04]
     2.1.4 - Controls in-game (Miscellaneous)............................. [B05]
3. Settings............................................................... [C01]
     3.1 - General Settings............................................... [C02]
     3.2 - VBA Settings................................................... [C03]
     3.3 - Path Settings.................................................. [C04]
     3.4 - Controls Settings.............................................. [C05]
4. Currently known issues (as of 1.0)..................................... [D01]
     4.2 - Sonic Advance 1/2 don't work................................... [D02]
     4.3 - Slow ZIP loading............................................... [D03]
5. Notes.................................................................. [E01]
     5.1 - General notes.................................................. [E02]
     5.2 - Performance notes.............................................. [E03]
     5.3 - Performance w/ pixel shaders................................... [E04]
6. Planned improvements................................................... [F01]
7. For Developers......................................................... [G01]
     7.1 - Source Code Repository......................................... [G02]
          7.1.1 - Getting the latest VBA-PS3 revision..................... [G03]
     7.2  - Compilation Instructions...................................... [G04]
	  7.2.3 - To make a debugging-friendly compile.................... [G05]
     7.3 - CellFramework.................................................. [G06]
          7.3.1 - Getting the latest Cellframework revision............... [G07]
8. Credits................................................................ [H01]
     
================================================================================
[A00]			0. Requirements
================================================================================
To play this on your PlayStation3 system, you have one of two options:

a) Jailbreak your PS3 using a USB exploit - this works up to firmware 3.41. 

or, 

b) You must update your firmware with a custom firmware. (required if you 
have firmware 3.55)

You must install the version of VBA PS3 that corresponds to the firmware
you're running. Go to 'Game' in the XMB, go to 'Install Packages', and select
the package you want to install.

================================================================================
			PS3 has firmware 3.41 installed (w/ jailbreak dongle)
================================================================================
Install vba-ps3-v1.0-fw3.41.pkg'.

================================================================================
			PS3 has firmware Geohot 3.55 CFW installed 
================================================================================
Install 'vba-ps3-v1.0-geohot-cfw3.55.pkg'.


================================================================================
[A01]			1. CURRENTLY IMPLEMENTED VBA-M FUNCTIONS
================================================================================
* Saving/loading of SRAM
* Savestate loading/saving support
  - Savestate slot selectable in-game
  - Up to 10 saveslots

================================================================================
[B01]			2. CONTROLS
================================================================================
================================================================================
[B02]			2.1.1 CONTROLS IN ROM MENU
================================================================================
Up			- Go up
Down			- Go down
Left			- Go back five file entries
Right			- Go forward five file entries
L1			- Go back one page
R1			- Go forward one page

Cross			- (If directory selected) enter directory/
			  (if ROM selected) start ROM
Triangle		- (If ROM selected) start ROM with multitap support
Circle			- (If not in root directory) Go back to previous directory
L2 + R2			- (If you previously exited a ROM) return to game
Select			- Go to settings menu
			  (see 'CONTROLS IN SETTINGS MENU' section)

================================================================================
[B03]			2.1.2 CONTROLS IN SETTINGS MENU
================================================================================
Up			- Go up one setting.
Down			- Go down one setting.
Left			- Change setting to the left.
Right			- Change setting to the right.

Circle			- Go back to ROM menu/Go back to previous Settings screen
Start			- Reset the setting back to the default value.
R1			- Go to the next Settings screen
L1			- Go to the previous Settings screen
L3 + R3			- Return back to game (if a ROM is loaded)

================================================================================
[B04]			2.1.3 CONTROLS IN-GAME
================================================================================
================================================================================
[B05]			2.1.7 CONTROLS IN-GAME - MISCELLANEOUS
================================================================================

R3 + L3				- Press these two buttons together while
			  	in-game to go back to the ROM browser menu.

R3 + R2				- Save to currently selected save state slot
R3 + L2				- Load from currently selected save state slot

Right analog stick - Left	- Move current savestate slot one slot backwards
Right analog stick - Right	- Move current savestate slot one slot forward

To play a game with a USB controller as Player 1, start up your PS3 and rather 
than using the Sixaxis/DualShock3, plug in an USB port before connecting the
controller to the PS3 - your USB pad should then become Controller 1.

================================================================================
[C01]			3. SETTINGS
================================================================================

================================================================================
[C02]			3.1 GENERAL SETTINGS
================================================================================
Display framerate		- This will show the FPS (Frames Per Second) onscreen
================================================================================
			Current save state slot
================================================================================
Set the save state slot - this way, you can save multiple states and switch 
inbetween save states.

================================================================================
			Resolution
================================================================================
Switch between resolutions (depending on your TV/monitor's supported 
resolutions)

================================================================================
			Selected Shader
================================================================================
Select a shader - the default shader is '/stock.cg'. Choose between 2XSaI, 
Bloom shader, curved CRT shader, HQ2x, Super2xSaI, SuperEagle, and more.

================================================================================
			Font Size		
================================================================================
The font size in menus. Set this to a value higher than 1.000 to enlarge the
font size, or set it to a value lower than 1.000 to shrink the font size.

================================================================================
			Aspect Ratio			
================================================================================
Switch between aspect ratios 'Scaled' (recommended for 4:3 TVs/monitors) and
'Stretched' (recommended for 16:9 TVs/monitors).

================================================================================
			Hardware Filtering
================================================================================
Switch between Linear interpolation (Bilinear filtering) (DEFAULT) and 
Point filtering.

Linear interpolation - A hardware bilinear filter is applied to the image.

Point filtering	     - No filters are applied. Most shaders look much better on
this setting.

================================================================================
			Overscan
================================================================================
Set this to a certain value so that no part of the screen is cutoff on your 
TV/monitor.

Which value to set varies depending on your monitor/TV.

================================================================================
			Sound
================================================================================
Switch between Normal mode and RSound mode.

Normal Mode - The normal audio output will be used by the PS3. (DEFAULT)

Rsound Mode - RSound basically lets you redirect the audio from the PS3 over the
network to a PC - so, using this, the audio from VBA PS3 can be outputted on a 
PC/laptop/netbook/HTPC's speakers/audio installation instead of going through 
the television speakers or the audio receiver connected to the television.

================================================================================
			Rsound Server IP Address
================================================================================
Set the IP address for the RSound server. This Will pop up an onscreen keyboard 
where you must input a valid IP address that points to the server that will be 
running an RSound server application.

================================================================================
			DEFAULT
================================================================================
Set all of the general settings back to their default values

================================================================================
[C03]			VBA SETTINGS
================================================================================

================================================================================
			Display Framerate
================================================================================
ON  - This will show an FPS (Frames Per Second) counter onscreen.

OFF - FPS counter is disabled. (DEFAULT)

================================================================================
			USE GBA BIOS
================================================================================
Select a GBA BIOS from the filesystem - upon pressing Cross, you can select 
a GBA BIOS. Note that Game Boy Classic ROMs will not work when a GBA BIOS has 
been selected. You can disable the GBA BIOS by pressing 'Start' on this setting.

================================================================================
			CONTROL STYLE
================================================================================

Original	-	The Game Boy face buttons are mapped as follows:
			B button - mapped to CROSS button
			A button - mapped to CIRCLE button

Better		-	The Game Boy face buttons are mapped as follows:
			B button - mapped to SQUARE button
			A button - mapped to CROSS button

================================================================================
			Default
================================================================================
Set all of the VBA settings back to their default values

================================================================================
[C04]			3.3 PATH SETTINGS
================================================================================

================================================================================
			Startup ROM Directory
================================================================================
Set the default ROM startup directory. You will have to restart the emulator
after changing the path for this change to have any effect.

DEFAULT - is set to the root of the PS3's filesystem. (/)

================================================================================
			Savestate Directory
================================================================================
Set the default savestate directory where your savestates will be saved and
loaded from.

DEFAULT - is set to the USRDIR directory of VBA PS3. 
(/dev_hdd0/game/VBAM90000/USRDIR)

================================================================================
			SRAM Directory
================================================================================
Set the default SRAM (SaveRAM) directory where all your SRAM files will be 
saved and loaded from.

DEFAULT - is set to the USRDIR directory of VBA PS3. 
(/dev_hdd0/game/VBAM90000/USRDIR)

================================================================================
			Cheatfile Directory
================================================================================
Set the default cheatfile directory - all your cheatfiles will be saved and 
loaded from here.

DEFAULT - is set to the USRDIR directory of VBA PS3. 
(/dev_hdd0/game/VBAM90000/USRDIR)

================================================================================
			Default
================================================================================
Set all of the path settings back to their default values

================================================================================
[C05]			3.3 CONTROLS SETTINGS
================================================================================
Most buttons can be reconfigured here.

================================================================================
[D01]			4. CURRENTLY KNOWN ISSUES (AS OF BUILD 1.0)
================================================================================

================================================================================
[D02]			4.1 SONIC ADVANCE 1/2 DON'T WORK
================================================================================
Sonic Advance 1/2 currently don't work with VBA. This is a known issue and 
will be looked into.

================================================================================
[D03]			4.1 SLOW ZIP LOADING
================================================================================
You may experience very slow ZIP loading. If this is the case, 
it's advised you unzip your ROMs so they will load instantly.

================================================================================
[E01]			5. NOTES
================================================================================

================================================================================
[E02]			5.1 GENERAL NOTES
================================================================================
For people running this on HDTVs complaining about input lag:

Turn off all post-processing filters you may have running - on Sony Bravia 
HDTVs, display Motion Flow (this also causes input lags with most games in 
general, not just this application. If your HDTV has a 'Game' mode or something 
of the sort, select that as well.

================================================================================
[E03]			5.2 PERFORMANCE NOTES
================================================================================
This emulator has been profiled/optimized to run most games at full-speed 
(at any resolution - 480p/720p/1080p).

Some of the games that are guaranteed to run at fullspeed (without 
frameskipping) are games like:

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

As ever, your mileage may vary. Please notify us of games that won't 
run optimally. 

================================================================================
[E04]			5.3 PERFORMANCE NOTES - SHADERS
================================================================================
Here is the performance of the various fragment/pixel shaders:

All shaders will run at fullspeed up until 1280x1080.

================================================================================
SHADER		1440x1080		1600x1080		1920x1080 1080p
================================================================================
Stock		60fps			60fps			60fps 
2xSaI		60fps			60fps			60fps
Bloom		60fps (*)		not fullspeed (**)	not fullspeed
Blur		60fps			60fps			60fps
CRT		not fullspeed (**)	not fullspeed (**)	not fullspeed
HQ2x		60fps			60fps			60fps
Lanzcos12	60fps			60fps			60fps
Lanzcos16	60fps			60fps			60fps
McGreen		60fps			60fps			60fps
Quad_Interp	60fps			60fps			60fps
Scale2xPlus	60fps			60fps			60fps
Scanlines	60fps			60fps			60fps
Sharpen		60fps			60fps			60fps
Super2xSaI	60fps			not fullspeed		not fullspeed
SuperEagle	60fps			60fps			not fullspeed

*  - Might fluctuate with certain chip games (such as SuperFX)
** - Is fullspeed at 4:3 but not at 16:9

NOTE: While the general rule of thumb is that shaders are relatively cost-
less compared to CPU image enhancing filters, do be aware that some 
taxing shaders (such as Lanzcos16) can bring performance down on certain 
demanding games like Super Monkey Ball Deluxe. Turn Lanzcos16 off
and try 2xSaI to get a nice speedup.

================================================================================
[F01]			6. PLANNED IMPROVEMENTS
================================================================================
* Get Sonic Advance 1/2 to work
* 7z archive support

================================================================================
[G01]			7. FOR DEVELOPERS
================================================================================

================================================================================
[G02]			9.1 SOURCE CODE REPOSITORY
================================================================================
Link : https://code.google.com/p/vba-ps3/

================================================================================
[G03]			9.1.1 GETTING THE LATEST VBA-PS3 REVISION
================================================================================
To be able to check out the latest revision, you must have Mercurial installed 
on your system.

To check out the latest revision in the trunk, type in the following from the
command-line:

hg clone https://vba-ps3.googlecode.com/hg/ vba-ps3

================================================================================
[G04]			7.2 COMPILATION INSTRUCTIONS
================================================================================
VBA PS3 can be compiled with GCC (w/ PS3 SDK).

================================================================================
[G05]			7.2.3 TO MAKE A DEBUGGING-FRIENDLY COMPILE
================================================================================
We provide a netlogger in case you want to have a way of debugging remotely.

1 - Edit the Makefile and comment out the following lines below the comment 
'debugging':

#PPU_CXXFLAGS 	+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.7\" \
#-DPS3_DEBUG_PORT=9002
#PPU_CFLAGS	+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.7\" \
#-DPS3_DEBUG_PORT=9002

The IP address needs to be changed to the IP address of the host machine that 
will be running netcat.

2 - Do 'make clean && make compile'.

3 - Install VBA PS3 on your PS3, start up netcat on your PC with the 
following command:

netcat -l -p 9002

4 - Start up VBA PS3. If all went well, you will see debugging messages on 
your PC with the netcat application.

================================================================================
[G06]			7.3 CELLFRAMEWORK
================================================================================
Link : https://code.google.com/p/cellframework/)

VBA PS3 is an implementation of 'Cellframework', which is written by the same 
developers as a rudimentary framework for PS3 app development. It provides 
more-or-less complete classes for graphics, input, audio, and network.

Cellframework is a subrepository of VBA PS3. To update the subrepository, from 
the commandline, go to the folder and type 'hg up'.

================================================================================
[G07]			7.3.1 GETTING LATEST CELLFRAMEWORK REVISION
================================================================================
To be able to check out the latest revision, you must have Mercurial 
installed on your system. Type in the following:

hg clone https://cellframework.googlecode.com/hg/ cellframework

================================================================================
[H01]			8. CREDITS
================================================================================
Lantus  -	Optimizations from VBA 360 0.03
Grandy  -	Special Thanks/Testing
Orioto	-	PIC1.PNG (http://orioto.deviantart.com/art/Bird-Chase-Time-147870358)
