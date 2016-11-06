CXXFLAGS += -std=c++11 -Wfatal-errors -pthread
LDFLAGS  += -pthread

GCC_HOME=/usr/bin

CC  := $(GCC_HOME)/g++
CXX := $(CC)
LD  := $(CC)

#ENABLE_BIN_FOLDER = 1

ifeq ($(OS),OSX)
    TP_INCLUDES_EXTRA += -I/opt/local/include
    TP_LINK_EXTRA     += -L/opt/local/lib
    LIBZIP_INCLUDE    := -I/opt/local/lib/libzip/include
endif

$(call enter,src)
