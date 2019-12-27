/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAF_iostream.h"
#include "STAFDataTypes.h"
#include "STAFTimestamp.h"

void printOutput(const STAFString &output);

int main(void)
{
    // First without any map-class

    STAFObjectPtr logList = STAFObject::createList();

    for (int i = 1; i < 31; ++i)
    {
        STAFObjectPtr logRecord = STAFObject::createMap();

        logRecord->put("level", "info");
        logRecord->put("message", STAFString("Message #") + i);
        logRecord->put("machine", "crankin3.austin.ibm.com");
        logRecord->put("timestamp", STAFTimestamp::now().asString());

        logList->append(logRecord);
    }

    STAFString output;
    logList->marshall(output);

    cout << "Output:" << endl;
    cout << output << endl << endl;
    cout << endl;

    // Now with a map-class

    // Construct the map-class

    STAFMapClassDefinitionPtr logRecordClass =
        STAFMapClassDefinition::create("STAF/Test/LogRecord");

    logRecordClass->addKey("timestamp", "Date-Time");
    logRecordClass->addKey("level", "Level");
    logRecordClass->addKey("machine", "Machine");
    logRecordClass->addKey("message", "Message");

    // Create the marshalling context

    STAFObjectPtr mc = STAFObject::createMarshallingContext();
    mc->setMapClassDefinition(logRecordClass);

    // Populate the list

    STAFObjectPtr logList2 = STAFObject::createList();

    for (int j = 1; j < 31; ++j)
    {
        STAFObjectPtr logRecord = logRecordClass->createInstance();

        logRecord->put("level", "info");
        logRecord->put("message", STAFString("Message #") + j);
        logRecord->put("machine", "crankin3.austin.ibm.com");
        logRecord->put("timestamp", STAFTimestamp::now().asString());

        logList2->append(logRecord);
    }

    mc->setRootObject(logList2);

    STAFString output2 = mc->marshall();

    cout << "Output MC:" << endl;
    cout << output2 << endl << endl;

    printOutput(output2);

    // Now test unmarshalling invalid marshalled data

    cout << "Test unmarshalling data that contains invalid marshalled data" << endl;

    // Register with STAF

    STAFHandlePtr handlePtr;

    try
    {
        unsigned int rc = STAFHandle::create("STAF/TestMarshalling", handlePtr);

        if (rc != 0)
        {
            cout << "Error registering with STAF, RC: " << rc << endl;
            return rc;
        }
    }
    catch (STAFException &se)
    {
        se.write("Error registering with STAF");
    }
    
    // Create some invalid marshalling data and queue it;  Get it off the
    // queue (auto-unmarshalling will be done) and verify results in the
    // invalid marshalling data string in the message

    STAFString message = "@SDT/{:177::2:RC@SDT/$S:1:0:6:IPInfo@SDT/$S:36:"
        "9.42.126.76|255.255.252.0|9.42.124.1:3:Msg@SDT/$S:46:Static IP "
        "arguments are processed successfully:9:Timestamp"
        "@SDT/$S:19:2009-01-16 14:41:45"
        "Connecting to: http://9.42.106.28:8080";
    
    STAFString request = STAFString("QUEUE MESSAGE ") + message;
    STAFResultPtr result = handlePtr->submit("local", "QUEUE", request);

    if (result->rc != 0) 
    {
        cout << "ERROR: STAF local QUEUE " << request << " failed with RC: "
             << result->rc << ", Result: " << result->result << endl;
        return result->rc;
    }

    request = STAFString("GET WAIT 5000");
    
    result = handlePtr->submit("local", "QUEUE", request);

    if (result->rc != 0) 
    {
        cout << "ERROR: STAF local QUEUE " << request << " failed with RC: "
             << result->rc << ", Result: " << result->result << endl;
        return result->rc;
    }

    STAFObjectPtr messageMap = result->resultObj;

    cout << endl << "Queued message map (where Message contains invalid "
         << "marshalled data)" << endl
         << result->resultContext->asFormattedString() << endl << endl;

    if (messageMap->get("message")->asString() != message)
    {
        cout << "ERROR: Message not same as original message sent" << endl;
        cout << "Expected:" << endl << message << endl;
        cout << "Found:" << endl
             << messageMap->get("message")->asFormattedString() << endl;
        return 1;
    }

    return 0;
}


void printOutput(const STAFString &output)
{
    STAFObjectPtr outputContext = STAFObject::unmarshall(output);
    STAFObjectPtr outputList = outputContext->getRootObject();

    STAFObjectIteratorPtr iter = outputList->iterate();
    int itemNumber = 1;

    while (iter->hasNext())
    {
        STAFObjectPtr logRecord = iter->next();

        cout << "Log Record #" << itemNumber++ << endl;;

        if (logRecord->hasKey("staf-map-class-name"))
        {
            STAFMapClassDefinitionPtr mapClass =
                outputContext->getMapClassDefinition(
                    logRecord->get("staf-map-class-name")->asString());
            STAFObjectIteratorPtr keyIter = mapClass->keyIterator();

            while (keyIter->hasNext())
            {
                STAFObjectPtr key = keyIter->next();

                cout << key->get("display-name")->asString() << ": "
                     << logRecord->get(key->get("key")->asString())->asString()
                     << endl;;
            }

            cout << endl;
        }
        else
        {
            cout << "Level  : " << logRecord->get("level")->asString() << endl;
            cout << "Message: " << logRecord->get("message")->asString() << endl;
            cout << "Machine: " << logRecord->get("machine")->asString() << endl;
            cout << endl;
        }
    }
}
