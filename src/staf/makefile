#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001, 2005                                        #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

################################################################################
#                             Master Variables                                 #
################################################################################

.SUFFIXES: .o .obj .d .cpp .rxp .rxl .cfg
SUBSYS := staf

ifndef BUILD_TYPE
    BUILD_TYPE := retail
endif

MAKEDIR := $(shell pwd)
SPACE := $(EMPTY) $(EMPTY)
INIT_INCLUDE := $(INCLUDE)

################################################################################
#                          Configuration Settings                              #
################################################################################

ifneq ($(wildcard makefile.cfg),)
    include makefile.cfg
endif

################################################################################
#                             Projects to build                                #
################################################################################

ifndef PROJECTS
    PROJECTS := staf connprov_tcp connprov_localipc
endif

ifeq ($(PROJECTS),all)
    PROJECTS := staf connprov_tcp connprov_localipc zip java perl python tcl ant demo
endif

# Here's a list of all possible projects to build, including external Java services    
# PROJECTS := staf connprov_tcp connprov_localipc zip java perl python tcl ant demo rexx docs authsample lite device jython stax event eventmanager cron http whoami email fsext sxe timer namedcounter namespace ftp

################################################################################
#                         Operating System Makefile                            #
################################################################################

ifndef OS_NAME
    OS_NAME := UNKNOWN_OPERATING_SYSTEM
    $(error Unknown operating system.  Please set the OS_NAME environment variable)
endif

include build/makefile.$(OS_NAME)

################################################################################
#                             Compiler Makefile                                #
################################################################################

ifndef CC_NAME
    CC_NAME := UNKNOWN_COMPILER
    $(error Unknown compiler.  Please set the CC_NAME environment variable)
endif

include build/makefile.$(CC_NAME)

################################################################################
#                                 Makefiles                                    #
################################################################################

ALLMAKEFILES = $(foreach PROJECT, $(PROJECTS),\
               $(shell find $(MAKEDIR) -name "*makefile.$(PROJECT)" -print))

################################################################################
# AT            : Turns echo off or on for some commands.  Turning echo on     #
#                 can be useful when debugging a build problem                 #
#                 Valid Values:  @ (echo off) or nothing (echo on)             #
#                 Default     : Turn echo off, AT=@                            #
################################################################################

ifndef AT
    AT := @
else    
    # Verify that AT is set to either @ or to nothing    
    ifeq ($(AT),@)
        $(info AT=@)
    else
        ifeq ($(AT),)
            $(info AT=)
        else
            $(error The AT environment variable can only be set to @ to turn off echo or to nothing to turn on echo)	
        endif
    endif	
endif

################################################################################
#                             Common Variables                                 #
################################################################################
# These variables represent common variables needed throughout the makefiles.  #
# These variables are only valid when interpreted by make or the unix/cygwin   #
# shell.  Caution should be used if/when passing them to operating system      #
# specific commands, or when using them in operating system specific ways.     #
# You should normally use the operating system specific versions defined later #
# on in the Operating System section.                                          #
################################################################################

OS_PATHNAME    := $(OS_NAME)

ifdef OS_SUBNAME
  OS_PATHNAME  := $(OS_NAME)/$(OS_SUBNAME)
endif

ROOT           := $(MAKEDIR:/src/$(SUBSYS)=)
SRC            := $(ROOT)/src/$(SUBSYS)
O              := $(ROOT)/obj/$(OS_PATHNAME)/$(SUBSYS)/$(BUILD_TYPE)
REL            := $(ROOT)/rel/$(OS_PATHNAME)/$(SUBSYS)/$(BUILD_TYPE)
PKG            := $(ROOT)/pkg/$(OS_PATHNAME)/$(SUBSYS)/$(BUILD_TYPE)/staf
PKG_ROOT       := $(ROOT)/pkg/$(OS_PATHNAME)/$(SUBSYS)/$(BUILD_TYPE)
SUBSYS_REL_SRC  = $(SRC)/$(SUBSYS_REL)
SR_SRC          = $(SUBSYS_REL_SRC)
LIB_STAF        = $(subst Name,STAF,$(DLL))
LIB_STAF_FP     = $(REL)/lib/$(LIB_STAF)

