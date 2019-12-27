#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2007                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

#===========================================================================
# TestMarshallingPerf - Tests performance of Perl marshalling and
#                       formatObject
#===========================================================================
# Accepts: The number of entries to marshall
# Returns: 0 if successful;  non-zero if not successful
#===========================================================================
# Purpose: Create some marshalled data for a specified number of entries to
# test performance of Perl marshalling and formatObject.
#
#===========================================================================

use PLSTAF;

# Verify the command line arguments

my $entries = 100000;

if (@_) {
    my $entries = $_[0];
}
else {
    #die "Syntax:  perl TestMarshallingPerf.pl <number>\n";
}

################################
# Test marshalling performance #
################################

print "\n**************************************************************\n";
print "Test Performance for Marshalling, FormatObject, and Unmarshall\n";
print "**************************************************************\n";
    
if (1) {
# Test using a list with the specified number of entries

print "\nTest using a list with $entries entries\n\n";
    
my @myList = ();

for (my $i = 0; $i < $entries; $i++) {
    push @myList, "entryValue ##$i";
}

my $mc = STAF::STAFMarshallingContext->new();
$mc->setRootObject(\@myList);

print "FormatObject starting...\n";
my $starttime = time;  # record starting time for formatObject
$mc->formatObject();
my $elapsed = time - $starttime;
print "FormatObject Elapsed Time: $elapsed seconds\n";
#print $mc->formatObject();

print "Marshalling starting...\n";
my $starttime = time; # record starting time for marshalling
my $result = $mc->marshall();
my $elapsed = time - $starttime;
print "Marshalling Elapsed Time: $elapsed seconds\n";
#print $result;

print "Length of marshalled string: ", length($result), "\n";

print "Unmarshalling starting...\n";
my $starttime = time;  # record starting time for marshalling
my $mc = STAF::STAFUnmarshall($result);
my $elapsed = time - $starttime;
print "Unmarshalling Elapsed Time: $elapsed seconds\n";
#print $mc->formatObject();
    
my @myList = ();
}

if (1) {
# Test using a map with the specified number of entries

print "\nTest using a map with $entries entries\n\n";

my $myMap = {};

for (my $i = 0; $i < $entries; $i++) {
    $myMap->{"key$i"} = "value$i";
}

my $mc = STAF::STAFMarshallingContext->new();
$mc->setRootObject($myMap);

print "FormatObject starting...\n";
my $starttime = time;  # record starting time for formatObject
$mc->formatObject();
my $elapsed = time - $starttime;
print "FormatObject Elapsed Time: $elapsed seconds\n";
#print $mc->formatObject();

print "Marshalling starting...\n";
my $starttime = time; # record starting time for marshalling
my $result = $mc->marshall();
my $elapsed = time - $starttime;
print "Marshalling Elapsed Time: $elapsed seconds\n";
#print $result;

print "Length of marshalled string: ", length($result), "\n";

print "Unmarshalling starting...\n";
my $starttime = time;  # record starting time for marshalling
my $mc = STAF::STAFUnmarshall($result);
my $elapsed = time - $starttime;
print "Unmarshalling Elapsed Time: $elapsed seconds\n";
#print $mc->formatObject();
    
my $myMap = {};
}

if (1) {
# Test using a list (with the specified number of entries / 10) of map
# class objects each with 10 keys

my $numEntries = $entries / 10;

if ($numEntries < 1 and $entries > 0) {
    $numEntries = 1;
}

my $numKeys = 10;

print "\nTest using a list with $numEntries entries of map class objects each with $numKeys keys\n\n";
    
# Define a map class with 10 keys

my $myMapClass = STAF::STAFMapClassDefinition->new('STAF/Test/MyMapClassDefinition');

for (my $k = 1; $k < $numKeys + 1; $k++) {
    $myMapClass->addKey("key$k", "Key #$k");
}

# Create a marshalling context and assign the map class definition

my $mc = STAF::STAFMarshallingContext->new();
$mc->setMapClassDefinition($myMapClass);
 
# Create an array (aka list) of hashes (aka maps)
my @resultList = ();

for (my $i = 1; $i < $numEntries + 1; $i++) {
    # Create an instance of this map class definition and assign
    # data to the map class instance

    my $theMap = $myMapClass->createInstance();

    for (my $j = 1; $j < $numKeys + 1; $j++) {
        $theMap->{"key$j"} = "Value $j $i";
    }

    push @resultList, $theMap;
}

$mc->setRootObject(\@resultList);

print "FormatObject starting...\n";
my $starttime = time;  # record starting time for formatObject
$mc->formatObject();
my $elapsed = time - $starttime;
print "FormatObject Elapsed Time: $elapsed seconds\n";
#print $mc->formatObject();

print "Marshalling starting...\n";
my $starttime = time; # record starting time for marshalling
my $result = $mc->marshall();
my $elapsed = time - $starttime;
print "Marshalling Elapsed Time: $elapsed seconds\n";
#print $result;

print "Length of marshalled string: ", length($result), "\n";

print "Unmarshalling starting...\n";
my $starttime = time;  # record starting time for marshalling
my $mc = STAF::STAFUnmarshall($result);
my $elapsed = time - $starttime;
print "Unmarshalling Elapsed Time: $elapsed seconds\n";
#print $mc->formatObject();
    
my $resultList = {};
}

exit 0;
