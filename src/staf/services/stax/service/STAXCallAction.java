/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Hashtable;
import org.python.core.*;

public class STAXCallAction extends STAXActionDefaultImpl
{
    // Ways arguments are passed on a Call
    public static final int CALL_ONE_ARG   = 1;  // <call> element
    public static final int CALL_LIST_ARGS = 2;  // <call-with-list> element
    public static final int CALL_MAP_ARGS  = 3;  // <call-with-map> element

    public STAXCallAction()
    { /* Do Nothing */ }

    public STAXCallAction(String name, String args)
    {
        fUnevalName = name;
        fName = name;
        fArgs = args;
    }

    public String getFunction() { return fName; } 
    public void setFunction(String function)
    {
        fUnevalName = function;
        fName = function;
    }

    public int getCallType() { return fCallType; } 
    public void setCallType(int callType)
    {
        fCallType = callType;
    }

    public String getArg() { return fArgs; } 
    public void setArgs(String args)
    {
        fArgs = args;
    }

    public ArrayList<String> getArgList() { return fArgList; } 
    public void addListArg(String arg)
    {
        fArgList.add(arg);
    }

    public HashMap<String, String> getArgMap() { return fArgMap; } 
    public void addMapArg(String key, String value)
    {
        fArgMap.put(key, value);
    }

    public String getXMLInfo()
    {
        StringBuffer result = new StringBuffer();

        if (fCallType == CALL_ONE_ARG)
        {
            if (fArgs == null)
            {
                result.append("<call function=\"").append(fUnevalName).
                    append("\"/>");
            }
            else
            {
                result.append("<call function=\"").append(fUnevalName).
                    append("\">").append(fArgs).append("</call>");
            }
        }
        else if (fCallType == CALL_LIST_ARGS)
        {
            if (fArgList.size() == 0)
            {
                result.append("<call-with-list function=\"").
                    append(fUnevalName).append("\"/>");
            }
            else
            {
                result.append("<call-with-list function=\"").
                    append(fUnevalName).append("\">\n");

                for (int i = 0; i < fArgList.size(); i++)
                {
                    if (fArgList.get(i) == null)
                    {
                        result.append("  <call-list-arg/>\n"); 
                    }
                    else
                    {    
                        result.append("  <call-list-arg>").
                            append(fArgList.get(i)).
                            append("</call-list-arg>\n");
                    }
                }

                result.append("</call-with-list>");
            }
        }
        else if (fCallType == CALL_MAP_ARGS)
        {
            if (fArgMap.size() == 0)
            {
                result.append("<call-with-map function=\"").
                    append(fUnevalName).append("\"/>");
            }
            else
            {
                result.append("<call-with-map function=\"").
                    append(fUnevalName).append("\">\n");

                for (Map.Entry<String, String> entry : fArgMap.entrySet())
                {
                    String key = entry.getKey();

                    if (key == null)
                    {
                        key = "";
                    }
                        
                    if (entry.getValue() == null)
                    {
                        result.append("  <call-map-arg name=\"").
                            append(key).append("\"/>\n");
                    }
                    else
                    {
                        result.append("  <call-map-arg name=\"").
                            append(key).append("\">").append(entry.getValue()).
                            append("</call-map-arg>\n");
                    }
                }

                result.append("</call-with-map>");
            }
        }

        return result.toString();
    }

    public String getInfo()
    {
        return fName;
    }

    public String getDetails()
    {
        StringBuffer result = new StringBuffer();
        
        result.append("Name:").append(fName).
            append(";CallType:").append(fCallType).append(";Arguments:");

        if (fCallType == CALL_ONE_ARG)
        {
            result.append(fArgs);
        }
        else if (fCallType == CALL_LIST_ARGS)
        {
            result.append(fArgList);
        }
        else if (fCallType == CALL_MAP_ARGS)
        {
            result.append(fArgMap);
        }

        return result.toString();
    }

