CXXFLAGS += -std=c++11 -Wfatal-errors -pthread
LDFLAGS  += -pthread

GPLUSPLUS ?= /usr/bin/g++

CC  = $(GPLUSPLUS)
CXX = $(CC)
LD  = $(CC)

no_top_bin_folder = 1

ifeq ($(OS),OSX)
    TP_INCLUDES_EXTRA += -I/opt/local/include
    TP_LINK_EXTRA     += -L/opt/local/lib
    LIBZIP_INCLUDE    := -I/opt/local/lib/libzip/include
    GCC_HOME           = /Users/dsicilia/dev/tools/bin
else
    # On linux let's do a static linkage
    #LDFLAGS += -static -static-libgcc -static-libstdc++
    LDFLAGS += -static-libgcc -static-libstdc++
endif

$(call enter,src)
