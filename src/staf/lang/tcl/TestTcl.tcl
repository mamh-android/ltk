#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2001                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

# This file contains tests for the STAF Tcl support
package require STAF
package require STAFMon
package require STAFLog

if {[STAF::Register "Tcl Test"] != 0} {
    puts "Error registering with STAF, RC: $STAF::RC"
    exit $STAF::RC
}

puts "Using handle $STAF::Handle"

#######################################
# First test some basic functionality #
#######################################

puts "Testing basic functionality"

STAF::Submit local ping ping

if {$STAF::Result != "PONG"} {
    puts "Wrong output for ping request"
    exit 1
}

if {[STAF::Submit local var "resolve string {STAF/Config/MachineNickname}"] != 0} {
    puts "Error resolving machine nickname, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

set self $STAF::Result

###############################################
# Next, lets test the monitor service wrapper #
###############################################

puts "Testing Monitor service functions"

# Log the message

set monitorMessage "Hello World"

if {[STAF::Monitor::Log $monitorMessage] != $STAF::kOk} {
    puts "Error on STAF::Monitor::Log, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Try to retrieve it

set request "query machine $self handle $STAF::Handle"

if {[STAF::Submit local monitor $request] != $STAF::kOk} {
    puts "Error querying monitor info, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Make sure we got back the correct message

if {[string first $monitorMessage $STAF::Result] == -1} {
    puts "Wrong output for monitor query request"
    puts "Expected to find '<Timestamp> $monitorMessage'"
    puts "Found '$STAF::Result'"
    exit 1
}

##########################################
# Now, lets test the log service wrapper #
##########################################

puts "Testing Log service functions"

# Setup logging

STAF::Log::Init TclTest handle "Fatal Error Warning Info"

# Log the message

set logMessage "A log message"

if {[STAF::Log::Log info $logMessage] != $STAF::kOk} {
    puts "Error on STAF::Log::Log, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Try to retrieve it

set request "query machine $self handle $STAF::Handle logname TclTest"

if {[STAF::Submit local log $request] != $STAF::kOk} {
    puts "Error on STAF::Log::Log, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Make sure we got back the correct message

if {[string first $logMessage $STAF::Result] == -1} {
    puts "Wrong output for log query request"
    puts "Expected to find '<Timestamp> $logMessage'"
    puts "Found '$STAF::Result'"
    exit 1
}

# Try to retrieve it from monitor

set request "query machine $self handle $STAF::Handle"

if {[STAF::Submit local Monitor $request] != $STAF::kOk} {
    puts "Error on querying monitor info, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Make sure we got back the correct message from monitor

if {[string first INFO:$logMessage $STAF::Result] == -1} {
    puts "Wrong output for monitor query request"
    puts "Expected to find '<Timestamp> INFO:$logMessage'"
    puts "Found '$STAF::Result'"
    exit 1
}

# Delete the log file

set request "delete machine $self handle $STAF::Handle logname TclTest confirm" 

if {[STAF::Submit local log $request] != 0} {
    puts "Error deleting log file, RC: $STAF::RC, Result: $STAF::Result"
    exit 1
}

# Log the message so that Monitor shouldn't get it

if {[STAF::Log::Log status $logMessage] != $STAF::kOk} {
    puts "Error on STAF::Log::Log, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Try to retrieve it

set request "query machine $self handle $STAF::Handle logname TclTest"

if {[STAF::Submit local log $request] != $STAF::kOk} {
    puts "Error on STAF::Log::Log, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Make sure we got back the correct message

if {[string first $logMessage $STAF::Result] == -1} {
    puts "Wrong output for log query request"
    puts "Expected to find '<Timestamp> $logMessage'"
    puts "Found '$STAF::Result'"
    exit 1
}

# Try to retrieve it from monitor

set request "query machine $self handle $STAF::Handle"

if {[STAF::Submit local Monitor $request] != $STAF::kOk} {
    puts "Error on querying monitor info, RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
} 

# Make sure we got back the correct (old) message from monitor

if {[string first INFO:$logMessage $STAF::Result] == -1} {
    puts "Wrong output for monitor query request"
    puts "Expected to find '<Timestamp> INFO:$logMessage'"
    puts "Found '$STAF::Result'"
    exit 1
}

##########################
# Test privacy functions #
##########################

puts "Testing privacy functions"

set data "secret"
set dataWithPD [STAF::AddPrivacyDelimiters $data]

set expectedResult "!!@secret@!!"
if {[STAF::AddPrivacyDelimiters $data] != $expectedResult} {
    puts "Error: \[STAF::AddPrivacyDelimiters $data\] : [STAF::AddPrivacyDelimiters $data]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set expectedResult "^!!@secret^@!!"
if {[STAF::EscapePrivacyDelimiters $dataWithPD] != $expectedResult} {
    puts "Error: \[STAF::EscapePrivacyDelimiters $dataWithPD\] = [STAF::EscapePrivacyDelimiters $dataWithPD]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set expectedResult "************"
if {[STAF::MaskPrivateData $dataWithPD] != $expectedResult} {
    puts "Error: \[STAF::MaskPrivateData $dataWithPD\] = [STAF::MaskPrivateData $dataWithPD]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set expectedResult "secret"
if {[STAF::RemovePrivacyDelimiters $dataWithPD] != $expectedResult} {
    puts "Error: \[STAF::RemovePrivacyDelimiters $dataWithPD\] = [STAF::RemovePrivacyDelimiters $dataWithPD]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set data "!!@Msg: ^!!@Pw is ^^!!@secret^^@!!.^@!!@!!"
set expectedResult "Msg: Pw is secret."
if {[STAF::RemovePrivacyDelimiters $data] != $expectedResult} {
    puts "Error: \[STAF::RemovePrivacyDelimiters $data\] = [STAF::RemovePrivacyDelimiters $data]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set expectedResult "Msg: Pw is secret."
if {[STAF::RemovePrivacyDelimiters $data 0] != $expectedResult} {
    puts "Error: \[STAF::RemovePrivacyDelimiters $data 0\] = [STAF::RemovePrivacyDelimiters $data 0]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set expectedResult "Msg: !!@Pw is ^!!@secret^@!!.@!!"
if {[STAF::RemovePrivacyDelimiters $data 1] != $expectedResult} {
    puts "Error: \[STAF::RemovePrivacyDelimiters $data 1\] = [STAF::RemovePrivacyDelimiters $data 1]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

set expectedResult "Msg: Pw is !!@secret@!!."
if {[STAF::RemovePrivacyDelimiters $data 2] != $expectedResult} {
    puts "Error: \[STAF::RemovePrivacyDelimiters $data 2\] = [STAF::RemovePrivacyDelimiters $data 2]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

# Test private methods passing in an empty string

set data ""
set expectedResult ""
if {[STAF::AddPrivacyDelimiters $data] != $expectedResult} {
    puts "Error: \[STAF::AddPrivacyDelimiters $data\] : [STAF::AddPrivacyDelimiters $data]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

if {[STAF::EscapePrivacyDelimiters $data] != $expectedResult} {
    puts "Error: \[STAF::EscapePrivacyDelimiters $data\] : [STAF::EscapePrivacyDelimiters $data]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

if {[STAF::MaskPrivateData $data] != $expectedResult} {
    puts "Error: \[STAF::MaskPrivateData $data\] : [STAF::MaskPrivateData $data]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}

if {[STAF::RemovePrivacyDelimiters $data] != $expectedResult} {
    puts "Error: \[STAF::RemovePrivacyDelimiters $data\] : [STAF::RemovePrivacyDelimiters $data]"
    puts "       Should return the following instead: $expectedResult"
    exit 1
}


##################################
# Now, let's test STAF::datatype #
##################################

puts "\nTesting data type functions:\n"

# For each type (None is handled a little differently),
#   create (without value)
#   validate type
#   get value
#   set value
#   get value
#   create (with value)
#   get value
#   set value
#   get value
#
# For list and map, also do the following
#   append to STAF::datatype value
#   get value

# Testing data type None

puts "Testing data type: None"

set dtNone [STAF::datatype createNone]

if {[STAF::datatype getType $dtNone] != $STAF::NoneType} {
    puts "Wrong type for a None data type"
    puts "Expected: $STAF::NoneType"
    puts "Found: [STAF::datatype getType $dtNone]"
    exit 1
}

if {[STAF::datatype getValue $dtNone] != "None"} {
    puts "Wrong value for a None data type"
    puts "Expected: None"
    puts "Found: [STAF::datatype getValue $dtNone]"
    exit 1
}

if {[catch {STAF::datatype setValue dtNone "Ok"}] == 0} {
    puts "Error: Successfully set the type of a None data type"
    exit 1
}

set dtNone2 [STAF::datatype createNone "NoneValue"]

if {[STAF::datatype getValue $dtNone2] != "None"} {
    puts "Wrong value for a None data type"
    puts "Expected: None"
    puts "Found: [STAF::datatype getValue $dtNone]"
    exit 1
}

# Testing data type Context

puts "Testing data type: Context"

set dtContext [STAF::datatype createContext]

if {[STAF::datatype getType $dtContext] != $STAF::ContextType} {
    puts "Wrong type for a Context data type"
    puts "Expected: $STAF::ContextType"
    puts "Found: [STAF::datatype getType $dtContext]"
    exit 1
}

# Testing data type Scalar

puts "Testing data type: Scalar"

set dtScalar [STAF::datatype createScalar]

if {[STAF::datatype getType $dtScalar] != "$STAF::ScalarType"} {
    puts "Wrong type for a Scalar data type"
    puts "Expected: $STAF::ScalarType"
    puts "Found: [STAF::datatype getType $dtScalar]"
    exit 1
}

if {[STAF::datatype getValue $dtScalar] != ""} {
    puts "Wrong value for data type"
    puts "Expected: <Empty string>"
    puts "Found: [STAF::datatype getValue $dtScalar]"
    exit 1
}

set scalarTestValue  "Scalar Test Value"

STAF::datatype setValue dtScalar $scalarTestValue

if {[STAF::datatype getValue $dtScalar] != $scalarTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $scalarTestValue"
    puts "Found: [STAF::datatype getValue $dtScalar]"
    exit 1
}

set scalarInitValue "Scalar Init Value"
set dtScalar [STAF::datatype createScalar $scalarInitValue]

if {[STAF::datatype getValue $dtScalar] != $scalarInitValue} {
    puts "Wrong value for data type"
    puts "Expected: $scalarInitValue"
    puts "Found: [STAF::datatype getValue $dtScalar]"
    exit 1
}

STAF::datatype setValue dtScalar $scalarTestValue

if {[STAF::datatype getValue $dtScalar] != $scalarTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $scalarTestValue"
    puts "Found: [STAF::datatype getValue $dtScalar]"
    exit 1
}

# Testing data type List

puts "Testing data type: List"

set dtList [STAF::datatype createList]

if {[STAF::datatype getType $dtList] != "$STAF::ListType"} {
    puts "Wrong type for a List data type"
    puts "Expected: $STAF::ListType"
    puts "Found: [STAF::datatype getType $dtList]"
    exit 1
}

if {[STAF::datatype getValue $dtList] != ""} {
    puts "Wrong value for data type"
    puts "Expected: <Empty string>"
    puts "Found: [STAF::datatype getValue $dtList]"
    exit 1
}

set listTestValue [list "List" "Test" "Value"]

STAF::datatype setValue dtList $listTestValue

if {[STAF::datatype getValue $dtList] != $listTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $listTestValue"
    puts "Found: [STAF::datatype getValue $dtList]"
    exit 1
}

set listInitValue [list "List" "Init" "Value"]
set dtList [STAF::datatype createList $listInitValue]

if {[STAF::datatype getValue $dtList] != $listInitValue} {
    puts "Wrong value for data type"
    puts "Expected: $listInitValue"
    puts "Found: [STAF::datatype getValue $dtList]"
    exit 1
}

STAF::datatype setValue dtList $listTestValue

if {[STAF::datatype getValue $dtList] != $listTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $listTestValue"
    puts "Found: [STAF::datatype getValue $dtList]"
    exit 1
}

lappend listTestValue "And"
lappend listTestValue "More"
lappend dtList "And"
lappend dtList "More"

if {[STAF::datatype getValue $dtList] != $listTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $listTestValue"
    puts "Found: [STAF::datatype getValue $dtList]"
    exit 1
}

# Testing data type Map

puts "Testing data type: Map"

set dtMap [STAF::datatype createMap]

if {[STAF::datatype getType $dtMap] != "$STAF::MapType"} {
    puts "Wrong type for a Map data type"
    puts "Expected: $STAF::MapType"
    puts "Found: [STAF::datatype getType $dtMap]"
    exit 1
}

if {[STAF::datatype getValue $dtMap] != ""} {
    puts "Wrong value for data type"
    puts "Expected: <Empty string>"
    puts "Found: [STAF::datatype getValue $dtMap]"
    exit 1
}

set mapTest(key1) value1
set mapTest(key2) value2
set mapTestValue [array get mapTest]

STAF::datatype setValue dtMap $mapTestValue

if {[STAF::datatype getValue $dtMap] != $mapTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $mapTestValue"
    puts "Found: [STAF::datatype getValue $dtMap]"
    exit 1
}

set mapInit(key3) value3
set mapInit(key4) value4
set mapInitValue [array get mapInit]
set dtMap [STAF::datatype createMap $mapInitValue]

if {[STAF::datatype getValue $dtMap] != $mapInitValue} {
    puts "Wrong value for data type"
    puts "Expected: $mapInitValue"
    puts "Found: [STAF::datatype getValue $dtMap]"
    exit 1
}

STAF::datatype setValue dtMap $mapTestValue

if {[STAF::datatype getValue $dtMap] != $mapTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $mapTestValue"
    puts "Found: [STAF::datatype getValue $dtMap]"
    exit 1
}

set mapAdditionalValue(key5) value5
append mapTestValue " " [array get mapAdditionalValue]
append dtMap        " " [array get mapAdditionalValue]

if {[STAF::datatype getValue $dtMap] != $mapTestValue} {
    puts "Wrong value for data type"
    puts "Expected: $mapTestValue"
    puts "Found: [STAF::datatype getValue $dtMap]"
    exit 1
}

# Using data type None

puts "\nUsing data type: None"

set dtNone [STAF::datatype createNone]
puts "dtNone value: [STAF::datatype getValue $dtNone]"

# Check if an object is a None data type
if {[STAF::datatype getType $dtNone] == $STAF::NoneType} {
    puts "Data Type: [STAF::datatype getType $dtNone]"
}

# Using data type Context

puts "\nUsing data type: Context"

# Create a Context data type with None as the root object
set dtContext [STAF::datatype createContext]
puts "dtContext value: [STAF::datatype getValue $dtContext]"

# Create a Context data type with a List data type as the root object
set dtList [STAF::datatype createList [list "List" "Test" "Value"]]
set dtContext [STAF::mcontext create $dtList]
puts "dtContext value: [STAF::datatype getValue $dtContext]"

# Check if an object is a Context data type
if {[STAF::datatype getType $dtContext] == $STAF::ContextType} {
    puts "Data Type: [STAF::datatype getType $dtContext]"
}

# Using data type Scalar

puts "\nUsing data type: Scalar"

# Create a Scalar data type with an empty string value
set dtScalar [STAF::datatype createScalar]
puts "dtScalar value: [STAF::datatype getValue $dtScalar]"

# Set the value for a Scalar data type to a string
set myString  "Testing 123..."
STAF::datatype setValue dtScalar $myString
puts "dtScalar value: [STAF::datatype getValue $dtScalar]"

# Set the value for a Scalar data type to a number
set myRC 99
set dtScalar [STAF::datatype createScalar $myRC]
puts "dtScalar value: [STAF::datatype getValue $dtScalar]"

# Check if an object is a Scalar data type
if {[STAF::datatype getType $dtScalar] == "$STAF::ScalarType"} {
    puts "Data Type: [STAF::datatype getType $dtScalar]"
}

# Using data type List

puts "\nUsing data type: List"

# Create an empty List data type
set dtList [STAF::datatype createList]
puts "dtList value: [STAF::datatype getValue $dtList]"

# Set the value for a List data type
set listTestValue [list "List" "Test" "Value"]
STAF::datatype setValue dtList $listTestValue
puts "dtList value: [STAF::datatype getValue $dtList]"

# Create a List data type assigning an initial value
set listInitValue [list "List" "Init" "Value"]
set dtList [STAF::datatype createList $listInitValue]
puts "dtList value: [STAF::datatype getValue $dtList]"

# Change the value for a List data type
lappend listTestValue "And"
lappend listTestValue "More"
STAF::datatype setValue dtList $listTestValue
puts "dtList value: [STAF::datatype getValue $dtList]"

# Append entries to a List data type
lappend dtList "And"
lappend dtList "More"
puts "dtList value: [STAF::datatype getValue $dtList]"

# Check if an object is a List data type
if {[STAF::datatype getType $dtList] == "$STAF::ListType"} {
    puts "Data Type: [STAF::datatype getType $dtList]"
}

# Using data type Map

puts "\nUsing data type: Map"

# Create an empty Map data type
set dtMap [STAF::datatype createMap]
puts "dtMap value: [STAF::datatype getValue $dtMap]"

# Set the value for a Map data type
set mapTest(key1) value1
set mapTest(key2) value2
set mapTestValue [array get mapTest]
STAF::datatype setValue dtMap $mapTestValue
puts "dtMap value: [STAF::datatype getValue $dtMap]"

# Create a Map data type assigning an initial value
set mapInit(key3) value3
set mapInit(key4) value4
set mapInitValue [array get mapInit]
set dtMap [STAF::datatype createMap $mapInitValue]
puts "dtMap value: [STAF::datatype getValue $dtMap]"

# Change the value for a Map data type
STAF::datatype setValue dtMap $mapTestValue
puts "dtMap value: [STAF::datatype getValue $dtMap]"

# Add an additional key/value to a Map data type
set mapAdditionalValue(key5) value5
append mapTestValue " " [array get mapAdditionalValue]
append dtMap        " " [array get mapAdditionalValue]
puts "dtMap value: [STAF::datatype getValue $dtMap]"

# Check if an object is a Map data type 
if {[STAF::datatype getType $dtMap] == "$STAF::MapType"} {
    puts "Data Type: [STAF::datatype getType $dtMap]"
}

# Using data types List and Map to create a list of maps

puts "\nUsing data types List and Map to create a list of maps"
set map1(key1) value1
set map1(key2) value2
set map1Value [array get map1]
set dtMap1 [STAF::datatype createMap $map1Value]

set map2(test1) C:/tests/test1.cmd
set map2(test2) C:/tests/test2.sh
set map2Value [array get map2]
set dtMap2 [STAF::datatype createMap $map2Value]

set listValue [list $dtMap1 $dtMap2]
set dtList [STAF::datatype createList $listValue]
puts "dtList: $dtList"
puts "dtList value: [STAF::datatype getValue $dtList]"


############################################
# Now, let's test STAF::mapclassdefinition #
############################################

puts "\nTesting map class definition functions\n"

# Create a map class definition

# create

set mcdName "foo"

set mcd [STAF::mapclassdef create $mcdName]

if {[STAF::mapclassdef getName $mcd] != $mcdName} {
    puts "Wrong name for map class def"
    puts "Expected: $mcdName"
    puts "Found: [STAF::mapclassdef getName $mcd]"
    exit 1
}

# addKey
# setKeyProperty
# getKeys

set mcdKey1Name        "key1"
set mcdKey1DisplayName "Name 1"
set mcdKey2Name        "key2"
set mcdKey2DisplayName "Name 2"
set mcdKey2PName       "display-short-name"
set mcdKey2PValue      "N2"

STAF::mapclassdef addKey mcd $mcdKey1Name $mcdKey1DisplayName
STAF::mapclassdef addKey mcd $mcdKey2Name $mcdKey2DisplayName
STAF::mapclassdef setKeyProperty mcd $mcdKey2Name $mcdKey2PName $mcdKey2PValue

set mcdKeysObj [STAF::mapclassdef getKeys $mcd]

if {[STAF::datatype getType $mcdKeysObj] != "$STAF::ListType"} {
    puts "Wrong type for map class def keys"
    puts "Expected: $STAF::ListType"
    puts "Found: [STAF::datatype getType $mcdKeysObj]"
    exit 1
}

foreach mcdKeyObj [STAF::datatype getValue $mcdKeysObj] {
    if {[STAF::datatype getType $mcdKeyObj] != "$STAF::MapType"} {
        puts "Wrong type for a map class def key"
        puts "Expected: $STAF::MapType"
        puts "Found: [STAF::datatype getType $mcdKeysObj]"
        exit 1
    }

    array set mcdKey [STAF::datatype getValue $mcdKeyObj]

    if {$mcdKey(key) == $mcdKey1Name} {
        if {$mcdKey(display-name) != $mcdKey1DisplayName} {
            puts "Error: Invalid key display-name in map class definition"
            puts "Expected: $mcdKey1DisplayName"
            puts "Found: $mcdKey(display-name)"
            exit 1
        }
    } elseif {$mcdKey(key) == $mcdKey2Name} {
        if {$mcdKey(display-name) != $mcdKey2DisplayName} {
            puts "Error: Invalid key display-name in map class definition"
            puts "Expected: $mcdKey2DisplayName"
            puts "Found: $mcdKey(display-name)"
            exit 1
        }

        if {$mcdKey($mcdKey2PName) != $mcdKey2PValue} {
            puts "Error: Invalid key property value in map class definition"
            puts "Expected: $mcdKey2PValue"
            puts "Found: $mcdKey($mcdKey2PName)"
            exit 1
        }
    } else {
        puts "Error: Invalid key in map class definition"
        puts "Expected: <No key>"
        puts "Found: $mcdKey(key)"
        exit 1
    }
}

# createInstance

set mcdInstanceObj [STAF::mapclassdef createInstance $mcd]

if {[STAF::datatype getType $mcdInstanceObj] != "$STAF::MapType"} {
    puts "Wrong type for map class def keys"
    puts "Expected: $STAF::MapType"
    puts "Found: [STAF::datatype getType $mcdInstanceObj]"
    exit 1
}

array set mcdInstance [STAF::datatype getValue $mcdInstanceObj]

if {$mcdInstance(staf-map-class-name) != $mcdName} {
    puts "Wrong map class definition type for map class definition instance"
    puts "Expected: $mcdName"
    puts "Found: $mcdInstance(staf-map-class-name)"
    exit 1
}

# getMapClassDefinitionObject

if {[STAF::mapclassdef getMapClassDefinitionObject $mcd] != $mcd} {
    puts "Wrong map class definition object"
    puts "Expected: $mcd"
    puts "Found: [STAF::mapclassdef getMapClassDefinitionObject $mcd]"
    exit 1
}

# create(with map class definition)

set mcd2Name "bar"

set mcd2 [STAF::mapclassdef create $mcd2Name [STAF::mapclassdef getMapClassDefinitionObject $mcd]]

if {[STAF::mapclassdef getName $mcd2] != $mcdName} {
    puts "Wrong name for map class def"
    puts "Expected: $mcdName"
    puts "Found: [STAF::mapclassdef getName $mcd2]"
    exit 1
}

if {[STAF::mapclassdef getMapClassDefinitionObject $mcd2] != $mcd} {
    puts "Wrong map class definition object"
    puts "Expected: $mcd"
    puts "Found: [STAF::mapclassdef getMapClassDefinitionObject $mcd2]"
    exit 1
}

# Run mapclassdef example in STAF Tcl User's Guide

set myMapClassDef [STAF::mapclassdef create "Test/MyMap"]
STAF::mapclassdef addKey myMapClassDef "name" "Name"
STAF::mapclassdef addKey myMapClassDef "exec" "Executable"
STAF::mapclassdef addKey myMapClassDef "testType" "Test Type"
STAF::mapclassdef setKeyProperty myMapClassDef "testType" "display-short-name" "Test"
STAF::mapclassdef addKey myMapClassDef "outputList" "Outputs"

set mapClassDefName [STAF::mapclassdef getName $myMapClassDef]
puts "The keys for map class definition '$mapClassDefName' are:"
puts "[STAF::formatObject [STAF::mapclassdef getKeys $myMapClassDef]]\n"

##################################
# Now, let's test STAF::mcontext #
##################################

puts "Testing marshalling context functions"

# create

set context [STAF::mcontext create]

if {[STAF::datatype getType $context] != "$STAF::ContextType"} {
    puts "Wrong type for marshalling context"
    puts "Expected: $STAF::ContextType"
    puts "Found: [STAF::datatype getType $context]"
    exit 1
}

if {[STAF::mcontext getRootObject $context] != [STAF::datatype createNone]} {
    puts "Error: New marshalling context should have a root object of None"
    puts "Found: [STAF::mcontext getRootObject $context]"
    exit 1
}

# XXX: Need an actual test of some type here

# setMapClassDefinition
# hasMapClassDefinition
# getMapClassDefinition

STAF::mcontext setMapClassDefinition context $mcd

if {! [STAF::mcontext hasMapClassDefinition $context $mcdName]} {
    puts "Error: marshalling context does not contain map class definition"
    puts "Expected to find map class definition $mcdName"
    puts "Did not find it"
    exit 1
}

set testMCD [STAF::mcontext getMapClassDefinition $context $mcdName]

if {$testMCD != $mcd} {
    puts "Error: map class definitions do not match"
    puts "Map class 1: $testMCD"
    puts "Map class 2: $mcd"
    exit 1
}

# getMapClassMap

set mapClassMap [STAF::mcontext getMapClassMap $context]

# getMapClassDefinitionNames

set mcd3Name "baz"
set mcd3 [STAF::mapclassdef create $mcd3Name]

STAF::mapclassdef addKey mcd3 $mcdKey1Name $mcdKey1DisplayName
STAF::mapclassdef addKey mcd3 $mcdKey2Name $mcdKey2DisplayName

STAF::mcontext setMapClassDefinition context $mcd3

set mcdNames [STAF::mcontext getMapClassDefinitionNames $context]

if {[llength $mcdNames] != 2} {
    puts "Error: Incorrect number of map class definition in marshalling context"
    puts "Expected: 2"
    puts "Found: [llength $mcdNames]"
    exit 1
}

if {( [lsearch -exact $mcdNames $mcdName] == -1 ) ||
    ( [lsearch -exact $mcdNames $mcd3Name] == -1 )} {
    puts "Error: Invalid map class definition names in marshalling context"
    puts "Expected: $ncdName and $mcd3Name"
    puts "Found: $mcdNames"
    exit 1
}

# setRootObject
# getRootObject

set testArray(a) "Hello"
set testArray(b) "World"
set testArrayString [array get testArray]
set arrayObject [STAF::datatype createMap $testArrayString]

STAF::mcontext setRootObject context $arrayObject
set testArrayObject [STAF::mcontext getRootObject $context]

if {$testArrayObject != $arrayObject} {
    puts "Error: Incorrect root object"
    puts "Expected: $arrayObject"
    puts "Found: $testArrayObject"
    exit 1
}

# getPrimaryObject

if {[STAF::mcontext getPrimaryObject $context] != $context} {
    puts "Error: Wrong primary object for marshalling context"
    puts "Expected: $context"
    puts "Found: [STAF::mcontext getPrimaryObject $context]"
    exit 1
}

set context2 [STAF::mcontext create]
STAF::mcontext setRootObject context2 $arrayObject

if {[STAF::mcontext getPrimaryObject $context2] != $arrayObject} {
    puts "Error: Wrong primary object for marshalling context"
    puts "Expected: $arrayObject"
    puts "Found: [STAF::mcontext getPrimaryObject $context2]"
    exit 1
}

# Run mcontext examples in STAF Tcl User's Guide for additional testing,
# including testing the STAF::mcontext marshall subcommand.

# Create a map class definition

set myMapClassDef [STAF::mapclassdef create "Test/MyMap"]
STAF::mapclassdef addKey myMapClassDef "name" "Name"
STAF::mapclassdef addKey myMapClassDef "exec" "Executable"

# Create a marshalling context and set the map class definition
# and assign myTestList as the root object

set mc [STAF::mcontext create]
STAF::mcontext setMapClassDefinition mc $myMapClassDef

# From a list of maps, create a list datatype of map class datatypes
# and marshall that data and assign to a message

set testList [list {exec /tests/TestA.py name TestA} \
                   {exec /tests/TestB.sh name TestB} \
                   {exec /tests/TestC.cmd name TestC}]

set myTestList [STAF::datatype createList]

foreach testObj $testList {
   array set test $testObj
   set testMapObj [STAF::mapclassdef createInstance $myMapClassDef]
   array set testMap [STAF::datatype getValue $testMapObj]
   set testMap(name) $test(name)
   set testMap(exec) $test(exec)
   lappend myTestList [STAF::datatype createMap [array get testMap]]
}

STAF::mcontext setRootObject mc $myTestList

set output1 [STAF::mcontext formatObject $mc]
puts "\nTest List:\n$output1"

# Create a string from the marshalling context
# This string could be a message that you log or send to a queue, etc.

set stringResult [STAF::mcontext marshall $mc]

# Convert the marshalled string representation back into a list

set mc2 [STAF::unmarshall $stringResult]
set theTestList [STAF::mcontext getRootObject $mc2]
puts "\nTest List:\n[STAF::mcontext formatObject $mc2]"

# Create a map class definition passing in the root object
set mc3 [STAF::mcontext create $myTestList]
STAF::mcontext setMapClassDefinition mc3 $myMapClassDef
set output2 [STAF::mcontext formatObject $mc3]
if {[string compare $output1 $output2] != 0} {
    puts "Error: Incorrect formatObject output"
    puts "Expected:\n$output1"
    puts "Found:\n$output2"
    exit 1
}

# Testing STAF::marshall

puts "\nTesting marshall function\n"

puts "Test marshalling a Scalar datatype object"
set myString "This is a test"
puts [STAF::mcontext marshall $myString]

if {[STAF::isMarshalledData $myString]} {
    puts "Error: STAF::isMarshalledData failed"
    puts "Expected: 0"
    puts "Found: [STAF::isMarshalledData $myString]"
    exit 1
}

puts "\nTest marshalling a Context datatype object that doesn't contain any map classes"
set dtList [STAF::datatype createList]
lappend dtList "value 1"
lappend dtList "value 2"
set mc [STAF::datatype createContext]
STAF::mcontext setRootObject mc $dtList
puts [STAF::mcontext marshall $mc]

puts "\nTest marshalling a Map datatype object"
set myTestArray(name) "TestA"
set myTestArray(exec) "/tests/TestA.py"
set myTestArray(testType) "FVT"
set myTestArray(outputs) {"TestA.out" "TestA.err"}
set myTestArrayString [array get myTestArray]
set myArrayObject [STAF::datatype createMap $myTestArrayString]

set message [STAF::marshall $myArrayObject]

if {![STAF::isMarshalledData $message]} {
    puts "Error: STAF::isMarshalledData failed"
    puts "Expected: 1"
    puts "Found: [STAF::isMarshalledData $message]"
    exit 1
}

set request "QUEUE MESSAGE [STAF::WrapData $message]"

if {[STAF::Submit local QUEUE $request] != $STAF::kOk} {
    puts "Error on STAF local QUEUE $request"
    puts "RC=$STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

# Another process could obtain the message from the queue and unmarshall
# it to get the original dictionary (map) object

if {[STAF::Submit local QUEUE GET] != $STAF::kOk} {
    puts "Error on STAF local QUEUE GET"
    puts "RC=$STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

set mc [STAF::unmarshall $STAF::Result]
set messageMapObj [STAF::mcontext getRootObject $mc]
array set messageMap [STAF::datatype getValue $messageMapObj]
array set yourTestArray [STAF::datatype getValue $messageMap(message)]
puts "Name     : $yourTestArray(name)"
puts "Exec     : $yourTestArray(exec)"
puts "Test Type: $yourTestArray(testType)"
puts "Outputs  : $yourTestArray(outputs)"

if {$myTestArray(name) != $yourTestArray(name)} {
    puts "Error: name mismatch"
    puts "Expected name=$myTestArray(name)"
    puts "Got name=$yourTestArray(name)"
}
if {$myTestArray(exec) != $yourTestArray(exec)} {
    puts "Error: exec mismatch"
    puts "Expected exec=$myTestArray(exec)"
    puts "Got exec=$yourTestArray(exec)"
}
if {$myTestArray(name) != $yourTestArray(name)} {
    puts "Error: testType mismatch"
    puts "Expected testType=$myTestArray(testType)"
    puts "Got testType=$yourTestArray(testType)"
}
if {$myTestArray(outputs) != $yourTestArray(outputs)} {
    puts "Error: outputs mismatch"
    puts "Expected outputs=$myTestArray(outputs)"
    puts "Got outputs=$yourTestArray(outputs)"
}

# Create a map class definition

set myMapClassDef [STAF::mapclassdef create "Test/MyMap"]
STAF::mapclassdef addKey myMapClassDef "name" "Name"
STAF::mapclassdef addKey myMapClassDef "exec" "Executable"

# Create a marshalling context and set the map class definition

set mc [STAF::mcontext create]
STAF::mcontext setMapClassDefinition mc $myMapClassDef

# From a list of maps, create a list datatype of map class datatypes
# and marshall that data and assign to a message

set testList [list {exec /tests/TestA.py name TestA} \
                   {exec /tests/TestB.sh name TestB} \
                   {exec /tests/TestC.cmd name TestC}]

set myTestList [STAF::datatype createList]

foreach testObj $testList {
   array set test $testObj
   set testMapObj [STAF::mapclassdef createInstance $myMapClassDef]
   array set testMap [STAF::datatype getValue $testMapObj]
   set testMap(name) $test(name)
   set testMap(exec) $test(exec)
   lappend myTestList [STAF::datatype createMap [array get testMap]]
}

# Test marshalling a List datatype object
set message [STAF::marshall -context $mc $myTestList]
puts "\nMarshalled Data:\n$message"
set formattedData1 [STAF::formatObject -context $mc $myTestList]
puts "\nFormatted Data:\n$formattedData1"

STAF::mcontext setRootObject mc $myTestList
set formattedData2 [STAF::formatObject $mc]

if {[string compare $formattedData1 $formattedData2] != 0} {
    puts "Error: Incorrect formatObject output testing marshall"
    puts "Expected:\n$formattedData1"
    puts "Found:\n$formattedData2"
    exit 1
}

# Testing STAF::unmarshall

puts "\nTesting unmarshall function\n"

# XXX: Causes error because references a mapclassdef that isn't in the context
# This may be an error in all unmarshall methods.  Need to investigate.
#set myMC [STAF::unmarshall $message]

# Submit a query request to the FS Service to query info about a file

set fileName "{STAF/Config/ConfigFile}"
set request "QUERY ENTRY $fileName"
puts "\nSTAF local FS $request\n"

if {[STAF::Submit local FS $request] != $STAF::kOk} {
    puts "Error on STAF local FS $request"
    puts "RC=$STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

set mc [STAF::unmarshall $STAF::Result]
set entryMapObj [STAF::mcontext getRootObject $mc]
array set entryMap [STAF::datatype getValue $entryMapObj]

# Submit a resolve requset to the VAR service to resolve the STAF variable
STAF::Submit local VAR "RESOLVE STRING $fileName"
set resolvedFileName $STAF::Result

if {$entryMap(type) != "F"} {
    puts "$resolvedFileName is not a file.  Type=$entryMap(type)"
} else {
    puts "File Name    : $resolvedFileName"
    puts "File Size    : $entryMap(lowerSize)"
    puts "Last Modified: $entryMap(lastModifiedTimestamp)"
}

# Submit a PROCESS START request and wait for it to complete

set command "dir {STAF/Config/STAFRoot}{STAF/Config/Sep/File}d*"
set request "START SHELL COMMAND [STAF::WrapData $command] RETURNSTDOUT STDERRTOSTDOUT WAIT"
puts "\nSTAF local PROCESS $request"

if {[STAF::Submit local PROCESS $request] != $STAF::kOk} {
    puts "Error on STAF local PROCESS $request"
    puts "Expected RC: 0"
    puts "Received RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

# Unmarshall the result which is a marshalling context whose 
# root object is a map containing keys 'rc', and 'fileList'.
# The value for 'fileList' is a list of the returned files.
# Each entry in the list consists of a map that contains keys
# 'rc' and 'data'.  In our PROCESS START request, we returned
# one file, stdout (and returned stderr to this same file).

set mc [STAF::unmarshall $STAF::Result]
set processMapObj [STAF::mcontext getRootObject $mc]
array set processMap [STAF::datatype getValue $processMapObj]

# Verify that the rc is 0 for returning data for the Stdout file

set fileListObj [STAF::datatype getValue $processMap(fileList)]
set stdoutFileObj [STAF::datatype getValue [lindex $fileListObj 0]]
array set stdoutFileMap [STAF::datatype getValue $stdoutFileObj]

if {$stdoutFileMap(rc) != $STAF::kOk} {
    puts "Error on retrieving process's stdout data."
    puts "Expected RC: 0"
    puts "Received RC: $stdoutFileMap(rc)"
    exit $stdoutFileMap(rc)
}

# Print the data in the stdout file created by the process

puts "\nProcess Stdout file contains:\n$stdoutFileMap(data)"

# Verify that the process rc is 0

if {$processMap(rc) != $STAF::kOk} {
    puts "Process RC: $processMap(rc)"
    exit $processMap(rc)
}

#foreach fileObj [STAF::datatype getValue $processMap(fileList)] {
#    array set fileMap [STAF::datatype getValue $fileObj]
#    if {$fileMap(rc) == 0} {
#        puts "File data:\n$fileMap(data)"
#    } else {
#        puts "Error getting file, RC: $fileMap(rc)"
#    }
#}

# Create some invalid marshalling data and queue it;  Get it off the queue,
# and unmarshall it and verify results in the invalid marshalling data string
# in the message
# XXX: The following test doesn't work because the fix made for Bug #2515811
#      hasn't been implemented yet for the Tcl unmarshall method so that
#      an error won't be raised if invaild marshalled data is unmarshalled.

#puts "\nTesting unmarshalling data that contains invalid marshalled data"

#set message "@SDT/{:177::2:RC@SDT/\$S:1:0"
#append message ":6:IPInfo@SDT/\$S:36:9.42.126.76|255.255.252.0|9.42.124.1"
#append message ":3:Msg@SDT/\$S:46:Static IP arguments are processed successfully"
#append message ":9:Timestamp@SDT/\$S:19:2009-01-16 14:41:4"
#append message "Connecting to: http://9.42.106.28:8080"

#set request "QUEUE MESSAGE [STAF::WrapData $message]"
#puts "\nSTAF local QUEUE $request"
#if {[STAF::Submit local QUEUE $request] != $STAF::kOk} {
#    puts "Error on STAF local QUEUE $request"
#    puts "Expected RC: 0"
#    puts "Received RC: $STAF::RC, Result: $STAF::Result"
#    exit $STAF::RC
#}

# Another process could obtain the message from the queue and unmarshall
# it to get the original dictionary (map) object
# XXX: This request will fail when it tries to auto-unmarshall the result

#set request "GET WAIT 5000"
#puts "\nSTAF local QUEUE $request"
#if {[STAF::Submit local QUEUE $request] != $STAF::kOk} {
#    puts "Error on STAF local QUEUE $request"
#    puts "Expected RC: 0"
#    puts "Received RC: $STAF::RC, Result: $STAF::Result"
#    exit $STAF::RC
#}

#set mc [STAF::unmarshall $STAF::Result]
#set msgMapObj [STAF::mcontext getRootObject $mc]
#array set messageMap [STAF::datatype getValue $msgMapObj]

#puts "\nQueued message map:"
#puts [STAF::formatObject $messageMap]

#if {$messageMap(message) != $message} {
#    puts "ERROR: Message not same as original message sent"
#    puts "Expected: $message"
#    puts "Received: $messageMap(message)"
#    exit $STAF::RC
#}
# End commented out test

######################################
# Now, let's test STAF::formatObject #
######################################

puts "Testing formatObject"

puts "\nTest printing a None datatype\n"
set dtNone [STAF::datatype createNone]
puts "STAF::formatObject \$dtNone:"
puts [STAF::formatObject $dtNone]

puts "\nTest printing a Scalar datatype\n"
set testString "This is a test"
puts "STAF::formatObject \$testString:"
puts [STAF::formatObject $testString]

puts "\nTest printing a List datatype containing only scalars\n"
set listValue [list "Item #1" "Item #2" "Item #3"]
lappend listValue $dtNone
set dtList [STAF::datatype createList $listValue]
puts "STAF::formatObject \$dtList:"
set listOutput1 [STAF::formatObject $dtList]
puts [STAF::formatObject $listOutput1]

puts "\nTest printing a List datatype using the -context option\n"
puts "STAF::formatObject -context [STAF::mcontext create \$dtList] \$dtList:"
set listOutput2 [STAF::formatObject -context [STAF::mcontext create $dtList] $dtList]
if {[string compare $listOutput1 $listOutput2]} {
    puts "Error: Incorrect formatObject output printing a List datatype"
    puts "Expected:\n$listOutput1"
    puts "Found:\n$listOutput2"
    exit 1
}

puts "\nTest printing a List datatype using the -context and -indentLevel options\n"
puts "STAF::formatObject -context [STAF::mcontext create \$dtList] -indentLevel 0 \$dtList:"
set listOutput2 [STAF::formatObject -context [STAF::mcontext create $dtList] -indentLevel 0 $dtList]
if {[string compare $listOutput1 $listOutput2]} {
    puts "Error: Incorrect formatObject output printing a List datatype"
    puts "Expected:\n$listOutput1"
    puts "Found:\n$listOutput2"
    exit 1
}

puts "\nTest printing a List datatype containing another list datatype\n"
set listValue [list "Item #4a" "Item #4b"]
lappend dtList [STAF::datatype createList $listValue]
#set dtList [STAF::datatype createList $listValue]
puts "STAF::formatObject \$dtList:"
puts [STAF::formatObject $dtList]

puts "\nTest printing a Map datatype containing only Scalar/None values\n"
set mapInit(key1) value1
set mapInit(key2) value2
set mapInit(key3) $dtNone
set mapInitValue [array get mapInit]
set dtMap [STAF::datatype createMap $mapInitValue]
puts "STAF::formatObject \$dtMap:"
puts [STAF::formatObject $dtMap]

puts "\nTest printing a Map datatype containing Scalar and List objects\n"
set myTestMap(name) TestA
set myTestMap(exec) "/tests/TestA.tcl"
set myTestMap(testType) FVT
set listValue [list TestA.out TestA.err]
set dtList [STAF::datatype createList $listValue]
set myTestMap(outputs) $dtList
set dtTestMap [STAF::datatype createMap [array get myTestMap]]
puts "STAF::formatObject \$dtTestMap:"
puts [STAF::formatObject $dtTestMap]

puts "\nTest printing the process Map datatype object without a context\n"
puts "STAF::formatObject \$processMapObj:"
puts [STAF::formatObject $processMapObj]

puts "\nTest printing the process Map datatype object with a context\n"
puts "STAF::formatObject -context \$mc \$processMapObj:"
set processOutput1 [STAF::formatObject -context $mc $processMapObj]
puts $processOutput1

puts "\nTest printing the result from a PROCESS START WAIT request"
puts "STAF::formatObject \$mc:"
set processOutput2 [STAF::formatObject $mc]
if {[string compare $processOutput1 $processOutput2]} {
    puts "Error: Incorrect formatObject output printing the PROCESS START result"
    puts "Expected:\n$processOutput1"
    puts "Found:\n$processOutput2"
    exit 1
}

puts "\nTest printing a Map datatype which is the output from a FS QUERY ENTRY request\n"
set fileName "{STAF/Config/ConfigFile}"
set request "QUERY ENTRY $fileName"
puts "STAF local FS $request\n"

if {[STAF::Submit local FS $request] != $STAF::kOk} {
    puts "Error on STAF local FS $request"
    puts "RC=$STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

set mc [STAF::unmarshall $STAF::Result]
puts "Formatted output:\n[STAF::formatObject $mc]"


puts "\n**************************************************************"
puts "Test Performance for Marshalling, FormatObject, and Unmarshall"
puts "**************************************************************"

# Test using a list with the specified number of entries

set entries 2500

puts "\nTest using a list with $entries entries"

set dtList [STAF::datatype createList]
set i 0 

while {$i < $entries} {
    lappend dtList "entryValue ##$i" 
    incr i +1
} 

set mc [STAF::datatype createContext]
STAF::mcontext setRootObject mc $dtList

puts "FormatObject started : [clock format [clock seconds]]"
STAF::formatObject $mc
puts "FormatObject ended   : [clock format [clock seconds]]"

puts "Marshalling started  : [clock format [clock seconds]]"
set result [STAF::mcontext marshall $mc] 
puts "Marshalling ended    : [clock format [clock seconds]]"

puts "Length of marshalled data: [string length $result]"

puts "Unmarshalling started: [clock format [clock seconds]]"
set mc [STAF::unmarshall $result]
puts "Unmarshalling ended  : [clock format [clock seconds]]"


puts "\nTest running a process that returns data beginning with -"

set command "echo"
set parms "-hello-"
set request "START SHELL COMMAND [STAF::WrapData $command] PARMS [STAF::WrapData $parms] RETURNSTDOUT STDERRTOSTDOUT WAIT"
puts "\nSTAF local PROCESS $request"

if {[STAF::Submit local PROCESS $request] != $STAF::kOk} {
    puts "Error on STAF local PROCESS $request"
    puts "Expected RC: 0"
    puts "Received RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

# Unmarshall the result which is a marshalling context whose 
# root object is a map containing keys 'rc', and 'fileList'.
# The value for 'fileList' is a list of the returned files.
# Each entry in the list consists of a map that contains keys
# 'rc' and 'data'.  In our PROCESS START request, we returned
# one file, stdout (and returned stderr to this same file).

set mc [STAF::unmarshall $STAF::Result]
set processMapObj [STAF::mcontext getRootObject $mc]
array set processMap [STAF::datatype getValue $processMapObj]

# Verify that the rc is 0 for returning data for the Stdout file

set fileListObj [STAF::datatype getValue $processMap(fileList)]
set stdoutFileObj [STAF::datatype getValue [lindex $fileListObj 0]]
array set stdoutFileMap [STAF::datatype getValue $stdoutFileObj]

if {$stdoutFileMap(rc) != $STAF::kOk} {
    puts "Error on retrieving process's stdout data."
    puts "Expected RC: 0"
    puts "Received RC: $stdoutFileMap(rc)"
    exit $stdoutFileMap(rc)
}

# Print the data in the stdout file created by the process

puts "\nProcess Stdout file contains:\n$stdoutFileMap(data)"

# Verify that the process rc is 0

if {$processMap(rc) != $STAF::kOk} {
    puts "Process RC: $processMap(rc)"
    exit $processMap(rc)
}


puts "\nTest running a process that returns data beginning with --"

set command "echo"
set parms "--hello--"
set request "START SHELL COMMAND [STAF::WrapData $command] PARMS [STAF::WrapData $parms] RETURNSTDOUT STDERRTOSTDOUT WAIT"
puts "\nSTAF local PROCESS $request"

if {[STAF::Submit local PROCESS $request] != $STAF::kOk} {
    puts "Error on STAF local PROCESS $request"
    puts "Expected RC: 0"
    puts "Received RC: $STAF::RC, Result: $STAF::Result"
    exit $STAF::RC
}

# Unmarshall the result which is a marshalling context whose 
# root object is a map containing keys 'rc', and 'fileList'.
# The value for 'fileList' is a list of the returned files.
# Each entry in the list consists of a map that contains keys
# 'rc' and 'data'.  In our PROCESS START request, we returned
# one file, stdout (and returned stderr to this same file).

set mc [STAF::unmarshall $STAF::Result]
set processMapObj [STAF::mcontext getRootObject $mc]
array set processMap [STAF::datatype getValue $processMapObj]

# Verify that the rc is 0 for returning data for the Stdout file

set fileListObj [STAF::datatype getValue $processMap(fileList)]
set stdoutFileObj [STAF::datatype getValue [lindex $fileListObj 0]]
array set stdoutFileMap [STAF::datatype getValue $stdoutFileObj]

if {$stdoutFileMap(rc) != $STAF::kOk} {
    puts "Error on retrieving process's stdout data."
    puts "Expected RC: 0"
    puts "Received RC: $stdoutFileMap(rc)"
    exit $stdoutFileMap(rc)
}

# Print the data in the stdout file created by the process

puts "\nProcess Stdout file contains:\n$stdoutFileMap(data)"

# Verify that the process rc is 0

if {$processMap(rc) != $STAF::kOk} {
    puts "Process RC: $processMap(rc)"
    exit $processMap(rc)
}


#############
# Finish up #
#############

if {[STAF::UnRegister] != 0} {
    puts "Error unregistering with STAF, RC: $STAF::RC"
    exit $STAF::RC
}

puts "All tests successful"

exit 0
