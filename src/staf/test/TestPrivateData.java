/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.ibm.staf.*;
import java.util.List;
import java.util.ArrayList;

public class TestPrivateData
{
    // This is the main command line entry point

    public static void main(String [] argv)
    {
        // Verify the command line arguments

        if (argv.length != 0)
        {
            System.out.println();
            System.out.println("Usage: java TestPrivateData");
            System.exit(1);
        }

        // Register with STAF

        try
        {
            handle = new STAFHandle("Private_Data_Test");
        }
        catch (STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            System.exit(1);
        }
        
        // Test private data methods

        List testData = new ArrayList(15);

        List subTest1 = new ArrayList(8);
        subTest1.add("secret");
        subTest1.add("secret");
        subTest1.add("!!@secret@!!");
        subTest1.add("secret");
        subTest1.add("secret");
        subTest1.add("secret");
        subTest1.add("secret");
        subTest1.add("!!@secret@!!");
        testData.add(subTest1);

        List subTest2 = new ArrayList(8);
        subTest2.add("!!@secret@!!");
        subTest2.add("************");
        subTest2.add("!!@secret@!!");
        subTest2.add("secret");
        subTest2.add("secret");
        subTest2.add("secret");
        subTest2.add("^!!@secret^@!!");
        subTest2.add("!!@^^!!@secret^^@!!@!!");
        testData.add(subTest2);

        List subTest3 = new ArrayList(8);
        subTest3.add("Pw: !!@pw@!!");
        subTest3.add("Pw: ********");
        subTest3.add("!!@Pw: ^!!@pw^@!!@!!");
        subTest3.add("Pw: !!@pw@!!");
        subTest3.add("Pw: pw");
        subTest3.add("Pw: pw");
        subTest3.add("Pw: ^!!@pw^@!!");
        subTest3.add("!!@Pw: ^^!!@pw^^@!!@!!");
        testData.add(subTest3);
        
        List subTest4 = new ArrayList(8);
        subTest4.add("^!!@secret@!!");
        subTest4.add("^!!@secret@!!");
        subTest4.add("!!@^^!!@secret^@!!@!!");
        subTest4.add("!!@secret@!!");
        subTest4.add("!!@secret@!!");
        subTest4.add("!!@secret@!!");
        subTest4.add("^^!!@secret^@!!");
        subTest4.add("!!@^^^!!@secret^^@!!@!!");
        testData.add(subTest4);
        
        List subTest5 = new ArrayList(8);
        subTest5.add("^!!@secret^@!!");
        subTest5.add("^!!@secret^@!!");
        subTest5.add("!!@^^!!@secret^^@!!@!!");
        subTest5.add("!!@secret@!!");
        subTest5.add("!!@secret@!!");
        subTest5.add("!!@secret@!!");
        subTest5.add("^^!!@secret^^@!!");
        subTest5.add("!!@^^^!!@secret^^^@!!@!!");
        testData.add(subTest5);
        
        List subTest6 = new ArrayList(8);
        subTest6.add("!!@secret");
        subTest6.add("!!@secret");
        subTest6.add("!!@^!!@secret@!!");
        subTest6.add("!!@secret");
        subTest6.add("!!@secret");
        subTest6.add("!!@secret");
        subTest6.add("^!!@secret");
        subTest6.add("!!@^^!!@secret@!!");
        testData.add(subTest6);
        
        List subTest7 = new ArrayList(8);
        subTest7.add("!!@secret^@!!");
        subTest7.add("!!@secret^@!!");
        subTest7.add("!!@^!!@secret^^@!!@!!");
        subTest7.add("!!@secret@!!");
        subTest7.add("!!@secret@!!");
        subTest7.add("!!@secret@!!");
        subTest7.add("^!!@secret^^@!!");
        subTest7.add("!!@^^!!@secret^^^@!!@!!");
        testData.add(subTest7);

        List subTest8 = new ArrayList(8);
        subTest8.add("Pw1=!!@a@!!, Pw2=!!@pw@!!.");
        subTest8.add("Pw1=*******, Pw2=********.");
        subTest8.add("!!@Pw1=^!!@a^@!!, Pw2=^!!@pw^@!!.@!!");
        subTest8.add("Pw1=!!@a@!!, Pw2=!!@pw@!!.");
        subTest8.add("Pw1=a, Pw2=pw.");
        subTest8.add("Pw1=a, Pw2=pw.");
        subTest8.add("Pw1=^!!@a^@!!, Pw2=^!!@pw^@!!.");
        subTest8.add("!!@Pw1=^^!!@a^^@!!, Pw2=^^!!@pw^^@!!.@!!");
        testData.add(subTest8);

        List subTest9 = new ArrayList(8);
        subTest9.add("^!!@a@!!^@!!b!!@");
        subTest9.add("^!!@a@!!^@!!b!!@");
        subTest9.add("!!@^^!!@a^@!!^^@!!b^!!@@!!");
        subTest9.add("!!@a@!!@!!b!!@");
        subTest9.add("!!@a@!!@!!b!!@");
        subTest9.add("!!@a@!!@!!b!!@");
        subTest9.add("^^!!@a^@!!^^@!!b^!!@");
        subTest9.add("!!@^^^!!@a^^@!!^^^@!!b^^!!@@!!");
        testData.add(subTest9);
        
        List subTest10 = new ArrayList(8);
        subTest10.add("Pw1=!!@secret, !!@pw@!!.");
        subTest10.add("Pw1=*******************.");
        subTest10.add("!!@Pw1=^!!@secret, ^!!@pw^@!!.@!!");
        subTest10.add("Pw1=!!@secret, !!@pw@!!.");
        subTest10.add("Pw1=secret, !!@pw.");
        subTest10.add("Pw1=secret, !!@pw.");
        subTest10.add("Pw1=^!!@secret, ^!!@pw^@!!.");
        subTest10.add("!!@Pw1=^^!!@secret, ^^!!@pw^^@!!.@!!");
        testData.add(subTest10);
        
        List subTest11 = new ArrayList(8);
        subTest11.add("Pw1=!!@secret@!!, !!@pw.");
        subTest11.add("Pw1=************, !!@pw.");
        subTest11.add("!!@Pw1=^!!@secret^@!!, ^!!@pw.@!!");
        subTest11.add("Pw1=!!@secret@!!, !!@pw.");
        subTest11.add("Pw1=secret, !!@pw.");
        subTest11.add("Pw1=secret, !!@pw.");
        subTest11.add("Pw1=^!!@secret^@!!, ^!!@pw.");
        subTest11.add("!!@Pw1=^^!!@secret^^@!!, ^^!!@pw.@!!");
        testData.add(subTest11);
        
        List subTest12 = new ArrayList(8);
        subTest12.add("Msg: !!@Pw: ^!!@pw^@!!@!!");
        subTest12.add("Msg: ********************");
        subTest12.add("!!@Msg: ^!!@Pw: ^^!!@pw^^@!!^@!!@!!");
        subTest12.add("Msg: !!@Pw: ^!!@pw^@!!@!!");
        subTest12.add("Msg: Pw: !!@pw@!!");
        subTest12.add("Msg: Pw: pw");
        subTest12.add("Msg: ^!!@Pw: ^^!!@pw^^@!!^@!!");
        subTest12.add("!!@Msg: ^^!!@Pw: ^^^!!@pw^^^@!!^^@!!@!!");
        testData.add(subTest12);
        
        List subTest13 = new ArrayList(8);
        subTest13.add("@!!a!!@b@!!");
        subTest13.add("@!!a*******");
        subTest13.add("!!@^@!!a^!!@b^@!!@!!");
        subTest13.add("@!!a!!@b@!!");
        subTest13.add("@!!ab");
        subTest13.add("@!!ab");
        subTest13.add("^@!!a^!!@b^@!!");
        subTest13.add("!!@^^@!!a^^!!@b^^@!!@!!");
        testData.add(subTest13);
        
        List subTest14 = new ArrayList(8);
        subTest14.add("Msg: !!@Pw is ^^!!@secret^^@!!.@!!");
        subTest14.add("Msg: *****************************");
        subTest14.add("!!@Msg: ^!!@Pw is ^^^!!@secret^^^@!!.^@!!@!!");
        subTest14.add("Msg: !!@Pw is ^^!!@secret^^@!!.@!!");
        subTest14.add("Msg: Pw is !!@secret@!!.");
        subTest14.add("Msg: Pw is !!@secret@!!.");
        subTest14.add("Msg: ^!!@Pw is ^^^!!@secret^^^@!!.^@!!");
        subTest14.add("!!@Msg: ^^!!@Pw is ^^^^!!@secret^^^^@!!.^^@!!@!!");
        testData.add(subTest14);
        
        List subTest15 = new ArrayList(8);
        subTest15.add("");
        subTest15.add("");
        subTest15.add("");
        subTest15.add("");
        subTest15.add("");
        subTest15.add("");
        subTest15.add("");
        subTest15.add("");
        testData.add(subTest15);
        
        List subTest16 = new ArrayList(8);
        subTest16.add(null);
        subTest16.add(null);
        subTest16.add(null);
        subTest16.add(null);
        subTest16.add(null);
        subTest16.add(null);
        subTest16.add(null);
        subTest16.add(null);
        testData.add(subTest16);

        System.out.println("KEY:\n  apd() = STAFUtil.addPrivacyDelimiters()\n" +
             "  mpd() = STAFUtil.maskPrivateData()\n" +
             "  rpd() = STAFUtil.removePrivacyDelimiters()\n" +
             "  epd() = STAFUtil.escapePrivacyDelimiters()\n");

        int numErrors = 0;

        for (int i = 0; i < testData.size(); ++i)
        {
            List subTest = (List)testData.get(i);
            String data = (String)subTest.get(0);

            System.out.println("\n" + (i+1) + ")  data: " + data + "\n");

            // 1

            String maskedData = STAFUtil.maskPrivateData(data);
            System.out.println("mpd(" + data + "): " + maskedData + "\n");
            
            if (maskedData == null)
            {
                if (subTest.get(1) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 1):  mpd(" + data + "): " +
                        maskedData + "\n" +
                        "        Expected: " + subTest.get(1));
                    numErrors++;
                }
            }
            else if (!maskedData.equals((String)subTest.get(1)))
            {
                System.out.println(
                    "ERROR(" + i + ", 1):  mpd(" + data + "): " +
                    maskedData + "\n" +
                    "        Expected: " + subTest.get(1));
                numErrors++;
            }

