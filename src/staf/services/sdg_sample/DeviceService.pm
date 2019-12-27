#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2007                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

package DeviceService;

use PLSTAFService;
use PLSTAF;
use 5.008;
use threads;
use threads::shared;
use Thread::Queue;

use strict;
use warnings;

use constant kDeviceInvalidSerialNumber => scalar 4001;
use constant kVersion => scalar "1.0.0";

# In this queue the master threads queue jobs for the slave worker
my $work_queue = new Thread::Queue;
my $free_workers : shared = 0;

our $fServiceName;         # passed in part of parms
our $fHandle;               # staf handle for service
our $fListParser;          # .
our $fQueryParser;         # parsers for different requests
our $fAddParser ;          # .
our $fDeleteParser;        # .
our $fLineSep;             # line separator
our %printerMap : shared;  # map to maintain list of printers
our %modemMap : shared;    # map to maintain list of modems

our $listDeviceMapClass;
our $queryDeviceMapClass;

sub new
{
    my ($class, $info) = @_;

    my $self =
    {
        threads_list => [],
        worker_created => 0,
        max_workers => 5, # do not create more than 5 workers
    };

    $fServiceName = $info->{ServiceName};

    $fHandle = STAF::STAFHandle->new("STAF/Service/" . $fServiceName);

    # Add Parser
    $fAddParser = STAFCommandParser->new();

    $fAddParser->addOption("ADD", 1, STAFCommandParser::VALUENOTALLOWED);
    $fAddParser->addOption("PRINTER", 1, STAFCommandParser::VALUEREQUIRED);
    $fAddParser->addOption("MODEL", 1, STAFCommandParser::VALUEREQUIRED);
    $fAddParser->addOption("SN", 1, STAFCommandParser::VALUEREQUIRED);
    $fAddParser->addOption("MODEM", 1, STAFCommandParser::VALUEREQUIRED);

    $fAddParser->addOptionGroup("PRINTER MODEM", 0, 1);

    # if you specify ADD, MODEM is required
    $fAddParser->addOptionNeed("ADD", "MODEL");

    # if you specify ADD, SN is required
    $fAddParser->addOptionNeed("ADD", "SN");

    # if you specify ADD, PRINTER or MODEM is required
    $fAddParser->addOptionNeed("PRINTER MODEM", "ADD");

    # if you specify PRINTER or MODEM, ADD is required
    $fAddParser->addOptionNeed("ADD", "PRINTER MODEM");

    # LIST parser
    $fListParser = STAFCommandParser->new();

    $fListParser->addOption("LIST", 1, STAFCommandParser::VALUENOTALLOWED);
    $fListParser->addOption("PRINTERS", 1, STAFCommandParser::VALUENOTALLOWED);
    $fListParser->addOption("MODEMS", 1, STAFCommandParser::VALUENOTALLOWED);

    # QUERY parser
    $fQueryParser = STAFCommandParser->new();

    $fQueryParser->addOption("QUERY", 1, STAFCommandParser::VALUENOTALLOWED);
    $fQueryParser->addOption("PRINTER", 1, STAFCommandParser::VALUEREQUIRED);
    $fQueryParser->addOption("MODEM", 1, STAFCommandParser::VALUEREQUIRED);

    # this means you can have PRINTER or MODEM, but not both
    $fQueryParser->addOptionGroup("PRINTER MODEM", 0, 1);

    # if you specify PRINTER or MODEM, QUERY is required
    $fQueryParser->addOptionNeed("PRINTER MODEM", "QUERY");

    # if you specify QUERY, PRINTER or MODEM is required
    $fQueryParser->addOptionNeed("QUERY", "PRINTER MODEM");

    # DELETE parser
    $fDeleteParser = STAFCommandParser->new();

    $fDeleteParser->addOption("DELETE", 1, STAFCommandParser::VALUENOTALLOWED);
    $fDeleteParser->addOption("PRINTER", 1, STAFCommandParser::VALUEREQUIRED);
    $fDeleteParser->addOption("MODEM", 1, STAFCommandParser::VALUEREQUIRED);
    $fDeleteParser->addOption("CONFIRM", 1,
                                STAFCommandParser::VALUENOTALLOWED);

    # this means you must have PRINTER or MODEM, but not both
    $fDeleteParser->addOptionGroup("PRINTER MODEM", 0, 1);

    # if you specify PRINTER or MODEM, DELETE is required
    $fDeleteParser->addOptionNeed("PRINTER MODEM", "DELETE");

    # if you specify DELETE, PRINTER or MODEM is required
    $fDeleteParser->addOptionNeed("DELETE", "PRINTER MODEM");

    # if you specify DELETE, CONFIRM is required
    $fDeleteParser->addOptionNeed("DELETE", "CONFIRM");

    # construct map class for the result from a LIST request.

    $listDeviceMapClass  =
        STAF::STAFMapClassDefinition->new('STAF/Service/Device/ListDevice');
    $listDeviceMapClass->addKey('name', 'Name');
    $listDeviceMapClass->addKey('type', 'Type');
    $listDeviceMapClass->addKey('model', 'Model');
    $listDeviceMapClass->addKey('serial#', 'Serial Number');
    $listDeviceMapClass->setKeyProperty(
        "serial#", "display-short-name", "Serial #");

    # construct map class for the result from a QUERY request.

    $queryDeviceMapClass  =
        STAF::STAFMapClassDefinition->new('STAF/Service/Device/QueryDevice');
    $queryDeviceMapClass->addKey('name', 'Name');
    $queryDeviceMapClass->addKey('type', 'Type');
    $queryDeviceMapClass->addKey('model', 'Model');
    $queryDeviceMapClass->addKey('serial#', 'Serial Number');
    $queryDeviceMapClass->setKeyProperty(
        "serial#", "display-short-name", "Serial #");

    my $lineSepResult = $fHandle->submit2($STAF::STAFHandle::kReqSync,
        "local", "var", "resolve string {STAF/Config/Sep/Line}");

    $fLineSep = $lineSepResult->{result};

    registerHelpData(kDeviceInvalidSerialNumber, "Invalid Serial Number",
                     "A non-numeric value was specified for serial number");

    return bless $self, $class;
}

