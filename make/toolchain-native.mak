
## GCC linker opts:
##   https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
##   https://sourceware.org/binutils/docs/ld/index.html

#### Setup ####
#TOOLCHAINPREFIX = arm-none-eabi-

#### Required Executables ####
CC = $(TOOLCHAINPREFIX)gcc
CPP = $(TOOLCHAINPREFIX)g++
LD = $(TOOLCHAINPREFIX)g++
CP = $(TOOLCHAINPREFIX)objcopy
OD = $(TOOLCHAINPREFIX)objdump
RD = $(TOOLCHAINPREFIX)readelf
AR = $(TOOLCHAINPREFIX)ar
SIZE = $(TOOLCHAINPREFIX)size

EXECUTABLES = CC CPP LD CP OD AR RD SIZE
K := $(foreach exec,$(EXECUTABLES),\
        $(if $(shell which $($(exec))),,\
        $(info $(exec) not found on PATH ($($(exec))).)$(exec)))
$(if $(strip $(value K)),$(info Required Program(s) $(strip $(value K)) not found))
