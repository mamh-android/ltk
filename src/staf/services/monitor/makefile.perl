#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafmonperl_targets += $(REL)/bin/STAFMon.pm
Targets += $(stafmonperl_targets)
CleanupTargets += cleanup_stafmonperl

$(stafmonperl_targets): SUBSYS_REL := services/monitor
SUBSYS_REL := services/monitor

$(REL)/bin/STAFMon.pm: $(SR_SRC)/STAFMon.pm
	$(COPY_FILE)

cleanup_stafmonperl:
	-@$(DEL) $(REL)/bin/STAFMon.pm $(OUT_ERR_TO_DEV_NULL)