sub AcceptRequest
{
    my ($self, $info) = @_;
    my %hash : shared = %$info;

    if ($free_workers <= 0 and
        $self->{worker_created} < $self->{max_workers})
    {
        my $thr = threads->create(\&Worker);
        push @{ $self->{threads_list} }, $thr;
        $self->{worker_created}++;
    }
    else
    {
        lock $free_workers;
        $free_workers--;
    }

    $work_queue->enqueue(\%hash);

    return $STAF::DelayedAnswer;
}

sub Worker
{
    my $loop_flag = 1;

    while ($loop_flag)
    {
        eval
        {
            # get the work from the queue
            my $hash_ref = $work_queue->dequeue();

            if (not ref($hash_ref) and $hash_ref->{request} eq 'stop')
            {
                $loop_flag = 0;
                return;
            }

            my ($rc, $result) = handleRequest($hash_ref);

            STAF::DelayedAnswer($hash_ref->{requestNumber}, $rc, $result);

            # increase the number of free threads
            {
                lock $free_workers;
                $free_workers++;
            }
        }
    }

    return 1;
}

sub handleRequest
{
    my $info = shift;

    my $lowerRequest = lc($info->{request});
    my $requestType = "";

    # get first "word" in request
    if($lowerRequest =~ m/\b(\w*)\b/)
    {
        $requestType = $&;
    }
    else
    {
        return (STAFResult::kInvalidRequestString,
            "Unknown DeviceService Request: " . ($info->{request}));
    }

    if ($requestType eq "list")
    {
        return handleList($info);
    }
    elsif ($requestType eq "query")
    {
        return handleQuery($info);
    }
    elsif ($requestType eq "add")
    {
        return handleAdd($info);
    }
    elsif ($requestType eq "delete")
    {
        return handleDelete($info);
    }
    elsif ($requestType eq "help")
    {
        return handleHelp();
    }
    elsif ($requestType eq "version")
    {
        return handleVersion();
    }
    else
    {
        return (STAFResult::kInvalidRequestString,
            "Unknown DeviceService Request: " . $info->{request});
    }

    return (0, "");
}

sub handleVersion
{
    return (STAFResult::kOk, kVersion);
}

