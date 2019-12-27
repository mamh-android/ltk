/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

//===========================================================================
// MarshallingPerfTest - Tests performance of Java marshalling and formatObject
//===========================================================================
// Accepts: The number of entries to marshall
// Returns: Nothing
//===========================================================================
// Purpose: Create some marshalled data for a specified number of entries to
// test performance of Java marshalling and formatObject.
//
// Expected Rseult:
/*
C:\dev\sf\rel\win32\staf\retail\bin>java MarshallingPerfTest 100000

************************************************************
Test for Errors in Marshalling, FormatObject, and Unmarshall
************************************************************

Test for errors using a list with 1 entries of map class objects with 2 keys

Verify you can format, marshall, and unmarshall an object that references a map
class but does not define the map class in the context:

FormatObject result without map class definition in context:
[
  {
    key1               : Value 1 1
    key2               : Value 2 1
    staf-map-class-name: STAF/Test/MyMapClassDefinition
  }
]

Marshalled string:
@SDT/[1:127:@SDT/{:116::4:key1@SDT/$S:9:Value 1 1:4:key2@SDT/$S:9:Value 2 1:19:s
taf-map-class-name@SDT/$S:30:STAF/Test/MyMapClassDefinition

Length of marshalled data: 139

Unmarshall and call FormatObject on the context:
[
  {
    key1               : Value 1 1
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key2               : Value 2 1
  }
]

Print root list object as a formatted string:
[
  {
    key1               : Value 1 1
    staf-map-class-name: STAF/Test/MyMapClassDefinition
    key2               : Value 2 1
  }
]

FormatObject result with wrong map class definition in context:
[
  {
    key1               : Value 1 1
    key2               : Value 2 1
    staf-map-class-name: STAF/Test/MyMapClassDefinition
  }
]

Marshalling string with wrong map class definition in context:
@SDT/*:289:@SDT/{:139::13:map-class-map@SDT/{:111::31:STAF/Test/MyMapClassDefini
tion2@SDT/{:66::4:keys@SDT/[0:0::4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition
2@SDT/[1:127:@SDT/{:116::4:key1@SDT/$S:9:Value 1 1:4:key2@SDT/$S:9:Value 2 1:19:
staf-map-class-name@SDT/$S:30:STAF/Test/MyMapClassDefinition

Length of marshalled data: 300

Unmarshall and call FormatObject on the context:
[
  {
    key1               : Value 1 1
    key2               : Value 2 1
    staf-map-class-name: STAF/Test/MyMapClassDefinition
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

Marshalled String:
@SDT/*:525:@SDT/{:375::13:map-class-map@SDT/{:347::30:STAF/Test/MyMapClassDefini
tion@SDT/{:191::4:keys@SDT/[2:124:@SDT/{:52::12:display-name@SDT/$S:6:Key #1:3:k
ey@SDT/$S:4:key1@SDT/{:52::12:display-name@SDT/$S:6:Key #2:3:key@SDT/$S:4:key2:4
:name@SDT/$S:30:STAF/Test/MyMapClassDefinition:31:STAF/Test/MyMapClassDefinition
2@SDT/{:66::4:keys@SDT/[0:0::4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition2@SD
T/[2:127:@SDT/%:72::30:STAF/Test/MyMapClassDefinition@SDT/$S:9:Value 1 1@SDT/$S:
9:Value 2 1@SDT/%:35::31:STAF/Test/MyMapClassDefinition2

Length of marshalled data: 536

Unmarshall and call FormatObject on the context:
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

Marshalled String:
@SDT/*:686:@SDT/{:508::13:map-class-map@SDT/{:480::30:STAF/Test/MyMapClassDefini
tion@SDT/{:191::4:keys@SDT/[2:124:@SDT/{:52::12:display-name@SDT/$S:6:Key #1:3:k
ey@SDT/$S:4:key1@SDT/{:52::12:display-name@SDT/$S:6:Key #2:3:key@SDT/$S:4:key2:4
:name@SDT/$S:30:STAF/Test/MyMapClassDefinition:31:STAF/Test/MyMapClassDefinition
2@SDT/{:198::4:keys@SDT/[2:130:@SDT/{:55::12:display-name@SDT/$S:7:Key YYY:3:key
@SDT/$S:6:KeyYYY@SDT/{:55::12:display-name@SDT/$S:7:Key XXX:3:key@SDT/$S:6:KeyXX
X:4:name@SDT/$S:31:STAF/Test/MyMapClassDefinition2@SDT/[2:155:@SDT/%:72::30:STAF
/Test/MyMapClassDefinition@SDT/$S:9:Value 1 1@SDT/$S:9:Value 2 1@SDT/%:63::31:ST
AF/Test/MyMapClassDefinition2@SDT/$0:0:@SDT/$S:8:ValueXXX

Length of marshalled data: 697

Unmarshall and call FormatObject on the context:
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

FormatObject started: 2006-10-05 at 05:10:50 PM
FormatObject ended  : 2006-10-05 at 05:10:50 PM
Marshalling started : 2006-10-05 at 05:10:50 PM
Marshalling ended   : 2006-10-05 at 05:10:50 PM
Length of marshalled data: 2888911
Unmarshalling started : 2006-10-05 at 05:10:50 PM
Unmarshalling ended : 2006-10-05 at 05:10:51 PM

Test using a map with 100000 entries

FormatObject started: 2006-10-05 at 05:10:51 PM
FormatObject ended  : 2006-10-05 at 05:10:52 PM
Marshalling started : 2006-10-05 at 05:10:52 PM
Marshalling ended   : 2006-10-05 at 05:10:52 PM
Length of marshalled data: 3167795
Unmarshalling started : 2006-10-05 at 05:10:52 PM
Unmarshalling ended : 2006-10-05 at 05:10:53 PM

Test using a list with 10000 entries of map class objects with 10 keys

FormatObject started: 2006-10-05 at 05:10:53 PM
FormatObject ended  : 2006-10-05 at 05:10:53 PM
Marshalling started : 2006-10-05 at 05:10:53 PM
Marshalling ended   : 2006-10-05 at 05:10:54 PM
Length of marshalled data: 2749668
Unmarshalling started : 2006-10-05 at 05:10:54 PM
Unmarshalling ended : 2006-10-05 at 05:10:54 PM

*/
//===========================================================================

