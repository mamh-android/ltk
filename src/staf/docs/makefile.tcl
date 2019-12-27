#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

staftcldocs_targets += $(REL)/docs/staf/STAFTcl.htm
Targets += $(staftcldocs_targets)
CleanupTargets += cleanup_staftcldocs

$(staftcldocs_targets): SUBSYS_REL := docs
SUBSYS_REL := docs

$(REL)/docs/staf/STAFTcl.htm: $(SR_SRC)/STAFTcl.htm
	$(COPY_FILE)

cleanup_staftcldocs:
	-@$(DEL) $(REL)/docs/staf/STAFTcl.htm $(OUT_ERR_TO_DEV_NULL)
