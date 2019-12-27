use PLSTAF;

$rc = STAF::Register("My program");

if ($rc != $STAF::kOk) {
    print "Error registering with STAF, RC: $STAF::RC\n";
    exit $rc;
}

my $machine = "local";
my $service = "DIAG";
my $request = "RESET FORCE";
my $stafCmd = "STAF $machine $service $request";
print "$stafCmd\n";
$rc = STAF::Submit($machine, $service, $request);

if ($rc != $STAF::kOk) {
    print "Error on $stafCmd\nRC: $rc, Result: $STAF::Result\n";
}

my $request = "ENABLE";
my $stafCmd = "STAF $machine $service $request";
print "$stafCmd\n";
$rc = STAF::Submit($machine, $service, $request);

if ($rc != $STAF::kOk) {
    print "Error on $stafCmd\nRC: $rc, Result: $STAF::Result\n";
    STAF::UnRegister();
    exit $rc;
}

my $trigger = "PROCESS QUERY";
my $source = "My program;client1.company.com";
my $request = "RECORD TRIGGER ".STAF::WrapData($trigger)." SOURCE ".STAF::WrapData($source);
my $stafCmd = "STAF $machine $service $request";
print "$stafCmd\n";
$rc = STAF::Submit($machine, $service, $request);

if ($rc != $STAF::kOk) {
   print "Error on $stafCmd\nRC: $rc, Result: $STAF::Result\n";
}

my $trigger = "PROCESS LIST";
my $request = "RECORD TRIGGER ".STAF::WrapData($trigger)." SOURCE ".STAF::WrapData($source);
my $stafCmd = "STAF $machine $service $request";
print "$stafCmd\n";
$rc = STAF::Submit($machine, $service, $request);

if ($rc != $STAF::kOk) {
   print "Error on $stafCmd\nRC: $rc, Result: $STAF::Result\n";
}

my $request = "LIST";
my $stafCmd = "STAF $machine $service $request";
print "$stafCmd\n";
$rc = STAF::Submit($machine, $service, $request);

if ($rc != $STAF::kOk) {
   print "Error on $stafCmd\nRC: $rc, Result: $STAF::Result\n";
}

my $mc = STAF::STAFUnmarshall($STAF::Result);
my $rootObject = $mc->getRootObject();
print "\nFormatted Output:\n", $mc->formatObject(), "\n";

print "\nOutput obtained from fields in rootObject:\n";
print "  From Date-Time    : $rootObject->{fromTimestamp}\n";
print "  To Date-Time      : $rootObject->{toTimestamp}\n";
print "  Elapsed Time      : $rootObject->{elapsedTime}\n";
print "  Number of Triggers: $rootObject->{numberOfTriggers}\n";
print "  Number of Sources : $rootObject->{numberOfSources}\n";
print "  Trigger/Source Combinations List:\n";

# Each item in the array is a "reference" to a hash/map

for $itemMap (@{$rootObject->{comboList}}) {
    print "\n    Trigger:  $itemMap->{trigger}\n";
    print "    Source :  $itemMap->{source}\n";
    print "    Count  :  $itemMap->{count}\n";
}
print "\n";

my $request = "DISABLE";
my $stafCmd = "STAF $machine $service $request";
print "$stafCmd\n";
$rc = STAF::Submit($machine, $service, $request);

if ($rc != $STAF::kOk) {
   print "Error on $stafCmd\nRC: $rc, Result: $STAF::Result\n";
}

STAF::UnRegister(); 
