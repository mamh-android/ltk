/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.ftp;

// Examine FTP folder, using the JDK.

import java.io.BufferedReader;
import java.io.IOException;

import com.ibm.staf.STAFException;
import com.ibm.staf.STAFResult;

public class FTPDir
{
    private FTPClientConnection fConn;
    private final String fUrlPath;
    private StringBuffer fResult;
    
    public FTPDir(String host, String port, String user, String password,
                  String urlPath) throws Exception
    {
        fConn= new FTPClientConnection(host, port, user, password);
        fResult = new StringBuffer();

        if (urlPath.endsWith("/"))
        {
            fUrlPath = urlPath;
        }
        else
        {
            // If a subdirectory is specified (e.g. /test/mySubDir) then an
            // ending / is required
            fUrlPath = urlPath + "/";
        }

        BufferedReader br = null;

        try
        {
            br = fConn.getBufferedReader(fUrlPath);
            String line = null;
            
            try
            {
                while ((line = br.readLine()) != null)
                {
                    fResult.append(line).append("\n");
                }
            }
            catch (IOException e)
            {
                throw new STAFException(
                    STAFResult.FileReadError,
                    "Error reading the url path directory information " +
                    "on the FTP host.\n" + e.toString());
            }
        }
        finally
        {
            if (br != null) br.close();
            fConn.close(); // section 3.2.5 of RFC1738            
        }
    }
    
    public StringBuffer getResult()
    {
        return fResult;
    }
}
