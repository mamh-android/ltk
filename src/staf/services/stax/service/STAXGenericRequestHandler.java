/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFResult;

public interface STAXGenericRequestHandler
{
    public STAFResult handleRequest(Object info, STAX staxService);

    public String getHelpInfo(String lineSep);
}
