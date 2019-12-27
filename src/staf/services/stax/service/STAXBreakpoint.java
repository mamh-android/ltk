/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXBreakpoint
{
    STAXBreakpoint(int type,
                   String function,
                   String line,
                   String file,
                   String machine)
    {
        this.type = type;
        this.function = function;
        this.line = line;
        this.file = file;
        this.machine = machine;
    }

    public String getFunction()
    {
        return function;
    }

    public String getLine()
    {
        return line;
    }

    public String getFile()
    {
        return file;
    }

    public String getMachine()
    {
        return machine;
    }

    private int type;
    private String function;
    private String line;
    private String file;
    private String machine;
}

