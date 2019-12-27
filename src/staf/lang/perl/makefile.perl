#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

perl_targets += $(REL)/bin/PLSTAF.pm     \
                $(REL)/bin/STAF.pl       \
                $(REL)/bin/STAF2.pl      \
                $(REL)/docs/staf/STAFPerl.htm
Targets += $(perl_targets)
CleanupTargets += cleanup_perl

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

ifeq ($(OS_NAME),win32)
    perl_objs += $(SRC)/lang/perl/PLSTAF.def
endif

# Include inference rules
include $(InferenceRules)

# STAFIF PERL targets

$(REL)/bin/PLSTAF.pm:      $(SRC)/lang/perl/PLSTAF.pm       $(MAKEFILE_NAME)
	$(COPY_FILE)

$(REL)/bin/STAF.pl:        $(SRC)/lang/perl/STAF.pl         $(MAKEFILE_NAME)
	$(COPY_FILE)

$(REL)/bin/STAF2.pl:       $(SRC)/lang/perl/STAF2.pl        $(MAKEFILE_NAME)
	$(COPY_FILE)
	
$(REL)/docs/staf/STAFPerl.htm:  $(SRC)/docs/STAFPerl.htm    $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_perl:
	-@$(DEL) $(O)/lang/perl/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/PLSTAF.pm $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/STAF.pl $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/STAF2.pl $(OUT_ERR_TO_DEV_NULL)

