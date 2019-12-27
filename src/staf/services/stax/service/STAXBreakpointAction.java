/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.Map;
//import java.util.List;
//import org.python.core.*;

public class STAXBreakpointAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;

    static final int COMPLETE = 1;

    public STAXBreakpointAction()
    { /* Do Nothing */ }

    public String getID() { return fID; }

    /*
    public TreeMap getLocals() { return fLocals; }

    public List getLongMarshalledList() { return fLongMarshalledList; }

    public List getShortMarshalledList() { return fShortMarshalledList; }
    */

    public boolean getIf() { return fIf; }

    public void setIf(String ifAttr)
    {
        fUnevalIf = ifAttr;
    }

    public String getXMLInfo()
    {
        String info = "<breakpoint";

        if (!fUnevalIf.equals("1"))
            info += " if=\"" + fUnevalIf + "\"";

        info += "/>";

        return info;
    }

    public String getInfo()
    {
        //return fID;
        return fInfo;
    }

    public void setInfo(String info)
    {
        fInfo = info;
    }

    public String getDetails()
    {
        return "Breakpoint: " + fID;
    }

    public STAXBreakpointActionFactory getActionFactory() { return fFactory; }

    public String getCurrentBlockName() { return fCurrentBlockName; }

    public void setActionFactory(STAXBreakpointActionFactory factory) 
    {
        fFactory = factory;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            fThread = thread;

            try
            {
                fCurrentBlockName = thread.pyStringEval("STAXCurrentBlock");
            }
            catch (STAXPythonEvaluationException e)
            {
                fCurrentBlockName = "";  //Shouldn't happen
            }

            //fID =
            //    String.valueOf(thread.getJob().getNextBreakpointNumber());
            fID = String.valueOf(thread.getThreadNumber());

            @SuppressWarnings("unchecked")
            TreeMap<String, STAXBreakpointAction> breakpointMap =
                (TreeMap<String, STAXBreakpointAction>)thread.getJob().getData(
                    "breakpointMap");

            synchronized (breakpointMap)
            {
                breakpointMap.put(fID, this);
            }

            HashMap<String, String> bpMap = new HashMap<String, String>();
            bpMap.put("type", "breakpoint");
            bpMap.put("block", fCurrentBlockName);
            bpMap.put("status", "start");
            bpMap.put("id", fID);
            bpMap.put("lineNumber", getLineNumber());
            bpMap.put("xmlFile", getXmlFile());
            bpMap.put("xmlMachine", getXmlMachine());

            StringBuffer actionInfo = new StringBuffer("");

            if ((getInfo() != null) && (getInfo().length() > 0))
            {
                actionInfo.append(getInfo());
            }
            else
            {
                actionInfo.append("breakpoint");
            }

            bpMap.put("info", actionInfo.toString());

            /*fLocals = (TreeMap)thread.getLocals();
            fLongMarshalledList = fFactory.createLongMarshalledList(fLocals);
            fShortMarshalledList = fFactory.createShortMarshalledList(fLocals);

            STAFMarshallingContext mc1 = new STAFMarshallingContext();
            mc1.setMapClassDefinition(fFactory.fBreakpointVariableMapClass);
            mc1.setRootObject(fLongMarshalledList);
            bpMap.put("variablesLong", mc1.marshall());

            STAFMarshallingContext mc2 = new STAFMarshallingContext();
            mc2.setMapClassDefinition(fFactory.fBreakpointVariableMapClass);
            mc2.setRootObject(fShortMarshalledList);
            bpMap.put("variablesShort", mc2.marshall());*/

            thread.getJob().generateEvent(
                STAXBreakpointActionFactory.BREAKPOINT, bpMap);

            fThread.addCondition(fHoldCondition);
        }
        else if (fState == COMPLETE)
        {
            TreeMap breakpointMap =
                (TreeMap)thread.getJob().getData("breakpointMap");

            synchronized (breakpointMap)
            {
                breakpointMap.remove(fID);
            }

            HashMap<String, String> bpMap = new HashMap<String, String>();
            bpMap.put("type", "breakpoint");
            bpMap.put("block", fCurrentBlockName);
            bpMap.put("status", "stop");
            bpMap.put("id", fID);
            bpMap.put("lineNumber", getLineNumber());
            bpMap.put("xmlFile", getXmlFile());
            bpMap.put("xmlMachine", getXmlMachine());

            thread.getJob().generateEvent(
                STAXBreakpointActionFactory.BREAKPOINT, bpMap);

            fThread.popAction();
        }
    }

    public void resumeBreakpoint()
    {
        fState = COMPLETE;

        fThread.setBreakpointCondition(false);
        fThread.setStepOverCondition(false);

        fThread.removeCondition(fHoldCondition);

        fThread.schedule();
    }

    public void stepIntoBreakpoint()
    {
        fState = COMPLETE;

        fThread.setBreakpointCondition(true);
        fThread.setStepOverCondition(false);

        fThread.removeCondition(fHoldCondition);

        fThread.schedule();
    }

    public void stepOverBreakpoint()
    {
        fState = COMPLETE;

        fThread.setStepOverCondition(true);
        fThread.setBreakpointCondition(false);

        fThread.removeCondition(fHoldCondition);

        fThread.schedule();
    }

    // Note that this entire method is synchronized since the state of the
    // action can be changed on another thread (via the requestComplete method).

    public synchronized void handleCondition(STAXThread thread,
        STAXCondition cond)
    {
        thread.removeCondition(fHoldCondition);

        HashMap<String, String> bpMap = new HashMap<String, String>();
        bpMap.put("type", "breakpoint");
        bpMap.put("block", fCurrentBlockName);
        bpMap.put("status", "stop");
        bpMap.put("id", fID);
        bpMap.put("lineNumber", getLineNumber());
        bpMap.put("xmlFile", getXmlFile());
        bpMap.put("xmlMachine", getXmlMachine());

        thread.getJob().generateEvent(
            STAXBreakpointActionFactory.BREAKPOINT, bpMap);

        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXBreakpointAction clone = new STAXBreakpointAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());
        clone.fFactory = fFactory;
        //clone.fLocals = fLocals;
        //clone.fLongMarshalledList = fLongMarshalledList;

        clone.fUnevalIf = fUnevalIf;
        clone.fIf = fIf;

        return clone;
    }

    int fState = INIT;
    private STAXThread fThread = null;
    private STAXBreakpointActionFactory fFactory;
    private String fID = new String();
    private String fInfo = new String();
    //private TreeMap fLocals;
    //private List fLongMarshalledList = null;
    //private List fShortMarshalledList = null;
    private String fUnevalIf = "1";
    private boolean fIf = true;
    private String fCurrentBlockName = new String();
    private STAXHoldThreadCondition fHoldCondition =
        new STAXHoldThreadCondition("Breakpoint");
}