            // 2

            String dataWithPrivacy = STAFUtil.addPrivacyDelimiters(data);
            System.out.println("apd(" + data + "): " + dataWithPrivacy);
            
            if (dataWithPrivacy == null)
            {
                if (subTest.get(2) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 2):  apd(" + data + "): " +
                        dataWithPrivacy + "\n" +
                        "        Expected: " + subTest.get(2));
                    numErrors++;
                }
            }
            else if (!dataWithPrivacy.equals((String)subTest.get(2)))
            {
                System.out.println(
                    "ERROR(" + i + ", 2):  apd(" + data + "): " +
                    dataWithPrivacy + "\n" +
                    "        Expected: " + subTest.get(2));
                numErrors++;
            }

            // 3

            String dataWithPrivacyRemoved = STAFUtil.removePrivacyDelimiters(
                dataWithPrivacy, 1);
            System.out.println("rpd(" + dataWithPrivacy + ", 1): " +
                dataWithPrivacyRemoved);

            if (dataWithPrivacyRemoved == null)
            {
                if (subTest.get(3) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 3):  rpd(" + dataWithPrivacy + ", 1): " +
                        dataWithPrivacyRemoved + "\n" +
                        "        Expected: " + subTest.get(3));
                    numErrors++;
                }
            }
            else if (!dataWithPrivacyRemoved.equals((String)subTest.get(3)))
            {
                System.out.println(
                    "ERROR(" + i + ", 3):  rpd(" + dataWithPrivacy + ", 1): " +
                    dataWithPrivacyRemoved + "\n" +
                    "        Expected: " + subTest.get(3));
                numErrors++;
            }

            // 4

            String dataWithPrivacyRemoved2 = STAFUtil.removePrivacyDelimiters(
                dataWithPrivacy, 2);
            System.out.println("rpd(" + dataWithPrivacy + ", 2): " +
                dataWithPrivacyRemoved2);

            if (dataWithPrivacyRemoved2 == null)
            {
                if (subTest.get(4) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 4):  rpd(" + dataWithPrivacy + ", 2): " +
                        dataWithPrivacyRemoved2 + "\n" +
                        "        Expected: " + subTest.get(4));
                    numErrors++;
                }
            }
            else if (!dataWithPrivacyRemoved2.equals((String)subTest.get(4)))
            {
                System.out.println(
                    "ERROR(" + i + ", 4):  rpd(" + dataWithPrivacy + ", 2): " +
                    dataWithPrivacyRemoved2 + "\n" +
                    "        Expected: " + subTest.get(4));
                numErrors++;
            }

            // 5

            String dataWithAllPrivacyRemoved =
                STAFUtil.removePrivacyDelimiters(dataWithPrivacy, 0);
            System.out.println("rpd(" + dataWithPrivacy + ", 0): "
                 + dataWithAllPrivacyRemoved);

            if (dataWithAllPrivacyRemoved == null)
            {
                if (subTest.get(5) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 5):  rpd(" + dataWithPrivacy + ", 0): " +
                        dataWithAllPrivacyRemoved + "\n" +
                        "        Expected: " + subTest.get(5));
                    numErrors++;
                }
            }
            else if (!dataWithAllPrivacyRemoved.equals((String)subTest.get(5)))
            {
                System.out.println(
                    "ERROR(" + i + ", 5):  rpd(" + dataWithPrivacy + ", 0): " +
                    dataWithAllPrivacyRemoved + "\n" +
                    "        Expected: " + subTest.get(5));
                numErrors++;
            }

            // 6

            String escapedData = STAFUtil.escapePrivacyDelimiters(data);
            System.out.println("\nepd(" + data + "): " + escapedData );

            if (escapedData == null)
            {
                if (subTest.get(6) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 6):  epd(" + data + "): " +
                        escapedData + "\n" +
                        "        Expected: " + subTest.get(6));
                    numErrors++;
                }
            }
            else if (!escapedData.equals((String)subTest.get(6)))
            {
                System.out.println(
                    "ERROR(" + i + ", 6):  epd(" + data + "): " +
                    escapedData + "\n" +
                    "        Expected: " + subTest.get(6));
                numErrors++;
            }

            // 7

            dataWithPrivacy = STAFUtil.addPrivacyDelimiters(escapedData);
            System.out.println("apd(" + escapedData + "): " + dataWithPrivacy);

            if (dataWithPrivacy == null)
            {
                if (subTest.get(7) != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 7):  apd(" + escapedData + "): " +
                        dataWithPrivacy + "\n" +
                        "        Expected: " + subTest.get(7));
                    numErrors++;
                }
            }
            else if (!dataWithPrivacy.equals((String)subTest.get(7)))
            {
                System.out.println(
                    "ERROR(" + i + ", 7):  apd(" + escapedData + "): " +
                    dataWithPrivacy + "\n" +
                    "        Expected: " + subTest.get(7));
                numErrors++;
            }

            // 8

            dataWithPrivacyRemoved =
                STAFUtil.removePrivacyDelimiters(dataWithPrivacy, 1);
            System.out.println("rpd(" + dataWithPrivacy + ", 1): "
                 + dataWithPrivacyRemoved);

            if (dataWithPrivacyRemoved == null)
            {
                if (data != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 8):  rpd(" + dataWithPrivacy + ", 1): " +
                        dataWithPrivacyRemoved + "\n" +
                        "        Expected: " + data);
                    numErrors++;
                }
            }
            else if (!dataWithPrivacyRemoved.equals(data))
            {
                System.out.println(
                    "ERROR(:" + i + ", 8):  rpd(" +
                    dataWithPrivacy + ", 1): " + dataWithPrivacyRemoved + "\n" +
                    "        Expected: " + data);
                numErrors++;
            }
            // 9

            dataWithAllPrivacyRemoved =
                STAFUtil.removePrivacyDelimiters(dataWithPrivacy);
            System.out.println(
                "rpd(" + dataWithPrivacy + "): " +
                dataWithAllPrivacyRemoved);

            if (dataWithAllPrivacyRemoved == null)
            {
                if (data != null)
                {
                    System.out.println(
                        "ERROR(" + i + ", 9):  rpd(" + dataWithPrivacy + "): " +
                        dataWithAllPrivacyRemoved + "\n" +
                        "        Expected: " + data);
                    numErrors++;
                }
            }
            else if (!dataWithAllPrivacyRemoved.equals(data))
            {
                System.out.println(
                    "ERROR:(" + i + ", 9):  rpd(" +
                    dataWithPrivacy + "): " + dataWithAllPrivacyRemoved +
                    "\n        Expected: " + data);
                numErrors++;
            }
        }
        
        // Test compareSTAFVersion method

        // These versions should all be < actual version of STAF running
        String[] requiredVersionList = {
            "3", "3.0", "3.0.0", "3.1 Beta 1", "3.1 Alpha 1", "2.6.9", "2"};

        for (int i = 0; i < requiredVersionList.length; ++i)
        {
            // Verify that the required version of STAF is running on the
            // local service machine.  

            STAFResult res = STAFUtil.compareSTAFVersion(
                "local", handle, requiredVersionList[i]);

            if (res.rc != STAFResult.Ok)
            {
                if (res.rc == STAFResult.InvalidSTAFVersion)
                {
                    System.out.println(
                        "ERROR: Minimum required STAF version for this " +
                        "service is not running.  " + res.result);
                    numErrors++;
                }
                else     
                {
                    System.out.println(
                        "ERROR: Error verifying the STAF version. RC: " + res.rc +
                        ", Additional info: " + res.result);
                    numErrors++;
                }
            }
        }

        // Test STAFVersion class
        
        if ((new STAFVersion("3")).compareTo((new STAFVersion("3.0"))) != 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Versions 3 and 3.0 are not equal");
            numErrors++;
        }

        if ((new STAFVersion("3")).compareTo((new STAFVersion("3.0.0"))) != 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Versions 3 and 3.0.0 are not equal");
            numErrors++;
        }

        if ((new STAFVersion("3")).compareTo((new STAFVersion("3.0.0.0"))) != 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Versions 3 and 3.0.0.0 are not equal");
            numErrors++;
        }

        if ((new STAFVersion("3")).compareTo((new STAFVersion("3.0.0  "))) != 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Versions 3 and 3.0.0   are not equal");
            numErrors++;
        }

        if ((new STAFVersion("3.0.0")).compareTo((new STAFVersion("3.1.0"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.0.0 >= 3.1.0");
            numErrors++;
        }

        if ((new STAFVersion("3.0.2")).compareTo((new STAFVersion("3.0.3"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.0.2 >= 3.0.3");
            numErrors++;
        }

        if ((new STAFVersion("3.0.0")).compareTo((new STAFVersion("3.1"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.0.0 >= 3.1");
            numErrors++;
        }

        if ((new STAFVersion("3.0.9")).compareTo((new STAFVersion("3.0.10"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.0.9 >= 3.0.10");
            numErrors++;
        }

        if ((new STAFVersion("3.0.0.1")).compareTo((new STAFVersion("3.1"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.0.0.1 >= 3.1");
            numErrors++;
        }
        
        if ((new STAFVersion("3.1.0 Alpha 1")).compareTo((new STAFVersion("3.1.0 Beta 1"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.1.0 Alpha1 >= 3.1.0 Beta 1");
            numErrors++;
        }
        
        if ((new STAFVersion("3.1.0 Beta 1")).compareTo((new STAFVersion("3.1.0"))) >= 0)
        {
            System.out.println(
                "ERROR: STAFVersion.compareTo(): " +
                "Version 3.1.0 Beta1 >= 3.1.0");
            numErrors++;
        }

        if (numErrors == 0)
        {
            System.out.println("\nTest completed successfully");
        }
        else
        {
            System.out.println("\nTest failed with " + numErrors +" errors");
        }
    }

    private static STAFHandle handle;
}
