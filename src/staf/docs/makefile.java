#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2005                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafjavadocs_targets += $(REL)/docs/staf/STAFJava.htm
Targets += $(stafjavadocs_targets)
CleanupTargets += cleanup_stafjavadocs

$(stafjavadocs_targets): SUBSYS_REL := docs
SUBSYS_REL := docs

$(REL)/docs/staf/STAFJava.htm: $(SR_SRC)/STAFJava.htm
	$(COPY_FILE)

cleanup_stafjavadocs:
	-@$(DEL) $(REL)/docs/staf/STAFJava.htm $(OUT_ERR_TO_DEV_NULL)