    public void execute(STAXThread thread)
    {
        thread.popAction();

        // Evaluate the function name

        try
        {
            fName = thread.pyStringEval(fUnevalName);
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(
                getElement(), "function"));

            thread.setSignalMsgVar(
                "STAXPythonEvalMsg",
                STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");

            return;
        }

        // Verify function called is defined

        STAXAction function = thread.getJob().getFunction(fName);

        if (function == null)
        {
            setElementInfo(new STAXElementInfo(
                getElement(), "function",
                "Function does not exist: " + fName));
            
            thread.setSignalMsgVar(
                "STAXFunctionDoesNotExistMsg",
                STAXUtil.formatErrorMessage(this));

            thread.raiseSignal("STAXFunctionDoesNotExist");

            return;
        }

        // if fArgs is blank, set to null
        if (fArgs != null && fArgs.equals("")) fArgs = null;

        // Put the arguments into a Python Object

        String elemName = getElement();
        String attrName = STAXElementInfo.NO_ATTRIBUTE_NAME;
        int elemIndex = 0;

        PyObject args = null;
            
        try
        {
            if (fCallType == CALL_ONE_ARG)
            {
                // Evaluate fArgs data and put into a Python object

                if (fArgs != null)
                {
                    setElementInfo(new STAXElementInfo(
                        getElement(), STAXElementInfo.NO_ATTRIBUTE_NAME));

                    args = thread.pyObjectEval(fArgs);
                }
            } 

            else if (fCallType == CALL_LIST_ARGS)
            {
                // Evaluate fArgList data and put into a PyList object
                
                args = new PyList();
                String value;

                for (int i = 0; i < fArgList.size(); i++)
                {
                    if (getElement().equals("call-with-list"))
                    {
                        elemName = "call-list-arg";
                        elemIndex = i;
                    }

                    value = (String)fArgList.get(i);

                    // If value is null or blank, set to None
                    if (fArgList.get(i) == null || fArgList.get(i).equals(""))
                        value = "None";
                    
                    ((PyList)args).append(
                        Py.java2py(thread.pyObjectEval(value)));
                }
            }
            
            else if (fCallType == CALL_MAP_ARGS)
            {
                // Evaluate fArgMap data and put in a Python dictionary object
                
                Map<PyObject, PyObject> evalArgMap =
                    new HashMap<PyObject, PyObject>();
                String key;
                String value;
                
                for (Map.Entry<String, String> entry : fArgMap.entrySet())
                {
                    key = entry.getKey();
                    value = entry.getValue();

                    // If value or key is null or blank, set to None
                    if (key == null || key.equals("")) key = "None";
                    if (value == null || value.equals("")) value = "None";
                            
                    try
                    {
                        evalArgMap.put(Py.java2py(thread.pyObjectEval(key)),
                                       Py.java2py(thread.pyObjectEval(value)));
                    }
                    catch (STAXPythonEvaluationException ex)
                    {
                        throw ex;
                    }
                    catch (Exception ex)
                    {
                        // Can get "TypeError: unhashable type" if don't
                        // specify the key in Python

                        setElementInfo(new STAXElementInfo(
                            elemName, attrName, elemIndex,
                            "Invalid argument name: " + key + "\n\n" +
                            ex.toString()));

                        thread.setSignalMsgVar(
                            "STAXFunctionArgValidateMsg",
                            STAXUtil.formatErrorMessage(this));

                        thread.raiseSignal("STAXFunctionArgValidate");

                        return;
                    }
                }

                args = new PyDictionary(new Hashtable<PyObject, PyObject>
                                        (evalArgMap));
            }

            else      
            {   // Should never happen
                System.out.println("STAXCallAction: Invalid call type=" +
                                   fCallType);
                return;
            }
        }
        catch (STAXPythonEvaluationException e)
        {
            setElementInfo(new STAXElementInfo(elemName, attrName, elemIndex));
            
            thread.setSignalMsgVar(
                "STAXPythonEvalMsg",
                STAXUtil.formatErrorMessage(this), e);

            thread.raiseSignal("STAXPythonEvaluationError");

            return;
        }
        
        // Clone the function before setting arguments to avoid a race
        // condition

        STAXAction cloneFunction = function.cloneAction();

        ((STAXFunctionAction)cloneFunction).setCallArgs(args);

        // Needed to get information about the call element (e.g. line
        // number, etc) if an error occurs in STAXFunctionAction.java.
        ((STAXFunctionAction)cloneFunction).setCallAction(this);

        // Put call of function on the action stack

        thread.pushAction(cloneFunction);
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        thread.popAction();
    }

    public STAXAction cloneAction()
    {
        STAXCallAction clone = new STAXCallAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fUnevalName = fUnevalName;
        clone.fName = fName;
        clone.fArgs = fArgs;
        clone.fCallType = fCallType;
        //clone.fArgList = (ArrayList<String>)fArgList.clone();
        clone.fArgList = new ArrayList<String>(fArgList);
        //clone.fArgMap = (HashMap<String, String>)fArgMap.clone();
        clone.fArgMap = new HashMap<String, String>(fArgMap);

        return clone;
    }

    private String    fUnevalName;
    private String    fName;
    private String    fArgs = null;
    private int       fCallType = 1;
    private ArrayList<String> fArgList = new ArrayList<String>();
    private HashMap<String, String> fArgMap = new HashMap<String, String>();
}
