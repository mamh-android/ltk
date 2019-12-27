#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###################################################
# STAFMon.tcl - STAF Monitor REXX Library         #
###################################################

package provide STAFMon 1.0
package require STAF

namespace eval STAF::Monitor {

    variable SystemName              local  
    variable ServiceName             monitor

}

##############################################################
# STAF::Monitor::Log - Logs data to the STAF Monitor Service #
#                                                            #
# Accepts: The message to log to the Monitor service         #
#          Optionally, an extra Monitor LOG options          #
#                                                            #
# Returns: 0 , if successful                                 #
#          >0, if not successful                             #
##############################################################

proc STAF::Monitor::Log {message {options ""}} {

  return [STAF::Submit $STAF::Monitor::SystemName $STAF::Monitor::ServiceName \
                       "LOG MESSAGE [STAF::WrapData $message] $options"]
}
