#############################################################################
# Software Testing Automation Framework (STAF)                              #
#                                                                           #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(PERL_BUILD_V512),1)

OS_COMMONFLAGS += -DPERL5_6PLUS

LIB_PLSTAF = $(subst Name,PLSTAF,$(DLL))

SUBSYS_REL_PERL = lang/perl/perl512

perl512_targets += $(REL)/lib/perl512/$(LIB_PLSTAF) \
                  $(REL)/bin/PLSTAFService.pm

Targets += $(perl512_targets)
CleanupTargets += cleanup_perl512

$(perl512_targets): SUBSYS_REL := lang/perl/perl512
SUBSYS_REL := lang/perl/perl512

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(perl512_targets): INCLUDEDIRS = $(PERL_V512_INCLUDEDIRS) $(SRC)/$(SUBSYS_REL)/..
$(perl512_targets): OBJS        = $(perl512_objs)
$(perl512_targets): LIBS        = STAF $(PERL_V512_LIBS)
$(perl512_targets): LIBDIRS     = $(PERL_V512_LIBDIRS)
$(perl512_targets): CFLAGS     := $(CC_EXPORT_SHARED_LIB_SYMBOLS)

perl512_objs := \
        STAFPerlService \
        STAFPerlGlue \
        STAFPerlSyncHelper \
        PLSTAF \
        PLSTAFCommandParser

perl512_objs         := $(foreach obj,$(perl512_objs),$(O)/$(SUBSYS_REL)/$(obj)$(OS_OE))
perl512_dependents   := $(perl512_objs:$(OS_OE)=.d)
$(perl512_dependents): SUBSYS_REL := $(SUBSYS_REL_PERL)
$(perl512_dependents): INCLUDEDIRS = $(PERL_V512_INCLUDEDIRS) $(STAFDIR)

ifeq ($(OS_NAME),win32)
    perl512_objs += $(SRC)/lang/perl/PLSTAF.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
	include $(perl512_dependents)
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

$(REL)/lib/perl512/$(LIB_PLSTAF): $(perl512_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

$(REL)/bin/PLSTAFService.pm:      $(SRC)/lang/perl/PLSTAFService.pm       $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_perl512:
	-@$(DEL) $(O)/$(SUBSYS_REL)/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/perl512/$(LIB_PLSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
