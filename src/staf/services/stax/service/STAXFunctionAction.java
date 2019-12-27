/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.List;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.LinkedList;
import java.util.Hashtable;
import java.util.Map;
import java.util.TreeMap;
import java.util.Iterator;
import org.python.util.PythonInterpreter;
import org.python.core.*;

public class STAXFunctionAction extends STAXActionDefaultImpl
{
    static final int INIT = 0;
    static final int FUNCTION_CALLED = 1;
    static final int COMPLETE = 2;

    static final String INIT_STRING = "INIT";
    static final String FUNCTION_CALLED_STRING = "FUNCTION_CALLED";
    static final String COMPLETE_STRING = "COMPLETE";
    static final String STATE_UNKNOWN_STRING = "UNKNOWN";
    
    // Function Argument Definitions
    public static final int FUNCTION_DEFINES_NO_ARGS   = 0;
    public static final int FUNCTION_ALLOWS_NO_ARGS    = 1;
    public static final int FUNCTION_DEFINES_ONE_ARG   = 2;
    public static final int FUNCTION_DEFINES_LIST_ARGS = 3;
    public static final int FUNCTION_DEFINES_MAP_ARGS  = 4;
    
    public static final String FUNCTION_DEFINES_NO_ARGS_STRING   = 
        "FUNCTION_DEFINES_NO_ARGS";
    public static final String FUNCTION_ALLOWS_NO_ARGS_STRING    = 
        "FUNCTION_ALLOWS_NO_ARGS";
    public static final String FUNCTION_DEFINES_ONE_ARG_STRING   = 
        "FUNCTION_DEFINES_ONE_ARG";
    public static final String FUNCTION_DEFINES_LIST_ARGS_STRING = 
        "FUNCTION_DEFINES_LIST_ARGS";
    public static final String FUNCTION_DEFINES_MAP_ARGS_STRING  = 
        "FUNCTION_DEFINES_MAP_ARGS";
    public static final String FUNCTION_DEFINES_UNKNOWN_ARGS_STRING =
        "FUNCTION_DEFINES_UNKNOWN_ARGS";
    
    // Types of Function Arguments
    public static final int ARG_REQUIRED = 1;
    public static final int ARG_OPTIONAL = 2;
    public static final int ARG_OTHER    = 3;

    public static final String ARG_REQUIRED_STRING = "ARG_REQUIRED";
    public static final String ARG_OPTIONAL_STRING = "ARG_OPTIONAL";
    public static final String ARG_OTHER_STRING    = "ARG_OTHER";

    public STAXFunctionAction()
    { /* Do Nothing */ }

    public STAXFunctionAction(String name, String prolog, String epilog, 
                              String scope, String requires, STAXAction action)
    {
        fName = name;
        fProlog = prolog;
        fEpilog = epilog;
        fScope = scope;
        fRequires = requires;
        fAction = action;
    }

    public String getName() { return fName; }
    public void setName(String name) { fName = name; }
    
    public String getProlog() { return fProlog; }
    public void setProlog(String prolog) { fProlog = prolog; }
    
    public String getEpilog() { return fEpilog; }
    public void setEpilog(String epilog) { fEpilog = epilog; }

    public String getScope() { return fScope; }
    public void setScope(String scope) { fScope = scope; }

    public STAXAction getAction() { return fAction; }
    public void setAction(STAXAction action) { fAction = action; }

    public int  getArgDefinition() { return fArgDefinition; }
    public void setArgDefinition(int def) { fArgDefinition = def; }

    public boolean getDefinedWithFunctionArgDef()
    {
        return fDefinedWithFunctionArgDef;
    }
    public void setDefinedWithFunctionArg(boolean flag)
    {
        fDefinedWithFunctionArgDef = flag;
    }

    public String getFunctionArgElementName(int argType)
    {
        String element = "";

        if (fDefinedWithFunctionArgDef)
            element = "function-arg-def";
        else
        {
            if (argType == ARG_REQUIRED)
                element = "function-required-arg";
            else if (argType == ARG_OPTIONAL)
                element = "function-optional-arg";
            else if (argType == ARG_OTHER)
                element = "function-other-args";
        }

        return element;
    }

    public ArrayList<STAXFunctionArgument> getArgList() { return fArgList; }
    public void addArg(STAXFunctionArgument arg) { fArgList.add(arg); }
    
