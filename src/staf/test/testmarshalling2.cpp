/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFDataTypes.h"

// The purpose of this program is to simply create some basic data structures
// and validate the output of the size() and asString() methods.

int main(void)
{
    STAFObjectPtr masterList = STAFObject::createList();

    STAFObjectPtr none = STAFObject::createNone();

    masterList->append(none);

    STAFObjectPtr aString = STAFObject::createScalar("Hello world");

    masterList->append(aString);

    STAFObjectPtr aList = STAFObject::createList();

    aList->append("String1");
    aList->append("String2");
    aList->append("String3");
    aList->append("String4");
    aList->append("String5");

    masterList->append(aList);

    STAFObjectPtr aSimpleMap = STAFObject::createMap();

    aSimpleMap->put("key1", "value1");
    aSimpleMap->put("key2", "value2");
    aSimpleMap->put("key3", "value3");
    aSimpleMap->put("key4", "value4");

    masterList->append(aSimpleMap);

    STAFMapClassDefinitionPtr simpleMapClassDef =
        STAFMapClassDefinition::create("STAF/Test/TestMC2/TestMC2MapClass");

    simpleMapClassDef->addKey("key1", "Key 1");
    simpleMapClassDef->addKey("key2", "Key 2");
    simpleMapClassDef->addKey("key3", "Key 3");

    STAFObjectPtr aMapClassInstance = simpleMapClassDef->createInstance();

    aMapClassInstance->put("key1", "value1");
    aMapClassInstance->put("key2", "value2");
    aMapClassInstance->put("key3", "value3");

    masterList->append(aMapClassInstance);

    STAFObjectPtr aContext = STAFObject::createMarshallingContext();

    STAFMapClassDefinitionPtr mapClassDef1 =
        STAFMapClassDefinition::create("STAF/Test/TestMC2/SimpleMapClass1");

    mapClassDef1->addKey("key1", "Key 1");

    STAFMapClassDefinitionPtr mapClassDef2 =
        STAFMapClassDefinition::create("STAF/Test/TestMC2/SimpleMapClass2");

    mapClassDef2->addKey("key1", "Key 1");

    aContext->setMapClassDefinition(mapClassDef1);
    aContext->setMapClassDefinition(mapClassDef2);

    masterList->append(aContext);

    for (STAFObjectIteratorPtr iter = masterList->iterate();
         iter->hasNext();)
    {
        STAFObjectPtr thisObj = iter->next();

        switch (thisObj->type())
        {
            case kSTAFNoneObject:
            {
                cout << "Type: None, ";
                break;
            }
            case kSTAFScalarStringObject:
            {
                cout << "Type: Scalar String, ";
                break;
            }
            case kSTAFListObject:
            {
                cout << "Type: List, ";
                break;
            }
            case kSTAFMapObject:
            {
                cout << "Type: Map, ";
                break;
            }
            case kSTAFMarshallingContextObject:
            {
                cout << "Type: Marshalling Context, ";
                break;
            }
            default:
            {
                cout << "Type: Unknown, ";
                break;
            }
        }

        cout << "Size: " << thisObj->size() << ", ";
        cout << "As String: " << thisObj->asString() << endl;
    }

    STAFString marshalledData = masterList->marshall();

    cout << endl
         << "After marshalling, does this appear to be marshalled data: "
         << STAFObject::isMarshalledData(marshalledData) << endl;

    STAFObjectIteratorPtr mcdNameIter = aContext->mapClassDefinitionIterator();

    cout << endl << "Map class definition names:" << endl;

    while (mcdNameIter->hasNext())
    {
        cout << mcdNameIter->next()->asString() << endl;
    }

    return 0;
}
