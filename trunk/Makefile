#
# VBA MAKEFILE
#
# shinhalsafar@gmail.com
#

# set build tools
CELL_HOST_PATH = $(CELL_SDK_DIR)

CELL_BUILD_TOOLS = GCC

# CELL_GPU_TYPE (currently RSX is only one option)
CELL_GPU_TYPE       = RSX

# CELL_PSGL_VERSION is debug, dpm or opt
CELL_PSGL_VERSION   = opt


# set path to important executables
MKFSELF_NPDRM = $(CELL_HOST_PATH)/bin/make_fself_npdrm
MKFSELF = $(CELL_HOST_PATH)/bin/make_fself
MKPKG_NPDRM = $(CELL_HOST_PATH)/bin/make_package_npdrm
STRIP = $(CELL_HOST_PATH)/ppu/bin/ppu-lv2-strip
COPY = cp
C = $(CELL_SDK_DIR)/ppu/bin/ppu-lv2-gcc
CC = $(CELL_SDK_DIR)/ppu/bin/ppu-lv2-g++


# set path to all relevants directories
SRC_DIR = src
CELL_FRAMEWORK_DIR = src/cellframework
VBA_API_DIR = src/vba
UTIL_DIR = utils
BUILD_DIR = objs


# build list of source directories
VBA_SRC_DIR		:=	$(VBA_API_DIR)/ $(VBA_API_DIR)/gb $(VBA_API_DIR)/gba \
					$(VBA_API_DIR)/apu $(VBA_API_DIR)/common
				
PS3_SRC_DIR	:=	$(CELL_FRAMEWORK_DIR)/graphics $(CELL_FRAMEWORK_DIR)/input $(CELL_FRAMEWORK_DIR)/audio \
				$(CELL_FRAMEWORK_DIR)/threads $(CELL_FRAMEWORK_DIR)/logger $(CELL_FRAMEWORK_DIR)/network \
				$(CELL_FRAMEWORK_DIR)/fileio \
				$(CELL_FRAMEWORK_DIR)/utility \
				$(SRC_DIR)/ $(SRC_DIR)/conf
				
UTIL_SRC_DIR :=	$(UTIL_DIR)/ $(UTIL_DIR)/7z_C $(UTIL_DIR)/zlib $(UTIL_DIR)/unrar $(UTIL_DIR)/fex \
				$(UTIL_DIR)/libpng $(UTIL_DIR)/SFML/src/SFML/Network $(UTIL_DIR)/SFML/src/SFML/Network/Unix

				