    public PyObject getCallArgs() { return fCallArgPyObject; }
    public void setCallArgs(PyObject args) { fCallArgPyObject = args; }

    public void setCallAction(STAXCallAction action) { fCallAction = action; }
    
    public String getRequires() { return fRequires; }
    public void setRequires(String requires) { fRequires = requires; }
    
    public List<STAXFunctionImport> getImportList() { return fImportList; }

    public void addToImportList(
        String file, String directory, String machine, String functions)
    {
        fImportList.add(
            new STAXFunctionImport(file, directory, machine, functions));
    }
    
    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case FUNCTION_CALLED:
                return FUNCTION_CALLED_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getArgDefinitionAsString()
    {
        switch (fArgDefinition)
        {
            case FUNCTION_DEFINES_NO_ARGS:
                return FUNCTION_DEFINES_NO_ARGS_STRING;
            case FUNCTION_ALLOWS_NO_ARGS:
                return FUNCTION_ALLOWS_NO_ARGS_STRING;
            case FUNCTION_DEFINES_ONE_ARG:
                return FUNCTION_DEFINES_ONE_ARG_STRING;
            case FUNCTION_DEFINES_LIST_ARGS:
                return FUNCTION_DEFINES_LIST_ARGS_STRING;
            case FUNCTION_DEFINES_MAP_ARGS:
                return FUNCTION_DEFINES_MAP_ARGS_STRING;
            default:
                return FUNCTION_DEFINES_UNKNOWN_ARGS_STRING;
        }
    }

    public String getArgTypeAsString(int type)
    {
        switch (type)
        {
            case ARG_REQUIRED:
                return ARG_REQUIRED_STRING;
            case ARG_OPTIONAL:
                return ARG_OPTIONAL_STRING;
            case ARG_OTHER:
                return ARG_OTHER_STRING;
            default:
                return ARG_OTHER_STRING;
        }
    }
    
    public Map<String, Object> getArgumentInfo(
        STAFMapClassDefinition functionInfoMapClass,
        STAFMapClassDefinition argInfoMapClass,
        STAFMapClassDefinition argPropertyInfoMapClass,
        STAFMapClassDefinition argPropertyDataInfoMapClass)
    {
        Map<String, Object> functionMap = new TreeMap<String, Object>();
        functionMap.put("staf-map-class-name", functionInfoMapClass.name());
        functionMap.put("functionName", fName);
        functionMap.put("prolog", fProlog);
        functionMap.put("epilog", fEpilog);
        functionMap.put("argDefinition", this.getArgDefinitionAsString());

        List<Map<String, Object>> argOutputList =
            new ArrayList<Map<String, Object>>();

        for (int i = 0; i < fArgList.size(); i++)
        {
            STAXFunctionArgument arg = fArgList.get(i);

            Map<String, Object> argMap = new TreeMap<String, Object>();
            argMap.put("staf-map-class-name", argInfoMapClass.name());
            argMap.put("argName", arg.getName());
            argMap.put("description", arg.getDescription());

            if (arg.getPrivate() == true)
                argMap.put("private", "Yes");
            else
                argMap.put("private", "No");

            List<Map<String, Object>> argPropertyOutputList =
                new ArrayList<Map<String, Object>>();

            for (STAXFunctionArgumentProperty property : arg.getProperties())
            {
                Map<String, Object> propertyMap = new TreeMap<String, Object>();
                propertyMap.put("staf-map-class-name",
                                argPropertyInfoMapClass.name());

                propertyMap.put("propertyName", property.getName());

                if (property.getDescription().equals(""))
                {
                    propertyMap.put("propertyDescription", "<None>");
                }
                else
                {
                    propertyMap.put("propertyDescription",
                                    property.getDescription());
                }

                if (property.getValue().equals(""))
                {
                    propertyMap.put("propertyValue", "<None>");
                }
                else
                {
                    propertyMap.put("propertyValue", property.getValue());
                }

                List<Map<String, Object>> argPropertyDataOutputList =
                    handlePropertyData(property.getData(),
                                       argPropertyDataInfoMapClass);

                propertyMap.put("propertyData", argPropertyDataOutputList);

                argPropertyOutputList.add(propertyMap);
            }

            argMap.put("properties", argPropertyOutputList);

            int argType = arg.getType();

            if (argType == ARG_REQUIRED)
            {
                argMap.put("type", ARG_REQUIRED_STRING);
            }
            else if (argType == ARG_OPTIONAL)
            {
                argMap.put("type", ARG_OPTIONAL_STRING);
                argMap.put("defaultValue", arg.getDefaultValue());
            }
            else
            {
                argMap.put("type", ARG_OTHER_STRING);
            }
            
            argOutputList.add(argMap);
        }

        functionMap.put("argList", argOutputList);
        
        return functionMap;
    }
    
