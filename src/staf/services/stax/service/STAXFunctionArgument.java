/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.LinkedList;

// Create a new class STAXFunctionArgument which is used to store each
// the definition for a function argument (for function-single-arg,
// function-list-args, or function-map-args).


public class STAXFunctionArgument
{
     public STAXFunctionArgument()
     { /* Do Nothing */ }
  
     public STAXFunctionArgument(String name, int type, String description)
     {
         fName = name;
         fType = type;
         fDescription = description;
     }
     
     public STAXFunctionArgument(String name, int type, String value,
                                 String description)
     {
         fName = name;
         fType = type;
         fDefaultValue = value;
         fDescription = description;
     }

     public STAXFunctionArgument(
         String name, int type, String value,
         String description, boolean argPrivate,
         LinkedList<STAXFunctionArgumentProperty> properties)
     {
         fName = name;
         fType = type;
         fDefaultValue = value;
         fDescription = description;
         fPrivate = argPrivate;
         fProperties = properties;
     }
     
     public String getName() { return fName; }
     public void setName(String name) { fName = name; }
    
     public String getDefaultValue() { return fDefaultValue; }
     public void setDefaultValue(String value) { fDefaultValue = value; }
    
     public int getType() { return fType; }
     public void setType(int type) { fType = type; }
     
     public String getDescription() { return fDescription; }
     public void setDescription(String description) 
        { fDescription = description; }

     public boolean getPrivate() { return fPrivate; }
     public void setPrivate(boolean argPrivate) { fPrivate = argPrivate; }

     public LinkedList<STAXFunctionArgumentProperty> getProperties()
     {
         return fProperties;
     }
     
    public void setProperties(LinkedList<STAXFunctionArgumentProperty> properties)
     {
         fProperties = properties;
     }

     public void addProperty(STAXFunctionArgumentProperty property)
     {
         fProperties.add(property);
     }
    
     private String fName;
     private String fDefaultValue = new String();
     private int    fType;
     private String fDescription = new String();
     private boolean fPrivate = false;
     private LinkedList<STAXFunctionArgumentProperty> fProperties =
         new LinkedList<STAXFunctionArgumentProperty>();
}