import com.ibm.staf.*; 
import java.util.*; 
import java.text.SimpleDateFormat; 
 
public class MarshallingPerfTest 
{
    public static SimpleDateFormat formatter = new SimpleDateFormat(
        "yyyy-MM-dd 'at' hh:mm:ss a");

    public static void main(String [] argv)  
    {
        // Verify the command line arguments

        if (argv.length != 1)
        {
            System.out.println();
            System.out.println("Usage: java MarshallingPerfTest <number>");
            System.exit(1);
        }

        int entries = Integer.parseInt(argv[0]);
 
        testErrors(1);

        System.out.println(
            "\n**************************************************************\n" +
            "Test Performance for Marshalling, FormatObject, and Unmarshall\n" +
            "**************************************************************");

        // Test using a list with the specified number of entries

        System.out.println("\nTest using a list with " + entries +
                           " entries\n");

        List myList = new ArrayList(); 
 
        for (int i = 0; i < entries; i++) 
        { 
            myList.add("entryValue ##" + i); 
        } 

        STAFMarshallingContext mc = new STAFMarshallingContext();
        mc.setRootObject(myList);

        System.out.println("FormatObject started: " + formatter.format(new Date()));
        STAFMarshallingContext.formatObject(mc);
        System.out.println("FormatObject ended  : " + formatter.format(new Date()));

        String startTime = formatter.format(new Date());
        System.out.println("Marshalling started : " + startTime); 
        String result = mc.marshall(); 
        String endTime = formatter.format(new Date()); 
        System.out.println("Marshalling ended   : " + endTime);

        System.out.println("Length of marshalled data: " + result.length()); 

        startTime = formatter.format(new Date());
        System.out.println("Unmarshalling started : " + startTime);
        mc = STAFMarshallingContext.unmarshall(result);
        endTime = formatter.format(new Date());
        System.out.println("Unmarshalling ended : " + endTime);

        myList = new ArrayList();

        // Test using a map with the specified number of entries

        System.out.println("\nTest using a map with " + entries +
                           " entries\n");

        Map myMap = new HashMap(); 
 
        for (int i = 0; i < entries; i++) 
        { 
            myMap.put("key" + i, "value" + i); 
        } 

        new STAFMarshallingContext();
        mc.setRootObject(myMap);

        formatter = new SimpleDateFormat("yyyy-MM-dd 'at' hh:mm:ss a");

        System.out.println("FormatObject started: " + formatter.format(new Date()));
        STAFMarshallingContext.formatObject(mc);
        System.out.println("FormatObject ended  : " + formatter.format(new Date()));

        startTime = formatter.format(new Date());
        System.out.println("Marshalling started : " + startTime); 
        result = mc.marshall(); 
        endTime = formatter.format(new Date()); 
        System.out.println("Marshalling ended   : " + endTime);

        System.out.println("Length of marshalled data: " + result.length());

        startTime = formatter.format(new Date());
        System.out.println("Unmarshalling started : " + startTime);
        mc = STAFMarshallingContext.unmarshall(result);
        endTime = formatter.format(new Date());
        System.out.println("Unmarshalling ended : " + endTime);

        myMap = new HashMap();

        // Test using a list (with the specified number of entries / 10) of map
        // class objects each with 10 keys

        int numEntries = entries / 10;

        System.out.println("\nTest using a list with " + numEntries +
                           " entries of map class objects with 10 keys\n");

        // Define a map class with 10 keys

        STAFMapClassDefinition myMapClass = new STAFMapClassDefinition(
            "STAF/Test/MyMapClassDefinition");

        int numKeys = 10;

        for (int k = 1; k <= numKeys; k++)
        {
            myMapClass.addKey("key" + k, "Key #" + k);
        }

        // Create a marshalling context and assign a map class definition to it

        mc = new STAFMarshallingContext();
        mc.setMapClassDefinition(myMapClass);
        
        List resultList = new ArrayList(); 
 
        for (int i = 1; i <= numEntries; i++) 
        {
            // Create an instance of this map class definition and assign
            // data to the map class instance

            Map theMap = myMapClass.createInstance();

            for (int j = 1; j <= numKeys; j++)
            {
                theMap.put("key" + j, "Value " + j + " " + i);
            }

            resultList.add(theMap); 
        } 

        mc.setRootObject(resultList);

        formatter = new SimpleDateFormat("yyyy-MM-dd 'at' hh:mm:ss a");

        System.out.println("FormatObject started: " + formatter.format(new Date()));
        STAFMarshallingContext.formatObject(mc);
        System.out.println("FormatObject ended  : " + formatter.format(new Date()));

        startTime = formatter.format(new Date());
        System.out.println("Marshalling started : " + startTime); 
        result = mc.marshall(); 
        endTime = formatter.format(new Date()); 
        System.out.println("Marshalling ended   : " + endTime);

        System.out.println("Length of marshalled data: " + result.length());

        startTime = formatter.format(new Date());
        System.out.println("Unmarshalling started : " + startTime);
        mc = STAFMarshallingContext.unmarshall(result);
        endTime = formatter.format(new Date());
        System.out.println("Unmarshalling ended : " + endTime);

        resultList = new ArrayList();
    }

