#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(PERL_BUILD_V56),1)

OS_COMMONFLAGS += -DPERL5_6PLUS

LIB_PLSTAF = $(subst Name,PLSTAF,$(DLL))

SUBSYS_REL_PERL = lang/perl/perl56

perl56_targets += $(REL)/lib/perl56/$(LIB_PLSTAF)

Targets += $(perl56_targets)
CleanupTargets += cleanup_perl56

$(perl56_targets): SUBSYS_REL := lang/perl/perl56
SUBSYS_REL := lang/perl/perl56

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(perl56_targets): INCLUDEDIRS = $(PERL_V56_INCLUDEDIRS) $(SRC)/$(SUBSYS_REL)/..
$(perl56_targets): OBJS = $(perl56_objs)
$(perl56_targets): LIBS = STAF $(PERL_V56_LIBS)
$(perl56_targets): LIBDIRS     = $(PERL_V56_LIBDIRS)
$(perl56_targets): CFLAGS := $(CC_EXPORT_SHARED_LIB_SYMBOLS)

perl56_objs := \
	PLSTAF \
	PLSTAFCommandParser

perl56_objs        := $(foreach obj,$(perl56_objs),$(O)/$(SUBSYS_REL)/$(obj)$(OS_OE))
perl56_dependents  := $(perl56_objs:$(OS_OE)=.d)
$(perl56_dependents): SUBSYS_REL := $(SUBSYS_REL_PERL)
$(perl56_dependents): INCLUDEDIRS = $(PERL_V56_INCLUDEDIRS) $(STAFDIR)

# Include dependencies
ifneq ($(InCleanup), "1")
	include $(perl56_dependents)
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

$(REL)/lib/perl56/$(LIB_PLSTAF): $(perl56_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)
                
cleanup_perl56:
	-@$(DEL) $(O)/$(SUBSYS_REL)/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/perl56/$(LIB_PLSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
