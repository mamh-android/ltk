#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###################################################
# STAFLog.tcl - STAF Logging TCL Library          #
###################################################

package provide STAFLog 1.0
package require STAF

namespace eval STAF::Log {

    variable kInvalidLevel              4004
    variable kInvalidLogFileFormat      4007
    variable kPurgeFailure              4008

    variable SystemName                 local
    variable ServiceName                log
    variable LogName                    ""
    variable LogType
    variable MonitorMask

}

####################################################################
# STAF::Log::Init - Initializes a logging structure used to log    #
#                   data to the STAF Log Service                   #
#                                                                  # 
# Accepts: The name of the log to write to                         #
#          Optionally, the type of the log (default = "MACHINE")   #
#          Optionally, a mask used to indicate which logging       #
#            levels should also be sent to the STAF Monitor        #
#            Service (default = "FATAL ERROR WARNING INFO STATUS") #
#                                                                  #
# Returns: 0 , if successful                                       #
#          >0, if not successful                                   #
####################################################################
proc STAF::Log::Init { logName { logType "MACHINE" } \
            { monitorMask "FATAL ERROR WARNING INFO STATUS" } } {

  set STAF::Log::LogName $logName
  set STAF::Log::LogType $logType
  set STAF::Log::MonitorMask [string toupper $monitorMask]

  return 0
}


################################################################
# STAF::Log::Log - Logs a message to the STAF Log Service      #
#                                                              #
# Accepts: The logging level                                   #
#          The message to log                                  #
#          Optionally, any extra parameters to the LOG command #
#                                                              #
# Returns: 0 , if successful                                   #
#          >0, if not successful                               #
################################################################
proc STAF::Log::Log {logLevel logMessage {logOptions ""} } { 

  set logRC [STAF::Submit $STAF::Log::SystemName $STAF::Log::ServiceName \
                          "log $STAF::Log::LogType
                          logname [STAF::WrapData $STAF::Log::LogName]
                          level $logLevel message [STAF::WrapData $logMessage]
                          $logOptions"]

  if {$STAF::RC != 0} {
      STAF::Monitor::Log "Logging failed, RC: $STAF::RC, on message: $logMessage"
  } elseif {[string first [string toupper $logLevel] \
             $STAF::Log::MonitorMask] != -1} {
      STAF::Monitor::Log [string toupper $logLevel]:$logMessage $logOptions
  }

  return $logRC
}
