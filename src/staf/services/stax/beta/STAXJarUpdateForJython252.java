import com.ibm.staf.*;
import java.util.*;
import java.util.jar.*;
import java.io.*;

public class STAXJarUpdateForJython252
{
    private static STAFHandle handle = null;
    private static String currentDir = null;

    private static void cleanup()
    {
        if (currentDir != null && handle != null)
        {
            System.out.println("\nPerforming clean-up...");

            // Delete META-INF, STAF-INF, JYTHON-INF directories if exist from
            // the current directory

            String[] deleteDirArray = { "META-INF", "STAF-INF", "JYTHON-INF" };

            for (int i = 0; i < deleteDirArray.length; i++)
            {
                String dirName = currentDir + File.separator + deleteDirArray[i];

                String request = "DELETE ENTRY " + STAFUtil.wrapData(dirName) +
                    " RECURSE CONFIRM";

                STAFResult result = handle.submit2("local", "FS", request);

                if (result.rc != 0 && result.rc != STAFResult.DoesNotExist)
                {
                    System.out.println(
                        "ERROR: STAF local FS " + request +
                        "\nfailed with RC: " + result.rc +
                        ", Additional Info: " + result.result);
                }
            }
        }

        if (handle != null)
        {
            // Unregister with STAF

            try
            {
                handle.unRegister();
            }
            catch (STAFException e)
            {
                System.out.println("Error unregistering with STAF, RC: " + e.rc);
                System.exit(1);
            } 
        }
    }

    /**
     * Verify that a specified file name or directory name exists
     */
    private static void verifyFileExists(String name) 
    {
        // Verify that the file or  directory exists

        File f = new File(name);

        if (!f.exists())
        {
            System.out.println(f + " does not exist");
            System.exit(1);
        }
    }

    // This is the main command line entry point

