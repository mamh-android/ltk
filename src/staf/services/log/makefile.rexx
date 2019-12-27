#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

staflogrexx_targets += $(REL)/bin/STAFLog.cmd\
                       $(REL)/bin/STAFRLog.cmd\
                       $(REL)/lib/STAFLog.rxl
Targets += $(staflogrexx_targets)
CleanupTargets += cleanup_staflogrexx

$(staflogrexx_targets): SUBSYS_REL := services/log
SUBSYS_REL := services/log

$(staflogrexx_targets): RXPP_PATH += $(OS_SRC)/lang/rexx $(OS_SRC)/services/log

$(REL)/bin/STAFLog.cmd: $(SRC)/services/log/STAFLog.rxp $(MAKEFILE_NAME)
	$(RXPP_IT)

$(REL)/bin/STAFRLog.cmd: $(SRC)/services/log/STAFRLog.rxp $(MAKEFILE_NAME)
	$(RXPP_IT)

$(REL)/lib/STAFLog.rxl: $(SRC)/services/log/STAFLog.rxl $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_staflogrexx:
	-@$(DEL) $(REL)/bin/STAFLog.cmd $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/STAFRLog.cmd $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/STAFLog.rxl $(OUT_ERR_TO_DEV_NULL)