    public List<Map<String, Object>> handlePropertyData(
        LinkedList<STAXFunctionArgumentPropertyData> data,
        STAFMapClassDefinition argPropertyDataInfoMapClass)
    {
        List<Map<String, Object>> argPropertyDataOutputList =
            new ArrayList<Map<String, Object>>();
        
        for (STAXFunctionArgumentPropertyData propertyData : data)
        {
            Map<String, Object> propertyDataMap =
                new TreeMap<String, Object>();
            propertyDataMap.put("staf-map-class-name",
                                argPropertyDataInfoMapClass.name());

            propertyDataMap.put("dataType", propertyData.getType());
            
            if (propertyData.getValue().equals(""))
            {
                propertyDataMap.put("dataValue", "<None>");
            }
            else
            {
                propertyDataMap.put("dataValue", propertyData.getValue());
            }

            LinkedList<STAXFunctionArgumentPropertyData> dataData =
                propertyData.getData();

            List<Map<String, Object>> argDataDataOutputList =
                new ArrayList<Map<String, Object>>();

            if (!(dataData.isEmpty()))
            {
                argDataDataOutputList =
                    handlePropertyData(dataData, argPropertyDataInfoMapClass);
            }

            propertyDataMap.put("dataData", argDataDataOutputList);

            argPropertyDataOutputList.add(propertyDataMap);
        }
        
        return argPropertyDataOutputList;
    }

    public String getInfo()
    {
        return fName;
    }

    public String getXMLInfo()
    {
        String xmlInfo = "<function=\"" + fName + "\"";

        if (!fScope.equals("global"))
            xmlInfo += " scope=\"" + fScope + "\"";

        if (!fRequires.equals(""))
            xmlInfo += " requires=\"" + fRequires + "\"";

        xmlInfo += "/>";

        return xmlInfo;
    }

    public String getDetails()
    {
        return "Name:" + fName +
               ";Scope:" + fScope +
               ";Requires:" + fRequires +
               ";State:" + getStateAsString() + 
               ";Action:" + fAction;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            try
            {
                fParentFunctionName = thread.pyStringEval(
                    "STAXCurrentFunction");
            }
            catch (STAXPythonEvaluationException e)
            {
                fParentFunctionName = "None";
            }
            
            try
            {
                fParentXmlFile = thread.pyStringEval("STAXCurrentXMLFile");
            }
            catch (STAXPythonEvaluationException e)
            {
                fParentXmlFile = "None";
            }
            
            try
            {
                fParentXmlMachine = thread.pyStringEval(
                    "STAXCurrentXMLMachine");
            }
            catch (STAXPythonEvaluationException e)
            {
                fParentXmlMachine = "None";
            }

            // If local scope, save PythonInterpreter and clone it

            if (fScope.equals("local"))
            {
                fSavePythonInterpreter = thread.getPythonInterpreter();
                thread.setPythonInterpreter(fSavePythonInterpreter.clonePyi());
            }

            // Verify call arguments before putting function's action on the 
            // thread's action stack.

            STAXSignalData signalData = new STAXSignalData();
            
            if (verifyArguments(thread, signalData) == 0)
            {
                // Set the STAXFunctionName variable to contain the name of
                // the function currently being executed
                thread.pySetVar("STAXCurrentFunction", fName);
                
                // Set STAX variables for the xml file path and machine for
                // the function currently being executed
                thread.pySetVar("STAXCurrentXMLFile", getXmlFile());
                thread.pySetVar("STAXCurrentXMLMachine", getXmlMachine());

                thread.pushAction(fAction.cloneAction());
                fState = FUNCTION_CALLED;
            }
            else // Verification of arguments failed
            {
                fState = COMPLETE;
                thread.popAction();

                // If local scope, reset PythonInterpreter before set STAXResult

                if (fScope.equals("local") && (fSavePythonInterpreter != null))
                {
                    thread.setPythonInterpreter(fSavePythonInterpreter);
                }

                // Set STAXResult to STAXFunctionError

                try
                {
                    thread.pyExec("STAXResult = STAXFunctionError");
                }
                catch (STAXPythonEvaluationException e)
                {
                    // This should never happen

                    fCallAction.setElementInfo(new STAXElementInfo(
                        fCallAction.getElement(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "Error assigning STAXResultError to STAXResult " +
                        "after calling function \n" + fName + "\"."));

                    thread.setSignalMsgVar(
                        "STAXPythonEvalMsg",
                        STAXUtil.formatErrorMessage(fCallAction), e);

                    thread.raiseSignal("STAXPythonEvaluationError");
                    return;
                }

                // Check if a signal should be raised due to invalid arguments

                if (!signalData.fSignalName.equals(""))
                {
                    thread.setSignalMsgVar(signalData.fMessageName,
                                           signalData.fMessage,
                                           signalData.fPythonException);
                    thread.raiseSignal(signalData.fSignalName);
                }
            }
            return;
        }

        else if (fState == FUNCTION_CALLED)
        {
            exitFunction(thread);
            
            thread.pySetVar("STAXResult", Py.None);
        }
    }

