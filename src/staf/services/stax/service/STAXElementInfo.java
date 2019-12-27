/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2007                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

// Create a new class STAXElementInfo which is used to store information
// about where an error occurred in an STAX XML file.  

public class STAXElementInfo
{
     public static final int LAST_ELEMENT_INDEX = -1;
     public static final int FIRST_ELEMENT_INDEX = 0;
     public static final String NO_ELEMENT_NAME = null;
     public static final String NO_ATTRIBUTE_NAME = null;
     public static final String NO_MESSAGE = null;
     
     public STAXElementInfo()
     { /* Do Nothing */ }
  
     public STAXElementInfo(String elementName, String attributeName,
                            int elementIndex, String errorMessage)
     {
         fElementName = elementName;
         fAttributeName = attributeName;
         fElementIndex = elementIndex;
         fErrorMessage = errorMessage;
     }
     
     public STAXElementInfo(String elementName, String attributeName,
                            String errorMessage)
     {
         fElementName = elementName;
         fAttributeName = attributeName;
         fElementIndex = FIRST_ELEMENT_INDEX;
         fErrorMessage = errorMessage;
     }

     public STAXElementInfo(String elementName)
     {
         fElementName = elementName;
         fAttributeName = NO_ATTRIBUTE_NAME;
         fElementIndex = FIRST_ELEMENT_INDEX;
         fErrorMessage = NO_MESSAGE;
     }

     public STAXElementInfo(String elementName, String attributeName)
     {
         fElementName = elementName;
         fAttributeName = attributeName;
         fElementIndex = FIRST_ELEMENT_INDEX;
         fErrorMessage = NO_MESSAGE;
     }

     public STAXElementInfo(String elementName, int elementIndex)
     {
         fElementName = elementName;
         fAttributeName = NO_ATTRIBUTE_NAME;
         fElementIndex = elementIndex;
         fErrorMessage = NO_MESSAGE;
     }

     public STAXElementInfo(String elementName, String attributeName,
                            int elementIndex)
     {
         fElementName = elementName;
         fAttributeName = attributeName;
         fElementIndex = elementIndex;
         fErrorMessage = NO_MESSAGE;
     }
     
     public String getElementName() { return fElementName; }
     public void setElementName(String elementName) { fElementName = elementName; }
    
     public String getAttributeName() { return fAttributeName; }
     public void setAttributeName(String attributeName) { fAttributeName = attributeName; }
    
     public int getElementIndex() { return fElementIndex; }
     public void setElementIndex(int elementIndex) { fElementIndex = elementIndex; }

     public String getErrorMessage() { return fErrorMessage; }
     public void setErrorMessage(String errorMessage) { fErrorMessage = errorMessage; }

     private String fElementName = new String("Unknown");
     private int    fElementIndex = 0;
     private String fAttributeName = null;
     private String fErrorMessage = null;
}
