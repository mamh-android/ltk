#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

STAF_INCLUDE_DIR = -I /usr/local/staf/include
TCLLIBS = -ltcl

all: libTCLSTAF.so

libTCLSTAF.so: TCLSTAF.cpp
	gcc -shared -o libTCLSTAF.so $(STAF_INCLUDE_DIR) $(TCLLIBS) TCLSTAF.cpp
