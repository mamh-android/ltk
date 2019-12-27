/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include <map>
#include "STAF_iostream.h"
#include "STAFDataTypes.h"
#include "STAFTimestamp.h"

//===========================================================================
// testMarshallingPerf - Tests performance of C++ marshalling
//===========================================================================
// Accepts: The number of entries to marshall
// Returns: Nothing
//===========================================================================
// Purpose: Create some marshalled data for a specified number of entries to
// test performance of C++ marshalling.
//
// Expected Result:
//
//
// C:\testMarshallingPerf 100000
//
// ************************************************************
// Test for Errors in Marshalling, FormatObject, and Unmarshall
// ************************************************************
//
// Test for errors using a list with 1 entries of map class objects with 2 keys
//
// Verify you can format, marshall, and unmarshall an object that references a map class but does not define the map class in the context:
//
// FormatObject Result without map class definition in context: 
// [
//   {
//     key1               : Value 1 1
//     key2               : Value 2 1
//     staf-map-class-name: STAF/Test/MyMapClassDefinition
//   }
// ]
// 
// Marshalled String: 
// @SDT/[1:127:@SDT/{:116::4:key1@SDT/$S:9:Value 1 1:4:key2@SDT/$S:9:Value 2 1:19:staf-map-class-name@SDT/$S:30:STAF/Test/MyMapClassDefinition
// 
// Length of marshalled data: 139
// 
// Unmarshall and call FormatObject on the context:
// [
//   {
//     key1               : Value 1 1
//     key2               : Value 2 1
//     staf-map-class-name: STAF/Test/MyMapClassDefinition
//   }
// ]
// 
// Print root list object as a formatted string:
// [
//   {
//     key1               : Value 1 1
//     key2               : Value 2 1
//     staf-map-class-name: STAF/Test/MyMapClassDefinition
//   }
// ]
// 
// FormatObject result with wrong map class definition in context: 
// [
//   {
//     key1               : Value 1 1
//     key2               : Value 2 1
//     staf-map-class-name: STAF/Test/MyMapClassDefinition
//   }
// ]
// 
// Marshalled string with wrong map class definition in context: 
// @SDT/*:289:@SDT/{:139::13:map-class-map@SDT/{:111::31:STAF/Test/MyMapClassDefinition2@SDT/{:66::4:keys@SDT/[0:0::4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition2@SDT/[1:127:@SDT/{:116::4:key1@SDT/$S:9:Value 1 1:4:key2@SDT/$S:9:Value 2 1:19:staf-map-class-name@SDT/$S:30:STAF/Test/MyMapClassDefinition
// 
// Length of marshalled data: 300
// 
// Unmarshall and call FormatObject on the context:
// [
//   {
//     key1               : Value 1 1
//     key2               : Value 2 1
//     staf-map-class-name: STAF/Test/MyMapClassDefinition
//   }
// ]
// 
// FormatObject result with map class definition in context: 
// [
//   {
//     Key #1: Value 1 1
//     Key #2: Value 2 1
//   }
// ]
// 
// Add a map object created for the map class definition with no keys
// 
// Verify you can format, marshall, and unmarshall an object that references a map class without any keys defined:
// 
// FormatObject Result: 
// [
//   {
//     Key #1: Value 1 1
//     Key #2: Value 2 1
//   }
//   {
//   }
// ]
// 
// Marshalled String: 
// @SDT/*:525:@SDT/{:375::13:map-class-map@SDT/{:347::30:STAF/Test/MyMapClassDefinition@SDT/{:191::4:keys@SDT/[2:124:@SDT/{:52::12:display-name@SDT/$S:6:Key #1:3:key@SDT/$S:4:key1@SDT/{:52::12:display-name@SDT/$S:6:Key #2:3:key@SDT/$S:4:key2:4:name@SDT/$S:30:STAF/Test/MyMapClassDefinition:31:STAF/Test/MyMapClassDefinition2@SDT/{:66::4:keys@SDT/[0:0::4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition2@SDT/[2:127:@SDT/%:72::30:STAF/Test/MyMapClassDefinition@SDT/$S:9:Value 1 1@SDT/$S:9:Value 2 1@SDT/%:35::31:STAF/Test/MyMapClassDefinition2
// 
// Length of marshalled data: 536
// 
// Unmarshall and call FormatObject on the context:
// [
//   {
//     Key #1: Value 1 1
//     Key #2: Value 2 1
//   }
//   {
//   }
// ]
// 
// Verify you can format, marshall, and unmarshall an object that references a map class without a key that it doesn't provide an entry for:
// 
// FormatObject Result: 
// [
//   {
//     Key #1: Value 1 1
//     Key #2: Value 2 1
//   }
//   {
//     Key YYY: <None>
//     Key XXX: ValueXXX
//   }
// ]
// 
// Marshalled String: 
// @SDT/*:686:@SDT/{:508::13:map-class-map@SDT/{:480::30:STAF/Test/MyMapClassDefinition@SDT/{:191::4:keys@SDT/[2:124:@SDT/{:52::12:display-name@SDT/$S:6:Key #1:3:key@SDT/$S:4:key1@SDT/{:52::12:display-name@SDT/$S:6:Key #2:3:key@SDT/$S:4:key2:4:name@SDT/$S:30:STAF/Test/MyMapClassDefinition:31:STAF/Test/MyMapClassDefinition2@SDT/{:198::4:keys@SDT/[2:130:@SDT/{:55::12:display-name@SDT/$S:7:Key YYY:3:key@SDT/$S:6:KeyYYY@SDT/{:55::12:display-name@SDT/$S:7:Key XXX:3:key@SDT/$S:6:KeyXXX:4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition2@SDT/[2:155:@SDT/%:72::30:STAF/Test/MyMapClassDefinition@SDT/$S:9:Value 1 1@SDT/$S:9:Value 2 1@SDT/%:63::31:STAF/Test/MyMapClassDefinition2@SDT/$0:0:@SDT/$S:8:ValueXXX
// 
// Length of marshalled data: 697
// 
// Unmarshall and call FormatObject on the context:
// [
//   {
//     Key #1: Value 1 1
//     Key #2: Value 2 1
//   }
//   {
//     Key YYY: <None>
//     Key XXX: ValueXXX
//   }
// ]
// 
// **************************************************************
// Test Performance for Marshalling, FormatObject, and Unmarshall
// **************************************************************
// 
// Test using a list with 100000 entries
// 
// FormatObject started: 20061005-17:05:51
// FormatObject ended  : 20061005-17:05:52
// Marshalling started : 20061005-17:05:52
// Marshalling ended   : 20061005-17:05:52
// Length of marshalled data: 2888911
// Unmarshalling started : 20061005-17:05:52
// Unmarshalling ended   : 20061005-17:05:53
// 
// Test using a map with 100000 entries
// 
// FormatObject started: 20061005-17:05:55
// FormatObject ended  : 20061005-17:05:57
// Marshalling started : 20061005-17:05:57
// Marshalling ended   : 20061005-17:05:58
// Length of marshalled data: 3167795
// Unmarshalling started : 20061005-17:05:58
// Unmarshalling ended   : 20061005-17:06:01
// 
// Test using a list with 10000 entries of map class objects with 10 keys
// 
// FormatObject started: 20061005-17:06:03
// FormatObject ended  : 20061005-17:06:06
// Marshalling started : 20061005-17:06:06
// Marshalling ended   : 20061005-17:06:08
// Length of marshalled data: 2749668
// Unmarshalling started : 20061005-17:06:08
// Unmarshalling ended   : 20061005-17:06:11