################################################################################
#                                   Java                                       #
################################################################################
# Note: Below XX refers to the generic version of Java being used.  For        #
#       example, V11 refers to JDK 1.1.x and V12 refers to JDK 1.2.x (or       #
#       higher, as there are no STAF related differences after JDK 1.2.x).     #
################################################################################
# Note: In general (particularly on Unix systems), it is sufficient to set     #
#       the JAVA_VXX_ROOT, JAVA_BUILD_VXX, and JAVA_DEFAULT_VERSION            #
#       variables.  The rest of the variables will be filled in appropriately. #
################################################################################
# JAVA_VXX_ROOT            : Root of JDK install.  This is usually the only    #
#                            thing that need needs to be set.                  #
#                            Default: set in OS include file                   #
#                                                                              #
# JAVA_VXX_BIN_DIR         : Location of Java executables, e.g. javac, etc.    #
#                            Default: $(JAVA_VXX_ROOT)/bin                     #
#                                                                              #
# JAVA_VXX_INCLUDE_DIR     : Location of Java include files                    #
#                            Default: $(JAVA_VXX_ROOT)/include                 #
#                                                                              #
# JAVA_VXX_OS_NAME         : Name by which the JDK knows this operating system #
#                            Default: $(OS_NAME)                               #
#                                                                              #
# JAVA_VXX_INCLUDE_OS_NAME : Name by which the JDK knows this operating system #
#                            with respect to the include directory             #
#                            Default: $(JAVA_VXX_OS_NAME)                      #
#                                                                              #
# JAVA_VXX_LIBDIR_OS_NAME  : Name by which the JDK knows this operating system #
#                            with respect to the lib directory                 #
#                            Default: $(JAVA_VXX_OS_NAME)                      #
#                                                                              #
# JAVA_VXX_INCLUDE_OS_DIR  : Location of OS specific Java include files        #
#                            Default: $(JAVA_VXX_INCLUDE_DIR)/                 #
#                                     $(JAVA_VXX_OS_NAME)                      #
#                                                                              #
# JAVA_VXX_INCLUDEDIRS     : All necessary Java include file directories       #
#                            Default: $(JAVA_VXX_INCLUDE_DIR)                  #
#                                     $(JAVA_VXX_INCLUDE_OS_DIR)               #
#                                                                              #
# JAVA_VXX_CC_FLAGS        : Compiler flags necessary to build JNI code        #
#                            Default: no default                               #
#                                                                              #
# JAVA_VXX_LIBS            : Libraries needed to link Java code                #
#                            Default: java (V11), jvm (V12)                    #
#                                                                              #
# JAVA_VXX_LIBDIRS         : Locations of Java libraries                       #
#                            Default: no default                               #
#                                                                              #
# JAVA_VXX_CLASSPATH       : CLASSPATH of necessary JDK classes                #
#                            Default: $(JAVA_VXX_ROOT)/lib/classes.zip         #
#                                                                              #
# JAVAC_VXX                : The javac command                                 #
#                            Default: $(JAVA_VXX_BIN_DIR)/javac                #
#                                                                              #
# JAVAH_VXX                : The javah command                                 #
#                            Default: $(JAVA_VXX_BIN_DIR)/javah                #
#                                                                              #
# JAR_VXX                  : The jar command                                   #
#                            Default: $(JAVA_VXX_BIN_DIR)/jar                  #
#                                                                              #
# JAVA_DEFAULT_VERSION     : The default version of Java to use for the        #
#                            following paths and commands                      #
#                            Default: V11                                      #
#                                                                              #
# JAVA_CLASSPATH           : The default CLASSPATH of necessary JDK classes    #
#                            Default: $(JAVA_$(JAVA_DEFAULT_VERSION)_CLASSPATH)#
#                                                                              #
# JAVAC                    : The default javac command                         #
#                            Default: $(JAVA_$(JAVA_DEFAULT_VERSION))          #
#                                                                              #
# JAVAH                    : The default javah command                         #
#                            Default: $(JAVA_$(JAVA_DEFAULT_VERSION))          #
#                                                                              #
# JAR                      : The default jar command                           #
#                            Default: $(JAVA_$(JAVA_DEFAULT_VERSION))          #
#                                                                              #
# JAVA_BUILD_VXX           : Wheter this version of java should be built (0/1) #
#                            Default: 0                                        #
################################################################################

