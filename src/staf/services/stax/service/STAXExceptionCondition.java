/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.*;
import com.ibm.staf.STAFMarshallingContext;

public class STAXExceptionCondition implements STAXCondition
{
    static final int PRIORITY = 400;

    public STAXExceptionCondition(String name, String data)
    {
        fName = name;
        fData = data;
    }


    public STAXExceptionCondition(String name, String data, String source)
    {
        fName = name;
        fData = data;
        fSource = source;
    }

    public STAXExceptionCondition(String name, String data, String source,
                                  List<String> stackTrace)
    {
        fName = name;
        fData = data;
        fSource = source;
        fStackTrace = stackTrace;
    }

    public boolean isInheritable() { return true; }
    public int getPriority() { return PRIORITY; }
    public String getSource() { return fSource; }
    public String getName() { return fName; }
    public String getData() { return fData; }
    public List<String> getStackTrace() { return fStackTrace; }

    private String fSource;
    private String fName;
    private String fData;
    private List<String> fStackTrace;
}