//===========================================================================

// Reel Data - contains OnScreenReel information
// for cheat values
struct Data
{
    Data()
    { /* Do Nothing */ }
    Data(const STAFString &aA, const STAFString &aB, const STAFString &aC) :
         a(aA), b(aB), c(aC)
    { /* Do Nothing */ }

    STAFString  a;				
    STAFString	b;			
    STAFString	c;			
};

typedef STAFRefPtr<Data> DataPtr;
typedef std::map<STAFString, DataPtr> DataMap;

typedef std::map<STAFString, STAFString> StringMap;

struct ElementData
{
    ElementData()
    { /* Do Nothing */ }
    ElementData(DataMap &aDataMap) :
 	dataMap(aDataMap)
    { /* Do Nothing */ }
    
    DataMap dataMap;    
};

typedef STAFRefPtr<ElementData> ElementDataPtr;
typedef std::map<STAFString, ElementDataPtr> ElementMap;

void testMapMarshallingPerf(int number);
void testListMarshallingPerf(int number);
void testMarshallingPerf(int number);
void testMarshallingPerf2(int numListEntries, int numKeys);
void testMarshallingErrors(int numListEntries, int numKeys);

STAFString gRegName("test");
STAFHandlePtr gHandle;

/** entry point of application **/
int main(int argc, char **argv)
{
    int result = 1;
    
    if (argc != 2 && argc != 3)
    {
        cout << "Usage: testMarshallingPerf <numEntries> [<numMapEntries>]" << endl;
        return result;
    }
    
    unsigned int rc = 0;
    //rc = STAFHandle::create(gRegName, gHandle);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    try
    {
        if (argc == 2)
        {
            cout << endl << "************************************************************" << endl;
            cout << "Test for Errors in Marshalling, FormatObject, and Unmarshall" << endl;
            cout << "************************************************************" << endl;
            testMarshallingErrors(1, 2);

            cout << endl << "**************************************************************" << endl;
            cout << "Test Performance for Marshalling, FormatObject, and Unmarshall" << endl;
            cout << "**************************************************************" << endl;

            testListMarshallingPerf(atoi(argv[1]));
            testMapMarshallingPerf(atoi(argv[1]));
            testMarshallingPerf2(atoi(argv[1]), 10);
            //testMarshallingPerf(atoi(argv[1]));
        }
        else
        {
            testMarshallingPerf2(atoi(argv[1]), atoi(argv[2]));
        }
    }
    catch(unsigned int rc)
    { 
        cout << "Caught exception rc: " << rc << endl; 
        return rc;
    }
    catch(STAFString message)
    {
        cout << message << endl;
        return result;
    }
    catch(...)
    {
        cout << "Caught unknown exception" << endl; 
        return result;
    }

    return result;
}

