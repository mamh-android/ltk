#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

###############################################################################
package STAFLog;
require PLSTAF;
require STAFMon;
###############################################################################

$STAF::Log::kInvalidLevel                = 4004;
$STAF::Log::kInvalidLogFileFormat        = 4007;
$STAF::Log::kPurgeFailure                = 4008;

$STAF::Log::SystemName                   = "LOCAL";
$STAF::Log::ServiceName                  = "LOG";
$STAF::Log::LogName                      = "";
$STAF::Log::LogType                      = "";
$STAF::Log::MonitorMask                  = "";

###############################################################################

sub STAF::Log::Init
{
    if ($#_ < 0)
    {
        die "Usage: STAF::Log::Init(NAME, [TYPE], [MASK])\n";
    }

    $STAF::Log::LogName     = shift;
    $STAF::Log::LogType     = $#_ >= 0 ? shift : "MACHINE";
    $STAF::Log::MonitorMask = $#_ >= 0 ? uc(shift)
                                       : "FATAL ERROR WARNING INFO STATUS";
    return 0;
}

###############################################################################

sub STAF::Log::Log
{
    if ($#_ < 1)
    {
        die "Usage: STAF::Log::Log(LEVEL, MESSAGE, [OPTIONS])\n";
    }

    my $logLevel   = shift;
    my $logMessage = STAF::WrapData(shift);
    my $logOptions = $#_ >= 0 ? shift : "";

    my $logRequest = "LOG $STAF::Log::LogType ".
                     "LOGNAME ".STAF::WrapData($STAF::Log::LogName)." ".
                     "LEVEL $logLevel MESSAGE $logMessage $logOptions";

    my $rc = STAF::Submit($STAF::Log::SystemName, $STAF::Log::ServiceName,
                          $logRequest);
    if ($rc != 0) 
    {
        STAF::Monitor::Log("Logging failed, RC: $rc, on message: $logMessage");
    }
    elsif (index($STAF::Log::MonitorMask, uc($logLevel)) >= 0)
    {
        STAF::Monitor::Log($logMessage);
    }

    return $rc;
}

###############################################################################

sub STAF::STAFLog::new
{
    if ($#_ < 2)
    {
        die "Usage: STAF::STAFLog->new(HANDLE, NAME, [TYPE], [MASK], ".
            "[SYSTEMNAME] [SERVICENAME])\n";
    }

    my $type = shift;
    my $self = {};

    $self->{handle}      = shift;
    $self->{logName}     = shift;
    $self->{logType}     = $#_ >= 0 ? shift : "MACHINE";
    $self->{monitorMask} = $#_ >= 0 ? uc(shift)
                                    : "FATAL ERROR WARNING INFO STATUS";
    $self->{sysName}     = $#_ >= 0 ? shift : $STAF::Log::SystemName;
    $self->{servName}    = $#_ >= 0 ? shift : $STAF::Log::ServiceName;

    return bless $self, $type;
}

###############################################################################

sub STAF::STAFLog::log
{
    if ($#_ < 2)
    {
        die "Usage: STAF::STAFLog->log(LEVEL, MESSAGE, [OPTIONS])\n";
    }

    my $self       = shift;
    my $logLevel   = shift;
    my $logMessage = STAF::WrapData(shift);
    my $logOptions = $#_ >= 0 ? shift : "";

    my $logRequest = "LOG $self->{logType} ".
                     "LOGNAME ".STAF::WrapData($self->{logName})." ".
                     "LEVEL $logLevel MESSAGE $logMessage $logOptions";

    my $result = $self->{handle}->submit($self->{sysName}, 
                                         $self->{servName},
                                         $logRequest);
    if ($result->{rc} != 0) 
    {
        STAF::Monitor::Log("Logging failed, RC: $result->{rc}, on ".
                           "message: $logMessage");
    }
    elsif (index($self->{monitorMask}, uc($logLevel)) >= 0)
    {
        STAF::Monitor::Log($logMessage);
    }

    return $result;
}

###############################################################################

sub STAF::STAFLog::getName
{
    my $self = shift;
    return $self->{logName};
}

###############################################################################

sub STAF::STAFLog::getLogType
{
    my $self = shift;
    return $self->{logType};
}

###############################################################################

sub STAF::STAFLog::getMonitorMask
{
    my $self = shift;
    return $self->{monitorMask};
}

###############################################################################

sub STAF::STAFLog::getSystemName
{
    my $self = shift;
    return $self->{sysName};
}

###############################################################################

sub STAF::STAFLog::getServiceName
{
    my $self = shift;
    return $self->{servName};
}

###############################################################################
1;  # require expects this module to return true (!0)
###############################################################################
