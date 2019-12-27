#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

#########################
# Setup C/C++ variables #
#########################

CC_CC             := c++
CC_C              := c++
CC_MK_SHARED_LIB  := c++

export _CXX_CXXSUFFIX=cpp
export _CXX_CCMODE=1

ifeq ($(BUILD_TYPE),retail)
    CC_BUILD_TYPE_FLAGS = $(CC_OPTIMIZE)
else
    CC_BUILD_TYPE_FLAGS = $(CC_DEBUG)
endif

ifeq ($(BUILD_ARCH),LP64)
    CC_BUILD_ARCH_FLAGS = LP64
else
    CC_BUILD_ARCH_FLAGS = XPLINK
endif

CC_DEBUG    := -g
CC_OPTIMIZE := -2

# Export shared library symbols
CC_EXPORT_SHARED_LIB_SYMBOLS := -Wc,EXPORTALL

# Enable support for long long
CC_LANGUAGELEVEL := "LANGLVL(EXTENDED),NORTTI"

# Set warning level to maximum
CC_WARNINGLEVEL := "INFO(ALL)"

# Disable standard libraries.  If the standard libraries are used, then the
# z/OS compiler/runtime will erroneously report file lengths of zero when
# using seekg/tellg on a stream

CC_STD_SUPPORT := -DSTAF_Config_NoSTDIOStreamSupport \
                  -DSTAF_Config_NoSTDFStreamSupport

CC_SHAREDLIBNAMEFLAGS = -DSTAF_SHARED_LIB_PREFIX=$(OS_SHARED_LIB_PREFIX) \
                        -DSTAF_SHARED_LIB_SUFFIX=$(OS_SHARED_LIB_SUFFIX)
CC_COMMONFLAGS   = $(OS_COMMONFLAGS) $(COMMONFLAGS) $(CC_SHAREDLIBNAMEFLAGS) \
                   $(CC_STD_SUPPORT) -DSTAF_NATIVE_COMPILER
CC_CFLAGS        = $(CC_COMMONFLAGS) $(CC_BUILD_TYPE_FLAGS) $(CFLAGS) \
                   -Wc,$(CC_BUILD_ARCH_FLAGS) \
                   -Wc,$(CC_WARNINGLEVEL),$(CC_LANGUAGELEVEL),"FLOAT(IEEE)" \
                   -Wc,"SPILL(1024)"
CC_LINKFLAGS     = -Wl,$(CC_BUILD_ARCH_FLAGS) $(CC_COMMONFLAGS) $(CC_BUILD_TYPE_FLAGS) \
                   $(LINKFLAGS)
CC_SHARED_LIB_LINKFLAGS = -Wl,DLL,$(CC_BUILD_ARCH_FLAGS) $(CC_COMMONFLAGS) \
                          $(CC_BUILD_TYPE_FLAGS) $(LINKFLAGS)

# Debug only
#CC_LINKFLAGS     = $(CC_COMMONFLAGS) $(LINKFLAGS) -Wl,XPLINK
#CC_SHARED_LINKFLAGS = $(CC_COMMONFLAGS) $(LINKFLAGS) -Wl,DLL,XPLINK

#We must link with a shared library via the corresponding definition side-deck.
ALL_LIB_LIST = $(foreach lib, $(ALL_LIB_LIST_RAW),$(REL)/lib/$(OS_SHARED_LIB_PREFIX)$(lib).x)

##########################################
# Set commands needed by master makefile #
##########################################

#define CC_DEPEND_IT
#    cd $(@D); $(CC_CC) -E -M $(CC_FLAGS) $(ALL_INCLUDEDIR_LIST) $< >/dev/null; \
#       cat $(subst .d,.u,$@) |\
#       sed -e 's@\(.*\)\.o:@$(@D)/\1.o $(@D)/\1.d:@' >$@; \
#       rm $(subst .d,.u,$@) 
#endef

CC_DEPEND_IT     = touch $@

CC_DEPEND_IT_C   = touch $@

CC_COMPILE_IT    = $(CC_CC) -c -o $@ $(CC_CFLAGS) $(ALL_INCLUDEDIR_LIST) $<

CC_COMPILE_IT_C  = $(CC_C) -c -o $@ $(CC_CFLAGS) $(ALL_INCLUDEDIR_LIST) $<

CC_LINK_IT       = $(CC_CC) -o $@ $(CC_LINKFLAGS) $(ALL_INCLUDEDIR_LIST)\
                   $(ALL_LIBDIR_LIST) $(OBJS) $(ALL_LIB_LIST)

CC_SHARED_LIB_IT = $(CC_MK_SHARED_LIB) -o $@ $(CC_SHARED_LIB_LINKFLAGS) \
                   $(ALL_INCLUDEDIR_LIST) \
                   $(ALL_LIBDIR_LIST) $(OBJS) $(ALL_LIB_LIST)

MOVE_SIDE_DECK   = $(MOVE) $(SRC)/$(@F:$(OS_SHARED_LIB_SUFFIX)=.x)\
                           $(@D)/$(@F:$(OS_SHARED_LIB_SUFFIX)=.x)   
