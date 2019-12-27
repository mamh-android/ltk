/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXBreakCondition implements STAXCondition
{
    static final int PRIORITY = 400;

    public STAXBreakCondition()
    { /* Do nothing */ }

    public STAXBreakCondition(String source)
    {
        fSource = source;
    }

    public boolean isInheritable() { return true; }
    public int getPriority() { return PRIORITY; }
    public String getSource() { return fSource; }
    public Object getData() { return null; }

    private String fSource = new String("");
}
