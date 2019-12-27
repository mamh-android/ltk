/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.ftp;

// Upload a file via FTP, using the JDK.

import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;

import com.ibm.staf.STAFException;
import com.ibm.staf.STAFResult;

public class FTPPut
{
    private FTPClientConnection fConn;
    private final String fLocalFile;
    private final String fUrlPath;
    private static final int MAX_BUFFER_SIZE = 4096;
    
    public FTPPut(String host, String port, String user, String password,
                  String localFile, String urlPath) throws Exception
    {
        fConn = new FTPClientConnection(host, port, user, password);
        fLocalFile = localFile;
        fUrlPath = urlPath;

        BufferedInputStream in = null;
        BufferedOutputStream out = null;

        try
        {
            out = fConn.openUploadStream(fUrlPath);

            try
            {
                in = new BufferedInputStream(new FileInputStream(fLocalFile));

            }
            catch (FileNotFoundException e)
            {
                throw new STAFException(
                    STAFResult.FileReadError,
                    "Error creating an input stream for the file.\n" +
                    e.toString());
            }

            byte[] buffer = new byte[MAX_BUFFER_SIZE];
            int count = 0;
            
            try
            {
                while ((count = in.read(buffer)) != -1)
                {
                    try
                    {
                        out.write(buffer, 0, count);
                    }
                    catch (IOException e)
                    {
                        throw new STAFException(
                            STAFResult.FileWriteError,
                            "Error writing to the url path on the FTP host.\n" +
                            e.toString());
                    }
                }
            }
            catch (IOException e)
            {
                throw new STAFException(
                    STAFResult.FileReadError,
                    "Error reading from the file.\n" + e.toString());
            }
        }
        finally
        {
            if (out != null) out.close();
            if (in != null) in.close();
            fConn.close(); // section 3.2.5 of RFC1738
        }
    }
}
