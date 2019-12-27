/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.*;
import java.text.*;

public class STAXLogAction extends STAXActionDefaultImpl
{
    public STAXLogAction()
    { /* Do Nothing */ }

    public STAXLogAction(String message, String level, int logfile)
    { 
        fUnevalMessage = message;
        fMessage = message;
        fUnevalLevel = level;
        fLevel = level;
        fLogfile = logfile;
    }

    public STAXLogAction(String message, String level, String messageAttr,
                         String ifAttr, int logfile)
    { 
        fUnevalMessage = message;
        fMessage = message;
        fUnevalLevel = level;
        fLevel = level;
        fUnevalMessageAttr = messageAttr;
        fUnevalIf = ifAttr;
        fLogfile = logfile;
    }

    public String getMessage() { return fMessage; } 
    public void setMessage(String message)
    {
        fUnevalMessage = message;
        fMessage = message;
    }

    public String getLevel() { return fLevel; }
    public void setLevel(String level)
    { 
        fUnevalLevel = level;
        fLevel = level;
    }

    public boolean getMessageAttr() { return fMessageAttr; }
    public void setMessageAttr(String messageAttr)
    {
        fUnevalMessageAttr = messageAttr;
    }

    public boolean getIf() { return fIf; }
    public void setIf(String ifValue)
    { 
        fUnevalIf = ifValue;
    }

    public int getLogfile() { return fLogfile; } 
    public void setLogfile(int logfile) { fLogfile = logfile; }

    public String getXMLInfo()
    {
        StringBuffer info = new StringBuffer("<log");

        if (!fUnevalLevel.equals("'info'"))
            info.append(" level=\"").append(fUnevalLevel).append("\"");
        if (!fUnevalMessageAttr.equals("STAXMessageLog"))
            info.append(" message=\"").append(fUnevalMessageAttr).append("\"");
        if (!fUnevalIf.equals("1"))
            info.append(" if=\"").append(fUnevalIf).append("\"");
        info.append(">").append(fUnevalMessage).append("</log>");

        return info.toString();
    } 

    public String getInfo()
    {
        int msgLength = fMessage.length();
        if (msgLength > 40)
            return fMessage.substring(0, 40) + "...";
        else
            return fMessage;
    } 

    public String getDetails()
    {
        return "Level:" + fLevel +
               ";Message:" + fMessage +
               ";MessageAttr:" + fMessageAttr +
               ";If:" + fIf +
               ";Logfile:" + fLogfile;
    } 

    public void execute(STAXThread thread)
    {
        fThread = thread;
        String evalElem = getElement();
        String evalAttr = "if";
        
        try
        {
            fIf = thread.pyBoolEval(fUnevalIf);
            
            if (!fIf)
            {   // Ignore log element if "if" attribute evaluates to FALSE
                fThread.popAction();
                return;
            }

            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
            fMessage = thread.pyStringEval(fUnevalMessage);

            evalAttr = "level";
            fLevel = thread.pyStringEval(fUnevalLevel);

            evalAttr = "message";
            fMessageAttr = thread.pyBoolEval(fUnevalMessageAttr);
        }
        catch (STAXPythonEvaluationException e)
        {
            fThread.popAction();

            setElementInfo(new STAXElementInfo(evalElem, evalAttr));

            fThread.setSignalMsgVar(
                "STAXPythonEvalMsg", STAXUtil.formatErrorMessage(this), e);

            fThread.raiseSignal("STAXPythonEvaluationError");
            return;
        }
        
        if (fMessageAttr)
        {
            // Send a message to the STAXMonitor (via an event)

            STAXTimestamp timestamp = new STAXTimestamp();

            HashMap<String, String> messageMap = new HashMap<String, String>();
            messageMap.put("messagetext", timestamp.getTimestampString() + 
                " " + STAFUtil.maskPrivateData(fMessage));

            fThread.getJob().generateEvent(
                STAXMessageActionFactory.STAX_MESSAGE, messageMap);
        }

        // Log the message

        STAFResult result = fThread.getJob().log(fLogfile, fLevel, fMessage);

        if (result.rc != 0 && result.rc != 2)
        {
            // Raise a STAXLogError signal

            fThread.popAction();

            String logFileName = "STAX Job User Log";

            if (fLogfile == STAXJob.JOB_LOG)
            {
                logFileName = "STAX Job Log";
            }
            else if (fLogfile == STAXJob.SERVICE_LOG)
            {
                logFileName = "STAX Service Log";
            }

            String msg = "Request to LOG service to log to the " +
                logFileName + " failed with RC=" + result.rc +
                " Result=" + result.result;

            if (result.rc == 4004)
                msg += "\n\nInvalid log level: " + fLevel;
            else
                msg += "\n\nLevel: " + fLevel + "  Message: " + fMessage;

            setElementInfo(new STAXElementInfo(
                evalElem, STAXElementInfo.NO_ATTRIBUTE_NAME, msg));

            fThread.setSignalMsgVar(
                "STAXLogMsg", STAXUtil.formatErrorMessage(this));
            
            fThread.raiseSignal("STAXLogError");

            return;
        } 
         
        fThread.popAction();
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXLogAction clone = new STAXLogAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalMessage = fUnevalMessage;
        clone.fMessage = fMessage;
        clone.fUnevalLevel = fUnevalLevel;
        clone.fLevel = fLevel;
        clone.fUnevalMessageAttr = fUnevalMessageAttr;
        clone.fMessageAttr = fMessageAttr;
        clone.fUnevalIf = fUnevalIf;
        clone.fIf = fIf;
        clone.fLogfile = fLogfile;

        return clone;
    }   

    STAXThread fThread = null;

    private String fUnevalMessage = new String();
    private String fUnevalLevel = new String("'info'");
    private String fUnevalMessageAttr = "STAXMessageLog";
    private String fUnevalIf = "1";
    private String fMessage = new String();
    private String fLevel = new String();
    private boolean fMessageAttr = false;
    private boolean fIf = true;
    private int fLogfile;
}
