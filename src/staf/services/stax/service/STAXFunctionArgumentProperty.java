/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.LinkedList;

// Create a new class STAXFunctionArgumentProperty which is used to store
// the definition for a function argument property


public class STAXFunctionArgumentProperty
{
    public STAXFunctionArgumentProperty()
    { /* Do Nothing */ }

    public STAXFunctionArgumentProperty(
        String name, String description, String value,
        LinkedList<STAXFunctionArgumentPropertyData> data)
    {
        fName = name;
        fDescription = description;
        fValue = value;
        fData = data;
    }

    public String getName() { return fName; }
    public void setName(String name) { fName = name; }

    public String getDescription() { return fDescription; }
    public void setDescription(String description)
       { fDescription = description; }

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

    private String fName;
    private String fDescription = new String();
    private String fValue = new String();
    private LinkedList<STAXFunctionArgumentPropertyData> fData =
        new LinkedList<STAXFunctionArgumentPropertyData>();
}
