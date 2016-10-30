CFLAGS         += -MMD -MP -m64 -Wall -Wpedantic
CXXFLAGS       += $(CFLAGS) -std=c++11 $(VISIBILITY_HIDDEN)

CFLAGS_DEBUG   += $(CXXFLAGS) -g -ggdb
CFLAGS_RELEASE += $(CXXFLAGS) -Ofast

CFLAGS_LIB     += -fPIC

ifdef OPT
    CXXFLAGS_TO_USE = $(CFLAGS_RELEASE)
else
    CXXFLAGS_TO_USE = $(CFLAGS_DEBUG)
endif

GCC_HOME=/usr/bin

CC  := $(GCC_HOME)/g++
CXX := $(CC)
LD  := $(CC)

LDFLAGS     :=
LDFLAGS_LIB := $(LDFLAGS) -shared 

ENABLE_BIN_FOLDER = 1

INSTALL_PREFIX := $(HOME)/tmp

######################################
CWD := $(TOPLEVELWD)
$(call enter,src)
