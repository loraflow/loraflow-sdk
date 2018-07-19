UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
PLATFORM=LINUX
else
PLATFORM=NONLINUX
endif
