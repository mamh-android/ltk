#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2010                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

package GetEnvVar;

use PLSTAFService;
use PLSTAF;
use 5.008;
use threads;
use threads::shared;
use Thread::Queue;

use strict;

use constant kVersion => scalar "1.0.0";

# In this queue the master threads queue jobs for the slave worker
my $work_queue = new Thread::Queue;
my $free_workers : shared = 0;

our $fServiceName;         # passed in part of parms
our $fHandle;              # staf handle for service
our $fGetParser;           # GET parser
our $fLineSep;             # line separator

our $listMapClass;

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

    # GET Parser
    $fGetParser = STAFCommandParser->new();

    $fGetParser->addOption("GET", 1, STAFCommandParser::VALUENOTALLOWED);
    $fGetParser->addOption("ENVVAR", 1, STAFCommandParser::VALUEREQUIRED);

    $fGetParser->addOptionNeed("GET", "ENVVAR");
    $fGetParser->addOptionNeed("ENVVAR", "GET");

    # construct map class for the result from a LIST request.

    $listMapClass  =
        STAF::STAFMapClassDefinition->new('STAF/Service/GetEnvVar/List');
    $listMapClass->addKey('name', 'Name');
    $listMapClass->addKey('value', 'Value');

    my $lineSepResult = $fHandle->submit2($STAF::STAFHandle::kReqSync,
        "local", "var", "resolve string {STAF/Config/Sep/Line}");

    $fLineSep = $lineSepResult->{result};

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
            "Unknown GetEnvVarService Request: " . ($info->{request}));
    }

    if ($requestType eq "get")
    {
        return handleGet($info);
    }
    elsif ($requestType eq "list")
    {
        return handleList($info);
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
            "Unknown GetEnvVarService Request: " . $info->{request});
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
            "GetEnvVar Service Help" . $fLineSep
            . $fLineSep . "GET     ENVVAR <varName>"
            . $fLineSep . "LIST"
            . $fLineSep . "VERSION" . $fLineSep . "HELP");

}

sub handleGet
{
    my $info = shift;

    if($info->{trustLevel} < 3)
    {
        return (STAFResult::kAccessDenied,
           "Trust level 3 required for GET request. Requesting " . 
           "machine's trust level: " . $info->{trustLevel});
    }

    my $result;
    my $resolveResult;
    my $resultString = "";
    my $envVarName = "";
    my $envVarValue = "";

    # parse request
    my $parsedRequest = $fGetParser->parse($info->{request});

    # check results of parse
    if($parsedRequest->{"rc"} != STAFResult::kOk)
    {
        return (STAFResult::kInvalidRequestString,
            $parsedRequest->{"errorBuffer"});
    }

    # resolve the value after 'ENVVAR' if necessary
    $resolveResult = resolveVar($info->{isLocalRequest},
                                  $parsedRequest->optionValue("ENVVAR"),
                                  $info->{requestNumber});

    # check results of resolve
    if ($resolveResult->{"rc"} != STAFResult::kOk)
    {
        return $resolveResult;
    }

    $envVarName = $resolveResult->{"result"};

    my %keys;
    %keys = keys(%ENV);

    $envVarValue = $ENV{$envVarName};

    if ($envVarValue eq '')
    {
        if (exists $keys{$envVarName})
        {
            # Variables exists but is set to blank
            return (STAFResult::kOk, $envVarValue);
        }
        else
        {
            return (STAFResult::kDoesNotExist, $envVarName);
        }
    }
    else
    {
        return (STAFResult::kOk, $envVarValue);
    }
}

sub handleList
{
    my $info = shift;

    if($info->{trustLevel} < 3)
    {
        return (STAFResult::kAccessDenied,
           "Trust level 3 required for LIST request. Requesting " . 
           "machine's trust level: " . $info->{trustLevel});
    }

    my $mc = STAF::STAFMarshallingContext->new();
    $mc->setMapClassDefinition($listMapClass);

    my @myEnvVarList;
    my $key;

    foreach $key (sort keys(%ENV))
    {
        my $envVarMap = $listMapClass->createInstance();
        $envVarMap->{'name'} = $key;
        $envVarMap->{'value'} = $ENV{$key};
        push @myEnvVarList, $envVarMap;
    }

    $mc->setRootObject(\@myEnvVarList);

    return (STAFResult::kOk, $mc->marshall());
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


sub DESTROY
{
    my ($self) = @_;

    # Ask all the threads to stop, and join them.
    for my $thr (@{ $self->{threads_list} })
    {
        $work_queue->enqueue('stop');
    }

    #Un-register the service handle
    $fHandle->unRegister();
}

1;