    public static void main(String [] argv)
    {
        // Verify the command line arguments

        if (argv.length != 1)
        {
            System.out.println();
            System.out.println("Usage: java STAXJarUpdateForJython252 <Jython252 Directory Name>");
            System.out.println("Examples:");
            System.out.println("  java STAXJarUpdateForJython252 C:\\jython2.5.2rc2");
            System.out.println("  java STAXJarUpdateForJython252 /tmp/jython2.5.2rc2");
            System.exit(1);
        }

        String jython252DirName = argv[0];

        // Verify that the Jython directory exists

        verifyFileExists(jython252DirName);

        // Verify that the Jython directory contains jython.jar

        String jythonJarName = jython252DirName + File.separator + "jython.jar";
        verifyFileExists(jythonJarName);

        // Verify that the Jython directory contains a Lib subdirectory

        String jythonLibName = jython252DirName + File.separator + "Lib";
        verifyFileExists(jythonLibName);

        // Verify that the current directory contains a STAX_NoJython.jar file

        currentDir = System.getProperty("user.dir");

        String staxJarFileName = currentDir + File.separator + "STAX_NoJython.jar";
        verifyFileExists(staxJarFileName);

        // Register with STAF (verifies that STAFProc is running)

        System.out.println("\nRegistering with STAF...");

        try
        {
            handle = new STAFHandle("STAX_Jar_Update_For_Jython252");
        }
        catch (STAFException e)
        {
            System.out.println("Error registering with STAF, RC: " + e.rc);
            System.out.println("Make sure that STAFProc is running on this machine.");
            System.exit(1);
        }

        // Extract the STAX_NoJython.jar file in the current directory

        System.out.println("\nExtracting the " + staxJarFileName + " file...");

        String command = "jar xf " + staxJarFileName;

        String request = "START SHELL COMMAND " + STAFUtil.wrapData(command) +
            " WORKDIR " + STAFUtil.wrapData(currentDir) +
            " RETURNSTDOUT STDERRTOSTDOUT WAIT";

        System.out.println("STAF local PROCESS " + request);

        STAFResult result = handle.submit2("local", "PROCESS", request);

        if (result.rc != 0)
        {
            System.out.println(
                "Error extracting the " + staxJarFileName + " file, RC: " +
                result.rc + ", Additional Info: " + result.result);

            cleanup();
            System.exit(1);
        }
 
        STAFMarshallingContext mc = STAFMarshallingContext.unmarshall(result.result);
        Map processMap = (Map)mc.getRootObject();
        String processRC = (String)processMap.get("rc");

        if (!processRC.equals("0"))
        {
            // Get the data from the Stdout file created by the process
 
            List fileList = (List)processMap.get("fileList");
            Map stdoutMap = (Map)fileList.get(0);
            String stdoutData = (String)stdoutMap.get("data");

            System.out.println(
                "Error extracting the " + staxJarFileName + " file, RC: " +
                processRC + " Stdout/Stderr:\n" + stdoutData);

            cleanup();
            System.exit(1);
        }

        // Copy the jython.jar file from the specified directory to the current directory

        System.out.println("\nCopying the jython.jar file...");

        String toFile = currentDir + File.separator + "STAF-INF" +
            File.separator + "jars" + File.separator + "jython.jar";

        request = "COPY FILE " + STAFUtil.wrapData(jythonJarName) +
            " TOFILE " + STAFUtil.wrapData(toFile);

        System.out.println("STAF local FS " + request);

        result = handle.submit2("local", "FS", request);

        if (result.rc != 0)
        {
            System.out.println(
                "ERROR: STAF local FS " + request +
                "\nfailed with RC: " + result.rc +
                ", Additional Info: " + result.result);

            cleanup();
            System.exit(1);
        }       

        // Copy the Lib directory from the specified Jython 2.5.2 directory

        System.out.println("\nCopying the Jython Lib directory...");

        String toDir = currentDir + File.separator + "JYTHON-INF" +
            File.separator + "Lib";

        request = "COPY DIRECTORY " + STAFUtil.wrapData(jythonLibName) +
            " TODIRECTORY " + STAFUtil.wrapData(toDir) + " RECURSE";

        System.out.println("STAF local FS " + request);

        result = handle.submit2("local", "FS", request);

        if (result.rc != 0)
        {
            System.out.println(
                "ERROR: STAF local FS " + request +
                "\nfailed with RC: " + result.rc +
                ", Additional Info: " + result.result);

            cleanup();
            System.exit(1);
        }

        // Delete file STAX.jar if it already exists

        String newStaxJarName = "STAX.jar";
        String newStaxJarPath = currentDir + File.separator + newStaxJarName;

        File f = new File(newStaxJarPath);

        if (f.exists())
        {
            System.out.println("\nDeleting previously created file " + newStaxJarPath + "...");

            request = "DELETE ENTRY " + STAFUtil.wrapData(newStaxJarPath) + " CONFIRM";

            System.out.println("STAF local FS " + request);

            result = handle.submit2("local", "FS", request);
           
            if (result.rc != 0)
            {
                System.out.println(
                    "Cannot delete file " + newStaxJarPath +
                    ", RC: " + result.rc + ", Result: " + result.result); 
            }
        }

        // Create file STAX.jar which includes Jython 2.5.2

        System.out.println("\nCreating " + newStaxJarName + " which includes Jython 2.5.2...");

        command = "jar cfm " + newStaxJarName + " META-INF/MANIFEST.MF STAF-INF JYTHON-INF";

        request = "START SHELL COMMAND " + STAFUtil.wrapData(command) +
            " WORKDIR " + STAFUtil.wrapData(currentDir) +
            " RETURNSTDOUT STDERRTOSTDOUT WAIT";

        System.out.println("STAF local PROCESS " + request);

        result = handle.submit2("local", "PROCESS", request);

        if (result.rc != 0)
        {
            System.out.println(
                "Error creating the " + newStaxJarName + " file, RC: " +
                result.rc + ", Additional Info: " + result.result);

            cleanup();
            System.exit(1);
        }
 
        mc = STAFMarshallingContext.unmarshall(result.result);
        processMap = (Map)mc.getRootObject();
        processRC = (String)processMap.get("rc");

        if (!processRC.equals("0"))
        {
            // Get the data from the Stdout file created by the process
 
            List fileList = (List)processMap.get("fileList");
            Map stdoutMap = (Map)fileList.get(0);
            String stdoutData = (String)stdoutMap.get("data");

            System.out.println(
                "Error creating the " + newStaxJarName + " file, RC: " +
                processRC + " Stdout/Stderr:\n" + stdoutData);

            cleanup();
            System.exit(1);
        }

        System.out.println(
            "\nUse jar file " + newStaxJarPath +
            " to register the STAX V3.5.0 Beta 1 service.");
        
        // Perform cleanup

        cleanup();

        System.out.println("\nCompleted successfully");
        System.exit(0);
    }   
}