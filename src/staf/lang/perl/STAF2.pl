#!/usr/bin/perl
#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
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
        print "Usage: STAF2.pl <TCPIP hostname | local> <Service> <Request>\n";
        return;
    }

    my $handle = STAFHandle->new("STAF/Client/Perl");

    if ($handle->{rc} != 0)
    {
        die "Error registering with STAF: RC = $handle->{rc}\n";
    }

    my $result = $handle->submit($ARGV[0], $ARGV[1], $ARGV[2]);

    if ($result->{rc} != 0)
    {
        print "Error submitting request: RC = $result->{rc}\n".
              "Additional info: $result->{result}\n";

        $handle->unRegister();

        return;
    }

    print "Response\n";
    print "--------\n";
    print "$result->{result}\n";
  
    my $rc = $handle->unRegister();

    if ($rc != 0)
    {
        die "Error unregistering with STAF: RC = $rc\n";
    }
}
###############################################################################
