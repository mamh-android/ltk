#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(TCL_BUILD_V86),1)

LIB_TCLSTAF = $(subst Name,TCLSTAF,$(DLL))

SUBSYS_REL_TCL = lang/tcl/tcl86

stafif_tcl86_targets += \
  $(REL)/lib/tcl86/$(LIB_TCLSTAF)

Targets += $(stafif_tcl86_targets)
CleanupTargets += cleanup_stafif_tcl

$(stafif_tcl86_targets): SUBSYS_REL := lang/tcl/tcl86
SUBSYS_REL := lang/tcl/tcl86

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(stafif_tcl86_targets)    : INCLUDEDIRS = $(TCL_V86_INCLUDEDIRS)
$(stafif_tcl86_targets)    : LIBDIRS     = $(TCL_V86_LIBDIRS)
$(stafif_tcl86_targets)    : LIBS        = STAF $(TCL_V86_LIBS)
$(stafif_tcl86_targets)    : OBJS        = $(stafif_tcl86_objs)

stafif_tcl86_objs :=\
  TCLSTAF

stafif_tcl86_objs        := $(foreach obj,$(stafif_tcl86_objs),$(O)/$(SUBSYS_REL)/$(obj)$(OS_OE))
stafif_tcl86_dependents  := $(stafif_tcl86_objs:$(OS_OE)=.d)
$(stafif_tcl86_dependents): SUBSYS_REL = $(SUBSYS_REL_TCL)
$(stafif_tcl86_dependents): INCLUDEDIRS = $(TCL_V86_INCLUDEDIRS)

ifeq ($(OS_NAME),win32)
    stafif_tcl86_objs += $(SRC)/lang/tcl/TCLSTAF.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
    include $(stafif_tcl86_dependents)
endif

# Include inference rules
include $(InferenceRules)

# These two rules allow us to build the Tcl specific versions from the common
# parent tree

$(O)/$(SUBSYS_REL)/%.d: $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(C_DEPEND_IT)

$(O)/$(SUBSYS_REL)/%$(OS_OE): $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(COMPILE_IT)

# STAF TCL targets

$(REL)/lib/tcl86/$(LIB_TCLSTAF): $(stafif_tcl86_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

cleanup_stafif_tcl86:
	-@$(DEL) $(O)/stafif/tcl/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/tcl86/$(LIB_TCLSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
