#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

staflogtcl_targets += $(REL)/lib/STAFLog.tcl
Targets += $(staflogtcl_targets)
CleanupTargets += cleanup_staflogtcl

$(staflogtcl_targets): SUBSYS_REL := services/log
SUBSYS_REL := services/log

$(REL)/lib/STAFLog.tcl: $(SR_SRC)/STAFLog.tcl
	$(COPY_FILE)

cleanup_staflogtcl:
	-@$(DEL) $(REL)/lib/STAFLog.tcl $(OUT_ERR_TO_DEV_NULL)
