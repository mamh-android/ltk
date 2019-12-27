#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(PERL_BUILD_V510),1)

OS_COMMONFLAGS += -DPERL5_6PLUS

LIB_PLSTAF = $(subst Name,PLSTAF,$(DLL))

SUBSYS_REL_PERL = lang/perl/perl510

perl510_targets += $(REL)/lib/perl510/$(LIB_PLSTAF) \
                  $(REL)/bin/PLSTAFService.pm

Targets += $(perl510_targets)
CleanupTargets += cleanup_perl510

$(perl510_targets): SUBSYS_REL := lang/perl/perl510
SUBSYS_REL := lang/perl/perl510

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(perl510_targets): INCLUDEDIRS = $(PERL_V510_INCLUDEDIRS) $(SRC)/$(SUBSYS_REL)/..
$(perl510_targets): OBJS = $(perl510_objs) $(PERL_V510_DYNALOADER)
$(perl510_targets): LIBS = STAF $(PERL_V510_LIBS)
$(perl510_targets): LIBDIRS     = $(PERL_V510_LIBDIRS)
$(perl510_targets): CFLAGS := $(CC_EXPORT_SHARED_LIB_SYMBOLS)

perl510_objs := \
       	STAFPerlService \
        STAFPerlGlue \
        STAFPerlSyncHelper \
        PLSTAF \
        PLSTAFCommandParser

perl510_objs        := $(foreach obj,$(perl510_objs),$(O)/$(SUBSYS_REL)/$(obj)$(OS_OE))
perl510_dependents  := $(perl510_objs:$(OS_OE)=.d)
$(perl510_dependents): SUBSYS_REL := $(SUBSYS_REL_PERL)
$(perl510_dependents): INCLUDEDIRS = $(PERL_V510_INCLUDEDIRS) $(STAFDIR)

ifeq ($(OS_NAME),win32)
    perl510_objs += $(SRC)/lang/perl/PLSTAF.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
	include $(perl510_dependents)
endif

# Include inference rules
include $(InferenceRules)

# These two rules allow us to build the Perl specific versions from the common
# parent tree

$(O)/$(SUBSYS_REL)/%.d: $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(C_DEPEND_IT)

$(O)/$(SUBSYS_REL)/%$(OS_OE): $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(COMPILE_IT)

# PERL targets

$(REL)/lib/perl510/$(LIB_PLSTAF): $(perl510_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)
                
$(REL)/bin/PLSTAFService.pm:      $(SRC)/lang/perl/PLSTAFService.pm       $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_perl510:
	-@$(DEL) $(O)/$(SUBSYS_REL)/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/perl510/$(LIB_PLSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
