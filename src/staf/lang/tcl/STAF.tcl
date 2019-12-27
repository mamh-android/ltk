#!/usr/bin/tcl
#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################
package require TCLSTAF

if {$argc != 3} {
    puts "Usage: $argv0 location service request"
    exit 1
}

set location [lindex $argv 0]
set service [lindex $argv 1]
set request [lindex $argv 2]

if {[STAF::Register "STAF/Client/TCL"] != 0} {
    puts "Error registering with STAF, RC: $STAF::RC"
    exit $STAF::RC
}

if {[STAF::Submit $location $service $request] != 0} {
    puts "Error submitting request, RC: $STAF::RC"

    if {[string length $STAF::Result] != 0} {
        puts "Additional info: $STAF::Result"
    }
} else {
    puts "Response"
    puts "--------"
    puts $STAF::Result
}

STAF::UnRegister

exit $STAF::RC