sub handleHelp
{
    return (STAFResult::kOk,
          "DeviceService Service Help" . $fLineSep
          . $fLineSep . "ADD     (PRINTER <printerName> | MODEM <modemName>)" .
          " MODEL <model> SN <serial#>"
          . $fLineSep . "DELETE  PRINTER <printerName> | MODEM <modemName> " .
          "CONFIRM"
          . $fLineSep . "LIST    [PRINTERS] [MODEMS]"
          . $fLineSep . "QUERY   PRINTER <printerName> | MODEM <modemName>"
          . $fLineSep . "VERSION" . $fLineSep . "HELP");

}

sub handleAdd
{
    my $info = shift;

    if($info->{trustLevel} < 3)
    {
        return (STAFResult::kAccessDenied,
           "Trust level 3 required for ADD request. Requesting " . 
           "machine's trust level: " . $info->{trustLevel});
    }

    my $result;
    my $resolveResult;
    my $resultString = "";
    my $printerValue = "";
    my $modemValue = "";
    my $modelValue = "";
    my $snValue;

    # parse request
    my $parsedRequest = $fAddParser->parse($info->{request});

    # check results of parse
    if($parsedRequest->{"rc"} != STAFResult::kOk)
    {
        return (STAFResult::kInvalidRequestString,
            $parsedRequest->{"errorBuffer"});
    }

    # resolve the value after 'printer' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("printer"),
                                  $info->{requestNumber});

    # check results of resolve
    if ($resolveResult->{"rc"} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $printerValue = $resolveResult->{"result"};

    # resolve the value after 'modem' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("modem"),
                                  $info->{requestNumber});

    # check the results of resovle
    if ($resolveResult->{"rc"} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $modemValue = $resolveResult->{"result"};

    # resolve the value after 'model' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("model"),
                                  $info->{requestNumber});

    if ($resolveResult->{"rc"} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $modelValue = $resolveResult->{"result"};

    # resolve the value after 'sn' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("sn"),
                                  $info->{requestNumber});

    if ($resolveResult->{"rc"} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    # make sure the result is a number
    if($resolveResult->{result} =~ m/^[0-9]+$/)
    {
        $snValue = $resolveResult->{"result"};
    }
    else
    {
        return (kDeviceInvalidSerialNumber, $snValue);
    }

    # add the printer to the map
    if ($printerValue ne "")
    {
        lock(%printerMap);
        $printerMap{$printerValue} =
            DeviceData->new($modelValue, $snValue);
    }
    # if no printer value, add the modem to the map
    elsif ($modemValue ne "")
    {
        lock(%modemMap);
        $modemMap{$modemValue} = DeviceData->new($modelValue, $snValue);
    }
    else
    {
        # this should only happen when the option value resolves
        # to an empty string
        return (STAFResult::kInvalidRequestString,
            "Device name resolved to empty string");
    }

    return (STAFResult::kOk, $resultString);
}