JAVA_V11_OS_NAME ?= $(OS_NAME)
JAVA_V11_INCLUDE_OS_NAME ?= $(JAVA_V11_OS_NAME)
JAVA_V11_LIBDIR_OS_NAME ?= $(JAVA_V11_OS_NAME)
JAVA_V11_INCLUDE_DIR ?= $(JAVA_V11_ROOT)/include
JAVA_V11_INCLUDE_OS_DIR ?= $(JAVA_V11_INCLUDE_DIR)/$(JAVA_V11_INCLUDE_OS_NAME)
JAVA_V11_INCLUDEDIRS ?= $(JAVA_V11_INCLUDE_DIR) $(JAVA_V11_INCLUDE_OS_DIR)
JAVA_V11_LIBDIRS ?= $(JAVA_V11_ROOT)/lib/$(JAVA_V11_LIBDIR_OS_NAME)/native_threads
JAVA_V11_LIBS ?= java
JAVA_V11_CLASSPATH ?= $(JAVA_V11_ROOT)/lib/classes.zip
JAVA_V11_BIN_DIR ?= $(JAVA_V11_ROOT)/bin
JAVAC_V11 ?= $(JAVA_V11_BIN_DIR)/javac
JAVA_V11 ?= $(JAVA_V11_BIN_DIR)/java
JAVAH_V11 ?= $(JAVA_V11_BIN_DIR)/javah
JAR_V11 ?= $(JAVA_V11_BIN_DIR)/jar

JAVA_V12_OS_NAME ?= $(OS_NAME)
JAVA_V12_INCLUDE_OS_NAME ?= $(JAVA_V12_OS_NAME)
JAVA_V12_LIBDIR_OS_NAME ?= $(JAVA_V12_OS_NAME)
JAVA_V12_INCLUDE_DIR ?= $(JAVA_V12_ROOT)/include
JAVA_V12_INCLUDE_OS_DIR ?= $(JAVA_V12_INCLUDE_DIR)/$(JAVA_V12_INCLUDE_OS_NAME)
JAVA_V12_INCLUDEDIRS ?= $(JAVA_V12_INCLUDE_DIR) $(JAVA_V12_INCLUDE_OS_DIR)
JAVA_V12_LIBDIRS ?= $(JAVA_V12_ROOT)/jre/bin/classic
JAVA_V12_CLASSPATH ?= $(JAVA_V12_ROOT)/lib/tools.jar
JAVA_V12_BIN_DIR ?= $(JAVA_V12_ROOT)/bin
JAVAC_V12 ?= $(JAVA_V12_BIN_DIR)/javac
JAVA_V12 ?= $(JAVA_V12_BIN_DIR)/java
JAVAH_V12 ?= $(JAVA_V12_BIN_DIR)/javah
JAR_V12 ?= $(JAVA_V12_BIN_DIR)/jar

JAVA_DEFAULT_VERSION ?= V12

JAVA_CLASSPATH ?= $(JAVA_$(JAVA_DEFAULT_VERSION)_CLASSPATH)
JAVAC ?= $(JAVAC_$(JAVA_DEFAULT_VERSION))
JAVA ?= $(JAVA_$(JAVA_DEFAULT_VERSION))
JAVAH ?= $(JAVAH_$(JAVA_DEFAULT_VERSION))
JAR   ?= $(JAR_$(JAVA_DEFAULT_VERSION))

