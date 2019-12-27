#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

sdgsample_perl_targets += $(REL)/bin/DeviceService.pm
Targets += $(sdgsample_perl_targets)
CleanupTargets += cleanup_sdgsample_perl

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

# Include inference rules
include $(InferenceRules)

# STAFIF PERL targets

$(REL)/bin/DeviceService.pm: $(SRC)/services/sdg_sample/DeviceService.pm $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_sdgsample_perl:
	-@$(DEL) $(REL)/bin/DeviceService.pm $(OUT_ERR_TO_DEV_NULL)

