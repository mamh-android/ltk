/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.util.Iterator;
import java.util.Hashtable;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import org.python.core.Py;
import org.python.core.PyObject;
import org.python.core.PyList;
import org.python.core.PyDictionary;

public class STAXCatchAction extends STAXActionDefaultImpl
{
    private static final int INIT = 0;
    private static final int CALLED_ACTION = 1;
    private static final int COMPLETE = 2;

    private static final String INIT_STRING = "INIT";
    private static final String CALLED_ACTION_STRING = "CALLED_ACTION";
    private static final String COMPLETE_STRING = "COMPLETE";
    private static final String STATE_UNKNOWN_STRING = "UNKNOWN";

    public STAXCatchAction()
    { /* Do Nothing */ }

    public STAXCatchAction(String exceptionName, String varName,
                           String typeVarName, String sourceVarName,
                           STAXAction action)
    {
        fExceptionName = exceptionName;
        fVarName = varName;
        fTypeVarName = typeVarName;
        fSourceVarName = sourceVarName;
        fAction = action;
    }

    public void setExceptionName(String exceptionName)
    {
        fExceptionName = exceptionName;
    }

    public void setVarName(String varName)
    {
        fVarName = varName;
    }

    public void setTypeVarName(String typeVarName)
    {
        fTypeVarName = typeVarName;
    }

    public void setSourceVarName(String sourceVarName)
    {
        fSourceVarName = sourceVarName;
    }

    public void setAction(STAXAction action)
    {
        fAction = action;
    }

    public String getCatchableExceptionName() { return fExceptionName; }
    
    public void setException(STAXExceptionCondition cond)
    {
        fThrownException = cond;
    }

    public String getStateAsString()
    {
        switch (fState)
        {
            case INIT:
                return INIT_STRING;
            case CALLED_ACTION:
                return CALLED_ACTION_STRING;
            case COMPLETE:
                return COMPLETE_STRING;
            default:
                return STATE_UNKNOWN_STRING;
        }
    }

    public String getInfo() { return fExceptionName; }

    public String getDetails()
    {
        return "State:" + getStateAsString() +
               ";ExceptionName:" + fExceptionName +
               ";VarName:" + fVarName +
               ";TypeVarName:" + fTypeVarName +
               ";SourceVarName:" + fSourceVarName +
               ";Action:" + fAction +
               ";ExceptionCondition:" + fThrownException;
    }

    public void execute(STAXThread thread)
    {
        if (fState == INIT)
        {
            if (fVarName != null)
                thread.pySetVar(fVarName, fThrownException.getData());

            if (fTypeVarName != null)
                thread.pySetVar(fTypeVarName, fThrownException.getName());

            if (fSourceVarName != null)
            {
                 String SOURCE_VAR_PYCODE =
                     fSourceVarName + " = STAXExceptionSource(" +
                     createSourceMap() + ") \n" +
                     "\n";

                 try
                 {
                     thread.pyExec(SOURCE_VAR_PYCODE);
                 }
                 catch (STAXPythonEvaluationException ex)
                 {
                     ex.printStackTrace();
                     System.out.println("SOURCE_VAR_PYCODE=" +
                         SOURCE_VAR_PYCODE);
                 }
            }

            thread.pushAction(fAction.cloneAction());
            fState = CALLED_ACTION;
        }
        else
        {
            fState = COMPLETE;
            thread.popAction();
        }
    }

    public void handleCondition(STAXThread thread, STAXCondition cond)
    {
        fState = COMPLETE;
        thread.popAction();

        // Remove any rethrow exception conditions from the condition stack
        // that this catch action handles.  This way, there won't be any
        // "dangling" rethrow exception conditions hanging around if a
        // terminate block condition is added to the condition stack.
        // Also, set a flag to indicate if any rethrow exception conditions
        // were on the condition stack so that we can add an exception
        // condition to the condition stack.

        fFoundRethrow = false;

        thread.visitConditions(new STAXVisitor()
        {
            public void visit(Object o, Iterator iter)
            {
                if (o instanceof STAXRethrowExceptionCondition)
                {
                    iter.remove();
                    fFoundRethrow = true;
                }
            }
        });

        if (fFoundRethrow)
        {
            thread.addCondition(fThrownException);
        }
    }

    public STAXAction cloneAction()
    {
        STAXCatchAction clone = new STAXCatchAction();

        clone.setElement(getElement());
        clone.setLineNumberMap(getLineNumberMap());
        clone.setXmlFile(getXmlFile());
        clone.setXmlMachine(getXmlMachine());

        clone.fExceptionName = fExceptionName;
        clone.fVarName = fVarName;
        clone.fTypeVarName = fTypeVarName;
        clone.fSourceVarName = fSourceVarName;
        clone.fAction = fAction;

        return clone;
    }

    private PyDictionary createSourceMap()
    {
        Map<PyObject, PyObject> m = new HashMap<PyObject, PyObject>();

        m.put(Py.java2py("source"),
              Py.java2py(fThrownException.getSource()));

        m.put(Py.java2py("stackTrace"),
              convertJavaStackTraceToPython(fThrownException.getStackTrace()));

        PyDictionary pyDictionary = new PyDictionary(
            new Hashtable<PyObject, PyObject>(m));

        return pyDictionary;
    }

    private static PyList convertJavaStackTraceToPython(List<String> javaList)
    {
        PyList pyList = new PyList();

        for (String currentItem : javaList)
        {
            pyList.append(Py.java2py(currentItem));
        }

        return pyList;
    }

    private int fState = INIT;
    private String fExceptionName = null;
    private String fVarName = null;
    private String fTypeVarName = null;
    private String fSourceVarName = null;
    private STAXAction fAction = null;
    private STAXExceptionCondition fThrownException;
    private boolean fFoundRethrow = false;
}
