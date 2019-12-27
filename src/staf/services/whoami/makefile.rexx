#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafwhoamirexx_targets += $(REL)/bin/WhoAmI.cmd
Targets += $(stafwhoamirexx_targets)
CleanupTargets += cleanup_stafwhoamirexx

$(stafwhoamirexx_targets): SUBSYS_REL := services/whoami
SUBSYS_REL := services/whoami

$(stafwhoamirexx_targets): RXPP_PATH += $(OS_SRC)/lang/rexx $(OS_SRC)/services/whoami

$(REL)/bin/WhoAmI.cmd: $(SRC)/services/whoami/WhoAmI.rxp $(MAKEFILE_NAME)
	$(RXPP_IT)

cleanup_stafwhoamirexx:
	-@$(DEL) $(REL)/bin/WhoAmI.cmd $(OUT_ERR_TO_DEV_NULL)
