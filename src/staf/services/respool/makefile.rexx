#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafpoolrexx_targets += $(REL)/bin/STAFPool.cmd\
                        $(REL)/lib/STAFPool.rxl
Targets += $(stafpoolrexx_targets)
CleanupTargets += cleanup_stafpoolrexx

$(stafpoolrexx_targets): SUBSYS_REL := services/respool
SUBSYS_REL := services/respool

$(stafpoolrexx_targets): RXPP_PATH += $(OS_SRC)/lang/rexx $(OS_SRC)/services/respool

$(REL)/bin/STAFPool.cmd: $(SRC)/services/respool/STAFPool.rxp $(MAKEFILE_NAME)
	$(RXPP_IT)

$(REL)/lib/STAFPool.rxl: $(SRC)/services/respool/STAFPool.rxl $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_stafpoolrexx:
	-@$(DEL) $(REL)/bin/STAFPool.cmd $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/STAFPool.rxl $(OUT_ERR_TO_DEV_NULL)