    public static void testErrors(int entries)  
    {
        System.out.println(
            "\n************************************************************\n" +
            "Test for Errors in Marshalling, FormatObject, and Unmarshall\n" +
            "************************************************************\n");

        int numKeys = 2;

        System.out.println(
            "Test for errors using a list with " + entries +
            " entries of map class objects with " + numKeys + " keys");

        // Define a map class with 2 keys

        STAFMapClassDefinition myMapClass = new STAFMapClassDefinition(
            "STAF/Test/MyMapClassDefinition");

        for (int k = 1; k <= numKeys; k++)
        {
            myMapClass.addKey("key" + k, "Key #" + k);
        }

        // Create a marshalling context and assign a map class definition to it

        STAFMarshallingContext mc = new STAFMarshallingContext();
        //mc.setMapClassDefinition(myMapClass);
        
        List resultList = new ArrayList(); 
 
        for (int i = 1; i <= entries; i++) 
        {
            // Create an instance of this map class definition and assign
            // data to the map class instance

            Map theMap = myMapClass.createInstance();

            for (int j = 1; j <= numKeys; j++)
            {
                theMap.put("key" + j, "Value " + j + " " + i);
            }

            resultList.add(theMap); 
        } 

        mc.setRootObject(resultList);

        // Verify that you can format, marshall, and unmarshall an object that
        // references a map class but does not define the map class in the
        // marshalling context.

        System.out.println(
            "\nVerify you can format, marshall, and unmarshall an object" +
            " that references a map class but does not define the map" +
            " class in the context:\n");

        System.out.println(
            "FormatObject result without map class definition in context:");
        System.out.println(STAFMarshallingContext.formatObject(mc));

        String result = mc.marshall(); 
        System.out.println("\nMarshalled string:\n" + result);

        System.out.println("\nLength of marshalled data: " + result.length());

        STAFMarshallingContext mc2 = STAFMarshallingContext.unmarshall(result);
        System.out.println(
            "\nUnmarshall and call FormatObject on the context:\n" + mc2);

        // Verify that calling formatObject on an object that references a
        // map class but does not provide a marshalling context does not
        // cause an error.

        List outputList = (List)mc2.getRootObject();
        System.out.println(
            "\nPrint root list object as a formatted string:\n" +
            STAFMarshallingContext.formatObject(outputList));

        // Now create another map class definition with no keys and add it to
        // the context but don't reference it.

        STAFMapClassDefinition myMapClass2 = new STAFMapClassDefinition(
            "STAF/Test/MyMapClassDefinition2");
        mc.setMapClassDefinition(myMapClass2);

        System.out.println(
            "\nFormatObject result with wrong map class definition " +
            "in context:\n" + mc);

        System.out.println(
            "\nMarshalling string with wrong map class definition " +
            "in context:");
        result = mc.marshall();
        System.out.println(result);

        System.out.println("\nLength of marshalled data: " + result.length());

        mc2 = STAFMarshallingContext.unmarshall(result);
        System.out.println(
            "\nUnmarshall and call FormatObject on the context:\n" + mc);

        // Now add the right map class definition to the context
        mc.setMapClassDefinition(myMapClass);
        System.out.println(
            "\nFormatObject result with map class definition in context:\n" +
            mc);

        // Now create an instance for the map class with no keys

        System.out.println(
            "\nAdd a map object created for the map class definition " +
            "with no keys");

        Map theMap2 = myMapClass2.createInstance();
        theMap2.put("KeyXXX", "ValueXXX");
        theMap2.put("KeyZZZ", "ValueZZZ");

        resultList.add(theMap2);

        mc.setRootObject(resultList);

        // Verify that you can format, marshall, and unmarshall an object that
        // references a map class that doesn't have any keys defined.

        System.out.println(
            "\nVerify you can format, marshall, and unmarshall an object " +
            "that references a map class without any keys defined:\n");

        System.out.println("FormatObject Result: ");
        System.out.println(mc);
        result = mc.marshall();
        System.out.println("\nMarshalled String: \n" + result);

        System.out.println("\nLength of marshalled data: " + result.length());

        System.out.println("\nUnmarshall and call FormatObject on the context:");
        mc2 = STAFMarshallingContext.unmarshall(result);
        System.out.println(mc2);

        // Add keys to the second map class definition.  One that doesn't match
        // the entry and one that does

        myMapClass2.addKey("KeyYYY", "Key YYY");
        myMapClass2.addKey("KeyXXX", "Key XXX");

        // Verify that you can format, marshall, and unmarshall an object that
        // references a map class that has a key that it doesn't provide an
        // entry for.

        System.out.println(
            "\nVerify you can format, marshall, and unmarshall an object" +
            " that references a map class without a key that it doesn't " +
            "provide an entry for:");

        System.out.println("FormatObject Result: ");
        System.out.println(mc);
        result = mc.marshall();
        System.out.println("\nMarshalled String: \n" + result);

        System.out.println("\nLength of marshalled data: " + result.length());

        System.out.println("\nUnmarshall and call FormatObject on the context:");
        mc2 = STAFMarshallingContext.unmarshall(result);
        System.out.println(mc2);

        // Register with STAF

        STAFHandle handle = null;

        try
        {
            handle = new STAFHandle("Java_MarshallingPerfTest");
        }
        catch (STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            System.exit(1);
        }

        // Create some invalid marshalling data and queue it;  Get it off the
        // queue (auto-unmarshalling will be done) and verify results in the
        // invalid marshalling data string in the message

        String message = "@SDT/{:177::2:RC@SDT/$S:1:0:6:IPInfo@SDT/$S:36:" +
            "9.42.126.76|255.255.252.0|9.42.124.1:3:Msg@SDT/$S:46:Static IP " +
            "arguments are processed successfully:9:Timestamp@SDT/$S:19:2009-01-16 14:41:45" +
            "Connecting to: http://9.42.106.28:8080";

        STAFResult res = handle.submit2(
            "local", "QUEUE", "QUEUE MESSAGE " + STAFUtil.wrapData(message));

        if (res.rc != STAFResult.Ok)
        {
            System.out.println("ERROR: QUEUE MESSAGE failed, RC" + res.rc +
                               " Result=" + res.result);
            System.exit(1);
        }
        
        res = handle.submit2("local", "QUEUE", "GET WAIT 5000");

        if (res.rc != STAFResult.Ok)
        {
            System.out.println("QUEUE GET failed, RC=" + res.rc +
                               " Result=" + res.result);
            System.exit(1);
        }

        Map messageMap = (Map)res.resultObj;

        System.out.println("Queued message map:\n" + res.resultContext);

        String msg = ((String)messageMap.get("message")).substring(0, message.length());

        if (!message.equals(msg))
        {
            System.out.println("ERROR: Message not same as original message sent");
            System.out.println("Expected:\n" + message);
            System.out.println("Found:\n" + msg);
            System.exit(1);
        }

        // Testing that can unmarshall data using IGNORE_INDIRECT_OBJECTS
        // that contains nested invalid marshalled data, and verifying that
        // can access the 1st layer of valid marshalled data correctly

        System.out.println(
            "\nTesting unmarshalling bad data using IGNORE_INDIRECT_OBJECTS flag");

        String validData =
            "@SDT/[2" + 
              ":18:@SDT/$S:8:1st item" +
              ":37:@SDT/[2:11:@SDT/$S:1:a:11:@SDT/$S:1:b";

        String request = "START SHELL COMMAND " +
            STAFUtil.wrapData("echo " + validData) + " RETURNSTDOUT WAIT";

        res = handle.submit2("local", "PROCESS", request);

        if (res.rc != STAFResult.Ok)
        {
            System.out.println("STAF local PROCESS %s failed, RC=" +
                               res.rc + " Result=" + res.result);
            System.exit(1);
        }

        mc = STAFMarshallingContext.unmarshall(
            res.result, res.resultContext,
            STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS);
        Map processMap = (Map)mc.getRootObject();
        List fileList = (List)processMap.get("fileList");
        Map stdoutFileMap = (Map)fileList.get(0);
        String stdoutFileData = (String)stdoutFileMap.get("data");

        System.out.println(
            "\nProcess Stdout File Data (containing valid marshalled data):\n" +
            stdoutFileData);

        // Have to account for the end of line char(s) added to stdoutFileData
        if (!(stdoutFileData.substring(0, validData.length()).equals(
            validData)))
        {
            System.out.println(
                "ERROR: Returned data not same as original stdout file contents" +
                "\n  Expected:\n" + validData +
                "\n  Found:\n" + stdoutFileData);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling marshalled string that only contains the " +
            "scalar marker");

        message = "@SDT/$S";
        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalling @SDT/$S resulted in: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling marshalled string with invalid " +
            "(non-numeric) string length");

