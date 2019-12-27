#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2008                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

httpsls_class_targets =\
  $(O)/services/httpsls/STAF-INF/classes/com/ibm/staf/service/httpsls/STAFHTTPSLS.class \
  $(O)/services/httpsls/MANIFEST.MF
  
httpsls_targets =\
  $(httpsls_class_targets)\
  $(REL)/lib/STAFHTTPSLS.jar
                
Targets += $(httpsls_targets)
CleanupTargets += cleanup_httpsls

$(httpsls_targets): SUBSYS_REL := services/httpsls
SUBSYS_REL := services/httpsls

# Include inference rules
include $(InferenceRules)

$(httpsls_targets): $(REL)/lib/JSTAF.zip

HTTPSLS_CLASSPATH = $(OS_O)/services/httpsls$(OS_PS)$(OS_REL)/lib/JSTAF.zip$(OS_PS)$(JAVA_CLASSPATH)
  
define HTTPSLS_JAVA_COMPILE_IT
   @echo "*** Compiling STAF HTTP Service Loader ***"
   @$(CREATE_PATH)
   @$(JAVAC) -d $(OS_O)/services/httpsls/STAF-INF/classes -classpath '$(HTTPSLS_CLASSPATH)' $(OS_SRC)/services/httpsls/STAFHTTPSLS.java
endef

$(O)/services/httpsls/STAF-INF/classes/com/ibm/staf/service/httpsls/%.class: $(SRC)/services/httpsls/%.java
	$(HTTPSLS_JAVA_COMPILE_IT)

# HTTPSLS Java targets

$(O)/services/httpsls/MANIFEST.MF: $(SR_SRC)/MANIFEST.MF
	$(COPY_FILE)

$(REL)/lib/STAFHTTPSLS.jar: $(httpsls_class_targets)
	@echo "*** Creating $(@F) ***"
	@cd $(O)/services/httpsls; $(JAR) cfm '$(OS_@)' MANIFEST.MF `find . -name "*.class" -print`

cleanup_httpsls:
	-@$(DEL) $(O)/services/httpsls/* $(OUT_ERR_TO_DEV_NULL)
