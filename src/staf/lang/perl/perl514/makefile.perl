#############################################################################
# Software Testing Automation Framework (STAF)                              #
#                                                                           #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(PERL_BUILD_V514),1)

OS_COMMONFLAGS += -DPERL5_6PLUS

LIB_PLSTAF = $(subst Name,PLSTAF,$(DLL))

SUBSYS_REL_PERL = lang/perl/perl514

perl514_targets += $(REL)/lib/perl514/$(LIB_PLSTAF) \
                  $(REL)/bin/PLSTAFService.pm

Targets += $(perl514_targets)
CleanupTargets += cleanup_perl514

$(perl514_targets): SUBSYS_REL := lang/perl/perl514
SUBSYS_REL := lang/perl/perl514

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(perl514_targets): INCLUDEDIRS = $(PERL_V514_INCLUDEDIRS) $(SRC)/$(SUBSYS_REL)/..
$(perl514_targets): OBJS        = $(perl514_objs)
$(perl514_targets): LIBS        = STAF $(PERL_V514_LIBS)
$(perl514_targets): LIBDIRS     = $(PERL_V514_LIBDIRS)
$(perl514_targets): CFLAGS     := $(CC_EXPORT_SHARED_LIB_SYMBOLS)

perl514_objs := \
        STAFPerlService \
        STAFPerlGlue \
        STAFPerlSyncHelper \
        PLSTAF \
        PLSTAFCommandParser

perl514_objs         := $(foreach obj,$(perl514_objs),$(O)/$(SUBSYS_REL)/$(obj)$(OS_OE))
perl514_dependents   := $(perl514_objs:$(OS_OE)=.d)
$(perl514_dependents): SUBSYS_REL := $(SUBSYS_REL_PERL)
$(perl514_dependents): INCLUDEDIRS = $(PERL_V514_INCLUDEDIRS) $(STAFDIR)

ifeq ($(OS_NAME),win32)
    perl514_objs += $(SRC)/lang/perl/PLSTAF.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
	include $(perl514_dependents)
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

$(REL)/lib/perl514/$(LIB_PLSTAF): $(perl514_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

$(REL)/bin/PLSTAFService.pm:      $(SRC)/lang/perl/PLSTAFService.pm       $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_perl514:
	-@$(DEL) $(O)/$(SUBSYS_REL)/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/perl514/$(LIB_PLSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
