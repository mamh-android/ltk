#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###############################
# STAF TCL Package Index File #
###############################

package ifneeded TCLSTAF 1.0 "load [list [file join $dir LIB_TCLSTAF]]"
package ifneeded STAF 3.0 "source [list [file join $dir STAFUtil.tcl]]"
package ifneeded STAFMon 1.0 "source [list [file join $dir STAFMon.tcl]]"
package ifneeded STAFLog 1.0 "source [list [file join $dir STAFLog.tcl]]"
