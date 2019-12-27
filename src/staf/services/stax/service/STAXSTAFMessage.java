/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFMarshallingContext;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class STAXSTAFMessage
{
    public STAXSTAFMessage(Map queueMap) throws Exception
    {
        fQueueMap = queueMap;

        try
        {
            fHandle = Integer.parseInt((String)queueMap.get("handle"));
        }
        catch (NumberFormatException e)
        {
            // XXX: Do what?
            System.out.println("STAXSTAFMessage: Handle not an integer." +
                               " Handle=" + queueMap.get("handle"));
        }
        
        fMachine = (String)queueMap.get("machine");
        fType    = (String)queueMap.get("type");
        fMessage = (String)queueMap.get("message");

        /* Debugging
        System.out.println("Handle : " + fHandle + ", Machine: " + fMachine +
                           ", Type: " + fType + ", Message: " + fMessage);
        */

        STAFMarshallingContext mc;

        try
        {
            // Queued messages with a non-null or blank type are from 3.x
            // clients

            if (fType != null &&
                fType.equalsIgnoreCase("STAF/RequestComplete"))
            {
                // A STAF/RequestComplete message is a map containing keys:
                //   requestNumber, rc, result

                // Unmarshall the message, but ignore indirect objects so
                // that the "result" field will be a string.

                mc = STAFMarshallingContext.unmarshall(
                    fMessage, STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS);

                Map messageMap = (Map)mc.getRootObject();
                
                fRequestResult = (String)messageMap.get("result");

                try
                {
                    // Convert request number to a negative integer if
                    // greater than Integer.MAX_VALUE

                    fRequestNumber = (STAXUtil.convertRequestNumber(
                        (String)messageMap.get("requestNumber"))).intValue();

                    fRequestRC = Integer.parseInt(
                        (String)messageMap.get("rc"));
                }
                catch (NumberFormatException e)
                {
                    System.out.println(
                        "STAXSTAFMessage: fRequestNumber/fRequestRC - " +
                        e.toString());
                }
                /* Debugging
                System.out.println("Request#: " + fRequestNumber +
                                   ", Request RC: " + fRequestRC + 
                                   ", Request Result: " + fRequestResult);
                */
            } 
            else if (fType != null &&
                     fType.equalsIgnoreCase("STAF/Process/End"))
            {
                // A STAF/Process/End message is a map containing keys:
                //   handle, endTimestamp, rc, key, fileList
                // where each entry in fileList is a map containing keys:
                //   rc, data

                // Unmarshall the message, but ignore indirect objects so
                // that the file "data" field in each fileList entry will
                // be a string

                mc = STAFMarshallingContext.unmarshall(
                    fMessage, STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS);
            
                Map messageMap = (Map)mc.getRootObject();
                fProcessTimestamp = (String)messageMap.get("endTimestamp");
                fProcessKey = (String)messageMap.get("key");
                fProcessFileList = (List)messageMap.get("fileList");

                try
                {
                    fProcessHandle = Integer.parseInt(
                        (String)messageMap.get("handle"));
                    fProcessRC = Long.parseLong((String)messageMap.get("rc"));
                }
                catch (NumberFormatException e)
                {
                    System.out.println("STAXSTAFMessage: fProcessHandle/" +
                                       "fProcessRC - " + e.toString());
                }
                /* Debugging
                System.out.println("Process handle: " + fProcessHandle +
                                   ", key: " + fProcessKey +
                                   ", timestamp: " + fProcessTimestamp);
                System.out.println("Process RC       : " + fProcessRC);
                System.out.println("Process fileList : " + fProcessFileList);
                */
            }
            else if (fType == null)
            {
                handleSTAF2xMessage();
            }
        }
        catch (Exception e)
        {
            String msg = "Exception unmarshalling a message with type '" +
                fType + "' from handle " + fHandle + " on machine " +
                fMachine + " \nMessage: " + fMessage;

            throw new Exception(msg, e);
        }
    }

    private void handleSTAF2xMessage()
    {
        // Break up the 2.x message from a QUEUE GET request

        if (fMessage.startsWith("STAF/RequestComplete"))
        {
            int requestPos = fMessage.indexOf(';');
            int rcPos = fMessage.indexOf(';', requestPos + 1);

            fRequestResult = fMessage.substring(rcPos + 1);

            try
            {
                // Convert request number to a negative integer if greater
                // then Integer.MAX_VALUE
                // Note: 21 = length of "STAF/RequestComplete "

                fRequestNumber = (STAXUtil.convertRequestNumber(
                        fMessage.substring(21, requestPos))).intValue();
                
                fRequestRC = Integer.parseInt(
                    fMessage.substring(requestPos + 1, rcPos));
            }
            catch (NumberFormatException e)
            {
                System.out.println(
                    "STAXSTAFMessage: fRequestNumber/fRequestRC - " +
                    e.toString());
            }

            /* Debugging
            System.out.println("Request number: " + fRequestNumber);
            System.out.println("Request rc    : " + fRequestRC);
            System.out.println("Request result: " + fRequestResult);
            */
        } 
        else if (fMessage.startsWith("STAF/PROCESS/END"))
        {
            int processHandlePos = fMessage.indexOf(';');
            int processTimestampPos = fMessage.indexOf(
                ';', processHandlePos + 1);
            fProcessTimestamp = fMessage.substring(
                processHandlePos + 1, processTimestampPos);

            try
            {
                // Note: 17 = length of "STAF/PROCESS/END "
                fProcessHandle = Integer.parseInt(fMessage.substring(17,
                                                  processHandlePos));

                String processRCAndResult = 
                    fMessage.substring(processTimestampPos + 1);

                int processReturnFilesIndex = processRCAndResult.indexOf(";");

                if (processReturnFilesIndex == -1)
                {
                    fProcessRC = Long.parseLong(processRCAndResult);
                }
                else
                {
                    fProcessRC = Long.parseLong(processRCAndResult.
                        substring(0, processReturnFilesIndex));

                    fProcessResult = processRCAndResult.substring(
                        processReturnFilesIndex + 1);

                    if (fProcessResult.startsWith("KEY="))
                    {
                        int keyIndex = fProcessResult.indexOf(";");

                        if (keyIndex == -1)
                        {
                            // Note: 4 = length of "KEY="
                            fProcessKey = fProcessResult.substring(4);
                            fProcessResult = "";
                        }
                        else
                        {
                            fProcessKey = fProcessResult.substring(
                                4, keyIndex);

                            fProcessResult = fProcessResult.
                                substring(keyIndex + 1);
                        }
                    }

                    // Create fProcessFileList from string fProcessResult
                    if (fProcessResult == null || fProcessResult.equals(""))
                    {
                        fProcessFileList = null;
                    }
                    else
                    {
                        // Convert fProcessResult to a Java List now that
                        // STAF V3.x clients are returning a list instead of
                        // a string containing the returned file information

                        int countIndex = fProcessResult.indexOf(";");
                        int count = (new Integer(
                            fProcessResult.substring(0, countIndex))).
                            intValue();

                        if (count == 0)
                        {
                            fProcessFileList = null;
                        }
                        else
                        {
                            // Create a Map containing the file rc and
                            // data for each returned file and add each
                            // map to a list:
                            //   [ { 'rc': File1RC, 'data': File1Data },
                            //     { 'rc': File2RC, 'data': File2Data },
                            //     ... ] 

                            List<Map<String, Object>> resultList =
                                new ArrayList<Map<String, Object>>();
                            int startIndex = countIndex + 1;
                            int i = 0;
                            int rcIndex = 0;
                            int lengthIndex = 0;
                            int length = 0;

                            try
                            {
                                for (; i < count; i++)
                                {
                                    // Create a Map to contain file info
                                    // (RC & data)
                                    Map<String, Object> fileInfoMap =
                                        new HashMap<String, Object>();

                                    // Assign file RC
                                    rcIndex = fProcessResult.indexOf(
                                        ":", startIndex + 1);

                                    String fileRC = fProcessResult.substring(
                                        startIndex, rcIndex);

                                    fileInfoMap.put("rc", fileRC);

                                    // Get length of file data
                                    lengthIndex = fProcessResult.indexOf(
                                        ":", rcIndex + 1);
                                    length = Integer.parseInt(
                                        fProcessResult.substring(
                                            rcIndex + 1, lengthIndex));

                                    // Assign file data
                                    startIndex = lengthIndex + 1 + length;
                                    fileInfoMap.put(
                                        "data", fProcessResult.substring(
                                            lengthIndex + 1, startIndex));

                                    // Add file info map to result List
                                    resultList.add(fileInfoMap);
                                }
                            }
                            catch (Exception ex)
                            {
                                // This should never happen

                                System.out.println(
                                    "STAXSTAFMessage: Error converting " +
                                    "string containing returned file " +
                                    "information to a list.\n" +
                                    "result=" + fProcessResult + "\n" +
                                    "msg=" + this.toString() + "\n" +
                                    ex.toString());
                                ex.printStackTrace();
                            }

                            fProcessFileList = resultList;
                        }
                    }
                }
            }
            catch (NumberFormatException e)
            {
                System.out.println("STAXSTAFMessage: fProcessRC - " +
                                   e.toString());
            }

            /* Debugging
            System.out.println("Process handle   : " + fProcessHandle);
            System.out.println("Process rc       : " + fProcessRC);
            System.out.println("Process timestamp: " + fProcessTimestamp);
            System.out.println("Process fileList : " + fProcessFileList);
            */
        }
    }

    public String toString()
    {
        return "machine=" + fMachine + ", handle=" + fHandle + ", type=" +
            fType + ", message=" + fMessage;
    }

    public String getResult()
    {
        try
        {
            STAFMarshallingContext resultMC = new STAFMarshallingContext();
            resultMC.setRootObject(fQueueMap);
            return resultMC.marshall();
        }
        catch (Exception e)
        {
            return "{" +
                  "\n  Machine: " + fMachine +
                  "\n  Handle : " + fHandle +
                  "\n  Type   : " + fType +
                  "\n  Message: " + fMessage +
                  "\n}";
        }
    }

    public int getHandle() { return fHandle; }
    public String getMachine() { return fMachine; }
    public String getType()    { return fType; }
    public String getMessage() { return fMessage; }
    
    public int getRequestNumber() { return fRequestNumber; }
    public int getRequestRC() { return fRequestRC; }
    public String getRequestResult() { return fRequestResult; }

    public int getProcessHandle() { return fProcessHandle; }
    public long getProcessRC() { return fProcessRC; }
    public String getProcessKey() { return fProcessKey; }
    public String getProcessTimestamp() { return fProcessTimestamp; }
    public String getProcessResult() { return fProcessResult; }
    
    public List getProcessResultAsList()
    {
        return fProcessFileList;
    }
    
    Map fQueueMap = new HashMap();

    // Common STAF Message Result fields

    int fHandle = 0;
    String fMachine;
    String fType;
    String fMessage;

    // STAF/RequestComplete message prefix fields

    int fRequestNumber = 0;
    int fRequestRC = 0;
    String fRequestResult;
    
    // STAF/Process/End message prefix fields

    int fProcessHandle = 0;
    long fProcessRC = 0;
    String fProcessKey = "";
    String fProcessTimestamp;
    String fProcessResult = "";
    List fProcessFileList = new ArrayList();
}