#!/usr/bin/perl
#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################
###############################################################################
use PLSTAF;
use strict;
###############################################################################
&main; 
###############################################################################
sub main
{
    if ($#ARGV != 2)
    {
        print "Usage: STAF.pl <TCPIP hostname | local> <Service> <Request>\n";
        return;
    }

    my $rc = STAF::Register("STAF/Client/Perl");

    if ($rc != 0)
    {
        die "Error registering with STAF: RC = $STAF::RC\n";
    }

    $rc = STAF::Submit($ARGV[0], $ARGV[1], $ARGV[2]);

    if ($rc != 0)
    {
        print "Error submitting request: RC = $STAF::RC\n".
              "Additional info: $STAF::Result\n";

        STAF::UnRegister();

        return;
    }

    print "Response\n";
    print "--------\n";
    print "$STAF::Result\n";
  
    $rc = STAF::UnRegister();

    if ($rc != 0)
    {
        die "Error unregistering with STAF: RC = $STAF::RC\n";
    }
}
###############################################################################
