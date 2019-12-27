#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

test_java_targets =\
  $(REL)/bin/JPing.class\
  $(REL)/bin/TestQueue.class\
  $(REL)/bin/TestPrivateData.class\
  $(REL)/bin/JStatic.class\
  $(REL)/bin/MarshallingPerfTest.class\
  $(REL)/bin/MarshallingTest.class
Targets += $(test_java_targets)
CleanupTargets += cleanup_test_java

$(test_java_targets): SUBSYS_REL := test
SUBSYS_REL := test

# Include inference rules
include $(InferenceRules)

TEST_JAVA_CLASSPATH = $(OS_SRC)/test$(OS_PS)$(OS_REL)/lib/JSTAF.zip$(OS_PS)$(JAVA_CLASSPATH)

define TEST_JAVA_COMPILE_IT
    @echo "*** Compiling $(@F) ***"
    @$(CREATE_PATH)
    @$(JAVAC) -d $(OS_REL)/bin -classpath '$(TEST_JAVA_CLASSPATH)' '$(OS_<)'
endef

$(REL)/bin/%.class: $(SRC)/test/%.java
	$(TEST_JAVA_COMPILE_IT)

# Test Java targets

$(test_java_targets): $(REL)/lib/JSTAF.zip

cleanup_test_java:
	-@$(DEL) $(REL)/bin/JPing.class $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/TestQueue.class $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/TestPrivateData.class $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/JStatic.class $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/MarshallingPerfTest.class $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/MarshallingTest.class $(OUT_ERR_TO_DEV_NULL)





