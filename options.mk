
-include $(ROOT_PROJECT_DIRECTORY)color.mk

BUILD=build
BUILD_DIRECTORY=$(ROOT_PROJECT_DIRECTORY)$(BUILD)/

BIN_DIRECTORY=$(BUILD_DIRECTORY)bin/
OBJ_DIRECTORY=$(BUILD_DIRECTORY)obj/
LIB_DIRECTORY=$(BUILD_DIRECTORY)lib/

REL_PATH=$(shell realpath --relative-to $(ROOT_PROJECT_DIRECTORY) .)/
SRC_PATH=$(ROOT_PROJECT_DIRECTORY)$(REL_PATH)
OBJ_PATH=$(OBJ_DIRECTORY)$(REL_PATH)

# erase automatic vars
.SUFFIXES:

OS=$(shell uname)
ifeq ($(OS),Linux)
CC=/usr/bin/clang
CXX:=/usr/bin/clang++
LD:=$(CXX)
YACC=/usr/bin/bison
LEX=/usr/bin/flex
AR=/usr/bin/ar
RANLIB=/usr/bin/ranlib
else ifeq ($(OS),Darwin)
CC=/usr/local/opt/llvm/bin/clang
CXX:=$(CC)
LD:=$(CC)
YACC=/usr/local/opt/bison/bin/bison
LEX=/usr/local/opt/flex/bin/flex
AR=/usr/local/opt/llvm/bin/llvm-ar
RANLIB=/usr/local/opt/llvm/bin/llvm-ranlib
else
$(error Unsupported build on $(OS))
endif
AS=nasm

WASM=0
ifeq ($(WASM), 1)
CC=emcc
CXX=em++
LD:=$(CC)
AR=${EMSDK}/upstream/bin/llvm-ar
RANLIB=${EMSDK}/upstream/bin/llvm-ranlib
endif

VERBOSE=0
ifeq ($(VERBOSE),0)
AT=@
else
AT=
endif

define strip_root
$$(shell sed s%$(ROOT_PROJECT_DIRECTORY)%% <<< $1)
endef
define _generate_verbose_call
override $1_0=@printf "$(COLOR_GREEN)$(COLOR_BOLD)%s %s$(COLOR_RESET)\n" $1 $(call strip_root,$$@); $($1)
override $1_1=$($1)
override $1=$$($1_$(VERBOSE))
endef
define generate_verbose_call
$(eval $(call _generate_verbose_call,$1))
endef

map = $(foreach a,$(2),$(call $(1),$(a)))
$(call map,generate_verbose_call,CC CXX LD YACC LEX AR RANLIB)

COMPILE_FLAGS=
LINK_FLAGS=
COMPILE_FLAGS+= -Wno-comment

ifeq ($(DEBUG),1)
COMPILE_FLAGS+= -DDEBUG=1 -g -O0 
LINK_FLAGS+= -g
else
COMPILE_FLAGS+= -O3
endif

ifeq ($(WASM),1)
COMPILE_FLAGS+= -DWASM=1 -fwasm-exceptions
else
COMPILE_FLAGS+= -masm=intel
endif
override COMPILE_FLAGS+= -Wall -Wextra

ifdef LINKER
LINK_FLAGS+= -fuse-ld=$(LINKER)
endif

override CFLAGS+= $(COMPILE_FLAGS) -std=c11 -D_XOPEN_SOURCE=700
override CXXFLAGS+= $(COMPILE_FLAGS) -std=c++17
override ASFLAGS+=
override LDFLAGS+= $(LINK_FLAGS)
override INCLUDE+=
override YFLAGS+= -Wall
override LFLAGS+=


