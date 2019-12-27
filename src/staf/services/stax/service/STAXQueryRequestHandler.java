/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.STAFResult;

public interface STAXQueryRequestHandler
{
    public STAFResult handleQueryRequest(String type, String key, STAXJob job,
                                         STAXRequestSettings settings);

    public STAFResult handleQueryJobRequest(STAXJob job,
                                            STAXRequestSettings settings);
}
