#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

LIB_RXSTAF = $(subst Name,RXSTAF,$(DLL))
LIB_RXTHREAD = $(subst Name,RxThread,$(DLL))

stafif_rexx_targets += $(REL)/lib/$(LIB_RXSTAF) \
                       $(REL)/lib/$(LIB_RXTHREAD) \
                       $(REL)/lib/STAFCPar.rxl \
                       $(REL)/lib/STAFUtil.rxl
Targets += $(stafif_rexx_targets)
CleanupTargets += cleanup_stafif_rexx

$(stafif_rexx_targets): SUBSYS_REL := lang/rexx
SUBSYS_REL := lang/rexx

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(stafif_rexx_targets)    : INCLUDEDIRS = $(REXX_INCLUDEDIRS)
$(stafif_rexx_targets)    : LIBS        = STAF $(REXX_LIBS)
$(stafif_rexx_targets)    : LIBDIRS     = $(REXX_LIBDIRS)
$(REL)/lib/$(LIB_RXSTAF)  : OBJS        = $(stafif_rexx_objs)
$(REL)/lib/$(LIB_RXTHREAD): OBJS        = $(stafif_rxthread_objs)

stafif_rexx_objs :=\
  RXSTAF \
  RexxVar \
  STAFRexxService

stafif_rxthread_objs :=\
  RexxVar \
  RxThread

stafif_rexx_unique_objs := $(sort $(stafif_rexx_objs) $(stafif_rxthread_objs))

stafif_rexx_objs        := $(foreach obj,$(stafif_rexx_objs),$(O)/lang/rexx/$(obj)$(OS_OE))
stafif_rxthread_objs    := $(foreach obj,$(stafif_rxthread_objs),$(O)/lang/rexx/$(obj)$(OS_OE))
stafif_rexx_unique_objs := $(foreach obj,$(stafif_rexx_unique_objs),$(O)/lang/rexx/$(obj)$(OS_OE))
stafif_rexx_dependents  := $(stafif_rexx_unique_objs:$(OS_OE)=.d)
$(stafif_rexx_dependents): INCLUDEDIRS = $(REXX_INCLUDEDIRS)

ifeq ($(OS_NAME),win32)
    stafif_rexx_objs += $(SR_SRC)/RXSTAF.def
    stafif_rxthread_objs += $(SR_SRC)/RXThread.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
    include $(stafif_rexx_dependents)
endif

# Include inference rules
include $(InferenceRules)

# STAFIF REXX targets

$(REL)/lib/$(LIB_RXSTAF): $(stafif_rexx_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

$(REL)/lib/$(LIB_RXTHREAD): $(stafif_rxthread_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

$(REL)/lib/STAFCPar.rxl: $(SRC)/lang/rexx/STAFCPar.rxl $(MAKEFILE_NAME)
	$(COPY_FILE)

$(REL)/lib/STAFUtil.rxl: $(SRC)/lang/rexx/STAFUtil.rxl $(MAKEFILE_NAME)
	$(COPY_FILE)

cleanup_stafif_rexx:
	-@$(DEL) $(O)/lang/rexx/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/$(LIB_RXSTAF) $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/$(LIB_RXTHREAD) $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/STAFCPar.rxl $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/STAFUtil.rxl $(OUT_ERR_TO_DEV_NULL)
