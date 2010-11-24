#
# VBA MAKEFILE
#
# shinhalsafar@gmail.com
#

# specify build tools
CELL_BUILD_TOOLS = GCC

# explicitly set some cell sdk defaults
CELL_SDK ?= /usr/local/cell

# CELL_GPU_TYPE (currently RSX is only one option)
CELL_GPU_TYPE       = RSX

# CELL_PSGL_VERSION is debug, dpm or opt
CELL_PSGL_VERSION   = opt

# CELL_DISPLAY_MODE (default: 720p)
CELL_DISPLAY_MODE   ?= 720p

CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

# set path to important executables
MKFSELF_NPDRM = $(CELL_HOST_PATH)/bin/make_fself_npdrm
MKPKG_NPDRM = $(CELL_HOST_PATH)/bin/make_package_npdrm
STRIP = $(CELL_HOST_PATH)/ppu/bin/ppu-lv2-strip
COPY = cp

# set path to all relevants directories
SRC_DIR = ./src
CELL_FRAMEWORK_DIR = ./src/cellframework
VBA_API_DIR = ./src/vba
UTIL_DIR = ./utils

# build list of source directories
SOURCES		:=	$(VBA_API_DIR)/ $(VBA_API_DIR)/gb $(VBA_API_DIR)/gba \
				$(VBA_API_DIR)/apu $(VBA_API_DIR)/common  \
				$(CELL_FRAMEWORK_DIR)/graphics $(CELL_FRAMEWORK_DIR)/input $(CELL_FRAMEWORK_DIR)/audio \
				$(CELL_FRAMEWORK_DIR)/threads $(CELL_FRAMEWORK_DIR)/logger $(CELL_FRAMEWORK_DIR)/network \
				$(CELL_FRAMEWORK_DIR)/fileio \
				$(CELL_FRAMEWORK_DIR)/utility \
				$(SRC_DIR)/ $(SRC_DIR)/conf \
				$(UTIL_DIR)/ $(UTIL_DIR)/7z_C $(UTIL_DIR)/zlib $(UTIL_DIR)/fex \
				$(UTIL_DIR)/libpng $(UTIL_DIR)/SFML/src/SFML/Network $(UTIL_DIR)/SFML/src/SFML/Network/Unix
				
# build srcs
PPU_SRCS		=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c)) $(VPSHADER_PPU_OBJS) $(FPSHADER_PPU_OBJS)

PPU_TARGET		=	vba.ppu.elf

# PLATFORM
PPU_CXXFLAGS	+=	-D__CELLOS_LV2__
PPU_CFLAGS		+= 	-D__CELLOS_LV2__

# include flags
PPU_CXXFLAGS	+=	-I. -I$(VBA_API_DIR) -I./utils/SFML/include/ -I./utils/ -I./utils/fex \
					-I./utils/libpng -I./utils/zlib 
PPU_CFLAGS		+=	-I. -I$(VBA_API_DIR) -I./utils/SFML/include/ -I./utils/ -I./utils/fex \
					-I./utils/libpng -I./utils/zlib 

# compiler optimizations
PPU_CXXFLAGS	+=	-ffast-math \
					-funroll-loops \
					-finline-functions \
					-funit-at-a-time \
					-finline-functions-called-once \
					-fprefetch-loop-arrays \
					-misel \
					--param inline-unit-growth=200 \
					--param large-function-growth=1000 \
					--param max-inline-insns-single=1000
					
PPU_CFLAGS		+=	-ffast-math \
					-funroll-loops \
					-finline-functions \
					-funit-at-a-time \
					-finline-functions-called-once \
					-fprefetch-loop-arrays \
					-misel \
					--param inline-unit-growth=200 \
					--param large-function-growth=1000 \
					--param max-inline-insns-single=1000
					
