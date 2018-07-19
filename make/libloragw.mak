### get external defined data

LIBLORAGW_SRC_ROOT := lib/lora_gateway/libloragw
LIBLORAGW_CONFIG_HEADER := $(LIBLORAGW_SRC_ROOT)/inc/config.h

LIBLORAGW_VERSION ?= `cat $(LIBLORAGW_SRC_ROOT)/../VERSION`
include $(LIBLORAGW_SRC_ROOT)/library.cfg

#CFLAGS := -O2 -Wall -Wextra -std=c99 -I$(LIBLORAGW_SRC_ROOT)/inc -I$(LIBLORAGW_SRC_ROOT)

LIBLORAGW_OBJDIR = build/$(TARGET)/libloragw/obj
LIBLORAGW_INCLUDES = $(wildcard $(LIBLORAGW_SRC_ROOT)/inc/*.h) $(LIBLORAGW_CONFIG_HEADER)

### linking options
#
#LIBS := -lloragw -lrt -lm
#
### general build targets
#
#all: libloragw.a test_loragw_spi test_loragw_reg test_loragw_hal test_loragw_gps test_loragw_cal
#
#clean:
#	rm -f libloragw.a
#	rm -f test_loragw_*
#	rm -f $(OBJDIR)/*.o
#	rm -f $(LIBLORAGW_CONFIG_HEADER)

### transpose library.cfg into a C header file : config.h

$(LIBLORAGW_OBJDIR):
	mkdir -p $(LIBLORAGW_OBJDIR)

$(LIBLORAGW_CONFIG_HEADER): $(LIBLORAGW_SRC_ROOT)/../VERSION $(LIBLORAGW_SRC_ROOT)/library.cfg
	@echo "*** Checking libloragw library configuration ***"
	@rm -f $@
	#File initialization
	@echo "#ifndef _LORAGW_CONFIGURATION_H" >> $@
	@echo "#define _LORAGW_CONFIGURATION_H" >> $@
	# Release version
	@echo "Release version   : $(LIBLORAGW_VERSION)"
	@echo "	#define LIBLORAGW_VERSION	"\"$(LIBLORAGW_VERSION)\""" >> $@
	# Debug options
	@echo "	#define DEBUG_AUX	$(DEBUG_AUX)" >> $@
	@echo "	#define DEBUG_SPI	$(DEBUG_SPI)" >> $@
	@echo "	#define DEBUG_REG	$(DEBUG_REG)" >> $@
	@echo "	#define DEBUG_HAL	$(DEBUG_HAL)" >> $@
	@echo "	#define DEBUG_GPS	$(DEBUG_GPS)" >> $@
	@echo "	#define DEBUG_GPIO	$(DEBUG_GPIO)" >> $@
	@echo "	#define DEBUG_LBT	$(DEBUG_LBT)" >> $@
	# end of file
	@echo "#endif" >> $@
	@echo "*** Configuration seems ok ***"

LIBLORAGW_CFLAGS := -O1 $(LIB_CFLAGS) -Wall -Wextra -std=c99 -I$(LIBLORAGW_SRC_ROOT)/inc -I$(LIBLORAGW_SRC_ROOT)

$(LIBLORAGW_OBJDIR)/%.o: $(LIBLORAGW_SRC_ROOT)/src/%.c $(LIBLORAGW_INCLUDES) $(LIBLORAGW_CONFIG_HEADER) | $(LIBLORAGW_OBJDIR)
	@+echo "CC $<" ;\
	$(CC) -c $(LIBLORAGW_CFLAGS) $< -o $@

$(LIBLORAGW_OBJDIR)/loragw_spi.o: $(LIBLORAGW_SRC_ROOT)/src/loragw_spi.native.c $(LIBLORAGW_INCLUDES) $(LIBLORAGW_CONFIG_HEADER) | $(LIBLORAGW_OBJDIR)
	@+echo "CC $<" ;\
	$(CC) -c $(LIBLORAGW_CFLAGS) $< -o $@

$(LIBLORAGW_OBJDIR)/loragw_hal.o: $(LIBLORAGW_SRC_ROOT)/src/loragw_hal.c $(LIBLORAGW_INCLUDES) $(LIBLORAGW_SRC_ROOT)/src/arb_fw.var $(LIBLORAGW_SRC_ROOT)/src/agc_fw.var $(LIBLORAGW_SRC_ROOT)/src/cal_fw.var $(LIBLORAGW_CONFIG_HEADER) | $(LIBLORAGW_OBJDIR)
	@+echo "CC $<" ;\
	$(CC) -c $(LIBLORAGW_CFLAGS) $< -o $@

LIBLORAGW_OBJS := $(LIBLORAGW_OBJDIR)/loragw_hal.o \
        $(LIBLORAGW_OBJDIR)/loragw_gps.o \
        $(LIBLORAGW_OBJDIR)/loragw_reg.o \
        $(LIBLORAGW_OBJDIR)/loragw_spi.o \
        $(LIBLORAGW_OBJDIR)/loragw_aux.o \
        $(LIBLORAGW_OBJDIR)/loragw_radio.o \
        $(LIBLORAGW_OBJDIR)/loragw_fpga.o \
        $(LIBLORAGW_OBJDIR)/loragw_lbt.o
