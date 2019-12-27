#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

LIB_TCLSTAF = $(subst Name,TCLSTAF,$(DLL))

tcl_targets += \
  $(REL)/bin/STAF.tcl \
  $(REL)/lib/STAFUtil.tcl \
  $(REL)/lib/pkgIndex.tcl

Targets += $(tcl_targets)
CleanupTargets += tcl_targets

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

# Include inference rules
include $(InferenceRules)

# STAF TCL targets

$(REL)/bin/STAF.tcl: $(SRC)/lang/tcl/STAF.tcl
	$(COPY_FILE)

$(REL)/lib/STAFUtil.tcl: $(SRC)/lang/tcl/STAFUtil.tcl
	$(COPY_FILE)

$(REL)/lib/pkgIndex.tcl: $(SRC)/lang/tcl/pkgIndex.tcl
	@echo "*** Generating pkgIndex.tcl ***"
	@cat $< | sed -e 's/LIB_TCLSTAF/$(LIB_TCLSTAF)/;' >$@

cleanup_stafif_tcl:
	-@$(DEL) $(O)/stafif/tcl/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/STAFUtil.tcl $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/pkgIndex.tcl $(OUT_ERR_TO_DEV_NULL)
