CXXFLAGS += -std=c++11

GCC_HOME=/usr/bin

CC  := $(GCC_HOME)/g++
CXX := $(CC)
LD  := $(CC)

ENABLE_BIN_FOLDER = 1

$(call enter,src)
