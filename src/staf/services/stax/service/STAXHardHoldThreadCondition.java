/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXHardHoldThreadCondition implements STAXCondition
{
    static final int PRIORITY = 50;

    public STAXHardHoldThreadCondition()
    { /* Do Nothing */ }

    public STAXHardHoldThreadCondition(String source)
    {
        fSource = source;
    }

    public boolean isInheritable() { return false; }
    public int getPriority() { return PRIORITY; }
    public String getSource() { return fSource; }
    
    private String fSource = new String("");
}
