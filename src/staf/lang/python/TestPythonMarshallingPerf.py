#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2006                                              #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

#===========================================================================
# TestPythonMarshallingPerf - Tests performance of Python marshalling and
#                             formatObject
#===========================================================================
# Accepts: The number of entries to marshall
# Returns: 0 if successful;  non-zero if not successful
#===========================================================================
# Purpose: Create some marshalled data for a specified number of entries to
# test performance of Python marshalling and formatObject.
#
# Expected Result:
"""

C:\dev\sf\src\staf\lang\python>python TestPythonMarshallingPerf.py 100000

*************************************************************
Test for Errors in Marshalling, FormatObject, and Unmarshall
************************************************************

Test for errors using a list with 1 entries of map class objects with 2 keys

Verify you can format, marshall, and unmarshall an object that references a map
class but does not define the map class in the context:

FormatObject result without map class definition in context:
[
  {
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key1               : Value 1 1
    key2               : Value 2 1
  }
]

Marshalled string:
@SDT/[1:127:@SDT/{:116::19:staf-map-class-name@SDT/$S:30:STAF/Test/MyMapClassDef
inition:4:key1@SDT/$S:9:Value 1 1:4:key2@SDT/$S:9:Value 2 1

Length of marshalled string: 139

Unmarshall and call FormatObject on the context:
[
  {
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key1               : Value 1 1
    key2               : Value 2 1
  }
]

Print root list object as a formatted string:
[
  {
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key1               : Value 1 1
    key2               : Value 2 1
  }
]

FormatObject result with wrong map class definition in context:
[
  {
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key1               : Value 1 1
    key2               : Value 2 1
  }
]

Marshalling string with wrong map class definition in context:
@SDT/*:289:@SDT/{:139::13:map-class-map@SDT/{:111::31:STAF/Test/MyMapClassDefini
tion2@SDT/{:66::4:keys@SDT/[0:0::4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition
2@SDT/[1:127:@SDT/{:116::19:staf-map-class-name@SDT/$S:30:STAF/Test/MyMapClassDe
finition:4:key1@SDT/$S:9:Value 1 1:4:key2@SDT/$S:9:Value 2 1

Length of marshalled data: 300

Unmarshall and call FormatObject on the context:
[
  {
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key1               : Value 1 1
    key2               : Value 2 1
  }
]

FormatObject result with map class definition in context:
[
  {
    Key #1: Value 1 1
    Key #2: Value 2 1
  }
]

Add a map object created for the map class definition with no keys

Verify you can format, marshall, and unmarshall an object that references a map
class without any keys defined:

FormatObject Result:
[
  {
    Key #1: Value 1 1
    Key #2: Value 2 1
  }
  {
  }
]

MarshalledString:
@SDT/*:525:@SDT/{:375::13:map-class-map@SDT/{:347::31:STAF/Test/MyMapClassDefini
tion2@SDT/{:66::4:keys@SDT/[0:0::4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition
2:30:STAF/Test/MyMapClassDefinition@SDT/{:191::4:keys@SDT/[2:124:@SDT/{:52::12:d
isplay-name@SDT/$S:6:Key #1:3:key@SDT/$S:4:key1@SDT/{:52::12:display-name@SDT/$S
:6:Key #2:3:key@SDT/$S:4:key2:4:name@SDT/$S:30:STAF/Test/MyMapClassDefinition@SD
T/[2:127:@SDT/%:72::30:STAF/Test/MyMapClassDefinition@SDT/$S:9:Value 1 1@SDT/$S:
9:Value 2 1@SDT/%:35::31:STAF/Test/MyMapClassDefinition2

Length of marshalled data: 536

Unmarshall and call FormatObject on the context
[
  {
    Key #1: Value 1 1
    Key #2: Value 2 1
  }
  {
  }
]

Verify you can format, marshall, and unmarshall an object that references a map
class without a key that it doesn't provide an entry for:

FormatObject Result:
[
  {
    Key #1: Value 1 1
    Key #2: Value 2 1
  }
  {
    Key YYY: <None>
    Key XXX: ValueXXX
  }
]

MarshalledString:
@SDT/*:686:@SDT/{:508::13:map-class-map@SDT/{:480::31:STAF/Test/MyMapClassDefini
tion2@SDT/{:198::4:keys@SDT/[2:130:@SDT/{:55::12:display-name@SDT/$S:7:Key YYY:3
:key@SDT/$S:6:KeyYYY@SDT/{:55::12:display-name@SDT/$S:7:Key XXX:3:key@SDT/$S:6:K
eyXXX:4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition2:30:STAF/Test/MyMapClassDe
finition@SDT/{:191::4:keys@SDT/[2:124:@SDT/{:52::12:display-name@SDT/$S:6:Key #1
:3:key@SDT/$S:4:key1@SDT/{:52::12:display-name@SDT/$S:6:Key #2:3:key@SDT/$S:4:ke
y2:4:name@SDT/$S:30:STAF/Test/MyMapClassDefinition@SDT/[2:155:@SDT/%:72::30:STAF
/Test/MyMapClassDefinition@SDT/$S:9:Value 1 1@SDT/$S:9:Value 2 1@SDT/%:63::31:ST
AF/Test/MyMapClassDefinition2@SDT/$0:0:@SDT/$S:8:ValueXXX

Length of marshalled data: 697

Unmarshall and call FormatObject onn the context
[
  {
    Key #1: Value 1 1
    Key #2: Value 2 1
  }
  {
    Key YYY: <None>
    Key XXX: ValueXXX
  }
]

**************************************************************
Test Performance for Marshalling, FormatObject, and Unmarshall
**************************************************************

Test using a list with 100000 entries

FormatObject starting...
FormatObject Elapsed Time: 1 seconds
Marshalling starting...
Marshalling Elapsed Time: 1 seconds
Length of marshalled string: 2888911
Unmarshalling starting...
Unmarshalling Elapsed Time: 2 seconds

Test using a map with 100000 entries

FormatObject starting...
FormatObject Elapsed Time: 2 seconds
Marshalling starting...
Marshalling Elapsed Time: 2 seconds
Length of marshalled string: 3167795
Unmarshalling starting...
Unmarshalling Elapsed Time: 3 seconds

Test using a list with 10000 entries of map class objects each with 10 keys

FormatObject starting...
FormatObject Elapsed Time: 3 seconds
Marshalling starting...
Marshalling Elapsed Time: 2 seconds
Length of marshalled string: 2749668
Unmarshalling starting...
Unmarshalling Elapsed Time: 3 seconds

"""
#===========================================================================

