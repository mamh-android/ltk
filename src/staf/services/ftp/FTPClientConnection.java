/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.ftp;

// FTP support, using the JDK.

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.net.MalformedURLException;
import java.net.ConnectException;
import java.net.UnknownHostException;

import com.ibm.staf.STAFException;
import com.ibm.staf.STAFResult;

class FTPClientConnection
{
    private final String fHost;
    private final String fPort;
    private final String fUser;
    private final String fPassword;
    private URLConnection fUrlConn;
    
    public FTPClientConnection(String host, String port, String user, String password)
    {
        fHost = host;
        fPort = port;
        fUser = user;
        fPassword = password;
        fUrlConn = null;
    }
    
    protected URL makeURL(String urlPath)
        throws MalformedURLException, UnsupportedEncodingException
    {
        // Don't encode the remoteFile as any "/" in the directory
        // structure need to be slashes not encoded as %2F

        String urlString = "ftp://" +
            URLEncoder.encode(fUser, "UTF-8") + ":" +
            URLEncoder.encode(fPassword, "UTF-8") + "@" +
            URLEncoder.encode(fHost, "UTF-8");
         
        if (fPort != null)
        {
            urlString += ":" + URLEncoder.encode(fPort, "UTF-8");
        }

        urlString += "/" + urlPath + ";type=i";

        return new URL(urlString);
    }
    
    protected BufferedInputStream openDownloadStream(String urlPath)
        throws Exception
    {
        URL url = makeURL(urlPath);
        fUrlConn = url.openConnection();

        try
        {
            return new BufferedInputStream(fUrlConn.getInputStream());
        }
        catch (ConnectException e)
        {
            String errMsg = "Error opening a conection to FTP host '" +
                fHost + "'";

            if (fPort != null) 
                errMsg += " at port '" + fPort + "'";

            errMsg += ".\n" + e.toString();

            throw new STAFException(FTPService.kHostConnectError, errMsg);
        }
        catch (UnknownHostException e)
        {
            String errMsg = "Error opening a conection to FTP host '" +
                fHost + "'";

            if (fPort != null) 
                errMsg += " at port '" + fPort + "'";

            errMsg += ".\n" + e.toString();

            throw new STAFException(FTPService.kHostConnectError, errMsg);
        }
        catch (sun.net.ftp.FtpLoginException e)
        {
            String errMsg = "";
            
            String host = "'" + fHost + "'";

            if (fPort != null) 
                host += " at port '" + fPort + "'";

            if (fUser.equals("anonymous"))
            {
                errMsg = "Error doing an anonymous FTP to host " + host + ".";
            }
            else
            {
                errMsg = "Login error opening a connection to FTP host " +
                    host + " with user '" + fUser + "' and the specified " +
                    "password.";
            }

            throw new STAFException(
                FTPService.kHostConnectError,
                errMsg + "  Retry using a valid user and password for the " +
                "host.\n" + e.toString());
        }
        catch (FileNotFoundException e)
        {
            throw new STAFException(
                STAFResult.FileOpenError,
                "Error creating an input stream for the url path on the " +
                "FTP host,\n" + e.toString());
        }
        catch (IOException e)
        {
            throw new STAFException(
                STAFResult.FileOpenError,
                "Error creating an input stream for the url path on the " +
                "FTP host.\n" + e.toString());
        }
    }
    
    protected BufferedOutputStream openUploadStream(String urlPath) throws Exception
    {
        URL url = makeURL(urlPath);
        fUrlConn = url.openConnection();

        try
        {
            return new BufferedOutputStream(fUrlConn.getOutputStream());
        }
        catch (ConnectException e)
        {
            String errMsg = "Error opening a conection to FTP host '" +
                fHost + "'";

            if (fPort != null) 
                errMsg += " at port '" + fPort + "'";

            errMsg += ".\n" + e.toString();

            throw new STAFException(FTPService.kHostConnectError, errMsg);
        }
        catch (UnknownHostException e)
        {
            String errMsg = "Error opening a conection to FTP host '" +
                fHost + "'";

            if (fPort != null) 
                errMsg += " at port '" + fPort + "'";

            errMsg += ".\n" + e.toString();

            throw new STAFException(FTPService.kHostConnectError, errMsg);
        }
        catch (sun.net.ftp.FtpLoginException e)
        {
            String errMsg = "";
            
            String host = "'" + fHost + "'";

            if (fPort != null) 
                host += " at port '" + fPort + "'";

            if (fUser.equals("anonymous"))
            {
                errMsg = "Error doing an anonymous FTP to host " + host + ".";
            }
            else
            {
                errMsg = "Login error opening a connection to FTP host '" +
                    host + " with user '" + fUser + "' and the specified " +
                    "password.";
            }

            throw new STAFException(
                FTPService.kHostConnectError,
                errMsg + "  Retry using a valid user and password for the " +
                "host.\n" + e.toString());
        }
        catch (FileNotFoundException e)
        {
            throw new STAFException(
                STAFResult.FileWriteError,
                "Error creating an output stream for url path '" +
                urlPath + "' on the FTP host.\n" + e.toString());
        }
        catch (IOException e)
        {
            throw new STAFException(
                STAFResult.FileWriteError,
                "Error creating an output stream for url path '" +
                urlPath + "' on the FTP host.\n" + e.toString());
        }
    }
    
    protected BufferedReader getBufferedReader(String urlPath) throws Exception
    {
        URL url = makeURL(urlPath);
        
        fUrlConn = url.openConnection();

        try
        {
            return new BufferedReader(new InputStreamReader(
                fUrlConn.getInputStream()));
        }
        catch (ConnectException e)
        {
            String errMsg = "Error opening a conection to FTP host '" +
                fHost + "'";

            if (fPort != null) 
                errMsg += " at port '" + fPort + "'";

            errMsg += ".\n" + e.toString();

            throw new STAFException(FTPService.kHostConnectError, errMsg);
        }
        catch (UnknownHostException e)
        {
            String errMsg = "Error opening a conection to FTP host '" +
                fHost + "'";

            if (fPort != null) 
                errMsg += " at port '" + fPort + "'";

            errMsg += ".\n" + e.toString();

            throw new STAFException(FTPService.kHostConnectError, errMsg);
        }
        catch (sun.net.ftp.FtpLoginException e)
        {
            String errMsg = "";

            String host = "'" + fHost + "'";

            if (fPort != null) 
                host += " at port '" + fPort + "'";

            if (fUser.equals("anonymous"))
            {
                errMsg = "Error doing an anonymous FTP to host " + host + ".";
            }
            else
            {
                errMsg = "Login error opening a connection to FTP host " +
                    host + " with user '" + fUser + "' and the specified " +
                    "password.";
            }

            throw new STAFException(
                FTPService.kHostConnectError,
                errMsg + "  Retry using a valid user and password for the " +
                "host.\n" + e.toString());
        }
        catch (FileNotFoundException e)
        {
            throw new STAFException(
                STAFResult.FileOpenError,
                "Error creating an input stream for the url path " +
                "on the FTP host.\n" + e.toString());
        }
        catch (IOException e)
        {
            throw new STAFException(
                STAFResult.FileOpenError,
                "Error creating an input stream for the url path " +
                "on the FTP host.\n" + e.toString());
        }
    }
    
    protected void close()
    {
        fUrlConn = null;
    }
}

