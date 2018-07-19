
include make/os.mak

$(warning  $(TOOLCHAIN))

ifeq ($(OS),Windows_NT)
TOOLCHAIN ?= native
else
TOOLCHAIN ?= mips
endif

PROD_MACROS += $(MAKE_DEFINES)

ASZVER ?= 0.0.0.
PROD_MACROS += ASZVER=$(ASZVER)

ifneq (,$(findstring CONFIG_SUPPORT_EMBEDNS,$(PROD_MACROS)))
PROD_MACROS += CONFIG_SUPPORT_REST
endif

$(warning  $(TOOLCHAIN))

include make/toolchain-$(TOOLCHAIN).mak

HOME = $(shell pwd)
SRCROOT = src
LIBROOT = lib

INCLUDES += -I$(LIBROOT)/json
INCLUDES += -I$(LIBROOT)/spdlog
INCLUDES += -I$(LIBROOT)/SQLiteCpp/include
INCLUDES += -I$(LIBROOT)/SQLiteCpp/sqlite3
INCLUDES += -I$(LIBROOT)
INCLUDES += -I$(SRCROOT)/app
INCLUDES += -I$(SRCROOT)/back
INCLUDES += -I$(SRCROOT)

SRC += $(wildcard $(SRCROOT)/app/*.cpp)
SRC += $(wildcard $(SRCROOT)/api/*.cpp)
SRC += $(wildcard $(SRCROOT)/conf/*.cpp)
SRC += $(wildcard $(SRCROOT)/lang/*.cpp)
SRC += $(wildcard $(SRCROOT)/router/*.cpp)
SRC += $(wildcard $(SRCROOT)/back/*.cpp)
SRC += $(wildcard $(SRCROOT)/back/external/*.cpp)
SRC += $(wildcard $(SRCROOT)/front/*.cpp)
SRC += $(wildcard $(SRCROOT)/types/*.cpp)

ifneq (,$(findstring CONFIG_SUPPORT_EMBEDNS,$(PROD_MACROS)))
SRC += $(wildcard $(SRCROOT)/lrwan/*.cpp)
SRC += $(wildcard $(SRCROOT)/back/local/*.cpp)
SRC += $(wildcard $(LIBROOT)/SQLiteCpp/src/*.cpp)
SRC += $(wildcard $(LIBROOT)/SQLiteCpp/sqlite3/sqlite3.c)
SRC += $(wildcard $(LIBROOT)/paho.mqtt.embedded-c/MQTTPacket/src/*.c)
endif

SRC += $(wildcard $(LIBROOT)/json/parson.c)

CFLAGS+= $(addprefix -D,$(PROD_MACROS))
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -MMD -MP -Wall -O1
#CFLAGS+= -g
CFLAGS+= -funroll-loops
CFLAGS+= $(INCLUDES)

CPPFLAGS = $(CFLAGS) -std=gnu++11 ##-fno-rtti -fno-exceptions -fno-unwind-tables

#LFLAGS = --entry am_reset_isr
#LFLAGS += -nostartfiles
#LFLAGS += -nodefaultlibs
#LFLAGS += -nostdlib
#LFLAGS += -static
#LFLAGS += -specs=nano.specs -nostdlib -static
#LFLAGS += -Wl,-Map,build/$(TARGET)/$(TARGET).map,--cref
#LFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group
#LFLAGS += -Wl,--gc-sections
#LFLAGS += -T ./make/$(TARGET).ld
LFLAGS += -lpthread -lrt -ldl   #-l$(HOME)/build/$(TARGET)/lib/libfruit.dll.a
LFLAGS += $(addprefix -l,$(PROD_LIBS))

CPFLAGS = -Obinary

ODFLAGS = -S

VPATH = $(dir $(SRC))

CPPSRC = $(filter-out %_test.cpp test.cpp,$(filter %.cpp,$(notdir $(SRC))))
CSRC = $(filter-out %_test.c test.c,$(filter %.c,$(notdir $(SRC))))
ASRC = $(filter %.s,$(notdir $(SRC)))

OBJS = $(CPPSRC:%.cpp=build/$(TARGET)/objs/%.o)
OBJS += $(CSRC:%.c=build/$(TARGET)/objs/%.o)
OBJS += $(ASRC:%.s=build/$(TARGET)/objs/%.o)

.PHONY: all clean show info
all: build/$(TARGET)/$(TARGET).elf

ifeq ($(CONFIG_SUPPORT_PF),1)
LIB_CFLAGS += -DDIRECTIPC
include make/libloragw.mak
include make/pktfwd.mak
OBJS += $(LIBLORAGW_OBJS)
OBJS += $(PKTFWD_OBJS)
CFLAGS += -DCONFIG_SUPPORT_PF
endif

include make/pktfwd-common.mak
OBJS += $(PKTFWD_COMMON_OBJS)

include make/copy-headers.mak

info:
	@$(CC) --version
	@$(CPP) --version
	@echo CFLAGS=$(CFLAGS)
	@echo LFLAGS=$(LFLAGS)
	@echo PKTFWD_VERSION=$(PKTFWD_VERSION)

include make/target.mak
