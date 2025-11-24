.SUFFIXES:

PWD= $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
.DEFAULT_GOAL := all

#---------------------------------------------------------------------------------
# Environment Setup
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPRO")
endif

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/3ds_rules

# ip address of 3ds for hblauncher/fbi target.
IP3DS := 192.168.6.26

#---------------------------------------------------------------------------------
# Directory Setup
#---------------------------------------------------------------------------------
BUILD := build
OUTPUT := output
SOURCES := source
DATA := data
INCLUDES := $(SOURCES) include
ROMFS := romfs
RESOURCES := resources

#---------------------------------------------------------------------------------
# Resource Setup
#---------------------------------------------------------------------------------
APP_INFO := $(RESOURCES)/AppInfo
BANNER_AUDIO := $(RESOURCES)/audio
BANNER_IMAGE := $(RESOURCES)/banner
ICON := $(RESOURCES)/icon.png
RSF := $(TOPDIR)/$(RESOURCES)/template.rsf

#---------------------------------------------------------------------------------
# Build Setup (code generation)
#---------------------------------------------------------------------------------
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard

COMMON_FLAGS := -g -Wall -Wno-strict-aliasing -O3 -mword-relocations -fomit-frame-pointer \
	-ffast-math $(ARCH) $(INCLUDE) -D__3DS__ $(BUILD_FLAGS)
CFLAGS := $(COMMON_FLAGS) -std=gnu99
CXXFLAGS := $(COMMON_FLAGS) -std=gnu++17
# CXXFLAGS += -fno-rtti -fno-exceptions

ASFLAGS := -g $(ARCH)
LDFLAGS = -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS := -lcitro2d -lcitro3d -lctru -lm
LIBDIRS := $(PORTLIBS) $(CTRULIB) ./lib

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Version File
#---------------------------------------------------------------------------------

include resources/AppInfo

VERSION_H = $(SOURCES)/version.h

$(VERSION_H): resources/AppInfo
	echo "#pragma once" > $(VERSION_H)
	echo "#define APP_TITLE \"$(APP_TITLE)\"" >> $(VERSION_H)
	echo "#define APP_AUTHOR \"$(APP_AUTHOR)\"" >> $(VERSION_H)
	echo "#define APP_VERSION \"$(APP_VER_MAJOR).$(APP_VER_MINOR).$(APP_VER_MICRO)\"" >> $(VERSION_H)

#---------------------------------------------------------------------------------
# Build Variable Setup
#---------------------------------------------------------------------------------
recurse = $(shell find $2 -type $1 -name '$3' 2> /dev/null)

# Locations of your grammar files
PARSER_DIR := source/lang/parser
LEXER_FILE := $(PARSER_DIR)/lexer.l
PARSER_FILE := $(PARSER_DIR)/parser.y

# Outputs
PARSER_CPP_3DS := $(PARSER_DIR)/parser_3ds.cpp
LEXER_CPP_3DS  := $(PARSER_DIR)/lex_3ds.cpp

# Host build outputs (host-only so we put them in build/)
PARSER_CPP_HOST := $(PARSER_DIR)/parser.cpp
LEXER_CPP_HOST  := $(PARSER_DIR)/lex.cpp

LEXER_HPP  := $(PARSER_DIR)/lex.hpp
PARSER_HPP := $(PARSER_DIR)/parser.hpp

PARSER_OUT  := $(PARSER_CPP_3DS) $(PARSER_CPP_HOST) $(PARSER_HPP) $(LEXER_CPP_3DS) $(LEXER_CPP_HOST) $(LEXER_HPP) \
	$(PARSER_DIR)/location.hh $(PARSER_DIR)/position.hh $(PARSER_DIR)/parser.output $(PARSER_DIR)/stack.hh \
	$(PARSER_DIR)/parser_3ds.hpp $(PARSER_DIR)/parser_3ds.output

CFILES := $(foreach dir,$(SOURCES),$(notdir $(call recurse,f,$(dir),*.c)))
ALL_CPP := $(foreach dir,$(SOURCES),$(call recurse,f,$(dir),*.cpp))
CPPFILES := $(notdir $(PARSER_CPP_3DS) $(LEXER_CPP_3DS) $(filter-out \
    %/lang/devel.cpp \
	%/parser_3ds.cpp %/lex_3ds.cpp, \
    ,$(foreach dir,$(SOURCES),$(call recurse,f,$(dir),*.cpp)))) 