# build src lists and object file lists
VBA_CPPSRCS		=	$(foreach dir,$(VBA_SRC_DIR),$(wildcard $(dir)/*.cpp))
VBA_CPPOBJS 	= 	$(VBA_CPPSRCS:.cpp=.ppu.o)
VBA_CSRCS		=	$(foreach dir,$(VBA_SRC_DIR),$(wildcard $(dir)/*.c))
VBA_COBJS 		= 	$(VBA_CSRCS:.c=.ppu.o)

PS3_CPPSRCS		=	$(foreach dir,$(PS3_SRC_DIR),$(wildcard $(dir)/*.cpp))
PS3_CPPOBJS 	= 	$(PS3_CPPSRCS:.cpp=.ppu.o) 
PS3_CSRCS		=	$(foreach dir,$(PS3_SRC_DIR),$(wildcard $(dir)/*.c))
PS3_COBJS 		= 	$(PS3_CSRCS:.c=.ppu.o)

UTIL_CPPSRCS	=	$(foreach dir,$(UTIL_SRC_DIR),$(wildcard $(dir)/*.cpp))
UTIL_CPPOBJS 	= 	$(UTIL_CPPSRCS:.cpp=.ppu.o) 
UTIL_CSRCS		=	$(foreach dir,$(UTIL_SRC_DIR),$(wildcard $(dir)/*.c))
UTIL_COBJS 		= 	$(UTIL_CSRCS:.c=.ppu.o) 

PPU_CPPOBJS		= 	$(VBA_CPPOBJS:%=$(BUILD_DIR)/%) $(PS3_CPPOBJS:%=$(BUILD_DIR)/%) $(UTIL_CPPOBJS:%=$(BUILD_DIR)/%)
PPU_COBJS		= 	$(VBA_COBJS:%=$(BUILD_DIR)/%) $(PS3_COBJS:%=$(BUILD_DIR)/%) $(UTIL_COBJS:%=$(BUILD_DIR)/%)


# Output file
PPU_TARGET		=	vba.ppu.elf


# default flags from sony
PPU_CXXFLAGS	=	-std=c++98 -W -Wall -Wuninitialized -Wcomment -Wchar-subscripts \
					-Wdeprecated-declarations -Wendif-labels -Wformat=2 -Wformat-extra-args \
					-Wimplicit -Wimport -Winline -Wmissing-braces -Wparentheses -Wpointer-arith \
					-Wredundant-decls -Wreturn-type -Wshadow -Wsign-compare -Wswitch \
					-Wunknown-pragmas -Wunused -Wwrite-strings
PPU_CFLAGS		=	-W -Wall -Wuninitialized -Wcomment -Wchar-subscripts \
					-Wdeprecated-declarations -Wendif-labels -Wformat=2 -Wformat-extra-args \
					-Wimplicit -Wimport -Winline -Wmissing-braces -Wparentheses -Wpointer-arith \
					-Wredundant-decls -Wreturn-type -Wshadow -Wsign-compare -Wswitch \
					-Wunknown-pragmas -Wunused -Wwrite-strings					


# FEX - blarggs fex flags
PPU_CXXFLAGS	+=	-DFEX_ENABLE_RAR -DBLARGG_BIG_ENDIAN=1
PPU_CFLAGS		+= 	-DFEX_ENABLE_RAR -DBLARGG_BIG_ENDIAN=1


# PLATFORM
PPU_CXXFLAGS	+=	-D__CELLOS_LV2__
PPU_CFLAGS		+= 	-D__CELLOS_LV2__


# include flags
PPU_CXXFLAGS	+=	-I. -I$(VBA_API_DIR) -I./utils/SFML/include/ -I./utils/ -I./utils/fex \
					-I./utils/libpng -I./utils/zlib -I./utils/unrar
PPU_CFLAGS		+=	-I. -I$(VBA_API_DIR) -I./utils/SFML/include/ -I./utils/ -I./utils/fex \
					-I./utils/libpng -I./utils/zlib -I./utils/unrar

# compiler optimizations
PPU_CXXFLAGS	+=	-O3 \
					-ffast-math \
					-finline-functions \
					-funit-at-a-time \
					--param inline-unit-growth=200 \
					--param large-function-growth=1000 \
					--param max-inline-insns-single=450
					
PPU_CFLAGS		+=	-O3 \
					-ffast-math \
					-finline-functions \
					-funit-at-a-time \
					--param inline-unit-growth=200 \
					--param large-function-growth=1000 \
					--param max-inline-insns-single=450
					
# port must define					
PPU_CXXFLAGS	+=	-DGEKKO -DPSGL -DPATH_MAX=1024
PPU_CFLAGS		+=	-DGEKKO -DPSGL -DPATH_MAX=1024		

# comment these to compile with 1.92 SDK
PPU_CFLAGS     += -DPS3_SDK_3_41
PPU_CXXFLAGS   += -DPS3_SDK_3_41


# VBA optimization flags
PPU_CXXFLAGS	+=	-DFINAL_VERSION -D__ppc__ -DSPEEDHAX -D__POWERPC__ -DC_CORE -DWORDS_BIGENDIAN
PPU_CFLAGS		+=	-DFINAL_VERSION -D__ppc__ -DSPEEDHAX -D__POWERPC__ -DC_CORE -DWORDS_BIGENDIAN


# VBA must define
PPU_CXXFLAGS	+= 	-Dlinux -DHAVE_NETINET_IN_H -DHAVE_ARPA_INET_H
PPU_CFLAGS		+=	-Dlinux -DHAVE_NETINET_IN_H -DHAVE_ARPA_INET_H
PPU_CXXFLAGS	+=	-D__need_timeval
PPU_CFLAGS		+=	-D__need_timeval


# use 7z less mem intensive
PPU_CXXFLAGS	+=	-D_SZ_ONE_DIRECTORY -D_LZMA_IN_CB -D_LZMA_OUT_READ
PPU_CFLAGS		+=	-D_SZ_ONE_DIRECTORY -D_LZMA_IN_CB -D_LZMA_OUT_READ


# DISABLE DEBUG, to enabled add -g
PPU_CXXFLAGS	+=	 
PPU_CFLAGS		+=	 


# enable frameskip
PPU_CXXFLAGS	+=	-DFRAMESKIP 
PPU_CFLAGS		+=	-DFRAMESKIP


# enable debug netlogger
PPU_CXXFLAGS 	+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.101\" -DPS3_DEBUG_PORT=9002
PPU_CFLAGS		+=	-DCELL_DEBUG -DPS3_DEBUG_IP=\"192.168.1.101\" -DPS3_DEBUG_PORT=9002


# enable screen debugger
#PPU_CXXFLAGS	+=	-DEMUDEBUG -DEMU_DBG_DELAY=500000
#PPU_CFLAGS		+=	-DEMUDEBUG -DEMU_DBG_DELAY=500000


# enable telnet control console
#PPU_CSTDFLAGS 	+=	-DCONTROL_CONSOLE -D__PPU__
#PPU_CSTDFLAGS 	+=	-DCONSOLE_USE_NETWORK
#PPU_CXXSTDFLAGS +=	-DCONTROL_CONSOLE -D__PPU__
#PPU_CXXSTDFLAGS +=	-DCONSOLE_USE_NETWORK


# default vba file libs
PPU_CXXVBAFLAGS 	= $(PPU_CXXFLAGS)
PPU_CVBAFLAGS 		= $(PPU_CFLAGS)


# libs, order matters!
PPU_LDLIBS		+= 	-L. -L$(CELL_SDK)/target/ppu/lib/PSGL/RSX/opt \
					-ldbgfont -lPSGL -lPSGLcgc -lcgc -lgcm_cmd -lgcm_sys_stub -lresc_stub \
					-lm -lio_stub -lfs_stub -lsysutil_stub -lcontrol_console_ppu -lsysmodule_stub \
					-lnet_stub -lnetctl_stub -laudio_stub -lpthread


# RULE: ALL
all: $(PPU_TARGET) $(PPU_TARGET:%.elf=%.self)


# RULE: All cpp files, can differentiate for vba dir
$(BUILD_DIR)/%.ppu.o: %.cpp 
	@mkdir -p $(@D)
	$(if $(findstring $@,$(VBA_CPPOBJS:%=$(BUILD_DIR)/%)),\
		$(CC) $(PPU_CXXVBAFLAGS) -o $@ -c $<,\
		$(CC) $(PPU_CXXFLAGS) -o $@ -c $<)


# RULE: All c files, can differentiate for vba dir		
$(BUILD_DIR)/%.ppu.o: %.c 
	@mkdir -p $(@D)
	$(if $(findstring $@,$(VBA_COBJS:%=$(BUILD_DIR)/%)),\
		ppu-lv2-gcc $(PPU_CVBAFLAGS) -o $@ -c $<,\
		ppu-lv2-gcc $(PPU_CFLAGS) -o $@ -c $<)		
	
	
# RULE: build the elf file	
$(PPU_TARGET): $(PPU_CPPOBJS) $(PPU_COBJS)
	$(CC) $(PPU_CXXFLAGS) $(PPU_LDLIBS) -o $(PPU_TARGET) $(PPU_CPPOBJS) $(PPU_COBJS)
	
	
# RULE: build the self file
$(PPU_TARGET:%.elf=%.self): $(PPU_TARGET)
	$(MKFSELF) $(PPU_TARGET) $(PPU_TARGET:%.elf=%.self)

	
# RULE: generates the compiler and linker flags for profile-arcs
genProfile: set_genProfile_flags all

set_genProfile_flags: $(BUILD_DIR)/$(CELL_FRAMEWORK_DIR)/network-stdio/net_stdio.ppu.o
	touch ./src/VbaPs3.cpp		
	$(eval PPU_CXXFLAGS +=	-DPS3_PROFILING -DPS3_PROFILING_IP=\"192.168.1.101\" -DPS3_PROFILING_PORT=9001)
	$(eval PPU_CFLAGS 	+=	-DPS3_PROFILING -DPS3_PROFILING_IP=\"192.168.1.101\" -DPS3_PROFILING_PORT=9001)
	$(eval PPU_CXXVBAFLAGS = $(PPU_CXXFLAGS) -fprofile-generate -fprofile-arcs)
	$(eval PPU_CVBAFLAGS = $(PPU_CFLAGS) -fprofile-generate -fprofile-arcs)
	$(eval PPU_LDLIBS += -lgcov)
	$(eval PPU_LDLIBS += -Wl,-wrap,fopen -Wl,-wrap,fclose -Wl,-wrap,fwrite -Wl,-wrap,fread -Wl,-wrap,fseek -Wl,-wrap,ftell -Wl,-wrap,fflush -Wl,-wrap,rewind -Wl,-wrap,fgetpos -Wl,-wrap,fsetpos -Wl,-wrap,setbuf -Wl,-wrap,setvbuf -Wl,-wrap,ungetc -Wl,-wrap,feof)
	
	
# RULE: use a generated profile (gcda files)
useProfile: set_useProfile_flags all

set_useProfile_flags: 
	touch ./src/VbaPs3.cpp
	$(eval PPU_CXXVBAFLAGS = $(PPU_CXXFLAGS) -fprofile-use)
	$(eval PPU_CVBAFLAGS = $(PPU_CFLAGS) -fprofile-use)
	$(eval PPU_LDLIBS += -fprofile-use)
	

# RULE: build the package
pkg: $(PPU_TARGET)
	$(STRIP) $(PPU_TARGET) 
	$(MKFSELF_NPDRM) $(PPU_TARGET) pkg/USRDIR/EBOOT.BIN
	$(COPY) -r ./src/cellframework/extra/shaders pkg/USRDIR/
	$(MKPKG_NPDRM) pkg/package.conf pkg


# RULE: testing...
echo:
	echo $(PPU_CPPOBJS)
	echo $(BUILD_DIR)/$(CELL_FRAMEWORK_DIR)/network-stdio/net_stdio.ppu.o
	
	
# RULE - force, for empty dependency
force: ;


# RULE - clean vba or ps3 or utils or all	
clean_vba:
	@rm $(VBA_CPPOBJS:%=$(BUILD_DIR)/%) -f
	@rm $(VBA_CPPOBJS:%.o=$(BUILD_DIR)/%.d) -f
	@rm $(VBA_COBJS:%=$(BUILD_DIR)/%) -f
	@rm $(VBA_COBJS:%.o=$(BUILD_DIR)/%.d) -f
	
clean_ps3:
	@rm $(PS3_CPPOBJS:%=$(BUILD_DIR)/%) -f
	@rm $(PS3_CPPOBJS:%.o=$(BUILD_DIR)/%.d) -f
	@rm $(PS3_COBJS:%=$(BUILD_DIR)/%) -f
	@rm $(PS3_COBJS:%.o=$(BUILD_DIR)/%.d) -f
	@rm $(BUILD_DIR)/$(CELL_FRAMEWORK_DIR)/network-stdio/net_stdio.ppu.o -f
	
clean_util:
	@rm $(UTIL_CPPOBJS:%=$(BUILD_DIR)/%) -f
	@rm $(UTIL_CPPOBJS:%.o=$(BUILD_DIR)/%.d) -f
	@rm $(UTIL_COBJS:%=$(BUILD_DIR)/%) -f
	@rm $(UTIL_COBJS:%.o=$(BUILD_DIR)/%.d) -f

clean: clean_vba clean_ps3 clean_util
	@rm $(PPU_TARGET) $(PPU_TARGET:%.elf=%.self) -f