from PySTAF import *
import string
import sys
import time

def main():

    # Verify the command line arguments

    args = sys.argv

    if len(args) != 2:
        print "Usage: python TestPythonMarshallingPerf <number>"
        sys.exit(1)

    entries = 1

    #############################################################
    # Test for errors in marshalling/unmarshalling/formatObject #
    #############################################################

    print "\n*************************************************************"
    print "Test for Errors in Marshalling, FormatObject, and Unmarshall"
    print "************************************************************"

    numKeys = 2;

    print "\nTest for errors using a list with %s entries of map class objects with %s keys" % (entries, numKeys)

    # Define a map class with 2 keys

    myMapClass = STAFMapClassDefinition('STAF/Test/MyMapClassDefinition')

    for k in range(1, numKeys + 1):
        myMapClass.addKey("key%s" % (k), "Key #%s" % (k))

    # Create a marshalling context

    mc = STAFMarshallingContext()
    
    resultList = []

    for i in range(1, entries + 1):
        # Create an instance of this map class definition and assign
        # data to the map class instance

        theMap = myMapClass.createInstance()

        for j in range(1, numKeys + 1):
            theMap["key%s" % (j)] = "Value %s %s" % (j, i)

        resultList.append(theMap)

    mc.setRootObject(resultList)

    print ("\nVerify you can format, marshall, and unmarshall an object " +
           "that references a map class but does not define the map class " +
           "in the context:")

    print "\nFormatObject result without map class definition in context:"
    print formatObject(mc)
    
    result = mc.marshall()
    print "\nMarshalled string:"
    print result

    print "\nLength of marshalled string: %s" % (len(result))

    mc2 = unmarshall(result)
    print "\nUnmarshall and call FormatObject on the context:"
    print formatObject(mc2)

    # Verify that calling formatObject on an object that references a
    # map class but does not provide a marshalling context does not
    # cause an error.

    outputList = mc2.getRootObject();
    print "\nPrint root list object as a formatted string:\n%s" % \
          (formatObject(outputList))
    
    # Now create another map class definition with no keys and add it to
    # the context but don't reference it.

    myMapClass2 = STAFMapClassDefinition("STAF/Test/MyMapClassDefinition2");
    mc.setMapClassDefinition(myMapClass2);

    print "\nFormatObject result with wrong map class definition in context:"
    print formatObject(mc)

    print "\nMarshalling string with wrong map class definition in context:"
    result = mc.marshall()
    print result

    print "\nLength of marshalled data: %s" % (len(result))

    mc2 = unmarshall(result)
    print "\nUnmarshall and call FormatObject on the context:"
    print formatObject(mc)

    # Now add the right map class definition to the context
    mc.setMapClassDefinition(myMapClass)
    print "\nFormatObject result with map class definition in context:"
    print formatObject(mc)

    # Now create an instance for the map class with no keys

    print "\nAdd a map object created for the map class definition with no keys"

    theMap2 = myMapClass2.createInstance()
    theMap2["KeyXXX"] = "ValueXXX"
    theMap2["KeyZZZ"] = "ValueZZZ"

    resultList.append(theMap2)

    mc.setRootObject(resultList)

    # Verify that you can format, marshall, and unmarsharll an object that
    # references a map class that doesn't have any keys defined.

    print "\nVerify you can format, marshall, and unmarshall an object that references a map class without any keys defined:"
    print "\nFormatObject Result:"
    print formatObject(mc)

    result = mc.marshall()
    print "\nMarshalledString: \n%s" % (result)

    print "\nLength of marshalled data: %s" % (len(result))

    print "\nUnmarshall and call FormatObject on the context"
    mc2 = unmarshall(result)
    print formatObject(mc2)

    # Add keys to the second map class definition.  One that doesn't match
    # the entry and one that does
    
    myMapClass2.addKey("KeyYYY", "Key YYY");
    myMapClass2.addKey("KeyXXX", "Key XXX");

    # Verify that you can format, marshall, and unmarshall an object that
    # references a map class that has a key that it doesn't provide an
    # entry for.

    print "\nVerify you can format, marshall, and unmarshall an object that references a map class without a key that it doesn't provide an entry for:"
    print "\nFormatObject Result:"
    print formatObject(mc)

    result = mc.marshall()
    print "\nMarshalledString: \n%s" % (result)

    print "\nLength of marshalled data: %s" % (len(result))

    print "\nUnmarshall and call FormatObject onn the context"
    mc2 = unmarshall(result)
    print formatObject(mc2)

    resultList = []

    #######################################
    # Test Python marshalling performance #
    #######################################

    print "\n**************************************************************"
    print "Test Performance for Marshalling, FormatObject, and Unmarshall"
    print "**************************************************************"
    
    entries = int(args[1])

    # Test using a list with the specified number of entries

    print "\nTest using a list with %s entries\n" % (entries)
    
    myList = []

    for i in range(0, entries):
        myList.append("entryValue ##%s" % (i))

    mc = STAFMarshallingContext()
    mc.setRootObject(myList)

    print "FormatObject starting..."
    starttime = time.time() # record starting time for formatObject
    formatObject(mc)
    #print formatObject(mc)
    elapsed = time.time() - starttime
    print "FormatObject Elapsed Time: %.0f seconds" % (elapsed)

    print "Marshalling starting..."
    starttime = time.time() # record starting time for marshalling
    result = mc.marshall()
    elapsed = time.time() - starttime
    print "Marshalling Elapsed Time: %.0f seconds" % (elapsed)
    #print result

    print "Length of marshalled string: %s" % (len(result))

    print "Unmarshalling starting..."
    starttime = time.time() # record starting time for marshalling
    mc = unmarshall(result)
    elapsed = time.time() - starttime
    print "Unmarshalling Elapsed Time: %.0f seconds" % (elapsed)
    #print formatObject(mc)
    
    myList = []

    # Test using a map with the specified number of entries

    print "\nTest using a map with %s entries\n" % (entries)
    
    myMap = {}

    for i in range(0, entries):
        myMap["key%s" % (i)] = "value%s" % (i)

    mc = STAFMarshallingContext()
    mc.setRootObject(myMap)

    print "FormatObject starting..."
    starttime = time.time() # record starting time for formatObject
    formatObject(mc)
    #print formatObject(mc)
    elapsed = time.time() - starttime
    print "FormatObject Elapsed Time: %.0f seconds" % (elapsed)

    print "Marshalling starting..."
    starttime = time.time() # record starting time for marshalling
    result = mc.marshall()
    elapsed = time.time() - starttime
    print "Marshalling Elapsed Time: %.0f seconds" % (elapsed)
    #print result

    print "Length of marshalled string: %s" % (len(result))

    print "Unmarshalling starting..."
    starttime = time.time() # record starting time for marshalling
    unmarshall(result)
    elapsed = time.time() - starttime
    print "Unmarshalling Elapsed Time: %.0f seconds" % (elapsed)
    
    myMap = {}


    # Test using a list (with the specified number of entries / 10) of map
    # class objects each with 10 keys

    numEntries = int(entries / 10)

    if numEntries < 1 and entries > 0:
        numEntries = 1

    numKeys = 10

    print "\nTest using a list with %s entries of map class objects each with %s keys\n" % (numEntries, numKeys)
    
    # Define a map class with 10 keys

    myMapClass = STAFMapClassDefinition('STAF/Test/MyMapClassDefinition')


    for k in range(1, numKeys + 1):
        myMapClass.addKey("key%s" % (k), "Key #%s" % (k))

    # Create a marshalling context and assign the map class definition

    mc = STAFMarshallingContext()
    mc.setMapClassDefinition(myMapClass)
    
    resultList = []

    for i in range(1, numEntries + 1):
        # Create an instance of this map class definition and assign
        # data to the map class instance

        theMap = myMapClass.createInstance()

        for j in range(1, numKeys + 1):
            theMap["key%s" % (j)] = "Value %s %s" % (j, i)

        resultList.append(theMap)

    mc.setRootObject(resultList)

    print "FormatObject starting..."
    starttime = time.time() # record starting time for formatObject
    formatObject(mc)
    #print formatObject(mc)
    elapsed = time.time() - starttime
    print "FormatObject Elapsed Time: %.0f seconds" % (elapsed)

    print "Marshalling starting..."
    starttime = time.time() # record starting time for marshalling
    result = mc.marshall()
    elapsed = time.time() - starttime
    print "Marshalling Elapsed Time: %.0f seconds" % (elapsed)
    #print result

    print "Length of marshalled string: %s" % (len(result))

    print "Unmarshalling starting..."
    starttime = time.time() # record starting time for marshalling
    unmarshall(result)
    elapsed = time.time() - starttime
    print "Unmarshalling Elapsed Time: %.0f seconds" % (elapsed)
    
    resultList = []

    sys.exit(0)

if __name__ == "__main__":
    sys.exit(main())
