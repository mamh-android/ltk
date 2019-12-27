#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2002                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

use PLSTAF;
use STAFMon;
use STAFLog;

# Register with STAF and get a STAF handle

$handle = STAF::STAFHandle->new("Lang/Perl/Test/Basic"); 

if ($handle->{rc} != $STAF::kOk) { 
    print "Error registering with STAF, RC: $handle->{rc}\n"; 
    exit $handle->{rc};
} 

print "Using handle: $handle->{handle}\n";

print "\nTesting Class STAFHandle Methods...\n";

# Test the STAFHandle->submit API

$result = $handle->submit("local", "ping", "ping");

if (($result->{rc} != $STAF::kOk) or ($result->{result} != "PONG")) {
    print "Error on ping request.\n";
    print "Expected RC: 0, Result: PONG\n";
    print "Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

$result = $handle->submit("local", "var", "resolve string {STAF/Config/Machine}");

if ($result->{rc} != $STAF::kOk) { 
    print "Error resolving machine, RC: $result->{rc}, Result: $result->{result}\n"; 
    exit $result->{rc};
}

print "  Verify that auto-unmarshalling result is turned on by default\n";

# Test the STAFHandle->getDoUnmarshallResult API

if ($handle->getDoUnmarshallResult() != 1) {
    print "ERROR: handle->getDoUnmarshallResult() != 1\n";
    print "Found: ", $handle->getDoUnmarshallResult(), "\n";
    exit 1;
}

$result = $handle->submit("local", "MISC", "WHOAMI");

if ($result->{rc} != $STAF::kOk) {
    print "Error on MISC WHOAMI request, RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc};
}

# Make sure that the resultContext and resultObj variables in the
# STAFResult class were set correctly since auto-unmarshalling result is on

my $mc = STAF::STAFUnmarshall($result->{result});
my $entryMap = $mc->getRootObject();

if ($result->{resultContext} == undef) {
    print "ERROR:  result->{resultContext} is undef\n";
    exit 1;
}
elsif ($result->{resultContext}->getRootObject()->{instanceUUID} != $entryMap->{instanceUUID}) {
    print "STAFResult resultContext variable is not set correctly.\n";
    print "Expected: $mc\n";
    print "Found: $result->{resultContext}\n";
    exit 1;
}

if ($result->{resultObj} == undef) {
    print "ERROR:  result->{resultObj} is undef\n";
    exit 1;
}
elsif ($result->{resultObj}->{instanceUUID} != $entryMap->{instanceUUID}) {
    print "STAFResult resultObj variable is not set correctly.\n";
    print "Expected: $entryMap\n";
    print "Found: $result->{resultObj}\n";
    exit 1;
}

# Make sure that if turn off auto-unmarshalling result that the
# resultContext and resultObj variables are set to None since
# auto-unmarshalling result is off

# Test the STAFHandle.setDoUnmarshallResult API

print "  Turn off auto-unmarshalling result";
$handle->setDoUnmarshallResult(0);

if ($handle->getDoUnmarshallResult() != 0) {
    print "ERROR: handle->getDoUnmarshallResult() != 0\n";
    print "Found: ", $handle->getDoUnmarshallResult(), "\n";
    exit 1;
}

$result = $handle->submit("local", "MISC", "WHOAMI");

if ($result->{rc} != $STAF::kOk) {
    print "Error on MISC WHOAMI request, RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc};
}

if ($result->{resultContext} != undef) {
    print "ERROR:  result->{resultContext} != undef\n";
    print "Found: $result->{resultContext}\n";
    exit 1;
}

if ($result->{resultObj} != undef) {
    print "ERROR:  result->{resultObj} != undef\n";
    print "Found: $result->{resultObj}\n";
    exit 1;
}

# Make sure that if turn on auto-unmarshalling result that the
# resultContext and resultObj variables are set correctly since
# auto-unmarshalling result is on

print "  Turn on auto-unmarshalling result";
$handle->setDoUnmarshallResult(1);

if ($handle->getDoUnmarshallResult() != 1) {
    print "ERROR: handle->getDoUnmarshallResult() != 1\n";
    print "Found: ", $handle->getDoUnmarshallResult(), "\n";
    exit 1;
}

$result = $handle->submit("local", "MISC", "WHOAMI");

if ($result->{rc} != $STAF::kOk) {
    print "Error on MISC WHOAMI request, RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc};
}

# Make sure that the resultContext and resultObj variables in the
# STAFResult class were set correctly since auto-unmarshalling result is on

my $mc = STAF::STAFUnmarshall($result->{result});
my $entryMap = $mc->getRootObject();

if ($result->{resultContext} == undef) {
    print "ERROR:  result->{resultContext} is undef\n";
    exit 1;
}
elsif ($result->{resultContext}->getRootObject()->{instanceUUID} != $entryMap->{instanceUUID}) {
    print "STAFResult resultContext variable is not set correctly.\n";
    print "Expected: $mc\n";
    print "Found: $result->{resultContext}\n";
    exit 1;
}

if ($result->{resultObj} == undef) {
    print "ERROR:  result->{resultObj} is undef\n";
    exit 1;
}
elsif ($result->{resultObj}->{instanceUUID} != $entryMap->{instanceUUID}) {
    print "STAFResult resultObj variable is not set correctly.\n";
    print "Expected: $entryMap\n";
    print "Found: $result->{resultObj}\n";
    exit 1;
}

# Test the STAF::Submit2 API

print "\nTesting STAF::Submit2 API with \"local PING PING\"\n";

print "\nTesting \$STAF::STAFHandle::kReqSync\n";

$rc = STAF::Submit2($STAF::STAFHandle::kReqSync, "local", "PING", "PING");

if (($rc != $STAF::kOk) or ($STAF::Result ne "PONG")) {
    print "  Error submitting \"local PING PING\", expected PONG\n";
    print "  Received RC: $STAF::RC, Result: $STAF::Result\n";
    exit $rc;
}

print "  Result: $STAF::Result\n";

print "\nTesting \$STAF::STAFHandle::kReqFireAndForget\n";

$rc = STAF::Submit2($STAF::STAFHandle::kReqFireAndForget, "local", "PING", "PING");

if (($rc != $STAF::kOk) or ($STAF::Result eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $STAF::RC, Result: $STAF::Result\n";
    exit $rc;
}

print "  Result: $STAF::Result\n";

print "\nTesting \$STAF::STAFHandle::kReqQueue\n";

$rc = STAF::Submit2($STAF::STAFHandle::kReqQueue, "local", "PING", "PING");

if (($rc != $STAF::kOk) or ($STAF::Result eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $STAF::RC, Result: $STAF::Result\n";
    exit $rc;
}

print "  Result: $STAF::Result\n";

print "\nTesting \$STAF::STAFHandle::kReqRetain\n";

$rc = STAF::Submit2($STAF::STAFHandle::kReqRetain, "local", "PING", "PING");

if (($rc != $STAF::kOk) or ($STAF::Result eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $STAF::RC, Result: $STAF::Result\n";
    exit $rc;
}

print "  Result: $STAF::Result\n";

print "\nTesting \$STAF::STAFHandle::kReqQueueRetain\n";

$rc = STAF::Submit2($STAF::STAFHandle::kReqQueueRetain, "local", "PING", "PING");

if (($rc != $STAF::kOk) or ($STAF::Result eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $STAF::RC, Result: $STAF::Result\n";
    exit $rc;
}

print "  Result: $STAF::Result\n";

# Test the STAFHandle->submit2 API

print "\nTesting STAFHandle->submit2 API with \"local PING PING\"\n";

print "\nTesting \$STAF::STAFHandle::kReqSync\n";

$result = $handle->submit2($STAF::STAFHandle::kReqSync, "local", "PING", "PING"); 

if (($result->{rc} != $STAF::kOk) or ($result->{result} ne "PONG")) {
    print "  Error submitting \"local PING PING\", expected PONG\n";
    print "  Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

print "  Result: $result->{result}\n";

print "\nTesting \$STAF::STAFHandle::kReqFireAndForget\n";

$result = $handle->submit2($STAF::STAFHandle::kReqFireAndForget, "local", "PING", "PING"); 

if (($result->{rc} != $STAF::kOk) or ($result->{result} eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

print "  Result: $result->{result}\n";

print "\nTesting \$STAF::STAFHandle::kReqQueue\n";

$result = $handle->submit2($STAF::STAFHandle::kReqQueue, "local", "PING", "PING"); 

if (($result->{rc} != $STAF::kOk) or ($result->{result} eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

print "  Result: $result->{result}\n";

print "\nTesting \$STAF::STAFHandle::kReqRetain\n";

$result = $handle->submit2($STAF::STAFHandle::kReqRetain, "local", "PING", "PING"); 

if (($result->{rc} != $STAF::kOk) or ($result->{result} eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

print "  Result: $result->{result}\n";

print "\nTesting \$STAF::STAFHandle::kReqQueueRetain\n";

$result = $handle->submit2($STAF::STAFHandle::kReqQueueRetain, "local", "PING", "PING"); 

if (($result->{rc} != $STAF::kOk) or ($result->{result} eq "PONG")) {
    print "  Error submitting \"local PING PING\", expected Request Number\n";
    print "  Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

print "  Result: $result->{result}\n";

# Test the privacy APIs

print "\nTesting Privacy APIs...\n";

my $pw = "secret";
my $pwWithPD = STAF::AddPrivacyDelimiters($pw);
my $expectedResult = "!!@secret@!!";
print "  STAF::AddPrivacyDelimiters($pw): $pwWithPD\n";
if ($pwWithPD != $expectedResult) {
    print "Error: STAF::AddPrivacyDelimiters($pw): $pwWithPD\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $outString = STAF::EscapePrivacyDelimiters($pwWithPD);
my $expectedResult = "^!!@secret^@!!";
print "  STAF::EscapePrivacyDelimiters($pwWithPD): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::EscapePrivacyDelimiters($pwWithPD): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $outString = STAF::MaskPrivateData($pwWithPD);
my $expectedResult = "************";
print "  STAF::MaskPrivateData($pwWithPD): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::MaskPrivateData($pwWithPD): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $outString = STAF::RemovePrivacyDelimiters($pwWithPD);
my $expectedResult = "secret";
print "  STAF::RemovePrivacyDelimiters($pwWithPD): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($pwWithPD): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $dataWith3LevelsPD = '!!@Msg: ^!!@Pw is ^^^!!@secret^^^@!!.^@!!@!!';
my $outString = STAF::RemovePrivacyDelimiters($dataWith3LevelsPD);
my $expectedResult = "Msg: Pw is !!@secret!!@";
print "  STAF::RemovePrivacyDelimiters($dataWith3LevelsPD): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($dataWith3LevelsPD): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $outString = STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 0);
print "  STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 0): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 0): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $outString = STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 3);
print "  STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 3): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 3): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $expectedResult = 'Msg: !!@Pw is ^^!!@secret^^@!!.@!!';
my $outString = STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 1);
print "  STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 1): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 1): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $expectedResult = 'Msg: Pw is !!@secret@!!.';
my $outString = STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 2);
print "  STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 2): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 2): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $expectedResult = 'Msg: !!@Pw is ^^!!@secret^^@!!.@!!';
my $outString = STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 1);
print "  STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 1): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($dataWith3LevelsPD, 1): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

my $expectedResult = 'Msg: Pw is !!@secret@!!.';
my $outString = STAF::RemovePrivacyDelimiters($outString, 1);
print "  STAF::RemovePrivacyDelimiters($outString, 1): $outString\n";
if ($outString != $expectedResult) {
    print "Error: STAF::RemovePrivacyDelimiters($outString, 1): $outString\n";
    print "       Should return the following instead: $expectedResult";
    exit 1;
}

# Test private methods passing in an empty string
my $data = '';
my $expectedResult = '';
my $outString = STAF::AddPrivacyDelimiters($data);
print "  STAF::AddPrivacyDelimiters($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::AddPrivacyDelimiters($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

my $outString = STAF::EscapePrivacyDelimiters($data);
print "  STAF::EscapePrivacyDelimiters($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::EscapePrivacyDelimiters($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

my $outString = STAF::MaskPrivateData($data);
print "  STAF::MaskPrivateData($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::MaskPrivateData($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

my $outString = STAF::RemovePrivacyDelimiters($data);
print "  STAF::RemovePrivacyDelimiters($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::RemovePrivacyDelimiters($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

# Test private methods passing in non-English data

my $data = "‰¥";
my $expectedResult = "!!@‰¥@!!";
my $outString = STAF::AddPrivacyDelimiters($data);
print "  STAF::AddPrivacyDelimiters($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::AddPrivacyDelimiters($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

my $data = "!!@‰¥@!!";
my $expectedResult = "^!!@‰¥!@!!";
my $outString = STAF::EscapePrivacyDelimiters($data);
print "  STAF::EscapePrivacyDelimiters($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::EscapePrivacyDelimiters($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

my $outString = STAF::MaskPrivateData($data);
my $expectedResult = "********";
print "  STAF::MaskPrivateData($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::MaskPrivateData($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

my $outString = STAF::RemovePrivacyDelimiters($data);
my $expectedResult = "‰¥";
print "  STAF::RemovePrivacyDelimiters($data, 1): $outString\n";
if ($outString != $expectedResult) {
   print "Error: STAF::RemovePrivacyDelimiters($data): $outString\n";
   print "       Should return the following instead: $expectedResult";
   exit 1;
}

# Test the Monitor wrapper APIs

print "\nTesting Monitor Service Wrapper...\n";

my $machine = $result->{result};
print "  STAF/Config/Machine=$machine\n";

print "  Log a message to the monitor service\n";
$rc = STAF::Monitor::Log("Hello World"); 
if ($rc != $STAF::kOk) { 
    print "Error logging message to Monitor, RC: $rc\n"; 
    exit $rc; 
}

$mon = STAF::STAFMonitor->new($handle); 
$result = $mon->log("Hello World Again"); 
if ($result->{rc} != $STAF::kOk) { 
    print "Error logging message to Monitor, RC: $result->{rc}\n"; 
    exit $rc; 
} 

print "\nTesting Log Service Wrapper...\n";

print "  Init Log\n";
$rc = STAF::Log::Init("TestCase1", "GLOBAL", "FATAL ERROR"); 
print "  Log a message\n";
$rc = STAF::Log::Log("WARNING", "Unable to find specified file"); 
if ($rc != $STAF::kOk) { 
    print "Error logging message to Log, RC: $rc\n"; 
    exit $rc; 
}

print "  Init TestCase2 log\n";
$log = STAF::STAFLog->new($handle, "TestCase2", "GLOBAL", "FATAL ERROR"); 
$result = $log->log("WARNING", "Unable to find specified file"); 
if ($result->{rc} != $STAF::kOk) { 
    print "Error logging message to Log, RC: $result->{rc}\n"; 
    exit $result->{rc}; 
}

$logtype = $log->getLogType(); 
print "  Log Type: $logtype\n"; 
$logmask = $log->getMonitorMask(); 
print "  Log's Monitor Mask: $logmask\n"; 
$system = $log->getSystemName(); 
print "  Log Service System Name: $system\n"; 
$service = $log->getServiceName(); 
print "  Log Service Name: $service\n"; 

print "\nTesting Class STAFMapClassDefinition's methods...\n";

print "  Testing STAFMapClassDefinition new() method...\n";
my $myMapClassDef = STAF::STAFMapClassDefinition->new('Test/MyMap');

print "  Testing STAFMapClassDefinition addKey() method...\n";
$myMapClassDef->addKey('name', 'Name');
$myMapClassDef->addKey('exec', "Executable");
$myMapClassDef->addKey('testType', 'Test Type');

print "  Testing STAFMapClassDefinition setKeyProperty() method...\n";
$myMapClassDef->setKeyProperty('testType', 'display-short-name', 'Test');
$myMapClassDef->addKey('outputs', 'Outputs');
 
print "  Testing STAFMapClassDefinition name() method...\n";
my $myMapClassName = $myMapClassDef->name();

my $foundKeys = 0;

print "  Testing STAFMapClassDefinition keys() method...\n";
for my $key ($myMapClassDef->keys()) {
    if ($key->{'key'} eq "name") {
        $foundKeys = $foundKeys + 1;
    }
    elsif ($key->{'key'} eq "exec") {
        $foundKeys = $foundKeys + 1;
    }
    elsif (($key->{'key'} eq "testType") && ($key->{'display-short-name'} eq "Test")) {
        $foundKeys = $foundKeys + 1;
        print "  key=$key->{'key'} display-name=$key->{'display-name'}".
              "  display-short-name=$key->{'display-short-name'}\n";
    }
    elsif ($key->{'key'} eq "outputs") {
        $foundKeys = $foundKeys + 1;
    }
}

if ($foundKeys != 4) {
    print "ERROR: Map Class Definition does not contain the correct 4 keys.\n".
          "Contains $foundKeys keys.\n";
    exit 1;
}

print "  Testing STAFMapClassDefinition createInstance() method...\n";
my $myMapClass = $myMapClassDef->createInstance();

# Create a marshalling context and assign a map class definition to it

print "\nTesting Class STAFMarshallingContext Methods...\n";

print "  Testing STAFMarshallingContext new() method...\n";
my $mc = STAF::STAFMarshallingContext->new();

print "  Testing STAFMarshallingContext setMapClassDefinition() method...\n";
$mc->setMapClassDefinition($myMapClassDef);

print "  Testing STAFMarshallingContext hasMapClassDefinition() method...\n";
if (!$mc->hasMapClassDefinition('Test/MyMap'))
{
    print "Oops, map class 'Test/MyMap' doesn't exist\n";
    exit 1;
}

# Get the map class definition from the marshalling context

print "  Testing STAFMarshallingContext getMapClassDefinition() method...\n";
my $mapClassDef = $mc->getMapClassDefinition('Test/MyMap');

# Set the root object for a marshalling context to be a string
# and get the root object.

$data = "This is a string";
$mc->setRootObject($data);
my $rootObj = $mc->getRootObject();

# Set the root object for a marshalling context to be a hash (aka map)

print "  Testing STAFMarshallingContext getRootObject() method...\n";

my $myTestMap = {
  'name' => 'TestA',
  'exec' => '/tests/TestA.py',
  'testType' => 'FVT',
  'outputs' => [ 'TestA.out', 'TestA.err' ]
  };

$mc->setRootObject($myTestMap);
my $rootObj = $mc->getRootObject();

print "  Testing STAFMarshallingContext getPrimaryObject() method...\n";
my $priObj = $mc->getPrimaryObject();

if (!($priObj eq $mc)) {
    print 'Error: $mc->getPrimaryObject() != $mc', "\n";
    print "mc->getPrimaryObject()=$priObj\n";
    print "mc-$mc\n";
    exit 1;
}

print "  Testing STAFMarshallingContext formatObject() method...\n";
my $formattedOutput1 = $mc->formatObject();
my $formattedOutput2 = STAF::STAFFormatObject($mc->getRootObject(), $mc);

if (!($formattedOutput1 eq $formattedOutput2)) {
    print "Error in mc->formatObject() or STAF::STAFFormatObject function\n";
    print "formattedOutput1:\n$formattedOutput1\n";
    print "formattedOutput2:\n$formattedOutput2\n";
    exit 1;
}

print "  Testing STAFMarshallingContext marshall() method...\n";

# Test the marshall function using a MapClassDefinition

my $expectedResult2 = 
    '@SDT/*:558:@SDT/{:398::13:map-class-map@SDT/{:370::10:Test/MyMap' .
    '@SDT/{:345::4:keys@SDT/[4:298:@SDT/{:50::12:display-name' .
    '@SDT/$S:4:Name:3:key@SDT/$S:4:name@SDT/{:57::12:display-name' .
    '@SDT/$S:10:Executable:3:key@SDT/$S:4:exec@SDT/{:95::12:display-name' .
    '@SDT/$S:9:Test Type:3:key@SDT/$S:8:testType:18:display-short-name' .
    '@SDT/$S:4:test@SDT/{:56::12:display-name@SDT/$S:7:Outputs:3:key' .
    '@SDT/$S:7:outputs:4:name@SDT/$S:10:Test/MyMap@SDT/{:138::7:outputs' .
    '@SDT/[2:38:@SDT/$S:9:TestA.out@SDT/$S:9:TestA.err:8:testType' .
    '@SDT/$S:3:FVT:4:name@SDT/$S:5:TestA:4:exec@SDT/$S:15:/tests/TestA.py';

my $marshalledResult2 = STAF::STAFMarshall($mc, $mc);

if (length($marshalledResult2) != length($expectedResult2)) {
    print "Error: Wrong output for marshall function\n";
    print "Expected to find:\n$expectedResult2\n";
    print "Found:\n$marshalledResult2\n";
    exit 1;
}

my $marshalledResult3 = $mc->marshall();

if (length($marshalledResult3) != length($expectedResult2)) {
    print "Error: Wrong output for marshall function\n";
    print "Expected to find:\n$expectedResult2\n";
    print "Found:\n$marshalledResult3\n";
    exit 1;
}

# Create a map class definition without a name

my $myDef = STAF::STAFMapClassDefinition->new();
$myDef->addKey('key1', 'Key 1');
my $myDefName = $myDef->name();

############################################
# Next, test the STAFUnmarshall function     #
############################################
print "\nTesting STAFUnmarshall()...\n";

# Submit a FS QUERY ENTRY request and unmarshall it's result (a map)

print "\n  STAF local FS QUERY ENTRY {STAF/Config/ConfigFile}\n\n";
$result = $handle->submit("local", "FS", "QUERY ENTRY {STAF/Config/ConfigFile}");

if ($result->{rc} != $STAF::kOk) {
    print "Error on FS QUERY ENTRY request.\n";
    print "Expected RC: 0\n";
    print "Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

if (!STAF::STAFIsMarshalledData($result->{result})) {
    print "ERROR: Not marshalled data:  Result: $result->{result}\n";
}

my $mc = STAF::STAFUnmarshall($result->{result});
my $entryMap = $mc->getRootObject();

if ($entryMap->{type} eq "F") {
    print "  File Name         : $entryMap->{name}\n";
    print "  Size              : $entryMap->{lowerSize}\n";
    print "  Date Last Modified: $entryMap->{lastModifiedTimestamp}\n";
}
else {
    print "Error on FS QUERY ENTRY result.\n";
    print "$fileName is not a file.  Type=$entryMap->{type}\n";
    exit 1;
}

# Determine if running test on a Windows or Unix machine

my $request = "RESOLVE STRING {STAF/Config/OS/Name}";
print "\n  STAF local VAR $request\n";

$result = $handle->submit("local", "VAR", $request);

if ($result->{rc} != $STAF::kOk) {
    print "Error on STAF local VAR $request\n";
    print "Expected RC: 0\n";
    print "Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

my $osName = $result->{result};
print "\n  STAF/Config/OS/Name: $osName\n";

# Use command "dir" if on Windows or use "ls" if on Unix

my $command = "dir {STAF/Config/STAFRoot}";

if (($osName eq "Linux") ||
    ($osName eq "HP-UX") ||
    ($osName eq "AIX") ||
    ($osName eq "SunOS") ||
    ($osName eq "OS400") ||
    ($osName eq "OS/390") ||
    ($osName eq "FreeBSD") ||
    ($osName eq "Darwin")) {
    $command = "ls {STAF/Config/STAFRoot}";
}

# Submit a PROCESS START request without a WAIT option

my $request = "START SHELL COMMAND ".STAF::WrapData($command).
              " RETURNSTDOUT STDERRTOSTDOUT";

print "\n  STAF local PROCESS $request\n";

$result = $handle->submit("local", "PROCESS", $request);

if ($result->{rc} != $STAF::kOk) {
    print "Error on STAF local PROCESS $request\n";
    print "Expected RC: 0\n";
    print "Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

print "\n  Process Handle: $result->{result}\n";

# Submit a PROCESS START request and wait for it to complete

my $request = "START SHELL COMMAND ".STAF::WrapData($command).
              " RETURNSTDOUT STDERRTOSTDOUT WAIT";

print "\n  STAF local PROCESS $request\n";

$result = $handle->submit("local", "PROCESS", $request);

if ($result->{rc} != $STAF::kOk) {
    print "Error on STAF local PROCESS $request\n";
    print "Expected RC: 0\n";
    print "Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

# Unmarshall the result which is a marshalling context whose 
# root object is a map containing keys 'rc', and 'fileList'.
# The value for 'fileList' is a list of the returned files.
# Each entry in the list consists of a map that contains keys
# 'rc' and 'data'.  In our PROCESS START request, we returned
# one file, stdout (and returned stderr to this same file).

my $mc = STAF::STAFUnmarshall($result->{result});
my $mcRootObject = $mc->getRootObject();

# Verify that the process rc is 0

my $processRC = $mcRootObject->{rc};

if ($processRC != $STAF::kOk) {
    print "  Process RC: $processRC\n";
    exit $processRC;
}

# Verify that the rc is 0 for returning data for the Stdout file 

my $stdoutRC = $mcRootObject->{fileList}[0]{rc};

if ($stdoutRC != $STAF::kOk) {
    print "Error on retrieving process's stdout data.\n";
    print "Expected RC: 0\n";
    print "Received RC: $stdoutRC\n";
    exit $stdoutRC; 
}

# Print the data in the stdout file created by the process

my $stdoutData = $mcRootObject->{fileList}[0]{data};

print "\n  Process Stdout File Contains:\n";
print "$stdoutData\n";

# Test unmarshalling data that contains indirect marshalled data

print "Test unmarshalling data that contains indirect marshalled data\n\n";

# Create an array (aka list) of hashes (aka maps)
my @testList = (
    {name => 'TestA', exec => '/tests/TestA.py'},
    {name => 'TestB', exec => '/tests/TestB.sh'},
    {name => 'TestC', exec => '/tests/TestC.cmd'}
  );

$result = $handle->submit("local", "FS", "QUERY ENTRY {STAF/Config/ConfigFile}");

if ($result->{rc} != $STAF::kOk) {
    print "Error on FS QUERY ENTRY request.\n";
    print "Expected RC: 0\n";
    print "Received RC: $result->{rc}, Result: $result->{result}\n";
    exit $result->{rc}; 
}

# Add some marshalled data as an entry to the list
push @testList, $result->{result};

my $mcWithIndirectObjects = $mc;
$mcWithIndirectObjects->setRootObject(\@testList);
my $marshalledData = $mcWithIndirectObjects->marshall();
print "Unmarshall using IGNORE_INDIRECT_OBJECTS=>0 flag\n";
my $mc2 = STAF::STAFUnmarshall($marshalledData, IGNORE_INDIRECT_OBJECTS=>0);
my $formattedData2 = STAF::STAFFormatObject($mc2->getRootObject(), $mc2);
print $formattedData2, "\n";

print "Unmarshall using IGNORE_INDIRECT_OBJECTS=>1 flag\n";
my $mc3 = STAF::STAFUnmarshall($marshalledData, $mc2, IGNORE_INDIRECT_OBJECTS=>1);
my $formattedData3 = STAF::STAFFormatObject($mc3->getRootObject(), $mc3);
print $formattedData3, "\n";

if ($formattedData2 eq $formattedData3) {
    print "formattedData2 eq formattedData3\n";
}

if (!($formattedData2 eq $formattedData3) &&
    (index($formattedData2, '@SDT/*') == -1) &&
    (index($formattedData3, '@SDT/*') > 0)) {
    # Success
}
else
{
    print "Error using IGNORE_INDIRECT_OBJECTS flag on unmarshall\n";
    print "\nFormatted output using IGNORE_INDIRECT_OBJECTS=>0 flag:\n";
    print $formattedData2, "\n";
    print "\nFormatted output using IGNORE_INDIRECT_OBJECTS=>1 flag:\n";
    print $formattedData3, "\n";
    exit 1;
}

# Test unmarshalling data that contains invalid marshalled data
# XXX: This test is commented out because it doesn't work yet because the fix
# made for Bug #2515811 hasn't been implemented yet for the Perl unmarshall
# method

#print "\n\nTest unmarshalling data that contains invalid marshalled data\n\n";

# Create some invalid marshalling data and queue it;  Get it off the queue,
# and unmarshall it and verify doesn't cause an error and returns the correct
# data

#my $message = '@SDT/{:177::2:RC@SDT/$S:1:0:6:IPInfo@SDT/$S:36:' .
#    '9.42.126.76|255.255.252.0|9.42.124.1:3:Msg@SDT/$S:46:Static IP ' .
#    'arguments are processed successfully:9:Timestamp@SDT/$S:19:2009-01-16 14:41:45' .
#    'Connecting to: http://9.42.106.28:8080';

#my $result = $handle->submit("local", "QUEUE", "QUEUE MESSAGE $message");

#if ($result->{rc} != $STAF::kOk) {
#    print "Error on QUEUE QUEUE MESSAGE request.\n";
#    print "Expected RC: 0\n";
#    print "Received RC: $result->{rc}, Result: $result->{result}\n";
#    exit $result->{rc}; 
#}

#my $mc = STAF::STAFUnmarshall($result->{result});
#my $messageMap = $mc->getRootObject();

#print "Queued message map:\n\n$messageMap\n\n";
#print STAF::STAFFormatObject($messageMap);
#print "\n\n";

#if (!($messageMap->{message} eq $message)) {
#    print "ERROR: Message containing invalid marshalled data not unmarshalled properly\n";
#    print "  Expected message: $message\n";
#    print "  Received message: $messageMap->{message}\n";
#    exit 1;
#}

############################################
# Next, test the STAFMarshall function     #
############################################

print "\n\nTesting STAFMarshall()...\n\n";

# Test marshalling a scalar (e.g. a string)

my $marshalledResult = STAF::STAFMarshall("This is a string");
print "Marshalled result from marshalling a string:\n$marshalledResult\n\n";
my $expectedLength = 27;
if (length($marshalledResult) != $expectedLength) {
    print "ERROR: Incorrect marshalled data.  Length is ", 
          length($marshalledResult), " but should be $expectedLength\n\n";
}

# Test marshalling a None scalar

my $data = undef;
my $marshalledResult = STAF::STAFMarshall($data);
print "Marshalled result from marshalling a None scalar:\n$marshalledResult\n\n";
if (!($marshalledResult eq '@SDT/$0:0:')) {
    print 'ERROR: Incorrect marshalled data. Expected: @SDT/$0:0:', "\n\n";
}
my $mc = STAF::STAFUnmarshall($marshalledResult);
if (!($mc->formatObject() eq "<None>")) {
    print "ERROR: FormatObject for a None scalar isn't <None>\n";
    print "       Result: $mc->formatObject()\n\n";
}

# Test marshalling a hash

my $myTestMap = {
  'name' => 'TestA',
  'exec' => '/tests/TestA.py',
  'testType' => 'FVT',
  'outputs' => [ 'TestA.out', 'TestA.err' ]
  };
my $marshalledResult = STAF::STAFMarshall($myTestMap);
print "Marshalled result from marshalling a hash:\n$marshalledResult\n\n";
my $expectedLength = 149;
if (length($marshalledResult) != $expectedLength) {
    print "ERROR: Incorrect marshalled data.  Length is ", 
          length($marshalledResult), " but should be $expectedLength\n\n";
}

# Test marshalling an array

my @testList = (
    {name => 'TestA', exec => '/tests/TestA.py'},
    {name => 'TestB', exec => '/tests/TestB.sh'},
    {name => 'TestC', exec => '/tests/TestC.cmd'}
  );
my $marshalledResult = STAF::STAFMarshall(\@testList);
print "Marshalled result from marshalling an array:\n$marshalledResult\n\n";
my $expectedLength = 208;
if (length($marshalledResult) != $expectedLength) {
    print "ERROR: Incorrect marshalled data.  Length is ", 
          length($marshalledResult), " but should be $expectedLength\n\n";
}

# Test marshalling data using a marshalling context

# Create a marshalling context and marshall it, and unmarshall it

# Create a map class definition
my $myMapClassDef = STAF::STAFMapClassDefinition->new('Test/MyMap');
$myMapClassDef->addKey('name', 'Name');
$myMapClassDef->addKey('exec', 'Executable');

# Create an array (aka list) of hashes (aka maps)
my @testList = (
    {name => 'TestA', exec => '/tests/TestA.py'},
    {name => 'TestB', exec => '/tests/TestB.sh'},
    {name => 'TestC', exec => '/tests/TestC.cmd'}
  );

# Create a reference to the array (aka list)
my $testList_ref = \@testList;

# Create a marshalling context with one map class definition

my $mc = STAF::STAFMarshallingContext->new();
$mc->setMapClassDefinition($myMapClassDef);

# Create an array (aka list) of map class map data

my @myTestList;

foreach my $test (@$testList_ref) {
    my $testMap = $myMapClassDef->createInstance();
    $testMap->{'name'} = $test->{'name'};
    $testMap->{'exec'} = $test->{'exec'};
    push @myTestList, $testMap;
}

# Assign a reference to the array (aka list) as the root object
# for the marshalling context
$mc->setRootObject(\@myTestList);

# Create a string from the marshalling context
# This string could be a message that you log or send to a queue, etc.

my $marshalledResult = $mc->marshall();
print "Marshalled result from marshalling a marshalling context:\n$marshalledResult\n\n";
my $expectedLength = 457;
if (length($marshalledResult) != $expectedLength) {
    print "ERROR: Incorrect marshalled data.  Length is ", 
          length($marshalledResult), " but should be $expectedLength\n\n";
}

# Convert the marshalled string representation back into an array (aka list)

my $mc2 = STAF::STAFUnmarshall($marshalledResult);
my $message2 = $mc2->marshall();
my $theTestList = $mc2->getRootObject();

if (length($marshalledResult) != length($message2)) {
    print 'Error: length($marshalledResult) != length($message2)', "\n";
    print "Message:\n$marshalledResult\n";
    print "Message2:\n$message2\n";
    exit 1;
}


############################################
# Next, test the STAFFormatObject function #
############################################

print "Testing STAFFormatObject()...";

print "\n\nPrinting formatted output for a scalar (e.g. a string):\n";
print STAF::STAFFormatObject("This is a string");

print "\n\nPrinting formatted output for None scalar (e.g. undef):\n";
print STAF::STAFFormatObject(undef);

if (!(STAF::STAFFormatObject(undef) eq '<None>')) {
    print "ERROR: Wrong FormatObject for a None scalar\n";
    print "       Result: ", STAF::STAFFormatObject(undef), "\n\n";
}

print "\n\nPrinting formatted output for an array:\n";
my @testList = (
    {name => 'TestA', exec => '/tests/TestA.py'},
    {name => 'TestB', exec => '/tests/TestB.sh'},
    {name => 'TestC', exec => '/tests/TestC.cmd'}
  );
print STAF::STAFFormatObject(\@testList);

print "\n\nPrinting formatted output for a hash:\n";
print STAF::STAFFormatObject($myTestMap);

my $fileName = '{STAF/Config/ConfigFile}';

my $result = $handle->submit('local', 'FS', "QUERY ENTRY $fileName");

if (result->{rc} != STAFResult.Ok) {
    print "FS QUERY ENTRY $fileName failed";
    print "RC=$result->{rc}, Result=$result->{result}";
    exit 1;
}

my $mc = STAF::STAFUnmarshall($result->{result});
print "\n\nPrinting formatted output for FS QUERY ENTRY using STAFFormatObject\n";
print STAF::STAFFormatObject($mc->getRootObject(), $mc);
print "\n\nPrinting formatted output for FS QUERY ENTRY using formatObject\n";
print $mc->formatObject();

# Test STAFResult->new

print "\n\nTesting STAFResult->new()...\n";

my $result = STAF::STAFResult->new(0, 'Successful');
if (($result->{rc} != $STAF::kOk) or
    (!$result->{result} eq 'Successful')) {
    print "ERROR: Wrong STAFResult.".
          "RC: $result->{rc} Result: $result->{result}\n";
    exit $result->{rc};
}

# Test STAF::WrapData function

print "\nTesting STAF::WrapData()...\n";

my $message = "Hello World";
$result = $handle->submit(
    "local", "monitor", "log message ".STAF::WrapData($message));

if ($result->{rc} != $STAF::kOk) { 
    print "Error logging message to Monitor,".
          " RC: $result->{rc} Result: $result->{result}\n"; 
    exit $result->{rc}; 
} 

# Unregister the handle

print "\nUnregistering handle $handle->{handle}\n";

$rc = $handle->unRegister();
if ($rc != $STAF::kOk) {
    print "Error unregistering with STAF, RC: $STAF::RC\n";
    exit $rc;
}

print "\n *** All tests successful ***\n";

