CXXFLAGS += -std=c++11 -Wfatal-errors -pthread
LDFLAGS  += -pthread

# Enable if you need to
#STATIC_LIBSTDCXX=

no_top_bin_folder = 1

ifeq ($(OS),OSX)
    TP_INCLUDES_EXTRA += -I/opt/local/include
    TP_LINK_EXTRA     += -L/opt/local/lib
    LIBZIP_INCLUDE    := -I/opt/local/lib/libzip/include
endif

$(call enter,src)
