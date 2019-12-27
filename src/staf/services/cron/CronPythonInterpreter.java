/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.cron;

import com.ibm.staf.*;
import org.python.util.PythonInterpreter;
import org.python.core.PyException;
import org.python.core.PyObject;
import org.python.core.PyCode;
import org.python.core.PyInteger;
import org.python.core.Py;
// Jython 2.1:  import org.python.core.__builtin__;
import org.python.core.CompileMode;

class CronPythonInterpreter
{
    private PythonInterpreter fPythonInterpreter = new PythonInterpreter();
    private CronService fService;

    private CronPythonInterpreter() {}

    CronPythonInterpreter(CronService service, Integer id)
    {
        fService = service;

        // Set Python variables for the triggered registration

        pySetVar("STAFCronID", new PyInteger(id.intValue()));
        pySetVar("STAFCronSubmit", "true");
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
