### Application-specific constants

APP_NAME := lora_pkt_fwd

### Environment constants

PKTFWD_SRC_ROOT := lib/packet_forwarder/lora_pkt_fwd
LGW_PATH ?= $(LIBLORAGW_SRC_ROOT)

PKTFWD_OBJDIR = build/$(TARGET)/pktfwd/obj
PKTFWD_INCLUDES = $(wildcard $(PKTFWD_SRC_ROOT)/inc/*.h)

### External constant definitions
# must get library build option to know if mpsse must be linked or not

include $(LGW_PATH)/library.cfg
PKTFWD_VERSION ?= `cat $(PKTFWD_SRC_ROOT)/../VERSION`

PKTFWD_CFLAGS := $(LIB_CFLAGS) -O1 -Wall -Wextra -std=c99 -I$(PKTFWD_SRC_ROOT)/inc -I$(PKTFWD_SRC_ROOT)
PKTFWD_VFLAG := -D VERSION_STRING="\"$(PKTFWD_VERSION)\""

### Constants for Lora concentrator HAL library
# List the library sub-modules that are used by the application

LGW_INC =
ifneq ($(wildcard $(LGW_PATH)/inc/config.h),)
  # only for HAL version 1.3 and beyond
  LGW_INC += $(LGW_PATH)/inc/config.h
endif
LGW_INC += $(LGW_PATH)/inc/loragw_hal.h
LGW_INC += $(LGW_PATH)/inc/loragw_gps.h

### Linking options

#LIBS := -lloragw -lrt -lpthread -lm

### General build targets

### Sub-modules compilation

$(PKTFWD_OBJDIR):
	mkdir -p $(PKTFWD_OBJDIR)

$(PKTFWD_OBJDIR)/%.o: $(PKTFWD_SRC_ROOT)/src/%.c $(PKTFWD_INCLUDES) | $(PKTFWD_OBJDIR)
	@+echo "CC $<" ;\
	$(CC) -c $(PKTFWD_CFLAGS) -I$(LGW_PATH)/inc $< -o $@

### Main program compilation and assembly

$(PKTFWD_OBJDIR)/$(APP_NAME).o: $(PKTFWD_SRC_ROOT)/src/$(APP_NAME).c $(LGW_INC) $(PKTFWD_INCLUDES) | $(PKTFWD_OBJDIR)
	@+echo "CC $<" ;\
	$(CC) -c $(PKTFWD_CFLAGS) $(PKTFWD_VFLAG) -I$(LGW_PATH)/inc $< -o $@

PKTFWD_OBJS := $(PKTFWD_OBJDIR)/$(APP_NAME).o \
        $(PKTFWD_OBJDIR)/base64.o \
        $(PKTFWD_OBJDIR)/jitqueue.o \
        $(PKTFWD_OBJDIR)/timersync.o

### EOF
