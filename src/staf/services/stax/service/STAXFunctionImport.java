/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2010                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

public class STAXFunctionImport
{
    public STAXFunctionImport(String file, String directory, String machine,
                              String functions)
    {
        fFile = file;
        fDirectory = directory;
        fMachine = machine;
        fFunctions = functions;
    }

    public String getFile() { return fFile; }
    public String getDirectory() { return fDirectory; }
    public String getMachine() { return fMachine; }
    public String getFunctions() { return fFunctions; }

    private String fFile = null;
    private String fDirectory = null;
    private String fMachine = null;
    private String fFunctions = null;
}