################################################################################
#                                   Rexx                                       #
################################################################################
# REXX_ROOT                : Root of installed Rexx tree                       #
#                            Default: /usr/local/orexx                         #
#                                                                              #
# REXX_BIN_DIR             : Location of rexx executable                       #
#                            Default: $(REXX_ROOT)/bin                         #
#                                                                              #
# REXX_IMAGE_DIR           : Location of rexx execution image files            #
#                            Default: $(REXX_BIN_DIR)                          #
#                                                                              #
# REXX                     : Command used to invoke the Rexx interpreter       #
#                            Default: $(REXX_BIN_DIR)/rexx                     #
#                                                                              #
# REXX_LIBS                : Rexx libraries necessary to link Rexx apps        #
#                            Default: rexx rexxapi                             #
#                                                                              #
# REXX_LIBDIRS             : Locations of Rexx libraries                       #
#                            Default: $(REXX_ROOT)/lib                         #
#                                                                              #
# REXX_INCLUDEDIRS         : Locations of Rexx include files                   #
#                            Default: $(REXX_ROOT)/include                     #
#                                                                              #
# RXPP_ROOT                : Directory where RxPP is installed                 #
#                            Default: /usr/local/rxpp                          #
#                                                                              #
# RXPP                     : Command used to invoke RxPP Rexx Pre-processor    #
#                            Default: $(REXX) $(RXPP_ROOT)/bin/RxPP            #
#                                                                              #
# RXPP_PATH                : Paths to RxPP libraries                           #
#                            Default: $(RXPP_ROOT)/lib $(REL)/lib              #
#                                                                              #
# B2H_ROOT                 : Directory where b2h is installed                  #
#                            Default: /usr/local/b2h                           #
#                                                                              #
# B2H                      : Command used to invoke b2h (Bookie-to-HTML)       #
#                            Default: PATH=$(REXX_IMAGE_DIR):$$PATH $(REXX)    #
#                                     $(B2H_ROOT)/bin/b2h                      #
#                                                                              #
# SAXON_ROOT               : Directory where Saxon is installed                #
#                                                                              #
# DOCBOOK_ROOT             : Directory where DocBook is installed              #
#                                                                              #
################################################################################

REXX_ROOT ?= /usr/local/orexx
REXX_BIN_DIR ?= $(REXX_ROOT)/bin
REXX_IMAGE_DIR ?= $(REXX_BIN_DIR)
REXX ?= $(REXX_BIN_DIR)/rexx
REXX_LIBS ?= rexx rexxapi
REXX_LIBDIRS ?= $(REXX_ROOT)/lib
REXX_INCLUDEDIRS ?= $(REXX_ROOT)/include

RXPP_ROOT ?= /usr/local/rxpp
RXPP ?= $(REXX) $(RXPP_ROOT)/bin/RxPP
RXPP_PATH ?= $(RXPP_ROOT)/lib:$(REL)/lib

B2H_ROOT ?= /usr/local/b2h
B2H ?= PATH=$(REXX_IMAGE_DIR):$$PATH $(REXX) $(B2H_ROOT)/bin/b2h

#SAXON_ROOT ?= /usr/local/saxon
#DOCBOOK_ROOT ?= /usr/local/docbook

################################################################################
#                                   Perl                                       #
################################################################################
# Note: Below XX refers to the generic version of Perl being used.  For        #
#       example, V56 refers to Perl 5.6 and V58 refers to Perl 5.8             #
################################################################################
# PERL_VXX_ROOT            : Root of installed Perl tree                       #
#                            Default: /usr/local/perl                          #
#                                                                              #
# PERL_VXX_LIBS            : Perl libraries necessary to link Perl apps        #
#                            Default: perl                                     #
#                                                                              #
# PERL_VXX_LIBDIRS         : Locations of Perl libraries                       #
#                            Default: $(PERL_ROOT)                             #
#                                                                              #
# PERL_VXX_INCLUDEDIRS     : Locations of Perl include files                   #
#                            Default: $(PERL_ROOT)                             #
#                                                                              #
# PERL_BUILD_VXX           : Wheter this version of Perl should be built (0/1) #
#                            Default: 0                                        #
################################################################################

