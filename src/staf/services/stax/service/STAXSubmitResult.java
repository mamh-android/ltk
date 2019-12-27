/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.Map;
import java.util.TreeMap;

// Create a new class STAXSubmitResult which is used to store the
// result from a STAX EXECUTE request (which consists of the STAFResult and
// the STAXJob objects).


public class STAXSubmitResult
{
     public STAXSubmitResult()
     { /* Do Nothing */ }
 
 
     public STAXSubmitResult(STAFResult result)
     {
         fResult = result;
     }
     public STAXSubmitResult(STAFResult result, STAXJob job)
     {
         fResult = result;
         fJob = job;

         // Check if an error occurred submitting the STAX EXECUTE request
         // and if the job is in a pending state with a job number assigned

         if ((fResult.rc != STAFResult.Ok) && 
             (fJob != null) &&
             (fJob.getState() == STAXJob.PENDING_STATE) &&
             (fJob.getJobNumber() != 0))
         {
             // Change fResult.result so that it is a marshalled map
             // containing the job ID and the error message
             
             STAFMarshallingContext mc = new STAFMarshallingContext();
             mc.setMapClassDefinition(STAX.fExecuteErrorResultMapClass);

             Map<String, Object> resultMap = new TreeMap<String, Object>();
             resultMap.put("staf-map-class-name",
                           STAX.fExecuteErrorResultMapClass.name());
             resultMap.put("jobID", String.valueOf(job.getJobNumber()));
             resultMap.put("errorMsg", fResult.result);
             mc.setRootObject(resultMap);
             fResult.result = mc.marshall();
         }
     }

     public STAFResult getResult() { return fResult; }
     public void setResult(STAFResult result) { fResult = result; }
    
     public STAXJob getJob() { return fJob; }
     public void setJob(STAXJob job) { fJob = job; }
    
     private STAFResult fResult;
     private STAXJob fJob = null;
}