SFILES := $(foreach dir,$(SOURCES),$(notdir $(call recurse,f,$(dir),*.s)))
PICAFILES := $(foreach dir,$(SOURCES),$(notdir $(call recurse,f,$(dir),*.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
	$(PICAFILES:.v.pica=.shbin.o) \
	$(SHLISTFILES:.shlist=.shbin.o) \
	$(CPPFILES:.cpp=.o) \
	$(CFILES:.c=.o) \
	$(SFILES:.s=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
	$(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

export DEPSDIR := $(CURDIR)/$(BUILD)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir) $(call recurse,d,$(CURDIR)/$(dir),*)) $(foreach dir,$(DATA),$(CURDIR)/$(dir) $(call recurse,d,$(CURDIR)/$(dir),*))

export TOPDIR := $(CURDIR)
OUTPUT_DIR := $(TOPDIR)/$(OUTPUT)

.PHONY: $(BUILD) clean all format

#---------------------------------------------------------------------------------
# Initial Targets
#---------------------------------------------------------------------------------
all: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

3dsx: parser_3ds $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

cia: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

3ds: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

elf: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

azahar: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

hblauncher: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

fbi: $(BUILD) $(OUTPUT_DIR)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile $@

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

$(OUTPUT_DIR):
	@[ -d $@ ] || mkdir -p $@

fmt:
	find . -regex '.*\.\(c\|cc\|cpp\|cxx\|h\|hh\|hpp\)' -exec clang-format -i {} +

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT) $(DEVEL_OBJECTS) $(PARSER_OUT)

parser_3ds: $(PARSER_CPP_3DS) $(LEXER_CPP_3DS)
parser_host: $(PARSER_CPP_HOST) $(LEXER_CPP_HOST)

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Build Information Setup
#---------------------------------------------------------------------------------
DEPENDS := $(OFILES:.o=.d)

include $(TOPDIR)/$(APP_INFO)
APP_TITLE := $(shell echo "$(APP_TITLE)" | cut -c1-128)
APP_DESCRIPTION := $(shell echo "$(APP_DESCRIPTION)" | cut -c1-256)
APP_AUTHOR := $(shell echo "$(APP_AUTHOR)" | cut -c1-128)
APP_PRODUCT_CODE := $(shell echo $(APP_PRODUCT_CODE) | cut -c1-16)
APP_UNIQUE_ID := $(shell echo $(APP_UNIQUE_ID) | cut -c1-7)
APP_VER_MAJOR := $(shell echo $(APP_VER_MAJOR) | cut -c1-3)
APP_VER_MINOR := $(shell echo $(APP_VER_MINOR) | cut -c1-3)
APP_VER_MICRO := $(shell echo $(APP_VER_MICRO) | cut -c1-3)
ifneq ("$(wildcard $(TOPDIR)/$(BANNER_IMAGE).cgfx)","")
	BANNER_IMAGE_FILE := $(TOPDIR)/$(BANNER_IMAGE).cgfx
	BANNER_IMAGE_ARG := -ci $(BANNER_IMAGE_FILE)
else
	BANNER_IMAGE_FILE := $(TOPDIR)/$(BANNER_IMAGE).png
	BANNER_IMAGE_ARG := -i $(BANNER_IMAGE_FILE)
endif

ifneq ("$(wildcard $(TOPDIR)/$(BANNER_AUDIO).cwav)","")
	BANNER_AUDIO_FILE := $(TOPDIR)/$(BANNER_AUDIO).cwav
	BANNER_AUDIO_ARG := -ca $(BANNER_AUDIO_FILE)
else
	BANNER_AUDIO_FILE := $(TOPDIR)/$(BANNER_AUDIO).wav
	BANNER_AUDIO_ARG := -a $(BANNER_AUDIO_FILE)
endif

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
OUTPUT_NAME := $(subst $(SPACE),,$(APP_TITLE))
OUTPUT_DIR := $(TOPDIR)/$(OUTPUT)
OUTPUT_FILE := $(OUTPUT_DIR)/$(OUTPUT_NAME)

APP_ICON := $(TOPDIR)/$(ICON)
APP_ROMFS := $(TOPDIR)/$(ROMFS)

COMMON_MAKEROM_PARAMS := -rsf $(RSF) -target t -exefslogo -elf $(OUTPUT_FILE).elf -icon icon.icn -banner banner.bnr \
	-DAPP_TITLE="$(APP_TITLE)" -DAPP_PRODUCT_CODE="$(APP_PRODUCT_CODE)" -DAPP_UNIQUE_ID="$(APP_UNIQUE_ID)" \
	-DAPP_ROMFS="$(APP_ROMFS)" -DAPP_SYSTEM_MODE="64MB" -DAPP_SYSTEM_MODE_EXT="Legacy" -major "$(APP_VER_MAJOR)" \
	-minor "$(APP_VER_MINOR)" -micro "$(APP_VER_MICRO)"

ifeq ($(OS),Windows_NT)
	MAKEROM = makerom.exe
	BANNERTOOL = bannertool.exe
else
	MAKEROM = makerom
	BANNERTOOL = bannertool
endif

_3DSXFLAGS += --smdh=$(OUTPUT_FILE).smdh
ifneq ($(ROMFS),)
	export _3DSXFLAGS += --romfs=$(APP_ROMFS)
endif

#---------------------------------------------------------------------------------
# Main Targets
#---------------------------------------------------------------------------------
.PHONY: all 3dsx cia elf 3ds azahar fbi hblauncher parser_3ds
all: $(OUTPUT_FILE).zip $(OUTPUT_FILE).3ds $(OUTPUT_FILE).cia

banner.bnr: $(BANNER_IMAGE_FILE) $(BANNER_AUDIO_FILE)
	@$(BANNERTOOL) makebanner $(BANNER_IMAGE_ARG) $(BANNER_AUDIO_ARG) -o banner.bnr > /dev/null

icon.icn: $(TOPDIR)/$(ICON)
	@$(BANNERTOOL) makesmdh -s "$(APP_TITLE)" -l "$(APP_TITLE)" -p "$(APP_AUTHOR)" -i $(TOPDIR)/$(ICON) -o icon.icn > /dev/null

$(OUTPUT_FILE).elf: parser_3ds $(OFILES)

$(OUTPUT_FILE).3dsx: $(OUTPUT_FILE).elf $(OUTPUT_FILE).smdh

$(OUTPUT_FILE).3ds: $(OUTPUT_FILE).elf banner.bnr icon.icn
	@$(MAKEROM) -f cci -o $(OUTPUT_FILE).3ds -DAPP_ENCRYPTED=true $(COMMON_MAKEROM_PARAMS)
	@echo "built ... $(notdir $@)"

$(OUTPUT_FILE).cia: $(OUTPUT_FILE).elf banner.bnr icon.icn
	@$(MAKEROM) -f cia -o $(OUTPUT_FILE).cia -DAPP_ENCRYPTED=false $(COMMON_MAKEROM_PARAMS)
	@echo "built ... $(notdir $@)"

$(OUTPUT_FILE).zip: $(OUTPUT_FILE).smdh $(OUTPUT_FILE).3dsx
	@cd $(OUTPUT_DIR); \
	mkdir -p 3ds/$(OUTPUT_NAME); \
	cp $(OUTPUT_FILE).3dsx 3ds/$(OUTPUT_NAME); \
	cp $(OUTPUT_FILE).smdh 3ds/$(OUTPUT_NAME); \
	zip -r $(OUTPUT_FILE).zip 3ds > /dev/null; \
	rm -r 3ds
	@echo "built ... $(notdir $@)"

3dsx : $(OUTPUT_FILE).3dsx

cia : $(OUTPUT_FILE).cia

3ds : $(OUTPUT_FILE).3ds

elf : $(OUTPUT_FILE).elf

AZAHAR=flatpak run org.azahar_emu.Azahar
azahar : $(OUTPUT_FILE).3dsx
	$(AZAHAR) output/SuperML.3dsx

fbi : $(OUTPUT_FILE).cia
	python ../buildtools/servefiles.py $(IP3DS) $(OUTPUT_FILE).cia

hblauncher : $(OUTPUT_FILE).3dsx
	3dslink -a $(IP3DS) $(OUTPUT_FILE).3dsx


#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval CURBIN := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $@
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
endef

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

# ---------------------------------------------------------
# Parser (Bison) + Lexer (Flex) generation
# ---------------------------------------------------------

$(PARSER_CPP_3DS): $(PARSER_FILE)
	bison -d -v -o $(PARSER_CPP_3DS) $(PARSER_FILE)

$(LEXER_CPP_3DS): $(LEXER_FILE)
	flex --header-file=$(LEXER_HPP) -o $(LEXER_CPP_3DS) $(LEXER_FILE)
	sed -i 's/#include <FlexLexer.h>/#include "FlexLexer.h"/' $(LEXER_CPP_3DS)

$(PARSER_CPP_HOST): $(PARSER_FILE)
	bison -d -v -o $(PARSER_CPP_HOST) $(PARSER_FILE)

$(LEXER_CPP_HOST): $(LEXER_FILE)
	flex --header-file=$(LEXER_HPP) -o $(LEXER_CPP_HOST) $(LEXER_FILE)
	sed -i 's/#include <FlexLexer.h>/#include "FlexLexer.h"/' $(LEXER_CPP_HOST)

# -----------------------------
# Host (desktop) interpreter build
# -----------------------------
ifdef ENABLE_DEVEL
# Host build: sources and objects
DEVEL_SOURCES := \
    $(filter-out %/parser.cpp %/lex.cpp \
        %/parser_3ds.cpp %/lex_3ds.cpp, \
        $(shell find $(PWD)$(SOURCES)/lang -type f -name '*.cpp') \
    ) \
    source/Notepad3DS/source/file.cpp \
    source/Notepad3DS/source/file_io.cpp \
	source/utils.cpp \
	$(PARSER_CPP_HOST) $(LEXER_CPP_HOST)

DEVEL_OBJECTS := $(DEVEL_SOURCES:.cpp=.o)

HOST_CXX := g++
HOST_CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -D__DEBUG__

DEVEL_BIN := $(BUILD)/devel

# Correct pattern rule — compiles ONE source per ONE object
$(DEVEL_OBJECTS): %.o : %.cpp
	$(HOST_CXX) $(HOST_CXXFLAGS) -c $< -o $@

# Build host binary
.PHONY: devel
devel: $(BUILD) parser_host $(DEVEL_OBJECTS)
	$(HOST_CXX) $(HOST_CXXFLAGS) -o $(DEVEL_BIN) $(DEVEL_OBJECTS)
endif
