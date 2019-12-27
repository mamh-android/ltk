/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.python.core.*;

public class STAXReturnCondition implements STAXCondition
{
    static final int PRIORITY = 400;

    public STAXReturnCondition(PyObject data)
    {
        fData = data;
    }

    public STAXReturnCondition(PyObject data, String source)
    {
        fData = data;
        fSource = source;
    }

    public boolean isInheritable() { return true; }
    public int getPriority() { return PRIORITY; }
    public String getSource() { return fSource; }
    public PyObject getData() { return fData; }

    private String fSource = new String("Return");
    private PyObject fData;
}
