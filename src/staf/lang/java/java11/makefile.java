#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(JAVA_BUILD_V11),1)

LIB_JSTAF = $(subst Name,JSTAF,$(DLL))
LIB_JSTAFSH = $(subst Name,JSTAFSH,$(DLL))

stafif_java11_targets += $(REL)/lib/java11/$(LIB_JSTAF) \
                         $(REL)/lib/java11/$(LIB_JSTAFSH)

Targets += $(stafif_java11_targets)
CleanupTargets += cleanup_stafif_java11

$(stafif_java11_targets): SUBSYS_REL := lang/java/java11
SUBSYS_REL := lang/java/java11

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(stafif_java11_targets): INCLUDEDIRS = $(OS_O)/lang/java $(JAVA_V11_INCLUDEDIRS)
$(stafif_java11_targets): COMMONFLAGS = $(JAVA_V11_CC_FLAGS)
$(REL)/lib/java11/$(LIB_JSTAF):   OBJS = $(stafif_java11_jstaf_objs)
$(REL)/lib/java11/$(LIB_JSTAFSH): OBJS = $(stafif_java11_jstafsh_objs)
$(stafif_java11_targets): LIBS        = STAF $(JAVA_V11_LIBS) $(OS_DL_LIB)
$(stafif_java11_targets): LIBDIRS     = $(JAVA_V11_LIBDIRS)

stafif_java11_jstaf_objs := \
  JSTAF \
  STAFJavaService

stafif_java11_jstafsh_objs := STAFJavaServiceHelper

stafif_java11_objs := $(stafif_java11_jstaf_objs)\
                      $(stafif_java11_jstafsh_objs)

stafif_java11_jstaf_objs := $(foreach obj,$(stafif_java11_jstaf_objs),$(O)/lang/java/java11/$(obj)$(OS_OE))
stafif_java11_jstafsh_objs := $(foreach obj,$(stafif_java11_jstafsh_objs),$(O)/lang/java/java11/$(obj)$(OS_OE))
stafif_java11_objs := $(foreach obj,$(stafif_java11_objs),$(O)/lang/java/java11/$(obj)$(OS_OE))
stafif_java11_dependents := $(stafif_java11_objs:$(OS_OE)=.d)
$(stafif_java11_dependents): SUBSYS_REL = lang/java/java11
$(stafif_java11_dependents): INCLUDEDIRS = $(OS_O)/lang/java $(JAVA_V11_INCLUDEDIRS)

ifeq ($(OS_NAME),win32)
    stafif_java11_objs += $(SRC)/lang/java/JSTAF.def
endif

# Include dependencies
ifneq ($(InCleanup), "1")
    include $(stafif_java11_dependents)
endif

# Include inference rules
include $(InferenceRules)

# These two rules allow us to build the Java specific versions from the common
# parent tree

$(O)/$(SUBSYS_REL)/%.d: $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(C_DEPEND_IT)

$(O)/$(SUBSYS_REL)/%$(OS_OE): $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(COMPILE_IT)

# STAFIF Java targets

$(O)/lang/java/java11/JSTAF.d: $(O)/lang/java/com_ibm_staf_STAFHandle.h

$(O)/lang/java/java11/STAFJavaServiceHelper.d: $(O)/lang/java/com_ibm_staf_service_STAFServiceHelper.h

$(REL)/lib/java11/$(LIB_JSTAF): $(stafif_java11_jstaf_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

$(REL)/lib/java11/$(LIB_JSTAFSH): $(stafif_java11_jstafsh_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

cleanup_stafif_java11:
	-@$(DEL) $(O)/lang/java/java11/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/java11/$(LIB_JSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
