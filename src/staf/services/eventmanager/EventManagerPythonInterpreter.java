/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.eventmanager;

import com.ibm.staf.*;
import org.python.util.PythonInterpreter;
import org.python.core.PyException;
import org.python.core.PyObject;
import org.python.core.PyCode;
import org.python.core.PyInteger;
import org.python.core.Py;   
// Jython 2.1: import org.python.core.__builtin__;
import org.python.core.CompileMode;

class EventManagerPythonInterpreter
{
    private PythonInterpreter fPythonInterpreter = new PythonInterpreter();
    private EventManagerService fService;

    private EventManagerPythonInterpreter() {}

    EventManagerPythonInterpreter(EventManagerService service, Integer id)
    {
        fService = service;
        
        // Set the Python variables for the triggered registration
        
        pySetVar("STAFEventManagerID", new PyInteger(id.intValue()));
        pySetVar("STAFEventManagerSubmit", "true");
    }

    STAFResult unmarshallMessage(String queuedMessage)
    {
        // Unmarshall the message using the Jython unmarshall method so that
        // can access the data in the message via Jython as Python data.

        pySetVar("STAFEventMessage", Py.java2py(queuedMessage));

        String pythonCode = 
        "# The STAFMarshalling module is in directory fJythonHome/Lib\n" +
        "import STAFMarshalling\n" +
        "\n" +
        "# Unmarshall the message from the event service\n" +
        "STAFMC = STAFMarshalling.unmarshall(STAFEventMessage)\n" +
        "STAFEventMsgMap = STAFMC.getRootObject()\n" +
        "\n" +
        "# Remove Python variables that no longer need\n" +
        "del STAFMC, STAFEventMessage\n" +
        "\n" +
        "# Assign the following Python variables\n" +
        "eventservice      = STAFEventMsgMap['eventServiceName']\n" +
        "eventid           = STAFEventMsgMap['eventID']\n" +
        "generatingmachine = STAFEventMsgMap['machine']\n" +
        "generatingprocess = STAFEventMsgMap['handleName']\n" +
        "generatinghandle  = STAFEventMsgMap['handle']\n" +
        "eventtimestamp    = STAFEventMsgMap['timestamp']\n" +
        "eventtype         = STAFEventMsgMap['type']\n" +
        "eventsubtype      = STAFEventMsgMap['subtype']\n" +
        "\n" +
        "# Create a eventinfo dictionary containing the data\n" +
        "eventinfo = {}\n" +
        "eventinfo['eventservice'] = eventservice\n" +
        "eventinfo['eventid'] = eventid\n" +
        "eventinfo['generatingmachine'] = generatingmachine\n" +
        "eventinfo['generatingprocess'] = generatingprocess\n" +
        "eventinfo['generatinghandle'] = generatinghandle\n" +
        "eventinfo['eventtimestamp'] = eventtimestamp\n" +
        "eventinfo['eventtype'] = eventtype\n" +
        "eventinfo['eventsubtype'] = eventsubtype\n" +
        "\n" +
        "# Assign a Python variable for each property\n" +
        "for STAFPropKey, STAFPropValue in STAFEventMsgMap['propertyMap'].items():\n" +
        "  locals()[STAFPropKey]  = STAFPropValue\n" +
        "  eventinfo[STAFPropKey] = STAFPropValue\n" +
        "\n" +
        "# Remove Python variables that no longer need\n" +
        "del STAFEventMsgMap\n" +
        "if locals().has_key('STAFPropKey'):\n" +
        "  del STAFPropKey\n" +
        "if locals().has_key('STAFPropValue'):\n" +
        "  del STAFPropValue\n";

        try
        {
            pyExec(pythonCode);
        }
        catch (PyException e)
        {
            String errMsg = "PyException:\n" + e;

            return new STAFResult(STAFResult.InvalidValue, errMsg);
        }

        return new STAFResult(STAFResult.Ok);
    }

    static void compileForPython(String pythonCode)
    {
        // Jython 2.1:
        // PyCode code = __builtin__.compile(pythonCode, "<string>", "exec");

        PyCode codeObject = Py.compile_flags(
            pythonCode, "<pyExec string>", CompileMode.exec,
            Py.getCompilerFlags());
    }

    String pyStringEval(String value)
    {
        PyObject result = fPythonInterpreter.eval(value);
        
        if (result == null)
            return null;
        else
            return result.toString();
    }

    void pySetVar(String varName, Object value)
    {
        try
        {
            fPythonInterpreter.set(varName, value);
        }
        catch (PyException e)
        {
            String msg = "PySetVar PyException caught setting " +
                " var=" + varName + " to value=" + value + e.toString();

            fService.log("error", msg);
        }
    }

    Object pyGetVar(String varName)
    {
        try
        {
            return fPythonInterpreter.get(varName);
        }
        catch (PyException e)
        {
            String msg = "PyGetVar PyException caught getting " +
                " var=" + varName + " to value=" + varName + e.toString();

            fService.log("error", msg);

            return null;
        }
    }

    void pyExec(String value)
    {
        fPythonInterpreter.exec(value);
    }
}
