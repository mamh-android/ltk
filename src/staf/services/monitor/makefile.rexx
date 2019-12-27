#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafmonrexx_targets += $(REL)/bin/STAFMon.cmd\
                       $(REL)/lib/STAFMon.rxl
Targets += $(stafmonrexx_targets)
CleanupTargets += cleanup_stafmonrexx

$(stafmonrexx_targets): SUBSYS_REL := services/monitor
SUBSYS_REL := services/monitor

$(stafmonrexx_targets): RXPP_PATH += $(OS_SRC)/lang/rexx $(OS_SRC)/services/monitor

$(REL)/bin/STAFMon.cmd: $(SRC)/services/monitor/STAFMon.rxp $(MAKEFILE_NAME)
	$(RXPP_IT)

$(REL)/lib/STAFMon.rxl: $(SRC)/services/monitor/STAFMon.rxl $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_stafmonrexx:
	-@$(DEL) $(REL)/bin/STAFMon.cmd $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/STAFMon.rxl $(OUT_ERR_TO_DEV_NULL)
