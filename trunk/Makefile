CELL_SDK ?= /usr/local/cell
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

MKFSELF_NPDRM = $(CELL_HOST_PATH)/bin/make_fself_npdrm
MKPKG_NPDRM = $(CELL_HOST_PATH)/bin/make_package_npdrm

STRIP = $(CELL_HOST_PATH)/ppu/bin/ppu-lv2-strip

SRC_DIR = ./src
CELL_FRAMEWORK_DIR = ./src/cellframework
VBA_API_DIR = ./src/vba
UTIL_DIR = ./utils


SOURCES		:=	$(VBA_API_DIR)/ $(VBA_API_DIR)/apu $(VBA_API_DIR)/common  \
				$(VBA_API_DIR)/gb $(VBA_API_DIR)/gba $(VBA_API_DIR)/filters \
				$(UTIL_DIR)/ $(UTIL_DIR)/7z_C $(UTIL_DIR)/zlib $(UTIL_DIR)/fex \
				$(UTIL_DIR)/libpng $(UTIL_DIR)/SFML/src/SFML/Network $(UTIL_DIR)/SFML/src/SFML/Network/Unix \
				$(CELL_FRAMEWORK_DIR)/graphics $(CELL_FRAMEWORK_DIR)/input $(CELL_FRAMEWORK_DIR)/audio \
				$(CELL_FRAMEWORK_DIR)/threads $(CELL_FRAMEWORK_DIR)/logger $(CELL_FRAMEWORK_DIR)/network \
				$(CELL_FRAMEWORK_DIR)/fileio \
				$(SRC_DIR)/ 
				
# not compiling yet
#   

PPU_SRCS		=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c)) $(VPSHADER_PPU_OBJS) $(FPSHADER_PPU_OBJS)
PPU_TARGET		=	vba.ppu.elf

PPU_CXXFLAGS	+=	-I. -I$(VBA_API_DIR) -I./utils/SFML/include/ -I./utils/fex -I./utils/libpng -I./utils/zlib -DPSS_STYLE=1 -DGEKKO -DPS3_SDK_3_41 -DPSGL -DPATH_MAX=1024 
PPU_CFLAGS		+=	-I. -I$(VBA_API_DIR) -I./utils/SFML/include/ -I./utils/fex -I./utils/libpng -I./utils/zlib -DPSS_STYLE=1 -DGEKKO -DPS3_SDK_3_41 -DPSGL -DPATH_MAX=1024

PPU_CXXFLAGS	+=	-D__need_timeval
PPU_CFLAGS		+=	-D__need_timeval

# use 7z less mem intensive
PPU_CXXFLAGS	+=	-D_SZ_ONE_DIRECTORY -D_LZMA_IN_CB -D_LZMA_OUT_READ
PPU_CFLAGS		+=	-D_SZ_ONE_DIRECTORY -D_LZMA_IN_CB -D_LZMA_OUT_READ

# enable FCEU frameskip
PPU_CXXFLAGS	+=	-DFRAMESKIP -DC_CORE -DWORDS_BIGENDIAN -Dlinux -DHAVE_NETINET_IN_H -DHAVE_ARPA_INET_H
PPU_CFLAGS		+=	-DFRAMESKIP -DC_CORE -DWORDS_BIGENDIAN -Dlinux -DHAVE_NETINET_IN_H -DHAVE_ARPA_INET_H

# enable FCEU debugging
#PPU_CXXFLAGS	+=	-DEMUDEBUG -DEMU_DBG_DELAY=500000
#PPU_CFLAGS		+=	-DEMUDEBUG -DEMU_DBG_DELAY=500000

# enable telnet control console
#PPU_CSTDFLAGS 	+=	-D__PPU__
#PPU_CSTDFLAGS 	+=	-DCONSOLE_USE_NETWORK
#PPU_CXXSTDFLAGS +=	-D__PPU__
#PPU_CXXSTDFLAGS +=	-DCONSOLE_USE_NETWORK

#PPU_LIBS		+=	$(CELL_TARGET_PATH)/ppu/lib/libgcm_cmd.a \
					$(CELL_TARGET_PATH)/ppu/lib/libgcm_sys_stub.a $(CELL_TARGET_PATH)/ppu/lib/libfs_stub.a $(CELL_TARGET_PATH)/ppu/lib/libl10n_stub.a
#PPU_LDLIBS		+= -lio_stub -lm -lfs_stub -lsysutil_stub -lsysmodule_stub -lpthread -laudio_stub
#PPU_LDLIBS		+= -lcontrol_console_ppu -lsysmodule_stub
#PPU_LDLIBS		+= -L$(CELL_SDK)/target/ppu/lib/PSGL/RSX/opt -lPSGL -lresc_stub -ldbgfont
#PPU_LDLIBS		+=	-lnet_stub -lnetctl_stub
PPU_LDLIBS		+= 	-L. -L$(CELL_SDK)/target/ppu/lib/PSGL/RSX/opt -ldbgfont -lPSGL -lgcm_cmd -lgcm_sys_stub -lresc_stub -lm -lio_stub -lfs_stub -lsysutil_stub -lcontrol_console_ppu -lsysmodule_stub  -lnet_stub -lnetctl_stub -laudio_stub -lpthread


ifeq ($(CONSOLE_USE_NETWORK),1)
PPU_CSTDFLAGS +=	-DCONSOLE_USE_NETWORK
PPU_CXXSTDFLAGS +=	-DCONSOLE_USE_NETWORK
PPU_LDLIBS+=	-lnet_stub -lnetctl_stub
endif



include $(CELL_MK_DIR)/sdk.target.mk



.PHONY: pkg
pkg: $(PPU_TARGET)
	$(STRIP) $(PPU_TARGET) 
	$(MKFSELF_NPDRM) $(PPU_TARGET) pkg/USRDIR/EBOOT.BIN
	$(MKPKG_NPDRM) pkg/package.conf pkg
