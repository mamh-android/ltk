/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.Map;
import java.util.HashMap;

public class STAXJobCompleteNotifiee implements STAXJobCompleteListener
{
    STAXJobCompleteNotifiee(int notifyOnEnd, String endpoint,
                            int handle, String handleName, int priority,
                            String key)
    {
        fNotifyOnEnd = notifyOnEnd;
        fEndpoint = endpoint;
        fHandle = handle;
        fHandleName = handleName;
        fPriority = priority;
        fKey = key;
    }

    public void jobComplete(STAXJob job)
    {
        // Create a job completion notification message

        STAFMarshallingContext mc = new STAFMarshallingContext();

        Map<String, Object> messageMap = new HashMap<String, Object>();

        messageMap.put("staxServiceName", job.getSTAX().getServiceName());
        messageMap.put("jobID", String.valueOf(job.getJobNumber()));
        messageMap.put("startTimestamp",
                       job.getStartTimestamp().getTimestampString());
        messageMap.put("endTimestamp",
                       job.getEndTimestamp().getTimestampString());
        messageMap.put("key", fKey);
        messageMap.put("status", job.getCompletionStatusAsString());
        messageMap.put("result", job.getResult().toString());

        // Remove the staf-map-class-name key since not using a map-class
        // for the testcase totals map in the message

        Map<String, String> testcaseTotalsMap = job.getTestcaseTotalsMap();
        testcaseTotalsMap.remove("staf-map-class-name");
        messageMap.put("testcaseTotals", testcaseTotalsMap);

        mc.setRootObject(messageMap);
        
        // Send the job completion message to the notifiee

        String queueRequest = "QUEUE";

        if (fNotifyOnEnd == STAXJob.NOTIFY_ONEND_BY_HANDLE)
            queueRequest += " HANDLE " + fHandle; 
        else
            queueRequest += " NAME " + STAFUtil.wrapData(fHandleName);

        if (fPriority != 5)
            queueRequest += " PRIORITY " + fPriority;

        queueRequest += " TYPE " + STAX.sQueueTypeJobEnd +
            " MESSAGE " + STAFUtil.wrapData(mc.marshall());

        job.submitAsyncForget(fEndpoint, "QUEUE", queueRequest);
    }

    public String getNotifyByString()
    {
        if (fNotifyOnEnd == STAXJob.NOTIFY_ONEND_BY_HANDLE)
            return "Handle";
        else
            return "Name";
    }
    public String getMachine() { return fEndpoint; }
    public int getHandle() { return fHandle; }
    public String getHandleName() { return fHandleName; }
    public int getPriority() { return fPriority; }
    public String getKey() { return fKey; }

    private int    fNotifyOnEnd;
    private String fEndpoint;
    private int    fHandle;
    private String fHandleName;
    private int    fPriority;
    private String fKey;
}
