#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

ifeq ($(JAVA_BUILD_V12),1)

LIB_JSTAF = $(subst Name,JSTAF,$(DLL))
LIB_JSTAFSH = $(subst Name,JSTAFSH,$(DLL))

stafif_java12_targets += $(REL)/lib/$(LIB_JSTAF)\
                         $(REL)/lib/$(LIB_JSTAFSH)

Targets += $(stafif_java12_targets)
CleanupTargets += cleanup_stafif_java12

$(stafif_java12_targets): SUBSYS_REL := lang/java/java12
SUBSYS_REL := lang/java/java12

#=====================================================================
#   C/C++ Info Flags
#=====================================================================

$(stafif_java12_targets): INCLUDEDIRS = $(OS_O)/lang/java $(JAVA_V12_INCLUDEDIRS)
$(stafif_java12_targets): COMMONFLAGS = $(JAVA_V12_CC_FLAGS)
$(REL)/lib/$(LIB_JSTAF):   OBJS = $(stafif_java12_jstaf_objs)
$(REL)/lib/$(LIB_JSTAFSH): OBJS = $(stafif_java12_jstafsh_objs)
$(stafif_java12_targets): LIBS        = STAF $(JAVA_V12_LIBS) $(OS_DL_LIB)
$(stafif_java12_targets): LIBDIRS     = $(JAVA_V12_LIBDIRS)
$(stafif_java12_targets): CFLAGS := $(CC_EXPORT_SHARED_LIB_SYMBOLS)

stafif_java12_jstaf_objs := \
  JSTAF \
  STAFJavaService

stafif_java12_jstafsh_objs := STAFJavaServiceHelper

stafif_java12_objs := $(stafif_java12_jstaf_objs)\
                      $(stafif_java12_jstafsh_objs)

stafif_java12_jstaf_objs := $(foreach obj,$(stafif_java12_jstaf_objs),$(O)/lang/java/java12/$(obj)$(OS_OE))
stafif_java12_jstafsh_objs := $(foreach obj,$(stafif_java12_jstafsh_objs),$(O)/lang/java/java12/$(obj)$(OS_OE))
stafif_java12_objs := $(foreach obj,$(stafif_java12_objs),$(O)/lang/java/java12/$(obj)$(OS_OE))
stafif_java12_dependents := $(stafif_java12_objs:$(OS_OE)=.d)
$(stafif_java12_dependents): SUBSYS_REL = lang/java/java12
$(stafif_java12_dependents): INCLUDEDIRS = $(OS_O)/lang/java $(JAVA_V12_INCLUDEDIRS)

ifeq ($(OS_NAME),win32)
    stafif_java12_jstaf_objs += $(SRC)/lang/java/JSTAF.def
endif

# For Mac OS X, the JNI libraries (JSTAF and JSTAFSH) must exist with a
# .jnilib extension in addition to the .dylib extension

ifeq ($(OS_NAME),macosx)
    stafif_java12_targets += $(REL)/lib/libJSTAF.jnilib\
                             $(REL)/lib/libJSTAFSH.jnilib
endif

# Include dependencies
ifneq ($(InCleanup), "1")
    include $(stafif_java12_dependents)
endif

# Include inference rules
include $(InferenceRules)

# These two rules allow us to build the Java specific versions from the common
# parent tree

$(O)/$(SUBSYS_REL)/%.d: $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(C_DEPEND_IT)

$(O)/$(SUBSYS_REL)/%$(OS_OE): $(SRC)/$(SUBSYS_REL)/../%.cpp
	$(COMPILE_IT)

# This special rule is for Mac OS X for copying the libJSTAF.dylib and
# libJSTAFSH.dylib files to .jnilib files

$(REL)/lib/%.jnilib: $(REL)/lib/%.dylib
	$(COPY_FILE)

# STAFIF Java targets

$(O)/lang/java/java12/JSTAF.d: $(O)/lang/java/com_ibm_staf_STAFHandle.h $(O)/lang/java/com_ibm_staf_STAFUtil.h

$(O)/lang/java/java12/STAFJavaServiceHelper.d: $(O)/lang/java/com_ibm_staf_service_STAFServiceHelper.h

$(REL)/lib/$(LIB_JSTAF): $(stafif_java12_jstaf_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)

$(REL)/lib/$(LIB_JSTAFSH): $(stafif_java12_jstafsh_objs) $(LIB_STAF_FP) $(MAKEFILE_NAME)
	$(SHARED_LIB_IT)
        
cleanup_stafif_java12:
	-@$(DEL) $(O)/lang/java/java12/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/lib/$(LIB_JSTAF) $(OUT_ERR_TO_DEV_NULL)

endif
