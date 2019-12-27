#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###############################################################################
package STAFMon;
require PLSTAF;
###############################################################################

$STAF::Monitor::SystemName            = "LOCAL";
$STAF::Monitor::ServiceName           = "MONITOR";

###############################################################################

sub STAF::Monitor::Log
{
    if ($#_ < 0)
    {
        die "Usage: STAF::Monitor::Log(MESSAGE, [OPTIONS])\n";
    }

    my $message = STAF::WrapData(shift);
    my $options = $#_ >= 0 ? shift : "";

    return STAF::Submit($STAF::Monitor::SystemName, 
                        $STAF::Monitor::ServiceName,
                        "LOG MESSAGE $message $options");
}

###############################################################################
    
sub STAF::STAFMonitor::new
{
    if ($#_ < 1)
    {
        die "Usage: STAF::STAFMonitor->new(HANDLE, [SYSTEMNAME], ".
            "[SERVICENAME])\n";
    }

    my $type = shift;
    my $self = {};

    $self->{handle}   = shift;
    $self->{sysName}  = $#_ >= 0 ? shift : $STAF::Monitor::SystemName;
    $self->{servName} = $#_ >= 0 ? shift : $STAF::Monitor::ServiceName;
    
    return bless $self, $type;
}

###############################################################################

sub STAF::STAFMonitor::log
{
    if ($#_ < 1)
    {
        die "Usage: STAF::STAFMonitor->log(MESSAGE, [OPTIONS])\n";
    }

    my $self    = shift;
    my $message = STAF::WrapData(shift);
    my $options = $#_ >= 0 ? shift : "";

    return $self->{handle}->submit($self->{sysName}, $self->{servName},
                                   "LOG MESSAGE $message $options");
}

###############################################################################

sub STAF::STAFMonitor::getSystemName
{
    my $self = shift;
    return $self->{sysName};
}

###############################################################################

sub STAF::STAFMonitor::getServiceName
{
    my $self = shift;
    return $self->{servName};
}

###############################################################################
1;  # require expects this module to return true (!0)
###############################################################################
