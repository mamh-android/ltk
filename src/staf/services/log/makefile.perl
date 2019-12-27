#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

staflogperl_targets += $(REL)/bin/STAFLog.pm
Targets += $(staflogperl_targets)
CleanupTargets += cleanup_staflogperl

$(staflogperl_targets): SUBSYS_REL := services/log
SUBSYS_REL := services/log

$(REL)/bin/STAFLog.pm: $(SR_SRC)/STAFLog.pm
	$(COPY_FILE)

cleanup_staflogperl:
	-@$(DEL) $(REL)/bin/STAFLog.pm $(OUT_ERR_TO_DEV_NULL)