void testListMarshallingPerf(int entries)
{
    cout << endl << "Test using a list with " << entries
         << " entries" << endl << endl;

    STAFObjectPtr myList = STAFObject::createList();
    
    for(int i = 0; i < entries; i++)
    {
        myList->append(STAFString("entryValue ##") + i);
    }

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setRootObject(myList);


    cout << "FormatObject started: " << STAFTimestamp::now().asString() << endl;
    mc->asFormattedString();
    //cout << mc->asFormattedString() << endl;
    cout << "FormatObject ended  : " << STAFTimestamp::now().asString() << endl;

    cout << "Marshalling started : " << STAFTimestamp::now().asString() << endl;
    STAFString marshalledData = mc->marshall();
    cout << "Marshalling ended   : " << STAFTimestamp::now().asString() << endl;
    //cout << marshalledData << endl;

    cout << "Length of marshalled data: " << marshalledData.length() << endl;

    cout << "Unmarshalling started : " << STAFTimestamp::now().asString() << endl;
    mc = STAFObject::unmarshall(marshalledData);
    cout << "Unmarshalling ended   : " << STAFTimestamp::now().asString() << endl;
    //cout << mc->asFormattedString() << endl;

    myList = STAFObject::createList();
}

void testMapMarshallingPerf(int entries)
{
    cout << endl << "Test using a map with " << entries << " entries" << endl << endl;

    STAFObjectPtr myMap = STAFObject::createMap();
    
    for(int i = 0; i < entries; i++)
    {		
        myMap->put(STAFString("key") + i, STAFString("value") + i);
    }

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setRootObject(myMap);


    cout << "FormatObject started: " << STAFTimestamp::now().asString() << endl;
    mc->asFormattedString();
    //cout << mc->asFormattedString() << endl;
    cout << "FormatObject ended  : " << STAFTimestamp::now().asString() << endl;

    cout << "Marshalling started : " << STAFTimestamp::now().asString() << endl;
    STAFString marshalledData = mc->marshall();
    cout << "Marshalling ended   : " << STAFTimestamp::now().asString() << endl;
    //cout << marshalledData << endl;

    cout << "Length of marshalled data: " << marshalledData.length() << endl;

    cout << "Unmarshalling started : " << STAFTimestamp::now().asString() << endl;
    mc = STAFObject::unmarshall(marshalledData);
    cout << "Unmarshalling ended   : " << STAFTimestamp::now().asString() << endl;
    //cout << mc->asFormattedString() << endl;

    myMap = STAFObject::createMap();
}

