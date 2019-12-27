/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2010                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXMonitorExecuteResult
{
    int fRC;
    Object fResult;

    STAXMonitorExecuteResult(int rc, Object result)
    {
        fRC = rc;
        fResult = result;
    }

    public int getRC() { return fRC; }
    public Object getResult() { return fResult; }
}
