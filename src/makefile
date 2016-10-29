ifndef TOPLEVELWD
    include $(dir $(lastword $(MAKEFILE_LIST)))../Makefile
else
    # In general, must enter in order of dependencies.
    TP_LINK_MAIN := -lzip
    $(call make_exe,MAIN,p-unzip)
endif
