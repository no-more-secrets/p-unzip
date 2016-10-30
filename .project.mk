CXXFLAGS += -std=c++11

GCC_HOME=/usr/bin

CC  := $(GCC_HOME)/g++
CXX := $(CC)
LD  := $(CC)

ENABLE_BIN_FOLDER = 1

ifeq ($(OS),OSX)
    TP_INCLUDES_EXTRA += -I/opt/local/include            \
                         -I/opt/local/lib/libzip/include
endif

$(call enter,src)