sub handleList
{
    my $info = shift;

    if($info->{trustLevel} < 2)
    {
        return (STAFResult::kAccessDenied,
            "Trust level 2 required for LIST request. Requesting " .
            "machine's trust level: " .  $info->{trustLevel});
    }

    my $result = (STAFResult::kOk, "");
    my $resultString = "";

    my $printersOption;
    my $modemsOption;

    # parse request
    my $parsedRequest = $fListParser->parse($info->{request});

    # check result of parse
    if ($parsedRequest->{rc} != STAFResult::kOk)
    {
        return (STAFResult::kInvalidRequestString,
            $parsedRequest->{errorBuffer});
    }

    # create a marshalling context with testList and one map class definition

    my $mc = STAF::STAFMarshallingContext->new();
    $mc->setMapClassDefinition($listDeviceMapClass);

    my @myDeviceList;

    # is 'printers' specified?
    $printersOption = $parsedRequest->optionTimes("printers");

    # is 'modems' specified?
    $modemsOption = $parsedRequest->optionTimes("modems");

    my $defaultList = 0;

    # if neither is specified, default is to show everything
    if ($printersOption == 0 && $modemsOption == 0)
    {
        $defaultList = 1;
    }

    # list all the printers
    if ($defaultList || ($printersOption > 0))
    {
        # XXX: does it make sense to have this lock here?
        # In perl, sometimes accessing vars in a way that
        # seems to be leave the variable unchanged on the surface
        # actually changes things behind the scenes, but it's
        # "hard to know when" according to perl docs
        lock(%printerMap);
        foreach my $key (sort keys %printerMap)
        {
           my $data = $printerMap{$key};
           my $deviceMap = $listDeviceMapClass->createInstance();
           $deviceMap->{'name'} = $key;
           $deviceMap->{'type'} = 'Printer';
           $deviceMap->{'model'} = $data->{'model'};
           $deviceMap->{'serial#'} = $data->{'sn'};
           push @myDeviceList, $deviceMap;
        }
    }

    # list all the modems
    if ($defaultList || ($modemsOption > 0))
    {
        # XXX: does it make sense to have this lock here?
        # see above comment
        lock(%modemMap);
        foreach my $key (sort keys %modemMap)
        {
           my $data = $modemMap{$key};
           my $deviceMap = $listDeviceMapClass->createInstance();
           $deviceMap->{'name'} = $key;
           $deviceMap->{'type'} = 'Modem';
           $deviceMap->{'model'} = $data->{'model'};
           $deviceMap->{'serial#'} = $data->{'sn'};
           push @myDeviceList, $deviceMap;
        }
    }

    $mc->setRootObject(\@myDeviceList);

    return (STAFResult::kOk, $mc->marshall());
}

sub handleQuery
{
    my $info = shift;

    # check whether Trust level is sufficient for this command.
    if ($info->{trustLevel} < 2)
    {
        return (STAFResult::kAccessDenied,
            "Trust level 2 required for QUERY request. Requesting " .
            "machine's trust level: " .  $info->{trustLevel});
    }

    my $result = (STAFResult::kOk, "");
    my $resultString = "";

    my $resolveResult;

    my $printerValue;
    my $modemValue;

    # parse request
    my $parsedRequest = $fQueryParser->parse($info->{request});

    # check result of parse
    if ($parsedRequest->{rc} != STAFResult::kOk)
    {
        return (STAFResult::kInvalidRequestString,
                              $parsedRequest->{errorBuffer});
    }

    # resolve value after 'printer' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("printer"),
                                  $info->{requestNumber});

    if ($resolveResult->{rc} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $printerValue = $resolveResult->{result};

    # resolve the result after 'modem' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("modem"),
                                  $info->{requestNumber});

    if ($resolveResult->{rc} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $modemValue = $resolveResult->{result};

    # create a marshalling context with testList and one map class definition

    my $mc = STAF::STAFMarshallingContext->new();
    $mc->setMapClassDefinition($queryDeviceMapClass);

    my $deviceMap = $queryDeviceMapClass->createInstance();

    # look up the information associated with $printerValue or $modemValue
    if ($printerValue ne "")
    {
        lock(%printerMap);

        if (defined $printerMap{$printerValue})
        {
            my $printer = $printerValue;
            my $data = $printerMap{$printerValue};
            $deviceMap->{'name'} = $printer;
            $deviceMap->{'type'} = 'Printer';
            $deviceMap->{'model'} = $data->{'model'};
            $deviceMap->{'serial#'} = $data->{'sn'};
        }
        else
        {
            return (STAFResult::kDoesNotExist,
                $printerValue);
        }
    }
    elsif ($modemValue ne (""))
    {
        lock(%modemMap);

        if (defined $modemMap{$modemValue})
        {
            my $modem = $modemValue;
            my $data = $modemMap{$modem};
            $deviceMap->{'name'} = $modem;
            $deviceMap->{'type'} = 'Modem';
            $deviceMap->{'model'} = $data->{'model'};
            $deviceMap->{'serial#'} = $data->{'sn'};
        }
        else
        {
            return (STAFResult::kDoesNotExist,
                $modemValue);
        }
    }
    else
    {
        # this should only happen when the option value resolves to
        # an empty string
        return (STAFResult::kInvalidRequestString,
            "Device name resolved to empty string");
    }

    $mc->setRootObject($deviceMap);

    return (STAFResult::kOk, $mc->marshall());
 }

