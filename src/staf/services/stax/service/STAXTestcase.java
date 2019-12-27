/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFUtil;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class STAXTestcase
{
    // Testcase Modes
    public static final int DEFAULT_MODE = 0;
    public static final int STRICT_MODE  = 1;

    public STAXTestcase(String name)
    {
        fName = name;
        fMode = DEFAULT_MODE;

        // Create a testcase stack using the periods to determine the parent
        // testcases.  For example, the testcase stack created for a testcase
        // with name "Scenario1.TestA.myMachine will be
        // ['Scenario1', 'Scenario1.TestA', 'Scenario1.TestA.myMachine']
        
        int fromIndex = 0;
        int periodIndex = -1;
        
        while ((periodIndex = fName.indexOf(".", fromIndex )) != -1)
        {
            fTestcaseStack.add(fName.substring(0, periodIndex));
            fromIndex = periodIndex + 1;
        }

        fTestcaseStack.add(fName);
    }

    public STAXTestcase(String name, int mode)
    {
        this(name);
        this.fMode = mode;
        this.fStartedTimestamp = new STAXTimestamp();
    }
    
    public STAXTestcase(String name, List<String> testcaseStack)
    {
        fName = name;
        fMode = DEFAULT_MODE;
        fTestcaseStack = new ArrayList<String>(testcaseStack);
    }

    public STAXTestcase(String name, int mode, List<String> testcaseStack)
    {
        fName = name;
        fMode = mode;
        fTestcaseStack = new ArrayList<String>(testcaseStack);
        fStartedTimestamp = new STAXTimestamp();
    }

    public String getName() { return fName; }
    public int getMode() { return fMode; }
    public void setMode(int mode) { fMode = mode; }
    public int getNumPass() { return fNumPass; }
    public int getNumFail() { return fNumFail; }
    public String getLastStatus() { return fLastStatus; }
    public String getLastMessage() { return fLastMessage; }
    public String getElapsedTime() { return fElapsedTime; }
    public int getNumStarts() { return fNumStarts; }
    public int getNumStops() { return fNumStops; }

    public List<String> getTestcaseStack()
    {
        synchronized(fTestcaseStack)
        {
            return new ArrayList<String>(fTestcaseStack);
        }
    }

    public boolean isActive()
    {
        synchronized(this)
        {
            if (fNumStarts - fNumStops > 0)
                return true;
            else
                return false;
        }
    }
    
    public STAXTimestamp getLastStatusTimestamp()
    { return fLastStatusTimestamp; }

    public STAXTimestamp getStartedTimestamp()
    { return fStartedTimestamp; }

    public void updateStatus(String status, String message, STAXJob job)
    {
        synchronized(this)
        {
            if (status.equalsIgnoreCase("pass")) 
            {
                ++fNumPass;
                fLastStatus = "pass";
            } 
            else if (status.equalsIgnoreCase("fail"))
            {
                ++fNumFail;
                fLastStatus = "fail";
            }
            else if (status.equalsIgnoreCase("info"))
            {
                fLastStatus = "info";
            }

            if (!message.equals(""))
            {
                fLastMessage = message;
            }

            // Get the current date and time and set as last status date/time
            fLastStatusTimestamp = new STAXTimestamp();
        }
        
        HashMap<String, String> testcaseMap = new HashMap<String, String>();
        testcaseMap.put("name", fName);
        testcaseMap.put("block", null);
        testcaseMap.put("status", "update");
        testcaseMap.put("status-pass", String.valueOf(fNumPass));
        testcaseMap.put("status-fail", String.valueOf(fNumFail));
        testcaseMap.put("laststatus", String.valueOf(fLastStatus));
        testcaseMap.put("elapsed-time", fElapsedTime);
        testcaseMap.put("num-starts", String.valueOf(fNumStarts));
        testcaseMap.put("message", message);
        testcaseMap.put("startedTimestamp",
                        fStartedTimestamp.getTimestampString());
        testcaseMap.put("lastStatusTimestamp",
                        fLastStatusTimestamp.getTimestampString());
            
        job.generateEvent(STAXTestcaseStatusActionFactory.STAX_TESTCASE_STATUS,
            testcaseMap);

        // Log to STAX Job log if a non-blank message is provided and if the
        // status is not "info"

        if (!message.equals("") && !status.equalsIgnoreCase("info"))
        {
            String msg = "Testcase: " + fName + ", Pass: " + 
                getNumPass() + ", Fail: " + getNumFail() +
                ", Last Status: " + fLastStatus + ", Message: "
                + message;
            
            job.log(STAXJob.JOB_LOG, status, msg);
        }
    }

    public void start(STAXJob job)
    {
        synchronized(this)
        {
            // Increment the number of times the testcase is started
            ++fNumStarts;
        }
        
        if (job.getLogTCStartStop())
        {
            // Log a testcase start entry into the STAX Job Log
            job.log(STAXJob.JOB_LOG, "start", "Testcase: " + fName);
        }
        
        if (fMode == STAXTestcase.STRICT_MODE)
        {
            // Generate a STAX Monitor testcase begin event
            HashMap<String, String> testcaseMap =
                new HashMap<String, String>();
            testcaseMap.put("name", fName);
            testcaseMap.put("block", null);
            testcaseMap.put("status", "begin");
            testcaseMap.put("status-pass", String.valueOf(fNumPass));
            testcaseMap.put("status-fail", String.valueOf(fNumFail));
            testcaseMap.put("laststatus", String.valueOf(fLastStatus));
            testcaseMap.put("elapsed-time", fElapsedTime);
            testcaseMap.put("num-starts", String.valueOf(fNumStarts));
            testcaseMap.put("startedTimestamp",
                        fStartedTimestamp.getTimestampString());

            job.generateEvent(STAXTestcaseActionFactory.STAX_TESTCASE,
                              testcaseMap);
        }
    }

    public void stop(STAXJob job, STAXTimestamp startTimestamp)
    {
        synchronized(this)
        {
            // Increment the number of times the testcase is stopped
            ++fNumStops;
        }

        if (startTimestamp == null)
        {
            // Should never happen

            job.log(STAXJob.JOB_LOG, "warning",
                    "STAXTestcase::stop() -  Testcase: " + fName +
                    ".  Start timestamp is null");

            startTimestamp = new STAXTimestamp();
        }

        // Calculate the elapsed time 
        String elapsedTime = STAXMonitorUtil.getElapsedTime(
            STAXMonitorUtil.getCalendar2(startTimestamp.getDateString(),
                                         startTimestamp.getTimeString()));
        
        if (job.getLogTCStartStop())
        {
            // Log a testcase stop entry into the STAX Job Log
            job.log(STAXJob.JOB_LOG, "stop", "Testcase: " + fName +
                    ", ElapsedTime: " + elapsedTime);
        }

        synchronized(this)
        {

            if (fElapsedTime.equals("<Pending>"))
            {
                // First time testcase stopped - set commited elapsed time.
                fElapsedTime = elapsedTime;
            }
            else
            {
                // Add elapsed time for this instance of the testcase to
                // the committed elapsed time for the testcase.
                fElapsedTime = addElapsedTimes(fElapsedTime, elapsedTime, job);
            }
        }


        if (fNumPass == 0 && fNumFail == 0 && fLastMessage.equals("") &&
            fMode != STAXTestcase.STRICT_MODE)
        {
            // Don't generate an end event for a testcase whose mode isn't
            // strict and no status updates have been recorded.
        }
        else
        {
            // Generate a STAX Monitor testcase end event
            HashMap<String, String> testcaseMap =
                new HashMap<String, String>();
            testcaseMap.put("name", fName);
            testcaseMap.put("block", null);
            testcaseMap.put("status", "end");
            testcaseMap.put("status-pass", String.valueOf(fNumPass));
            testcaseMap.put("status-fail", String.valueOf(fNumFail));
            testcaseMap.put("laststatus", String.valueOf(fLastStatus));
            testcaseMap.put("elapsed-time", fElapsedTime);
            testcaseMap.put("num-starts", String.valueOf(fNumStarts));
            testcaseMap.put("startedTimestamp",
                        fStartedTimestamp.getTimestampString());

            job.generateEvent(STAXTestcaseActionFactory.STAX_TESTCASE,
                              testcaseMap);
        }
    }

    // Adds two elapsed times together and returns the total elapsed time
    // If one of the elapsed times is invalid, it's value will be
    // viewed as 00:00:00.

    private String addElapsedTimes(String time1, String time2, STAXJob job)
    {
        boolean invalidTime = false;

        int colon1Pos = time1.indexOf(":");
        int colon2Pos = 0;

        if (colon1Pos < 0)
        {
            // Should never happen

            job.log(STAXJob.JOB_LOG, "warning",
                    "STAXTestcase.addElapsedTimes() - " +
                    "Testcase: " + fName + "  Elapsed time: " +
                    time1 + "  Format not hh:mm:ss.");

            invalidTime = true;
        }
        
        if (!invalidTime)
        {
            colon2Pos = time1.indexOf(":", colon1Pos + 1);

            if (colon2Pos < 0)
            {
                // Should never happen

                job.log(STAXJob.JOB_LOG, "warning",
                        "STAXTestcase.addElapsedTimes() - " +
                        "Testcase: " + fName + "  Elapsed time: " +
                        time1 + "  Format not hh:mm:ss.");

                invalidTime = true;
            }
        }

        int hours1 = 0;
        int mins1 = 0;
        int secs1 = 0;

        if (!invalidTime)
        {
            try
            {
                hours1 = Integer.valueOf(
                    time1.substring(0, colon1Pos)).intValue();
                mins1 = Integer.valueOf(
                    time1.substring(colon1Pos + 1, colon2Pos)).intValue();
                secs1 = Integer.valueOf(
                    time1.substring(colon2Pos + 1)).intValue();
            }
            catch (NumberFormatException e)
            {
                // Should never happen

                job.log(STAXJob.JOB_LOG, "warning",
                        "STAXTestcase.addElapsedTimes() - Testcase: " +
                        fName + "  Elapsed time: " + time1 +
                        "  NumberFormatException: " + e.toString());
            }
        }

        invalidTime = false;

        colon1Pos = time2.indexOf(":");

        if (colon1Pos < 0)
        {
            // Should never happen

            job.log(STAXJob.JOB_LOG, "warning",
                    "STAXTestcase.addElapsedTimes() - " +
                    "Testcase: " + fName + "  Elapsed time: " +
                    time2 + "  Format not hh:mm:ss.");

            invalidTime = true;
        }

        if (!invalidTime)
        {
            colon2Pos = time2.indexOf(":", colon1Pos + 1);

            if (colon2Pos < 0)
            {
                // Should never happen

                job.log(STAXJob.JOB_LOG, "warning",
                        "STAXTestcase.addElapsedTimes() - " +
                        "Testcase: " + fName + "  Elapsed time: " +
                        time2 + "  Format not hh:mm:ss.");

                invalidTime = true;
            }
        }

        int hours2 = 0;
        int mins2 = 0;
        int secs2 = 0;

        if (!invalidTime)
        {
            try
            {
                hours2 = Integer.valueOf(
                    time2.substring(0, colon1Pos)).intValue();
                mins2 = Integer.valueOf(time2.substring(
                    colon1Pos + 1, colon2Pos)).intValue();
                secs2 = Integer.valueOf(
                    time2.substring(colon2Pos + 1)).intValue();
            }
            catch (NumberFormatException e)
            {
                // Should never happen

                job.log(STAXJob.JOB_LOG, "warning",
                        "STAXTestcase.addElapsedTimes() - Testcase: " +
                        fName + "  Elapsed time: " + time2 +
                        "  NumberFormatException: " + e.toString());
            }
        }

        int totalHours = 0;
        int totalMins = 0;
        int totalSecs = secs1 + secs2;

        if (totalSecs >= 60)
        {
            totalSecs = totalSecs - 60;
            totalMins = 1;
        }

        totalMins = totalMins + mins1 + mins2;
        if (totalMins >= 60)
        {
            totalMins = totalMins - 60;
            totalHours = 1;
        }

        totalHours = totalHours + hours1 + hours2;

        String totalElapsedTime = "";

        if (totalHours < 10)
            totalElapsedTime = "0";
        totalElapsedTime += String.valueOf(totalHours) + ":";

        if (totalMins < 10)
            totalElapsedTime += "0";
        totalElapsedTime += String.valueOf(totalMins) + ":";

        if (totalSecs < 10)
            totalElapsedTime += "0";
        totalElapsedTime += String.valueOf(totalSecs);

        return totalElapsedTime;
    }

    private String fName = new String();
    private List<String> fTestcaseStack = new ArrayList<String>();
    private int fNumPass = 0;
    private int fNumFail = 0;
    private String fLastStatus = new String();
    private STAXTimestamp fLastStatusTimestamp = null;
    private STAXTimestamp fStartedTimestamp = null;
    private String fLastMessage = new String();
    private String fElapsedTime = new String("<Pending>");
    private int fMode;  // Default or strict
    private int fNumStarts = 0;
    private int fNumStops = 0;
}
