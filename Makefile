TOPLEVELWD := $(dir $(lastword $(MAKEFILE_LIST)))

makerules = $(TOPLEVELWD)makerules

include $(makerules)/presrc.mk

CWD := $(TOPLEVELWD)
$(call enter,src)

include $(makerules)/postsrc.mk
