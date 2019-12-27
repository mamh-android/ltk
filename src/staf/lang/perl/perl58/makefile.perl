#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(PERL_BUILD_V58),1)

OS_COMMONFLAGS += -DPERL5_6PLUS

LIB_PLSTAF = $(subst Name,PLSTAF,$(DLL))

SUBSYS_REL_PERL = lang/perl/perl58

perl58_targets += $(REL)/lib/perl58/$(LIB_PLSTAF) \
                  $(REL)/bin/PLSTAFService.pm

Targets += $(perl58_targets)
CleanupTargets += cleanup_perl58

$(perl58_targets): SUBSYS_REL := lang/perl/perl58
SUBSYS_REL := lang/perl/perl58

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(perl58_targets): INCLUDEDIRS = $(PERL_V58_INCLUDEDIRS) $(SRC)/$(SUBSYS_REL)/..
$(perl58_targets): OBJS = $(perl58_objs) $(PERL_V58_DYNALOADER)
$(perl58_targets): LIBS = STAF $(PERL_V58_LIBS)
$(perl58_targets): LIBDIRS     = $(PERL_V58_LIBDIRS)
$(perl58_targets): CFLAGS := $(CC_EXPORT_SHARED_LIB_SYMBOLS)

perl58_objs := \
       	STAFPerlService \
        STAFPerlGlue \
        STAFPerlSyncHelper \
        PLSTAF \
        PLSTAFCommandParser

perl58_objs        := $(foreach obj,$(perl58_objs),$(O)/$(SUBSYS_REL)/$(obj)$(OS_OE))
perl58_dependents  := $(perl58_objs:$(OS_OE)=.d)
$(perl58_dependents): SUBSYS_REL := $(SUBSYS_REL_PERL)
$(perl58_dependents): INCLUDEDIRS = $(PERL_V58_INCLUDEDIRS) $(STAFDIR)

ifeq ($(OS_NAME),win32)
    perl58_objs += $(SRC)/lang/perl/PLSTAF.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
	include $(perl58_dependents)
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

$(REL)/lib/perl58/$(LIB_PLSTAF): $(perl58_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)
                
$(REL)/bin/PLSTAFService.pm:      $(SRC)/lang/perl/PLSTAFService.pm       $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_perl58:
	-@$(DEL) $(O)/$(SUBSYS_REL)/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/perl58/$(LIB_PLSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