void testMarshallingPerf(int number)
{
    cout << endl << "Test using a list with " << number
         << " entries where each entry is a map of maps" << endl << endl;

    STAFMapClassDefinitionPtr fListMapClass; 

    fListMapClass = STAFMapClassDefinition::create(
        "STAF/Service/OnScreenElement/testListElement");

    fListMapClass->addKey("values", "Values");
    fListMapClass->addKey("data", "Data");

    STAFString aelement="A";
    STAFString belement="B";
    STAFString celement="C";

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(fListMapClass->reference());

    DataMap dataMap;
    
    for(int j=0; j<5; ++j)
    {		
        dataMap[STAFString(j)] = DataPtr(
            new Data(aelement, belement, celement),DataPtr::INIT);
    }

    ElementMap	fElementMap;

    for(int i=0; i<number; ++i)
    {
        fElementMap[STAFString(i)] = ElementDataPtr(
            new ElementData(dataMap),ElementDataPtr::INIT);
    }

    // Create an empty result list to contain the result
    STAFObjectPtr resultList = STAFObject::createList();
    
    //loop through all cheat values

    STAFString output="";
    
    for(ElementMap::iterator elementIter = fElementMap.begin();
        elementIter != fElementMap.end(); elementIter++)
    {	
        STAFObjectPtr dataOutputMap = STAFObject::createMap();
	
        // loop through all reels
        for(DataMap::iterator dataIter = elementIter->second->dataMap.begin();
            dataIter != elementIter->second->dataMap.end(); dataIter++)
        {	
            STAFObjectPtr elementMap = STAFObject::createMap();
            elementMap->put("top",dataIter->second->a);
            elementMap->put("middle",dataIter->second->b);
            elementMap->put("bottom",dataIter->second->c);
			
            dataOutputMap->put(dataIter->first, elementMap);
        }
	
        STAFObjectPtr resultMap = fListMapClass->createInstance();
        resultMap->put("values", elementIter->first);
        resultMap->put("reels", dataOutputMap);
		
        resultList->append(resultMap);
    }
    
    mc->setRootObject(resultList);

    cout << "FormatObject started: " << STAFTimestamp::now().asString() << endl;
    mc->asFormattedString();
    cout << "FormatObject ended  : " << STAFTimestamp::now().asString() << endl;

    cout << "Marshalling started : " << STAFTimestamp::now().asString() << endl;
    STAFString marshalledData = mc->marshall();
    cout << "Marshalling ended   : " << STAFTimestamp::now().asString() << endl;
    cout << "Length of marshalled data: " << marshalledData.length() << endl;

    cout << "Unmarshalling started : " << STAFTimestamp::now().asString() << endl;
    mc = STAFObject::unmarshall(marshalledData);
    cout << "Unmarshalling ended   : " << STAFTimestamp::now().asString() << endl;
    
    resultList = STAFObject::createList();
}

