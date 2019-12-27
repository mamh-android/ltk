/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXSignalExecutionAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int PERFORMED_ACTION = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String PERFORMED_ACTION_STRING = "PERFORMED_ACTION";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    private STAXSignalExecutionAction()
    { /* Do Nothing */ }

    public STAXSignalExecutionAction(String name, STAXAction action)
    {
        fName = name;
        fAction = action;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case PERFORMED_ACTION:
                return PERFORMED_ACTION_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo()
    {
        return fName;
    }

    public String getDetails()
    {
        return "Name:" + fName + 
               ";Action:" + fAction + 
               ";State:" + getStateAsString();
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            thread.pushAction(fAction.cloneAction());
            fState = PERFORMED_ACTION;
        }
        else if (fState == PERFORMED_ACTION)
        {
            fState = COMPLETE;
            thread.handledSignal(fName);
            thread.popAction();
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        if (fState != COMPLETE)
            thread.handledSignal(fName);

        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXSignalExecutionAction clone = new STAXSignalExecutionAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fName = fName;
        clone.fAction = fAction;

        return clone;
    }

    private int fState = INIT;
    private String fName;
    private STAXAction fAction;
}
