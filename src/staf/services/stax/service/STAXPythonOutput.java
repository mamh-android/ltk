/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2008                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.io.Writer;
import java.io.IOException;
import java.util.HashMap;
import com.ibm.staf.STAFUtil;
import com.ibm.staf.STAFResult;

/**
 * This class is to redirect a Python Interpreter's stdout/stderr
 * (e.g. from a print statement within a <script> element in a STAX job)
 * by calling setOut() and setErr() on the Python Interpreter at the
 * beginning of the job.  The constructor is passed a STAXJob object and
 * it gets the Python Output from the job to know whether to redirect
 * Python stdout/stderr by logging to the STAX Job User log and/or by sending
 * a message to the STAX Monitor, or to write to the JVM Log for the STAX
 * service.
 */
public class STAXPythonOutput extends Writer
{
    // Valid strings for the PYTHONOUTPUT option and their codes

    public static final int JOBUSERLOG = 1;
    public static final String JOBUSERLOG_STRING = new String("JobUserLog");

    public static final int MESSAGE = 2;
    public static final String MESSAGE_STRING = new String("Message");
    
    public static final int JOBUSERLOGANDMSG = 3;
    public static final String JOBUSERLOGANDMSG_STRING = new String(
        "JobUserLogAndMsg");

    public static final int JVMLOG = 4;
    public static final String JVMLOG_STRING = new String("JVMLog");

    /**
     * Note: Extensions, etc should not use LOGLEVELS as we will probably move
     * this static variable into JSTAF (e.g. STAFUtil.java) in the future.
     */
    public static final String[] LOGLEVELS =
    {
        "Info", "Fatal", "Error", "Warning", "Trace", "Trace2", "Trace3",
        "Debug", "Debug2", "Debug3", "Start", "Stop", "Pass", "Fail", "Status",
        "User1", "User2", "User3", "User4", "User5", "User6", "User7", "User8" 
    };

    // Default log level to use when Jython output is redirected
    // to the STAX Job User Log
    public static String LOGLEVEL_DEFAULT = new String("Info");

    
    private STAXJob fJob = null;
    private int fPythonOutput;
    private String fLogLevel = null;
    private StringBuffer fData = new StringBuffer();
    private boolean fClosed = false;

    private STAXPythonOutput()
    {
        // Do nothing
    }

    /**
     * Creates a custom output stream used to redirect Python output.
     * If redirected to the STAX job log, Python stdout uses the log level
     * specified for the job.
     * @param job A STAXJob object representing the job
     */
    public STAXPythonOutput(STAXJob job)
    {
        this(job, job.getPythonLogLevel());
    }

    /**
     * Creates a custom output stream used to redirecting Python output.
     * If redirected to the STAX job log, Python stdout uses the specified
     * log level.
     * @param job A STAXJob object representing the job
     * @param logLevel A string containing the log level to use for
     * Python stdout
     */
    public STAXPythonOutput(STAXJob job, String logLevel)
    {
        fJob = job;
        fPythonOutput = fJob.getPythonOutput();
        fLogLevel = logLevel;
    }

    /**
     * Writes the specified characters to this writer.
     * @param cbuf the characters to write
     * @param off start offset n cbuf of first character to write
     * @param len number of characters to write
     * @throws IOException if an I/O error occurs. In particular,
     *                     an IOException may be thrown if the
     *                     writer has been closed.
     */
    public void write(char[] cbuf, int off, int len ) throws IOException
    {
        if (fClosed)
            throw new IOException("The Writer is closed");
        fData.append(cbuf, off, len);

    }

    /**
     * Closes the stream after flushing it, so later write() calls
     * will throw IOException.
     */
    public void close() throws IOException
    {
        flush();
        fClosed = true;
    }
    
   /**
    * Flushes the stream to force any buffered output characters
    * to be written out to the intended destination(s).
    */
    public void flush() throws IOException
    {
        // Remove the new line character(s) from the end of the data
        // (that was added when flushing from Writer)..

        if ((STAX.lineSep.length() > 0) &&
            (fData.length() >= STAX.lineSep.length()))
        {
            int start = fData.length() - STAX.lineSep.length();

            if (fData.substring(start).equals(STAX.lineSep))
                fData.delete(start, fData.length());
        }

        // Don't log blank lines

        if (fData.length() == 0) return;

        // Write to the specified output destination for the STAX Job

        if (fPythonOutput == JVMLOG)
        {
            writeToJVMLog();
        }
        else
        {
            if ((fPythonOutput == MESSAGE) ||
                (fPythonOutput == JOBUSERLOGANDMSG))
            {
                sendToSTAXMonitor();
            }

            if ((fPythonOutput == JOBUSERLOG) ||
                (fPythonOutput == JOBUSERLOGANDMSG))
            {
                writeToJobUserLog();
            }
        }

        fData.setLength(0);
    }