void testMarshallingPerf2(int numListEntries, int numKeys)
{
    numListEntries = numListEntries / 10;

    cout << endl << "Test using a list with " << numListEntries
         << " entries of map class objects with "
         << numKeys << " keys" << endl << endl;

    STAFMapClassDefinitionPtr mapClass = STAFMapClassDefinition::create(
        "STAF/Test/MyMapClassDefinition");
    
    for (unsigned int k = 1; k <= numKeys; k++)
    {
        mapClass->addKey(STAFString("key") + k, STAFString("Key #") + k);
    }
    
    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(mapClass);

    // Create an empty result list to contain the result
    STAFObjectPtr resultList = STAFObject::createList();
    
    // Create a list for the specified number of entries

    for (unsigned int i = 1; i <= numListEntries; i++)
    {		
        STAFObjectPtr theMap = mapClass->createInstance();
	
        for (unsigned int j = 1; j <= numKeys; j++)
        {	
            theMap->put(STAFString("key") + j,
                        STAFString("Value ") + j + " " + i);
        }
		
        resultList->append(theMap);
    }
	
    mc->setRootObject(resultList);
    
    cout << "FormatObject started: " << STAFTimestamp::now().asString() << endl;
    mc->asFormattedString();
    cout << "FormatObject ended  : " << STAFTimestamp::now().asString() << endl;
    
    cout << "Marshalling started : " << STAFTimestamp::now().asString() << endl;
    STAFString marshalledData = mc->marshall();
    cout << "Marshalling ended   : " << STAFTimestamp::now().asString() << endl;
    //cout << marshalledData << endl;

    cout << "Length of marshalled data: " << marshalledData.length() << endl;

    cout << "Unmarshalling started : " << STAFTimestamp::now().asString() << endl;
    STAFObjectPtr mc2 = STAFObject::unmarshall(marshalledData);
    cout << "Unmarshalling ended   : " << STAFTimestamp::now().asString() << endl;

    resultList = STAFObject::createList();
}

