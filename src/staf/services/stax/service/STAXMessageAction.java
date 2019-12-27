/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.HashMap;

public class STAXMessageAction extends STAXActionDefaultImpl
{
    public STAXMessageAction()
    { /* Do Nothing */ }

    public STAXMessageAction(String messageValue)
    { 
        fUnevalMessageValue = messageValue;
        fMessageValue = messageValue;
    }

    public STAXMessageAction(String messageValue, String logAttr, String level,
                             String ifAttr)
    { 
        fUnevalMessageValue = messageValue;
        fMessageValue = messageValue;
        fUnevalLogAttr = logAttr;
        fUnevalLevel = level;
        fLevel = level;
        fUnevalIf = ifAttr;
    }

    public String getMessageValue() { return fMessageValue; } 

    public void setMessageValue(String messageValue) 
    {
        fUnevalMessageValue = messageValue;
        fMessageValue = messageValue; 
    }

    public boolean getLogAttr() { return fLogAttr; }
    public void setLogAttr(String logAttr)
    {
        fUnevalLogAttr = logAttr;
    }

    public String getLevel() { return fLevel; }
    public void setLevel(String level)
    { 
        fUnevalLevel = level;
        fLevel = level;
    }

    public boolean getIf() { return fIf; }
    public void setIf(String ifValue)
    { 
        fUnevalIf = ifValue;
    }

    public String getXMLInfo()
    {
        StringBuffer info = new StringBuffer("<message");

        if (!fUnevalLogAttr.equals("STAXLogMessage"))
            info.append(" log=\"").append(fUnevalLogAttr).append("\"");
        if (!fUnevalLevel.equals("'info'"))
            info.append(" level=\"").append(fUnevalLevel).append("\"");
        if (!fUnevalIf.equals("1"))
            info.append(" if=\"").append(fUnevalIf).append("\"");
        info.append(">").append(STAFUtil.maskPrivateData(fUnevalMessageValue)).
            append("</message>");
        
        return info.toString();
    }
    
    public String getInfo()
    {
        String info = STAFUtil.maskPrivateData(fMessageValue);

        int msgLength = info.length();

        if (msgLength > 40)
            return info.substring(0, 40) + "...";
        else
            return info;
    }

    public String getDetails()
    {
        return "MessageValue:" + STAFUtil.maskPrivateData(fMessageValue) +
               ";LogAttr:" + fLogAttr +
               ";LogLevel:" + fLevel +
               ";If: " + fIf;
    }

    public void execute(STAXThread thread)
    {         
        fThread = thread;
        String evalElem = getElement();
        String evalAttr = "if";

        try
        {
            fIf = fThread.pyBoolEval(fUnevalIf);

            if (!fIf)
            {   // Ignore message element if "if" attribute evaluates to FALSE
                fThread.popAction();
                return;
            }
            
            evalAttr = STAXElementInfo.NO_ATTRIBUTE_NAME;
            fMessageValue = fThread.pyStringEval(fUnevalMessageValue);

            evalAttr = "log";
            fLogAttr = fThread.pyBoolEval(fUnevalLogAttr);

            evalAttr = "level";
            fLevel = fThread.pyStringEval(fUnevalLevel);
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

        // Send a message to the STAXMonitor (via an event)

        STAXTimestamp timestamp = new STAXTimestamp();

        HashMap<String, String> messageMap = new HashMap<String, String>();
        messageMap.put("messagetext", timestamp.getTimestampString() + 
            " " + STAFUtil.maskPrivateData(fMessageValue));
        
        fThread.getJob().generateEvent(
            STAXMessageActionFactory.STAX_MESSAGE, messageMap);

        if (fLogAttr)
        {
            // Log the message in the STAX Job User log

            STAFResult result = fThread.getJob().log(
                STAXJob.USER_JOB_LOG, fLevel, fMessageValue);

            if ((result.rc != STAFResult.Ok) &&
                (result.rc != STAFResult.UnknownService))
            {
                fThread.popAction();

                String msg = "Request to LOG service to log to the STAX " +
                    "Job User log failed with RC: " + result.rc +
                    " Result: " + result.result;

                if (result.rc == 4004)
                    msg += "\n\nInvalid log level: " + fLevel;
                else
                    msg += "\n\nLevel: " + fLevel +
                        "  Message: " + fMessageValue;

                setElementInfo(new STAXElementInfo(
                    evalElem, STAXElementInfo.NO_ATTRIBUTE_NAME, msg));

                fThread.setSignalMsgVar(
                    "STAXLogMsg", STAXUtil.formatErrorMessage(this));

                fThread.raiseSignal("STAXLogError");

                return;
            } 
        }

        fThread.popAction();
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXMessageAction clone = new STAXMessageAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalMessageValue = fUnevalMessageValue;
        clone.fMessageValue = fMessageValue;
        clone.fUnevalLogAttr = fUnevalLogAttr;
        clone.fLogAttr = fLogAttr;
        clone.fUnevalLevel = fUnevalLevel;
        clone.fLevel = fLevel;
        clone.fUnevalIf = fUnevalIf;
        clone.fIf = fIf;

        return clone;
    }   

    private STAXThread fThread = null;
    private String fUnevalMessageValue = new String();
    private String fUnevalLogAttr = "STAXLogMessage";
    private String fUnevalLevel = new String("'info'");
    private String fUnevalIf = "1";

    private String fMessageValue = new String();
    private boolean fLogAttr = false;
    private String fLevel = new String();
    private boolean fIf = true;
  }