PERL_V58_ROOT ?= /usr/local/perl
PERL_V58_LIBS ?= perl
PERL_V58_LIBDIRS ?= $(PERL_V58_ROOT)
PERL_V58_INCLUDEDIRS ?= $(PERL_V58_ROOT)
PERL_BUILD_V58 ?= 0
PERL_BUILD_V56 ?= 0
PERL_BUILD_V50 ?= 0

################################################################################
#                                    Tcl                                       #
################################################################################
# TCL_ROOT                 : Root of installed Tcl tree                        #
#                            Default: No default                               #
#                                                                              #
# TCL_INCLUDEDIRS          : Location of Tcl include files                     #
#                            Default: $(TCL_ROOT)/include                      #
#                                                                              #
# TCL_LIBDIRS              : Location of Tcl libraries                         #
#                            Default: $(TCL_ROOT)/libs                         #
#                                                                              #
# TCL_LIBS                 : Tcl libraries necessary to link Tcl apps          #
#                            Default: tcl                                      #
################################################################################

TCL_INCLUDEDIRS ?= $(TCL_ROOT)/include
TCL_LIBDIRS ?= $(TCL_ROOT)/lib
TCL_LIBS ?= tcl

################################################################################
#                                  Python                                      #
################################################################################
# PYTHON_ROOT              : Root of installed Python tree                     #
#                            Default: No default                               #
#                                                                              #
# PYTHON_LIBS              : Python libraries necessary to link Python apps    #
#                            Default: No default                               #
#                                                                              #
# PYTHON_LIBDIRS           : Locations of Python libraries                     #
#                            Default: $(PYTHON_ROOT)/libs                      #
#                                                                              #
# PYTHON_INCLUDEDIRS       : Locations of Python include files                 #
#                            Default: $(PYTHON_ROOT)/include                   #
################################################################################

PYTHON_LIBDIRS ?= $(PYTHON_ROOT)/libs
PYTHON_INCLUDEDIRS ?= $(PYTHON_ROOT)/include

################################################################################
#                                  OpenSSL                                     #
################################################################################
# STAF_USE_SSL             : Indicates to build the STAF TCP connection        #
#                            provider with support for secure tcp in addition  #
#                            to support for non-secure tcp if set to any       #
#                            non-empty value.                                  #
#                            Default: 1                                        #
#									       #
# OPENSSL_ROOT             : Root of installed OpenSSL tree                    #
#                            Default: No default                               #
#                                                                              #
# OPENSSL_LIBDIRS          : Locations of OpenSSL libraries                    #
#                            Default on Unix:    $(OPENSSL_ROOT)/lib           #
#                            Default on Windows: $(OPENSSL_ROOT)/bin           #
#                                                                              #
# OPENSSL_INCLUDEDIRS      : Locations of OpenSSL include files                #
#                            Default: $(OPENSSL_ROOT)/include                  #
#									       #
# Note:  Defaults are assigned in connproviders/tcp/makefile.connprov_tcp      #
################################################################################

STAF_USE_SSL ?= 1

################################################################################
#                                 Commands                                     #
################################################################################
# MAKE_PATH                : The command which will create a directory path.   #
#                            Note: Must support creating intermediate          #
#                                  directories                                 #
#                            Default: mkdir -p                                 #
#                                                                              #
# CREATE_PATH              : Creates the path represented by $(@D)             #
#                            Default: $(MAKE_PATH) $(@D)                       #
#                                                                              #
# COPY                     : Command used to copy a file                       #
#                            Default: cp                                       #
#                                                                              #
# MOVE                     : Command used to move a file                       #
#                            Default: mv                                       #
#                                                                              #
# DEL                      : Command used to delete a file                     #
#                            Default: rm                                       #
#                                                                              #
# DELTREE                  : Command used to delete a directory tree           #
#                            Default: rm -Rf                                   #
#                                                                              #
# TOUCH                    : Command used to create a file or update the       #
#                            timestamp of a file                               #
#                            Default: touch                                    #
#                                                                              #
# OUT_ERR_TO_DEV_NULL      : Syntax to ignore stdout and stderr output         #
#                            Default: >/def/null 2>&1                          #
################################################################################