void testMarshallingErrors(int numListEntries, int numKeys)
{
    cout << "\nTest for errors using a list with " << numListEntries
         << " entries of map class objects with "
         << numKeys << " keys" << endl;

    STAFMapClassDefinitionPtr mapClass = STAFMapClassDefinition::create(
        "STAF/Test/MyMapClassDefinition");
    
    for (unsigned int k = 1; k <= numKeys; k++)
    {
        mapClass->addKey(STAFString("key") + k, STAFString("Key #") + k);
    }
    
    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    // Create an empty result list to contain the result
    STAFObjectPtr resultList = STAFObject::createList();
    
    // Create a list for the specified number of entries

    for (unsigned int i = 1; i <= numListEntries; i++)
    {		
        STAFObjectPtr theMap = mapClass->createInstance();
	
        for (unsigned int j = 1; j <= numKeys; j++)
        {	
            theMap->put(STAFString("key") + j,
                        STAFString("Value ") + j + " " + i);
        }
		
        resultList->append(theMap);
    }
	
    // To keep ownership of result list, add a reference of it as the root object
    mc->setRootObject(STAFObject::createReference(resultList));
    
    // Verify that you can format, marshall, and unmarshall an object that
    // references a map class but does not define the map class in the
    // marshalling context.

    cout << endl << "Verify you can format, marshall, and unmarshall an object"
         << " that references a map class but does not define the map"
         << " class in the context:" << endl;

    cout << endl << "FormatObject Result without map class definition in context: " << endl;
    cout << mc->asFormattedString() << endl;
    
    cout << endl << "Marshalled String: " << endl;
    STAFString marshalledData = mc->marshall();
    cout << marshalledData << endl;

    cout << endl << "Length of marshalled data: " << marshalledData.length() << endl;

    STAFObjectPtr mc2 = STAFObject::unmarshall(marshalledData);
    
    cout << endl << "Unmarshall and call FormatObject on the context:" << endl;
    cout << mc2->asFormattedString() << endl;

    // Verify that calling formatObject on an object that references a
    // map class but does not provide a marshalling context does not
    // cause an error.

    STAFObjectPtr outputList = mc2->getRootObject();
    cout << endl << "Print root list object as a formatted string:" << endl;
    cout << outputList->asFormattedString() << endl;

    // Now create another map class definition with no keys and add it to
    // the context but don't reference it.

    STAFMapClassDefinitionPtr mapClass2 = STAFMapClassDefinition::create(
    "STAF/Test/MyMapClassDefinition2");
    mc->setMapClassDefinition(mapClass2);

    cout << endl << "FormatObject result with wrong map class definition in context: " << endl;
    cout << mc->asFormattedString() << endl;
    
    cout << endl << "Marshalled string with wrong map class definition in context: " << endl;
    marshalledData = mc->marshall();
    cout << marshalledData << endl;

    cout << endl << "Length of marshalled data: " << marshalledData.length() << endl;

    cout << endl << "Unmarshall and call FormatObject on the context:" << endl;
    mc2 = STAFObject::unmarshall(marshalledData);
    cout << mc2->asFormattedString() << endl;
    
    // Now add the right map class definition to the context
    mc->setMapClassDefinition(mapClass);
    cout << endl << "FormatObject result with map class definition in context: " << endl;
    cout << mc->asFormattedString() << endl;

    // Now create an instance for the map class with no keys

    cout << endl << "Add a map object created for the map class definition with no keys" << endl;
    
    STAFObjectPtr theMap2 = mapClass2->createInstance();
    theMap2->put(STAFString("KeyXXX"), STAFString("ValueXXX"));
    theMap2->put(STAFString("KeyZZZ"), STAFString("ValueZZZ"));
    
    resultList->append(theMap2);
    
    // To keep ownership of result list, add a reference of it as the root object
    mc->setRootObject(STAFObject::createReference(resultList));
    
    // Verify that you can format, marshall, and unmarshall an object that
    // references a map class that doesn't have any keys defined.

    cout << endl << "Verify you can format, marshall, and unmarshall an object"
         << " that references a map class without any keys defined:" << endl;

    cout << endl << "FormatObject Result: " << endl;
    cout << mc->asFormattedString() << endl;
    
    cout << endl << "Marshalled String: " << endl;
    marshalledData = mc->marshall();
    cout << marshalledData << endl;

    cout << endl << "Length of marshalled data: " << marshalledData.length() << endl;

    mc2 = STAFObject::unmarshall(marshalledData);
    
    cout << endl << "Unmarshall and call FormatObject on the context:" << endl;
    cout << mc2->asFormattedString() << endl;

    // Add keys to the second map class definition.  One that doesn't match
    // the entry and one that does
    
    mapClass2->addKey(STAFString("KeyYYY"), STAFString("Key YYY"));
    mapClass2->addKey(STAFString("KeyXXX"), STAFString("Key XXX"));

    // Verify that you can format, marshall, and unmarshall an object that
    // references a map class that has a key that it doesn't provide an
    // entry for.

    cout << endl << "Verify you can format, marshall, and unmarshall an object"
         << " that references a map class without a key that it doesn't provide an entry for:" << endl;

    cout << endl << "FormatObject Result: " << endl;
    cout << mc->asFormattedString() << endl;
    
    cout << endl << "Marshalled String: " << endl;
    marshalledData = mc->marshall();
    cout << marshalledData << endl;

    cout << endl << "Length of marshalled data: " << marshalledData.length() << endl;

    mc2 = STAFObject::unmarshall(marshalledData);
    
    cout << endl << "Unmarshall and call FormatObject on the context:" << endl;
    cout << mc2->asFormattedString() << endl;
}

