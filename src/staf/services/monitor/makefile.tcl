#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafmontcl_targets += $(REL)/lib/STAFMon.tcl
Targets += $(stafmontcl_targets)
CleanupTargets += cleanup_stafmontcl

$(stafmontcl_targets): SUBSYS_REL := services/monitor
SUBSYS_REL := services/monitor

$(REL)/lib/STAFMon.tcl: $(SR_SRC)/STAFMon.tcl
	$(COPY_FILE)

cleanup_stafmontcl:
	-@$(DEL) $(REL)/lib/STAFMon.tcl $(OUT_ERR_TO_DEV_NULL)
