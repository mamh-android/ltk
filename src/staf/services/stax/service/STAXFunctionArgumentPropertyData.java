/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.LinkedList;

// Create a new class STAXFunctionArgumentPropertyData which is used to store
// the definitions for a function argument property data


public class STAXFunctionArgumentPropertyData
{
    public STAXFunctionArgumentPropertyData()
    { /* Do Nothing */ }

    public STAXFunctionArgumentPropertyData(String type, String value)
    {
        fType = type;
        fValue = value;
    }

    public String getType() { return fType; }
    public void setType(String type) { fType = type; }

    public String getValue() { return fValue; }
    public void setValue(String value) { fValue = value; }

    public LinkedList<STAXFunctionArgumentPropertyData> getData()
    {
        return fData;
    }

    public void setData(LinkedList<STAXFunctionArgumentPropertyData> data)
    {
        fData = data;
    }

    public void addData(STAXFunctionArgumentPropertyData data)
    {
        fData.add(data);
    }

    private String fType;
    private String fValue = new String();
    private LinkedList<STAXFunctionArgumentPropertyData> fData =
        new LinkedList<STAXFunctionArgumentPropertyData>();
}