MAKE_PATH ?= mkdir -p
CREATE_PATH ?= $(MAKE_PATH) $(@D)
COPY ?= cp
RECURSIVE_COPY ?= cp -R
MOVE ?= mv
DEL ?= rm
DELTREE ?= rm -Rf
TOUCH ?= touch
OUT_ERR_TO_DEV_NULL ?= >/dev/null 2>&1

################################################################################
#                        Operating System Variables                            #
################################################################################
# OS_SHARED_LIB_PREFIX     : Prefix used by shared libraries                   #
#                            Default: lib                                      #
#                                                                              #
# OS_SHARED_LIB_SUFFIX     : Suffix used by shared libraries                   #
#                            Default: .so                                      #
#                                                                              #
# OS_LIBS                  : Libraries that the system requires you link with  #
#                            Default: no default                               #
#                                                                              #
# OS_PTHREAD_LIB           : Library that implements threads                   #
#                            Default: pthread                                  #
#                                                                              #
# OS_DL_LIB                : Library that implements dynamic linking           #
#                            Default: dl                                       #
#                                                                              #
# OS_TYPE                  : The type of the operating system                  #
#                            Default: set in OS include file                   #
#                                                                              #
# OS_LD_PATH_NAME          : The name of the "LD_LIBRARY_PATH" variable, which #
#                            dlopen() searches                                 #
#                            Default: LD_LIBRARY_PATH                          #
#                                                                              #
# OS_OE                    : The object file extension                         #
#                            Default: .o                                       #
#                                                                              #
# OS_EE                    : The executable file extension                     #
#                            Default: no default                               #
#                                                                              #
# OS_PS                    : The path separator                                #
#                            Default: ':'                                      #
#                                                                              #
# OS_FS                    : The file separator                                #
#                            Default: '/'                                      #
#                                                                              #
# OS_SRC                   : The operating system specific version of $(SRC)   #
#                            Default: $(SRC)                                   #
#                                                                              #
# OS_REL                   : The operating system specific version of $(REL)   #
#                            Default: $(REL)                                   #
#                                                                              #
# OS_PKG                   : The operating system specific version of $(PKG)   #
#                            Default: $(PKG)                                   #
#                                                                              #
# OS_O                     : The operating system specific version of $(O)     #
#                            Default: $(O)                                     #
#                                                                              #
# OS_OBJS                  : The operating system specific version of $(OBJS)  #
#                            Default: $(OBJS)                                  #
#                                                                              #
# OS_@                     : The operating system specific version of $@       #
#                            Default: $@                                       #
#                                                                              #
# OS_<                     : The operating system specific version of $<       #
#                            Default: $<                                       #
#                                                                              #
# DLL                      : This is a convenient shorthand used to specify    #
#                            the name of a DLL.  Typically usage is            #
#                            JSTAFDLL = $(subst Name,JSTAF,$(DLL))             #
#                                                                              #
################################################################################

OS_SHARED_LIB_PREFIX ?= lib
OS_SHARED_LIB_SUFFIX ?= .so
OS_PTHREAD_LIB       ?= pthread
OS_DL_LIB            ?= dl
OS_LD_PATH_NAME      ?= LD_LIBRARY_PATH
OS_OE                ?= .o
OS_PS                ?= :
OS_FS                ?= /
OS_SRC               ?= $(SRC)
OS_REL               ?= $(REL)
OS_PKG               ?= $(PKG)
OS_O                 ?= $(O)
OS_OJBS              ?= $(OBJS)
OS_@                 ?= $@
OS_<                 ?= $<

DLL = $(OS_SHARED_LIB_PREFIX)Name$(OS_SHARED_LIB_SUFFIX)