    /**
     * Writes the message to the JVM Log, prepending the timestamp and job ID
     * to the message so that the following format is used:
     * <Timestamp> [JobID: <JobID>] <Message>
     */
    private void writeToJVMLog()
    {
        STAXTimestamp timestamp = new STAXTimestamp();

        System.out.println(
            timestamp.getTimestampString() +
            " [JobID: " + fJob.getJobNumber() + "] " +
            STAFUtil.maskPrivateData(fData.toString()));
    }

    /**
     * Logs the message in the STAX Job User Log.  If that fails, write the
     * message to the JVM Log
     */ 
    private void writeToJobUserLog()
    {
        STAFResult result = fJob.log(
            STAXJob.USER_JOB_LOG, fLogLevel, fData.toString());

        if (result.rc != 0)
        {
            // Error logging to the STAX Job User Log, so write the message
            // to the JVM Log instead

            writeToJVMLog();
        }
    }

    /**
     *  Send a message to the STAXMonitor (via an event)
     */ 
    private void sendToSTAXMonitor()
    {
        STAXTimestamp timestamp = new STAXTimestamp();
        HashMap<String, String> messageMap = new HashMap<String, String>();

        messageMap.put("messagetext", timestamp.getTimestampString() +
            " " + STAFUtil.maskPrivateData(fData.toString()));

        fJob.generateEvent(
            STAXMessageActionFactory.STAX_MESSAGE, messageMap);
    }

    /**
     * Checks if a specified python output value is valid.
     * @param output A String containing the python output
     * @return STAFResult A STAFResult object.  If not valid, returns a
     * STAFResult with an InvalidValue RC and an error message in the result.
     * If valid, returns RC 0 with the integer version of the python output
     * string value in the result.
     */
    public static STAFResult isValidPythonOutput(String output)
    {
        if (output.equalsIgnoreCase(JOBUSERLOG_STRING))
        {
            return new STAFResult(
                STAFResult.Ok, Integer.toString(JOBUSERLOG));
        }
        else if (output.equalsIgnoreCase(MESSAGE_STRING))
        {
            return new STAFResult(
                STAFResult.Ok, Integer.toString(MESSAGE));
        }
        else if (output.equalsIgnoreCase(JOBUSERLOGANDMSG_STRING))
        {
            return new STAFResult(
                STAFResult.Ok, Integer.toString(JOBUSERLOGANDMSG));
        }
        else if (output.equalsIgnoreCase(JVMLOG_STRING))
        {
            return new STAFResult(
                STAFResult.Ok, Integer.toString(JVMLOG));
        }
        else
        {
            return new STAFResult(
                STAFResult.InvalidValue,
                "Invalid value for PYTHONOUTPUT: " + output +
                ".  Valid values: " + JOBUSERLOG_STRING + ", " +
                MESSAGE_STRING + ", " + JOBUSERLOGANDMSG_STRING + ", or " +
                JVMLOG_STRING);
        }
    }

    /**
     * Converts an int Python Output flag to it's string representation.
     * @param output An int representing the Python Output flag
     */
    public static String getPythonOutputAsString(int output)
    {
        if (output == JOBUSERLOG)
            return JOBUSERLOG_STRING;
        else if (output == MESSAGE)
            return MESSAGE_STRING;
        else if (output == JOBUSERLOGANDMSG)
            return JOBUSERLOGANDMSG_STRING;
        else 
            return JVMLOG_STRING;
    }

    /**
     * Checks if a log level is valid.
     * Note: Extensions, etc should not use this method as we will probably
     * move this static method into JSTAF (e.g. STAFUtil.java) in the future.
     * @param level A String containing the log level
     * @return STAFResult A STAFResult object.  If not valid, returns a
     * STAFResult with an InvalidValue RC and an error message in the result.
     * If valid, returns RC 0 with the "pretty" version of the log level in
     * the result.
     */ 
    public static STAFResult isValidLogLevel(String level)
    {
        for (int i = 0; i < LOGLEVELS.length; i++)
        {
            if (level.equalsIgnoreCase(LOGLEVELS[i]))
                return new STAFResult(STAFResult.Ok, LOGLEVELS[i]);
        }
        
        StringBuffer errMsg = new StringBuffer(
            "Invalid value for PYTHONLOGLEVEL: " + level + ".  Valid values:");

        for (int i = 0; i < LOGLEVELS.length; i++)
        {
            if (i != 0)
                errMsg.append(", ");
            else
                errMsg.append(" ");

            errMsg.append(LOGLEVELS[i]);
        }

        return new STAFResult(STAFResult.InvalidValue, errMsg.toString());
    }
}
