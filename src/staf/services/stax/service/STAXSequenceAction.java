/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.ArrayList;

public class STAXSequenceAction extends STAXActionDefaultImpl
{
    static final int CALLING_ACTIONS = 0;
    static final int COMPLETE = 1;

    static final String CALLING_ACTIONS_STRING = "CALLING_ACTIONS";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";


    public STAXSequenceAction()
    { /* Do Nothing */ }

    public STAXSequenceAction(ArrayList<STAXAction> actionList)
    {
        fActionList = actionList;
    }

    public void setActionList(ArrayList<STAXAction> actionList)
    {
        fActionList = actionList;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case CALLING_ACTIONS:
                return CALLING_ACTIONS_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo()
    {
        return fCurrActionIndex + "/" + fActionList.size();
    }

    public String getDetails()
    {
        return "State:" + getStateAsString() +
               ";ActionListSize:" + fActionList.size() + 
               ";CurrActionIndex:" + fCurrActionIndex +
               ";ActionList:" + fActionList;

    }

    public void execute(STAXThread thread)
    {
        if (fState == CALLING_ACTIONS)
        {
            if (fActionList.size() > fCurrActionIndex)
            {
                STAXAction currAction = fActionList.get(
                    fCurrActionIndex++);
                thread.pushAction(currAction.cloneAction());
            }
            else
            {
                fState = COMPLETE;
                thread.popAction();
            }
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXSequenceAction clone = new STAXSequenceAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fActionList = fActionList;

        return clone;
    }

    private int fState = CALLING_ACTIONS;
    private int fCurrActionIndex = 0;
    private ArrayList<STAXAction> fActionList = new ArrayList<STAXAction>();
}