################################################################################
#                          C++ Compiler Variables                              #
################################################################################
# Note: The following variables are common to all subordiante makefiles and    #
#       should not generally be changed, unless you REALLY know what you are   #
#       doing.  The compiler should honor these variables.                     #
################################################################################
# COMMON_LIBDIRS           : This specifies common library paths on which the  #
#                            compiler should look.                             #
#                            Default: $(REL)/lib                               #
#                                                                              #
# COMMON_INCLUDEDIRS       : This specifies common include paths on which the  #
#                            compiler should look                              #
#                            Default: $(SUBSYS_REL_SRC)                        #
#                                     $(SUBSYS_REL_SRC)/$(OS_TYPE)             #
#                                     $(SRC)/stafif/$(OS_TYPE) $(SRC)/stafif   #
#                                                                              #
################################################################################
# Note: The following variables may be specified in subordinate makefiles and  #
#       the compiler makefile should honor them.                               #
################################################################################
# LIBS                     : This specifies libraries that the executable or   #
#                            library needs to be linked with                   #
#                            Default: no default                               #
#                                                                              #
# LIBDIRS                  : This specifies library paths on which the         #
#                            compiler should look                              #
#                            Default: no default                               #
#                                                                              #
# INCLUDEDIRS              : This specifies include paths on which the         #
#                            compiler should look                              #
#                            Default: no default                               #
#                                                                              #
# COMMONFLAGS              : This specifies common compiler/linger flags with  #
#                            which the executable/library needs to be          #
#                            compiled/linked                                   #
#                            Default: no default                               #
#                                                                              #
# CFLAGS                   : This specifies compiler flags with which the      #
#                            executable or library needs to be compiled        #
#                            Default: no default                               #
#                                                                              #
# LINKFLAGS                : This specifies link flags with which the          #
#                            executable or library needs to be linked          #
#                            Default: no default                               #
#                                                                              #
# OBJS                     : This specifies the set of files which need to be  #
#                            linked to generate an executable or library       #
#                            Default: no default                               #
################################################################################
# Note: The following variables must be defined by the compiler makefile as    #
#       they are used by the rest of the makefile                              #
################################################################################
# CC_DEPEND_IT             : Used to create a dependency file for a C++ file.  #
#                            This should process the file $< and generate $@.  #
#                                                                              #
# CC_DEPEND_IT_C           : Used to create a dependency file for a C file.    #
#                            This should process the file $< and generate $@.  #
#                                                                              #
# CC_COMPILE_IT            : Used to compile a C++ file.  This should process  #
#                            the file $< and generate $@.                      #
#                                                                              #
# CC_COMPILE_IT_C          : Used to compile a C file.  This should process    #
#                            the file $< and generate $@.                      #
#                                                                              #
# CC_LINK_IT               : Used to link an executable.  This should process  #
#                            $(OBJS) and generate $@.                          #
#                                                                              #
# CC_SHARED_LIB_IT         : Used to link a shared library.  This should       #
#                            process $(OBJS) and generate $@.                  #
################################################################################
# Note: The following variables are not required to be used or implemented by  #
#       any compiler makefile.  They are provided as they are useful for most  #
#       compilers.                                                             #
################################################################################
# ALL_LIB_LIST_RAW         : The set of all necessary link linbraries          #
#                                                                              #
# ALL_LIB_LIST             : The expended set of all necessary link libraries  #
#                                                                              #
# ALL_LIBDIR_LIST_RAW      : The set of all necessary library directories      #
#                                                                              #
# ALL_LIBDIR_LIST          : The expanded set of all necessary library         #
#                            directories                                       #
# ALL_INCLUDEDIR_LIST_RAW  : The set of all necessary include directories      #
#                                                                              #
# ALL_INCLUDEDIR_LIST      : The expanded set of all necessary include         #
#                            directories                                       #
################################################################################

COMMON_LIBDIRS ?= $(REL)/lib
COMMON_INCLUDEDIRS ?= $(SUBSYS_REL_SRC) $(SUBSYS_REL_SRC)/$(OS_TYPE)\
                      $(SRC)/stafif/$(OS_TYPE) $(SRC)/stafif

