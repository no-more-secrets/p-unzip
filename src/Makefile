ifndef root
    include $(dir $(lastword $(MAKEFILE_LIST)))../Makefile
else
    # In general, must enter in order of dependencies.
    TP_LINK_MAIN := -lzip
    TP_INCLUDES_MAIN := $(LIBZIP_INCLUDE)
    $(call make_exe,MAIN,p-unzip$(opt-suffix))
endif