    public void exitFunction(STAXThread thread)
    {
        fState = COMPLETE;
        thread.popAction();

        // If local scope, reset PythonInterpreter before set STAXResult
        if (fScope.equals("local") && (fSavePythonInterpreter != null))
        {
            thread.setPythonInterpreter(fSavePythonInterpreter);
        }
        
        thread.pySetVar("STAXCurrentFunction", fParentFunctionName);
        thread.pySetVar("STAXCurrentXMLFile", fParentXmlFile);
        thread.pySetVar("STAXCurrentXMLMachine", fParentXmlMachine);
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        exitFunction(thread);

        if (cond instanceof STAXReturnCondition)
        {
            STAXReturnCondition returnCondition = (STAXReturnCondition)cond;

            thread.pySetVar("STAXResult", returnCondition.getData());

            thread.removeCondition(returnCondition);
        }
        else
        {
            thread.pySetVar("STAXResult", Py.None);
        }

        // Remove any return conditions from the condition stack that the
        // function action handles.  This way, there won't be any "dangling"
        // return conditions hanging around if a terminate block condition is
        // added to the condition stack.

        thread.visitConditions(new STAXVisitor()
        {
            public void visit(Object o, Iterator iter)
            {
                if (o instanceof STAXReturnCondition)
                {
                    iter.remove();
                }
            }
        });

    }

    public STAXAction cloneAction()
    {
        STAXFunctionAction clone = new STAXFunctionAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fName = fName;
        clone.fProlog = fProlog;
        clone.fEpilog = fEpilog;
        // clone.fImportList = (ArrayList<STAXFunctionImport>)fImportList.clone();
        clone.fImportList = new ArrayList<STAXFunctionImport>(fImportList);
        clone.fScope = fScope;
        clone.fRequires = fRequires;
        clone.fAction = fAction;
        clone.fSavePythonInterpreter = fSavePythonInterpreter;
        clone.fArgDefinition = fArgDefinition;
        clone.fDefinedWithFunctionArgDef = fDefinedWithFunctionArgDef;
        // clone.fArgList = (ArrayList<STAXFunctionArgument>)fArgList.clone();
        clone.fArgList = new ArrayList<STAXFunctionArgument>(fArgList);
        clone.fCallArgPyObject = fCallArgPyObject;
        clone.fCallAction = (STAXCallAction)fCallAction.cloneAction();
        clone.fParentFunctionName = fParentFunctionName;

        return clone;
    }

    // Verify valid arguments are passed to the function
    // Returns:  0 if valid arguments; non-zero if invalid arguments
    // Sets signalData values to indicate if a signal should be raised.