ALL_LIB_LIST_RAW ?= $(CC_LIBS) $(OS_LIBS) $(LIBS)
ALL_LIB_LIST ?= $(foreach lib, $(ALL_LIB_LIST_RAW),-l$(lib))

ALL_LIBDIR_LIST_RAW ?= $(CC_LIBDIRS) $(OS_LIBDIRS) $(COMMON_LIBDIRS) $(LIBDIRS)
ALL_LIBDIR_LIST ?= $(foreach libdir, $(ALL_LIBDIR_LIST_RAW),-L$(libdir))

ALL_INCLUDEDIR_LIST_RAW ?= $(INIT_INCLUDE) $(CC_INCLUDEDIRS) $(OS_INCLUDEDIRS) $(COMMON_INCLUDEDIRS) $(INCLUDEDIRS)
ALL_INCLUDEDIR_LIST ?= $(foreach incdir, $(ALL_INCLUDEDIR_LIST_RAW),-I$(incdir))

# Setup the library prefix and suffix based on OS as well
# as other OS dependent defines, libraries, flags, etc.

define COPY_FILE
    @$(CREATE_PATH)
    @echo "*** Copying $(@F) ***"
    $(AT)$(COPY) $< $@
endef

define RXPP_IT
    @$(CREATE_PATH)
    @echo "*** Building $(@F) ***"
    @PATH=$(REXX_IMAGE_DIR):$$PATH RXPPPATH='$(subst $(SPACE),$(OS_PS),$(strip $(RXPP_PATH)))' $(RXPP) '$(OS_<)' '$(OS_@)'
endef

define STRIP_IT
    @echo "*** Stripping $(@F) ***"
    $(AT)$(STRIP) $@
endef

define C_DEPEND_IT
    $(AT)$(CREATE_PATH)
    @echo "*** Generating dependency for $(@F) *** "
    $(AT)$(CC_DEPEND_IT)
endef

define C_DEPEND_IT_C
    $(AT)$(CREATE_PATH)
    @echo "*** Generating dependency for $(@F) *** "
    $(AT)$(CC_DEPEND_IT_C)
endef

define COMPILE_IT
    $(AT)$(CREATE_PATH)
    @echo "*** Compiling $(@F) ***"
    $(AT)$(CC_COMPILE_IT)
endef

define COMPILE_IT_C
    $(AT)$(CREATE_PATH)
    @echo "*** Compiling $(@F) ***"
    $(AT)$(CC_COMPILE_IT_C)
endef

define LINK_IT
    $(AT)$(CREATE_PATH)
    @echo "*** Linking $(@F) ***"
    $(AT)$(CC_LINK_IT)
endef

define SHARED_LIB_IT
    $(AT)$(CREATE_PATH)
    @echo "*** Linking shared library $(@F) ***"
    $(AT)$(CC_SHARED_LIB_IT)
    $(AT)$(MOVE_SIDE_DECK)
endef

# Setup our global inference rules

InferenceRules := $(SRC)/build/makefile.inf

# Check to see if we are in cleanup.  If so, we won't want to include
# dependency makefiles

ifneq ($(find cleanup, $(MAKECMDGOALS)), "")
InCleanup := 1
else
inCleanup := 0
endif

# Default target.  This is a fake target that points to the real default which 
# is defined after all the subordinate makefiles have been included

all: real_all 

# Include subordinate makefiles
include $(ALLMAKEFILES)

# Undefine SUBSYS_REL to be safe
SUBSYS_REL := $(EMPTY)

export RXPPPATH

# Targets

showvars:
	@echo LibPref = $(OS_SHARED_LIB_PREFIX)
	@echo LibSuff = $(OS_SHARED_LIB_SUFFIX)
	@echo MakeDir = $(MAKEDIR)
	@echo AllMakeFiles = $(ALLMAKEFILES)
	@echo INCLUDE = $(INCLUDE)

cleanup: $(CleanupTargets)

real_all: $(Targets)