sub handleDelete
{
    my $info = shift;

    # check whether Trust level is sufficient for this command.
    if ($info->{trustLevel} < 4)
    {
        return (STAFResult::kAccessDenied,
            "Trust level 4 required for DELETE request. Requesting " .
            "machine's trust level: " .  $info->{trustLevel});
    }

    my $result = (STAFResult::kOk, "");

    my $resultString = "";
    my $resolveResult;

    my $printerValue;
    my $modemValue;

    # parse request
    my $parsedRequest = $fDeleteParser->parse($info->{request});

    # check results of parse
    if ($parsedRequest->{rc} != STAFResult::kOk)
    {
        return (STAFResult::kInvalidRequestString,
                              $parsedRequest->{errorBuffer});
    }

    # resolve value after 'printer' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("printer"),
                                  $info->{requestNumber});

    if ($resolveResult->{rc} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $printerValue = $resolveResult->{result};

    # resolve value after 'modem' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("modem"),
                                  $info->{requestNumber});

    if ($resolveResult->{rc} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $modemValue = $resolveResult->{result};

    # delete printer or modem if it can find it
    if ($printerValue ne "")
    {
        lock(%printerMap);

        if(defined $printerMap{$printerValue})
        {
            delete $printerMap{$printerValue};
        }
        else
        {
            return (STAFResult::kDoesNotExist,
                $printerValue);
        }
    }
    elsif ($modemValue ne "")
    {
        lock(%modemMap);

        if(defined $modemMap{$modemValue})
        {
            delete $modemMap{$modemValue};
        }
        else
        {
            return (STAFResult::kDoesNotExist, $modemValue);
        }
    }
    else
    {
        return (STAFResult::kInvalidRequestString,
            "Device name resolved to empty string");
    }

    return (STAFResult::kOk, $resultString);
}

# This method will resolve any STAF variables that
# are contained within the Option Value
sub resolveVar
{
    my ($machine, $optionValue, $requestNumber) = @_;
    my $value = "";
    my $resolvedResult;

    # look for something starting with '{'
    if ($optionValue =~ m/^\{/)
    {
        $resolvedResult =
            $fHandle->submit2($machine, "var", "resolve request " . 
                $requestNumber . " string " . $optionValue);

        if ($resolvedResult->{rc} != 0)
        {
            return $resolvedResult;
        }

        $value = $resolvedResult->{result};
    }
    else
    {
        $value = $optionValue;
    }

    return STAF::STAFResult->new(STAFResult::kOk, $value);
}

# Register error codes for the Device Service with the HELP service
sub registerHelpData
{
   my ($errorNumber, $info, $description) = @_;

   my $res = $fHandle->submit2($STAF::STAFHandle::kReqSync, "local", "HELP",
                      "REGISTER SERVICE " . $fServiceName .
                      " ERROR " . $errorNumber .
                      " INFO " . STAF::WrapData($info) .
                      " DESCRIPTION " .
                      STAF::WrapData($description));
}

#Un-register error codes for the Device Service with the HELP service
sub unregisterHelpData
{
   my $errorNumber = shift;

   my $res = $fHandle->submit2($STAF::STAFHandle::kReqSync, "local", "HELP",
                   "UNREGISTER SERVICE " . $fServiceName .
                   " ERROR " . $errorNumber);
}

sub DESTROY
{
    my ($self) = @_;

    # Ask all the threads to stop, and join them.
    for my $thr (@{ $self->{threads_list} })
    {
        $work_queue->enqueue('stop');
    }

    # perform any cleanup for the service here

    unregisterHelpData(kDeviceInvalidSerialNumber);

    #Un-register the service handle
    $fHandle->unRegister();

# XXX:  The following block will cause a trap if multiple
# STAFCommandParsers have been created
#    for my $thr (@{ $self->{threads_list} })
#    {
#        eval { $thr->join() };
#        print STDERR "On destroy: $@\n" if $@;
#    }
}

package DeviceData;

sub new
{
   my $type = shift;
   my $model : shared = shift;
   my $sn : shared = shift;

   my $obj = &threads::shared::share({});

   $obj->{model} = $model;
   $obj->{sn} = $sn;

   bless ($obj, $type);

   return $obj;
}

1;