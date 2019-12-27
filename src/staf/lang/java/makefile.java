#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

stafif_java_class_targets =\
  $(O)/lang/java/classes/com/ibm/staf/STAFException.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFResult.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFHandle.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFUtil.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFLogFormatter.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFLogViewer.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFJVMLogViewer.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFQueueMessage.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFMapClassDefinition.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFMarshallingContext.class\
  $(O)/lang/java/classes/com/ibm/staf/STAFVersion.class\
  $(O)/lang/java/classes/com/ibm/staf/TestJSTAF.class\
  $(O)/lang/java/classes/com/ibm/staf/wrapper/STAFMonitor.class\
  $(O)/lang/java/classes/com/ibm/staf/wrapper/STAFLog.class\
  $(O)/lang/java/classes/com/ibm/staf/service/STAFServiceInterfaceLevel30.class\
  $(O)/lang/java/classes/com/ibm/staf/service/STAFServiceHelper.class\
  $(O)/lang/java/classes/com/ibm/staf/service/STAFServiceJarClassLoader.class\
  $(O)/lang/java/classes/com/ibm/staf/service/STAFServiceSharedJython.class\
  $(O)/lang/java/classes/com/ibm/staf/service/STAFCommandParseResult.class\
  $(O)/lang/java/classes/com/ibm/staf/service/STAFCommandParser.class
stafif_java_targets += $(stafif_java_class_targets)\
                       $(O)/lang/java/com_ibm_staf_service_STAFServiceHelper.h\
                       $(O)/lang/java/com_ibm_staf_STAFHandle.h\
                       $(O)/lang/java/com_ibm_staf_STAFUtil.h\
                       $(REL)/lib/JSTAF.zip\
                       $(REL)/lib/JSTAF.jar\
                       $(REL)/bin/JavaSTAF.class
Targets += $(stafif_java_targets)
CleanupTargets += cleanup_stafif_java

$(stafif_java_targets): SUBSYS_REL := lang/java
SUBSYS_REL := lang/java

# Include inference rules
include $(InferenceRules)

STAFIF_JAVA_CLASSPATH = $(OS_O)/lang/java/classes$(OS_PS)$(JAVA_CLASSPATH)

define JAVA_COMPILE_IT
    @echo "*** Compiling STAF Java Sources ***"
    @$(CREATE_PATH)
    @$(JAVAC) -d $(OS_O)/lang/java/classes $(OS_SRC)/lang/java/classes/*.java $(OS_SRC)/lang/java/service/*.java $(OS_SRC)/lang/java/wrapper/*.java
endef

#    @$(JAVAC) -d $(OS_O)/lang/java/classes -classpath '$(STAFIF_JAVA_CLASSPATH)' $(OS_SRC)/lang/java/classes/*.java $(OS_SRC)/lang/java/service/*.java $(OS_SRC)/lang/java/wrapper/*.java


$(O)/lang/java/classes/com/ibm/staf/%.class: $(SRC)/lang/java/classes/%.java
	$(JAVA_COMPILE_IT)

$(O)/lang/java/classes/com/ibm/staf/%.class: $(SRC)/lang/java/%.java
	$(JAVA_COMPILE_IT)

# STAFIF Java targets

# Note: We build this header here, as our Linux system has problems running
#       the 1.2 JDK, even though it can build against it.  Plus, this file
#       is common between the different JDKs, but it would be nice to move
#       its creation into the JDK specific makefiles at some later time.

$(O)/lang/java/com_ibm_staf_STAFHandle.h: $(stafif_java_class_targets)
	@echo "*** Creating com_ibm_staf_STAFHandle.h ***"
	@$(CREATE_PATH)
	@$(JAVAH) -jni -d $(OS_O)/lang/java -classpath '$(STAFIF_JAVA_CLASSPATH)' com.ibm.staf.STAFHandle
	-@$(TOUCH) $@

$(O)/lang/java/com_ibm_staf_STAFUtil.h: $(stafif_java_class_targets)
	@echo "*** Creating com_ibm_staf_STAFUtil.h ***"
	@$(CREATE_PATH)
	@$(JAVAH) -jni -d $(OS_O)/lang/java -classpath '$(STAFIF_JAVA_CLASSPATH)' com.ibm.staf.STAFUtil
	-@$(TOUCH) $@
        
$(O)/lang/java/com_ibm_staf_service_STAFServiceHelper.h: $(stafif_java_class_targets)
	@echo "*** Creating com_ibm_staf_service_STAFServiceHandle.h ***"
	@$(CREATE_PATH)
	@$(JAVAH) -jni -d $(OS_O)/lang/java -classpath '$(STAFIF_JAVA_CLASSPATH)' com.ibm.staf.service.STAFServiceHelper
	-@$(TOUCH) $@


$(REL)/lib/JSTAF.zip: $(stafif_java_class_targets)
	@echo "*** Creating $(@F) ***"
	@$(CREATE_PATH)
	@cd $(O)/lang/java/classes; $(JAR) cfM '$(OS_@)' `find . -name "*.class" -print`
	
$(REL)/lib/JSTAF.jar: $(stafif_java_class_targets)
	@echo "*** Creating $(@F) ***"
	@$(CREATE_PATH)
	@cd $(O)/lang/java/classes; $(JAR) cfM '$(OS_@)' `find . -name "*.class" -print`

$(REL)/bin/JavaSTAF.class: $(stafif_java_class_targets) $(SR_SRC)/JavaSTAF.java
	@echo "*** Compiling $(@F) ***"
	@$(CREATE_PATH)
	@$(JAVAC) -d $(OS_REL)/bin -classpath '$(STAFIF_JAVA_CLASSPATH)' '$(OS_SRC)$(OS_FS)lang$(OS_FS)java$(OS_FS)JavaSTAF.java'

cleanup_stafif_java:
	-@$(DEL) $(O)/lang/java/* $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/JSTAF.zip $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/JSTAF.jar $(OUT_ERR_TO_DEV_NULL)
	-@$(DEL) $(REL)/bin/JavaSTAF.class $(OUT_ERR_TO_DEV_NULL)