        message = "@SDT/$S:1.:xxxxxxxxx";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling marshalled string with string length > " +
            "actual string length");

        message = "@SDT/$S:10:xxxxxxxxxxx";
        
        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling list that only contains the " +
            "list marker");
        
        message = "@SDT/[";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling list with an invalid number-of-items: 1.");
        
        message = "@SDT/[1.:13:@SDT/$S:3:XXX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling list with an invalid " +
            "number-of-items (2 but only 1 item)");  
        
        message = "@SDT/[2:13:@SDT/$S:3:XXX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling list with an extra X tacked on");
        
        message = "@SDT/[1:13:@SDT/$S:3:XXXX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling map with only the map marker");
        
        message = "@SDT/{";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling map with a blank map-length");
        
        message = "@SDT/{:::1:A@SDT/$S:3:abc";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }
        
        System.out.println(
            "Test unmarshalling map with map-length that is > actual " +
            "map length");
        
        message = "@SDT/{:18::1:A@SDT/$S:3:abc";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }
        
        System.out.println(
            "Test unmarshalling map with an invalid key length: 1.");
        
        message = "@SDT/{:17::1.:A@SDT/$S:3:abc";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling map with an extra X tacked on");
        
        message = "@SDT/{:17::1:A@SDT/$S:3:abcX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling map class with only the map class marker");
        
        message = "@SDT/%";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

        System.out.println(
            "Test unmarshalling context with extra data at end");
        
        message =
          "@SDT/*:380:" +
            "@SDT/{:227::13:map-class-map@SDT/{:199::10:Test/MyMap@SDT/{:174:" +
               ":4:keys@SDT/[2:127:" +
                 "@SDT/{:50::12:display-name@SDT/$S:4:Name:3:key@SDT/$S:4:name" +
                 "@SDT/{:57::12:display-name@SDT/$S:10:Executable:3:key@SDT/$S:4:exec" +
               ":4:name@SDT/$S:10:Test/MyMap" +
            "@SDT/[2:130:" +
               "@SDT/%:55::10:Test/MyMap@SDT/$S:5:TestA@SDT/$S:15:/tests/TestA.py" +
               "@SDT/%:55::10:Test/MyMap@SDT/$S:5:TestB@SDT/$S:15:/tests/TestB.sh" +
          "XXXXX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }
        
        System.out.println(
            "Test unmarshalling context with an invalid map-class-length");
        
        message = 
           "@SDT/*:251:" +
             "@SDT/{:327::13:map-class-map@SDT/{:199::10:Test/MyMap@SDT/{:174:" +
               ":4:keys@SDT/[2:127:" +
                 "@SDT/{:50::12:display-name@SDT/$S:4:Name:3:key@SDT/$S:4:name" +
                 "@SDT/{:57::12:display-name@SDT/$S:10:Executable:3:key@SDT/$S:4:exec" +
               ":4:name@SDT/$S:10:Test/MyMap" +
             "@SDT/$S:3:XXX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }
        
        System.out.println(
            "Test unmarshalling context with an invalid root object length");
        
        message = 
           "@SDT/*:251:" +
             "@SDT/{:227::13:map-class-map@SDT/{:199::10:Test/MyMap@SDT/{:174:" +
               ":4:keys@SDT/[2:127:" +
                 "@SDT/{:50::12:display-name@SDT/$S:4:Name:3:key@SDT/$S:4:name" +
                 "@SDT/{:57::12:display-name@SDT/$S:10:Executable:3:key@SDT/$S:4:exec" +
               ":4:name@SDT/$S:10:Test/MyMap" +
             "@SDT/$S:4:XXX";

        mc = STAFMarshallingContext.unmarshall(message);
        msg = (String)mc.getRootObject();

        if (!msg.equals(message))
        {
            System.out.println(
                "ERROR: Unmarshalled root object: " + msg +
                "  Expected: " + message);
            System.exit(1);
        }

    }
} 
 