# port must define					
PPU_CXXFLAGS	+=	-DGEKKO -DPS3_SDK_3_41 -DPSGL -DPATH_MAX=1024
PPU_CFLAGS		+=	-DGEKKO -DPS3_SDK_3_41 -DPSGL -DPATH_MAX=1024		

# VBA optimization flags
PPU_CXXFLAGS	+=	-DFINAL_VERSION -D__ppc__ -DSPEEDHAX
PPU_CFLAGS		+=	-DFINAL_VERSION -D__ppc__ -DSPEEDHAX

# VBA must define
PPU_CXXFLAGS	+= 	-DC_CORE -DWORDS_BIGENDIAN -Dlinux -DHAVE_NETINET_IN_H -DHAVE_ARPA_INET_H
PPU_CFLAGS		+=	-DC_CORE -DWORDS_BIGENDIAN -Dlinux -DHAVE_NETINET_IN_H -DHAVE_ARPA_INET_H
PPU_CXXFLAGS	+=	-D__need_timeval
PPU_CFLAGS		+=	-D__need_timeval

# use 7z less mem intensive
PPU_CXXFLAGS	+=	-D_SZ_ONE_DIRECTORY -D_LZMA_IN_CB -D_LZMA_OUT_READ
PPU_CFLAGS		+=	-D_SZ_ONE_DIRECTORY -D_LZMA_IN_CB -D_LZMA_OUT_READ

# enable FCEU frameskip
PPU_CXXFLAGS	+=	-DFRAMESKIP 
PPU_CFLAGS		+=	-DFRAMESKIP

# enable debug netlogger
PPU_CXXFLAGS 	+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.201\" -DPS3_DEBUG_PORT=9001
PPU_CFLAGS		+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.201\" -DPS3_DEBUG_PORT=9001

# enable screen debugger
#PPU_CXXFLAGS	+=	-DEMUDEBUG -DEMU_DBG_DELAY=500000
#PPU_CFLAGS		+=	-DEMUDEBUG -DEMU_DBG_DELAY=500000

# enable telnet control console
#PPU_CSTDFLAGS 	+=	-DCONTROL_CONSOLE -D__PPU__
#PPU_CSTDFLAGS 	+=	-DCONSOLE_USE_NETWORK
#PPU_CXXSTDFLAGS +=	-DCONTROL_CONSOLE -D__PPU__
#PPU_CXXSTDFLAGS +=	-DCONSOLE_USE_NETWORK

# libs, order matters!
PPU_LDLIBS		+= 	-L. -L$(CELL_SDK)/target/ppu/lib/PSGL/RSX/opt \
					-ldbgfont -lPSGL -lcgc -lPSGLcgc -lgcm_cmd -lgcm_sys_stub -lresc_stub \
					-lm -lio_stub -lfs_stub -lsysutil_stub -lcontrol_console_ppu -lsysmodule_stub \
					-lnet_stub -lnetctl_stub -laudio_stub -lpthread


include $(CELL_MK_DIR)/sdk.target.mk



.PHONY: pkg
pkg: $(PPU_TARGET)
	$(STRIP) $(PPU_TARGET) 
	$(MKFSELF_NPDRM) $(PPU_TARGET) pkg/USRDIR/EBOOT.BIN
	$(COPY) -r ./src/cellframework/extra/shaders pkg/USRDIR/
	$(MKPKG_NPDRM) pkg/package.conf pkg

	
set_genProfile_flags: 
	$(eval PPU_CXXFLAGS += -fprofile-generate -fprofile-arcs) 
	$(eval PPU_CFLAGS += -fprofile-generate -fprofile-arcs)
	$(eval PPU_LDLIBS += -lgcov)
	
	
.PHONY: genProfile
genProfile: set_genProfile_flags all


set_useProfile_flags: 
	$(eval PPU_CXXFLAGS += -fprofile-use) 
	$(eval PPU_CFLAGS += -fprofile-use)
	$(eval PPU_LDLIBS += -fprofile-use)

.PHONY: useProfile
useProfile: set_useProfile_flags all