    public int verifyArguments(STAXThread thread, STAXSignalData signalData)
    {
        if (fArgDefinition == FUNCTION_DEFINES_NO_ARGS)
        {
            // No verification can be done - assign arguments to STAXArg

            thread.pySetVar("STAXArg", fCallArgPyObject);
        }

        else if (fArgDefinition == FUNCTION_ALLOWS_NO_ARGS)
        {
            if (fCallArgPyObject != null &&
                !(fCallArgPyObject.toString().equals("None")))
            {
                // Error - no arguments can be passed on the call

                signalData.fSignalName ="STAXFunctionArgValidate";
                signalData.fMessageName = "STAXFunctionArgValidateMsg";
                
                fCallAction.setElementInfo(new STAXElementInfo(
                    fCallAction.getElement(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME,
                    "Function " + fName + " does not allow arguments to be " +
                    "passed to it."));

                signalData.fMessage = STAXUtil.formatErrorMessage(fCallAction);

                return 1;
            }
        }

        else if (fArgDefinition == FUNCTION_DEFINES_ONE_ARG)
        {
            STAXFunctionArgument arg = fArgList.get(0);
            int argType = arg.getType();
            String argName = arg.getName();

            if (argType == ARG_REQUIRED)
            {
                if (fCallArgPyObject == null)
                {
                    // Error - must provide required arguments

                    signalData.fSignalName ="STAXFunctionArgValidate";
                    signalData.fMessageName = "STAXFunctionArgValidateMsg";

                    fCallAction.setElementInfo(new STAXElementInfo(
                        fCallAction.getElement(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "Required argument \"" + argName + "\" is not " +
                        "provided in the call to function \"" + fName +
                        "\"."));

                    signalData.fMessage = STAXUtil.formatErrorMessage(
                        fCallAction);

                    return 1;
                }
                else
                {   
                    thread.pySetVar(argName, fCallArgPyObject);
                }
            } 
            else if (argType == ARG_OPTIONAL)
            {
                if (fCallArgPyObject == null)
                {
                    // Evaluate the default value

                    try
                    {
                        thread.pyExec(argName + " = " + arg.getDefaultValue());
                    }
                    catch (STAXPythonEvaluationException e)
                    {
                        signalData.fSignalName = "STAXPythonEvaluationError";
                        signalData.fMessageName = "STAXPythonEvalMsg";
                        
                        setElementInfo(new STAXElementInfo(
                            getFunctionArgElementName(argType), "default",
                            "Argument \"" + argName + "\" has an " +
                            "invalid default value: " +
                            arg.getDefaultValue()));

                        signalData.fMessage = STAXUtil.formatErrorMessage(
                            this);

                        signalData.fPythonException = e;

                        return 2;
                    }
                }
                else
                {
                    thread.pySetVar(argName, fCallArgPyObject);
                }
            }
            else      
            {   // Should never happen
                System.out.println("Invalid function argType=" + argType);
                return 2;
            }
        }

        else if (fArgDefinition == FUNCTION_DEFINES_LIST_ARGS)
        {
            // Extract a Python tuple or list (call arguments) into a Java List

            ArrayList<PyObject> callArgList = new ArrayList<PyObject>(); 

            if (fCallArgPyObject != null)
            { 
                if ((fCallArgPyObject instanceof PySequence) &&
                    !(fCallArgPyObject instanceof PyString))
                { 
                    thread.pySetVar("STAXTempList", fCallArgPyObject);

                    callArgList =  new ArrayList<PyObject>(Arrays.asList(
                        (PyObject[])thread.getPythonInterpreter().
                              get("STAXTempList", PyObject[].class)));
                }
                else
                {
                    callArgList.add(fCallArgPyObject);
                }
            }

            ListIterator<PyObject> callArgIter = callArgList.listIterator();

            // Walk function's argument definition list

            for (int i = 0; i < fArgList.size(); i++)
            {
                STAXFunctionArgument arg = fArgList.get(i);
                int argType = arg.getType();
                String argName = arg.getName();

                if (argType == ARG_REQUIRED)
                {
                    if (!callArgIter.hasNext())
                    {
                        // Raise signal - must provide required argument

                        signalData.fSignalName = "STAXFunctionArgValidate";
                        signalData.fMessageName = "STAXFunctionArgValidateMsg";

                        fCallAction.setElementInfo(new STAXElementInfo(
                            fCallAction.getElement(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            "Required argument \"" + argName + "\" is not " +
                            "provided in the call to function \"" + fName +
                            "\"."));

                        signalData.fMessage = STAXUtil.formatErrorMessage(
                            fCallAction);

                        return 1;
                    }
                    else
                    {
                        thread.pySetVar(argName, callArgIter.next());
                    }
                } 
                else if (argType == ARG_OPTIONAL)
                {
                    if (!callArgIter.hasNext())
                    {
                        // Evaluate the default value

                        try
                        {
                            thread.pyExec(argName + " = " + 
                                          arg.getDefaultValue());
                        }
                        catch (STAXPythonEvaluationException e)
                        {
                            signalData.fSignalName =
                                "STAXPythonEvaluationError";
                            signalData.fMessageName = "STAXPythonEvalMsg";

                            setElementInfo(new STAXElementInfo(
                                getFunctionArgElementName(argType),
                                "default", i,
                                "Argument \"" + argName + "\" has an " +
                                "invalid default value: " +
                                arg.getDefaultValue()));

                            signalData.fMessage = STAXUtil.formatErrorMessage(
                                this);

                            signalData.fPythonException = e;
                            return 2;
                        }
                    }
                    else
                    {
                        thread.pySetVar(argName, callArgIter.next());
                    }
                }
                else if (argType == ARG_OTHER)
                {
                    // Create a PyList of remaining arguments passed by call

                    PyList otherArgList = new PyList();

                    while (callArgIter.hasNext())
                    {
                        otherArgList.append(Py.java2py(callArgIter.next()));
                    }

                    thread.pySetVar(argName, otherArgList);
                }
                else
                {   // Should never happen
                    System.out.println("Invalid function argType=" + argType);
                    return 2;
                }
            } // end for

            // Check if CallArgList has any additional entries

            if (callArgIter.hasNext())
            {
                // Error - more call arguments than function recognizes

                signalData.fSignalName = "STAXFunctionArgValidate";
                signalData.fMessageName = "STAXFunctionArgValidateMsg";

                String msg = "Too many call arguments (" + callArgList.size() +
                    ").  Function \"" + fName + "\" only handles " +
                    fArgList.size() + " arguments.";

                try
                {
                    msg += "\nFirst additional argument: " +  callArgIter.next();
                }
                catch (Exception e)
                {
                    // Do nothing as already have enough info in signal message
                    // Need to do in case getting string value of callArgIter.
                    // next() raises a Python exception (as it does if it
                    // contains a STAXGlobal variable.
                }
                
                fCallAction.setElementInfo(new STAXElementInfo(
                    fCallAction.getElement(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME, msg));

                signalData.fMessage = STAXUtil.formatErrorMessage(fCallAction);

                return 1;
            }
        }

        else if (fArgDefinition == FUNCTION_DEFINES_MAP_ARGS)
        {
            // Extract a Python dictionary (call arguments) into a Java Map

            Map<Object, PyObject> callArgMap = new HashMap<Object, PyObject>(); 

            if (fCallArgPyObject != null && 
                !(fCallArgPyObject.toString().equals("None")))
            { 
                if (fCallArgPyObject instanceof PyDictionary)
                { 
                    PyList pa = ((PyDictionary)fCallArgPyObject).items();

                    while (pa.__len__() != 0)
                    {
                        PyTuple po = (PyTuple)pa.pop();
                        Object first  = po.__finditem__(0).
                                          __tojava__(Object.class);
                        PyObject second = po.__finditem__(1);
                        callArgMap.put(first, second);
                    }
                }
                else
                {
                    // Raise signal - arguments must be in a map form

                    signalData.fSignalName = "STAXFunctionArgValidate";
                    signalData.fMessageName = "STAXFunctionArgValidateMsg";

                    fCallAction.setElementInfo(new STAXElementInfo(
                        fCallAction.getElement(),
                        STAXElementInfo.NO_ATTRIBUTE_NAME,
                        "Function \"" + fName + "\" requires arguments in " +
                        "a map (Python dictionary) form."));

                    signalData.fMessage = STAXUtil.formatErrorMessage(
                        fCallAction);

                    return 1;
                }
            }

            // Walk function's argument definition list

            for (int i = 0; i < fArgList.size(); i++)
            {
                STAXFunctionArgument arg = fArgList.get(i);
                int argType = arg.getType();
                String argName = arg.getName();

                if (argType == ARG_REQUIRED)
                {
                    if (callArgMap.containsKey(argName))
                    {
                        thread.pySetVar(argName, callArgMap.get(argName));
                        callArgMap.remove(argName);
                    }
                    else
                    {
                        // Raise signal - must provide required argument

                        signalData.fSignalName = "STAXFunctionArgValidate";
                        signalData.fMessageName = "STAXFunctionArgValidateMsg";

                        fCallAction.setElementInfo(new STAXElementInfo(
                            fCallAction.getElement(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            "Required argument \"" + argName + "\" is not " +
                            "provided in the call to function \"" + fName +
                            "\"."));

                        signalData.fMessage = STAXUtil.formatErrorMessage(
                            fCallAction);

                        return 1;
                    }
                } 
                else if (argType == ARG_OPTIONAL)
                {
                    if (!callArgMap.containsKey(argName))
                    {
                        // Evaluate the default value

                        try
                        {
                            thread.pyExec(argName + " = " +
                                          arg.getDefaultValue());
                        }
                        catch (STAXPythonEvaluationException e)
                        {
                            setElementInfo(new STAXElementInfo(
                                getFunctionArgElementName(argType),
                                "default", i,
                                "Argument \"" + argName + "\" has an " +
                                "invalid default value: " +
                                arg.getDefaultValue()));

                            signalData.fSignalName =
                                "STAXPythonEvaluationError";
                            signalData.fMessageName = "STAXPythonEvalMsg";
                            signalData.fMessage =
                                STAXUtil.formatErrorMessage(this);
                            signalData.fPythonException = e;
                            return 2;
                        }
                    }
                    else
                    {
                        thread.pySetVar(argName, callArgMap.get(argName));
                        callArgMap.remove(argName);
                    }
                }
                else if (argType == ARG_OTHER)
                {
                    // Convert the map of remaining arguments to a PyDictionary

                    Map<PyObject, PyObject> m =
                        new HashMap<PyObject, PyObject>();

                    for (Map.Entry<Object, PyObject> entry :
                         callArgMap.entrySet())
                    {
                        m.put(Py.java2py(entry.getKey()),
                              Py.java2py(entry.getValue()));
                    }

                    callArgMap = new HashMap<Object, PyObject>(); // Reset to empty
                    
                    // PyDictionary constructor wants a Hashtable
                    thread.pySetVar(
                        argName, 
                        new PyDictionary(new Hashtable<PyObject, PyObject>(m)));
                }
                else
                {   // Should never happen
                    System.out.println("Invalid function argType=" + argType);
                    return 2;
                }
            } // end for

            // Check if CallArgMap has any additional entries

            if (!callArgMap.isEmpty())
            {
                // Error - more call arguments than function recognizes
                
                signalData.fSignalName = "STAXFunctionArgValidate";
                signalData.fMessageName = "STAXFunctionArgValidateMsg";
                
                String msg = "Too many call arguments for function \"" +
                    fName + "\".  Extra arguments: ";
                
                // Iterate thru callArgMap to put extra call argument names
                // into the error message
                
                Iterator it = callArgMap.entrySet().iterator();
                int extraArgNum = 1;

                while (it.hasNext())
                {
                    Map.Entry e = (Map.Entry)it.next();
                    
                    if (extraArgNum != 1) msg += ", ";
                    
                    msg += Py.java2py(e.getKey());
                    extraArgNum++;
                }
                
                fCallAction.setElementInfo(new STAXElementInfo(
                    fCallAction.getElement(),
                    STAXElementInfo.NO_ATTRIBUTE_NAME, msg));

                signalData.fMessage = STAXUtil.formatErrorMessage(fCallAction);

                return 1;
            }
        }

        else
        {   // This should never happen
            System.out.println("Invalid function argument definition. " +
                               "fArgDefinition=" + fArgDefinition);
        }

        return 0;
    }

    private int fState = INIT;
    private String fName = new String();
    private String fProlog = new String();
    private String fEpilog = new String();
    private String fScope = new String();
    private String fRequires = new String();
    private ArrayList<STAXFunctionImport> fImportList =
        new ArrayList<STAXFunctionImport>();
    private STAXAction fAction = null;
    private STAXPythonInterpreter fSavePythonInterpreter = null;
    private boolean fDefinedWithFunctionArgDef = false;

    private int       fArgDefinition = FUNCTION_DEFINES_NO_ARGS;
    private ArrayList<STAXFunctionArgument> fArgList =
        new ArrayList<STAXFunctionArgument>();

    // Contains the arguments passed by the call to the function
    private PyObject fCallArgPyObject;

    private STAXCallAction fCallAction = new STAXCallAction();

    private String fParentFunctionName = new String();
    private String fParentXmlFile = new String();
    private String fParentXmlMachine = new String();
    
    private class STAXSignalData
    {
        private STAXSignalData() {}

        private String fSignalName = "";
        private String fMessageName = "";
        private String fMessage = "";
        private STAXPythonEvaluationException fPythonException = null;
    }
}
