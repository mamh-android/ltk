/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2007                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

/**
 * This class allows you to submit a command-line STAF request using the STAF Java
 * support which is useful if you want to verify that the STAF Java support is
 * working correctly, without requiring a GUI display or any modifications to
 * the CLASSPATH.
 * <p>
 * The syntax to invoke this class is: 
 * <code>java com.ibm.staf.TestJSTAF &lt;Endpoint | LOCAL> &lt;Service> &lt;Request></code>
 * 
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_TestJSTAF">
 *      Section "5.1 Class TestJSTAF" in the STAF Java User's Guide</a>
 */ 
public class TestJSTAF
{
    /**
     * This method invokes the TestJSTAF application.
     * <p>
     * Here are some examples of invoking the TestJSTAF application:
     * <ul>
     * <li><code>C:\>java com.ibm.staf.TestJSTAF LOCAL MISC VERSION<br>
     * TestJSTAF using STAF handle 15<br>
     * RC=0<br>
     * Result=3.2.1</code>
     * <li><code>C:\>C:\jdk1.5.0_02\jre\bin\java com.ibm.staf.TestJSTAF LOCAL PING PING<br>
     * TestJSTAF using STAF handle 19
     * RC=0
     * Result=PONG</code>
     * <li><code>C:\>java com.ibm.staf.TestJSTAF machineA.company.com VAR RESOLVE STRING {STAF/Config/OS/Name}<br>
     * TestJSTAF using STAF handle 27
     * RC=0
     * Result=WinXP</code>
     * </ul>
     * 
     * @param args Command-line arguments provided by the user.  The three arguments you
     *             must specify are:
     *             <ul>
     *             <li>Endpoint or <code>"local"</code>
     *             <li>STAF service name
     *             <li>Request to submit to the STAF service
     *             </ul>
     */ 
    public static void main(String[] args)
    {
        if (args.length < 3)
        {
            System.out.println("Usage: java com.ibm.staf.TestJSTAF <Endpoint " +
                               "| LOCAL> <Service> <Request> ");
            System.exit(1);
        }

        String where = args[0];
        String service = args[1];
        String request = "";

        for (int i = 2; i < args.length; i++)
        {
            request += args[i] + " ";
        }
        
        request = request.trim();

        STAFHandle handle = null;

        try
        {
            handle = new STAFHandle("TestJSTAF");
        }
        catch(STAFException e)
        {
            e.printStackTrace();
            System.exit(1);
        }

        System.out.println("TestJSTAF using STAF handle " + handle.getHandle());

        STAFResult result = handle.submit2(where, service, request);

        System.out.println("RC=" + result.rc);

        if (STAFMarshallingContext.isMarshalledData(result.result))
        {
            System.out.println("Result=");
            System.out.println(STAFMarshallingContext.unmarshall(result.result));
        }
        else
        {
            System.out.println("Result=" + result.result);
        }
    }
}
