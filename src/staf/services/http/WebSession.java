package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: WebSession                                                         */
/* Description: This class provides the handle to maintain a http session.   */
/*                                                                           */
/*****************************************************************************/

import com.ibm.staf.*;
import java.util.Vector;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.StringReader;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileWriter;

import org.w3c.dom.Document;

import com.ibm.staf.service.STAFServiceInterfaceLevel30;
import com.ibm.staf.STAFUtil;
import com.ibm.staf.service.http.html.InvalidParameterValueException;
import com.ibm.staf.service.http.html.WebLink;
import com.ibm.staf.service.http.html.WebForm;
import com.ibm.staf.service.http.html.HTMLParser;

// HTTP Client
import org.apache.commons.httpclient.*;
import org.apache.commons.httpclient.util.EncodingUtil;
import org.apache.commons.httpclient.methods.*;
import org.apache.commons.httpclient.params.HttpMethodParams;
import org.apache.commons.httpclient.auth.AuthScope;
import org.apache.commons.httpclient.methods.multipart.*;

// nekoHTML
import org.cyberneko.html.parsers.DOMParser;
import org.w3c.dom.html.HTMLDocument;

// xerces
import org.xml.sax.SAXException;
import org.xml.sax.InputSource;

public class WebSession 
{
    // These constants define the type of identification of the page element to
    //  be accessed.
    public static final int NAME_ID_TYPE = 0;
    public static final int ID_ID_TYPE = 1;
    public static final int INDEX_ID_TYPE = 2;
    
    // These constants define the parts of the summary
    public static final String ID = "ID";
    public static final String URL = "URL";
    public static final String TITLE = "TITLE";
    public static final String POLICY = "POLICY";
    public static final String OWNER_INSTANCE_UUID = "OWNER_INSTANCE_UUID";
    public static final String OWNER_MACHINE = "OWNER_MACHINE";
    public static final String OWNER_HANDLE_NAME = "OWNER_HANDLE_NAME";
    public static final String OWNER_HANDLE = "OWNER_HANDLE";
    public static final String PARSE_CONTENT = "PARSE_CONTENT";
    public static final String HTTP_PROXY_HOST = "HTTP_PROXY_HOST";
    public static final String HTTP_PROXY_PORT = "HTTP_PROXY_PORT";
    
    // These constants are used to define the hash map of required components 
    //  of a requestMethod call    
    public static final String REQUEST_PARAMETERS = "PARAMETERS";
    public static final String REQUEST_FILES = "FILES";
    public static final String REQUEST_HEADERS = "HEADERS";
    public static final String REQUEST_METHOD = "METHOD";
    public static final String REQUEST_URL = "URL";
    public static final String REQUEST_CONTENT = "CONTENT";
    public static final String REQUEST_REDIRECT = "AUTO REDIRECT";

    public static final String RETURN_STATUS_CODE = "RETURN STATUS CODE";
    public static final String RETURN_STATUS_MESSAGE = "RETURN STATUS MSG";
    public static final String RETURN_HEADERS = "RETURN HEADERS";

    // These constants define the valid values for PARSECONTENT option
    public static final String ENABLED = "Enabled";
    public static final String DISABLED = "Disabled";
    public static final String AUTODETECT = "AutoDetect";

    protected int id;
    protected String ownerInstanceUUID;
    protected String ownerMachine;
    protected String ownerHandleName;
    protected int ownerHandle;
    protected HTTP httpService;
    protected HttpClient session;
    protected HttpMethodBase lastRequest;
    protected HTMLParser parser;
    protected HashMap defaultHeaders;
    protected HashMap authenticationSets;
    protected boolean followRedirect;
    protected String fParseContent;
    protected String currentContents;
    protected String currentContentsFileName;
    
    // true means that a temporary session was opened for a single request
    // (e.g. the SESSION option wasn't specified on the request)
    protected boolean tempSession;

    private static final int BUFF_SIZE = 4096;
    private static final int MAX_STRING_SIZE = 51200;
    
/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/* Parameter: parentList - session list that this session is to be a part of */
/*                         it is required to determine the id of this session*/
/*            info - request info                                            */
/*                                                                           */
/*****************************************************************************/    

    public WebSession(SessionList parentList,
                      STAFServiceInterfaceLevel30.RequestInfo info,
                      HTTP httpService) 
    {
        this(parentList, info, httpService, false);
    }

    public WebSession(SessionList parentList,
                      STAFServiceInterfaceLevel30.RequestInfo info,
                      HTTP httpService, boolean tempSession)
    {
        this.ownerInstanceUUID = info.stafInstanceUUID;
        this.ownerMachine = info.endpoint;
        this.ownerHandleName = info.handleName;
        this.ownerHandle = info.handle;
        this.httpService = httpService;
        this.tempSession = tempSession;
        id = parentList.addSession(this);
        session = new HttpClient();

        lastRequest = null;
        parser = new HTMLParser();
        defaultHeaders = new HashMap();
        authenticationSets = new HashMap();
        followRedirect = false;
        fParseContent = AUTODETECT;
        currentContents = "";
        currentContentsFileName = "";
    }

    public void releaseConnection()
    {
        currentContents = "";
        
        if (!currentContentsFileName.equals(""))
        {
            // Delete temporary file
            (new File(currentContentsFileName)).delete();

            currentContentsFileName = "";
        }

        if (lastRequest != null)
        {
            // Must call releaseConnection to release resources
            lastRequest.releaseConnection();
            lastRequest = null;
        }

        // Close any idle connections.  This is required to close the socket
        // so it doesn't remain open in a CLOSE_WAIT state.

        session.getHttpConnectionManager().closeIdleConnections(0);

        try
        {
            parser.setContent(HTMLParser.EMPTY_DOC);
        }
        catch (Exception e)
        {
           // HTMLParser.EMPTY_DOC is safe and will not cause an exception
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getID                                                             */
/* Description: gets the session id of this WebSesson.                       */
/* Returns: the ID which identifies the session in the list                  */
/*                                                                           */
/*****************************************************************************/    

    public int getID() 
    {
        return id;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getOwnerMachine                                                   */
/* Description: gets the owner machine for this WebSesson.                   */
/* Returns: the machine for the owner of this session                        */
/*                                                                           */
/*****************************************************************************/    

    public String getOwnerMachine() 
    {
        return ownerMachine;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getOwnerHandleName                                                */
/* Description: gets the owner handle name for this WebSesson.               */
/* Returns: the handle name for the owner of this session                    */
/*                                                                           */
/*****************************************************************************/    

    public String getOwnerHandleName() 
    {
        return ownerHandleName;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getOwnerHandle                                                    */
/* Description: gets the owner handle for this WebSesson.                    */
/* Returns: the handle for the owner of this session                         */
/*                                                                           */
/*****************************************************************************/    

    public int getOwnerHandle() 
    {
        return ownerHandle;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentContentsAsStream                                        */
/* Description: get the contents returned by the last interaction            */
/* Parameters: none                                                          */
/* Returns: the contents of the last interaction in an InputStream.          */
/*          This is the html code for the current page if the last action    */
/*          was a goto or link.  Otherwise it is the message generated by    */
/*          the METHOD call.                                                 */
/*                                                                           */
/*****************************************************************************/

    public InputStream getCurrentContentsAsStream() throws IOException
    {
        if (HTTP.DEBUG)
            System.out.println("In WebSession.getResponseBodyAsStream()");

        try
        {
            return lastRequest.getResponseBodyAsStream();
        }
        catch (NullPointerException e)
        {
            // No previous requests
        }

        // Return null if an error occurs
        return null;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentContents                                                */
/* Description: get the contents returned by the last interaction            */
/* Parameters: none                                                          */
/* Returns: the contents of the last interaction in a String.                */
/*          This is the html code for the current page if the last action    */
/*          was a goto or link. Otherwise it is the message generated by the */
/*          METHOD call.                                                     */
/*                                                                           */
/*****************************************************************************/

    public String getCurrentContents() throws STAFException,
                                              ContentTooLargeException
    {
        if (HTTP.DEBUG)
            System.out.println("In WebSession.getCurrentContents()");

        // Get content from where setCurrentContent stored it.
        // If content is small, the content is stored in a string.
        // If content is large, the content is stored in a temporary file.

        if (currentContentsFileName.equals(""))
        {
            // Current contents <= 50k are stored in a string

            if (HTTP.DEBUG)
                System.out.println("Current contents in string since < 50k");

            return currentContents;
        }
        else
        {
            // Current contents > 50k are stored in a temporary file
            // Read from the file specified by currentContentsFileName

            if (HTTP.DEBUG)
                System.out.println("Current contents in temp file " +
                                   currentContentsFileName);

            StringBuffer output = new StringBuffer();
            InputStream instream = null;

            final byte[] buffer = new byte[BUFF_SIZE];

            try
            {
                instream = new DataInputStream(new FileInputStream(
                    currentContentsFileName));

                // Read InputStream

                while (true)
                {
                    int amountRead = instream.read(buffer);

                    if (amountRead == -1)
                    {
                        break;
                    }

                    // Write to a string
                    output.append(new String(buffer, 0, amountRead));
                } 
            }
            catch(IOException e)
            {
                output = new StringBuffer(HTMLParser.EMPTY_DOC);

                throw new STAFException(
                     STAFResult.FileReadError,
                    "Error getting current contents.\n" + e.toString());
            }
            catch(OutOfMemoryError e)
            {
                output = new StringBuffer();
                
                throw new ContentTooLargeException(
                   "Getting the session content caused an OutOfMemoryError " +
                   "because it is too large. Recommend redirecting the content " +
                   "to a file using the FILE option or, if the content is not " +
                   "needed, you may be able to specify the RETURNNOCONTENT " +
                   "option depending on the request.");
            }
            finally
            {
                try
                {
                    if (instream != null) instream.close();
                }
                catch(IOException e)
                {
                    throw new STAFException(
                        STAFResult.FileReadError,
                        "Error getting current contents while closing " +
                        "stream.\n" + e.toString());
                }
            }
            
            if (HTTP.DEBUG)
                System.out.println("WebSession.getCurrentContents() return");

            return output.toString();
        }
    }
    

/*****************************************************************************/
/*                                                                           */
/* Method: setCurrentContents                                                */
/* Description: Set the current contents for the last interaction by reading */
/*          from the input stream.  If temporary file (e.g. fileName == null)*/
/*          and contents <= 50K, store the contents in a string.  Otherwise, */
/*          store the contents in a file.                                    */
/*          Also, set the content for the HTML parser if the contents for    */
/*          the last interaction is text/html.                               */
/* Parameters: instream - input stream containing the contents for the last  */
/*                        interaction for this session.                      */
/*                                                                           */
/*****************************************************************************/
    public void setCurrentContents(InputStream instream,
                                   String fileName, String machine)
        throws STAFException, IOException
    {
        if (HTTP.DEBUG)
            System.out.println("In WebSession.setCurrentContents()");

        currentContents = "";
        currentContentsFileName = "";
        
        if (instream == null) return;
        
        String localFileName = httpService.getTempDir() +
            "session" + id + ".tmp";
        
        // Write the result string as a stream of raw bytes (e.g.
        // binary mode)

        BufferedInputStream bis = null;
        OutputStream outstream = null;
        StringBuffer output = new StringBuffer();
        long totalRead = 0;
        final byte[] buffer = new byte[BUFF_SIZE];

        // Read content from input stream and write content to a string if
        // fileName is not specified (e.g. is null) or if the content stream
        // size is small (e.g. <= MAX_STRING_SIZE). Otherwise, write the
        // content to a file.

        try
        {
            bis = new BufferedInputStream(instream);

            outstream = new DataOutputStream(new FileOutputStream(
                localFileName));

            while (true)
            {
                int amountRead = bis.read(buffer);

                if (amountRead == -1) break;

                if ((fileName == null) && (totalRead <= MAX_STRING_SIZE))
                {
                    // Write to a string
                    output.append(new String(buffer, 0, amountRead));
                }

                // Write to a file
                outstream.write(buffer, 0, amountRead);

                totalRead += amountRead;
            }

            if (HTTP.DEBUG)
                System.out.println("totalRead=" + totalRead);
        }
        catch(IOException e)
        {
            // Delete the temporary file before throwing the exception

            if (outstream != null) outstream.close();
            (new File(localFileName)).delete();
            
            throw new STAFException(
                 STAFResult.FileWriteError,
                "Error setting current contents.\n" + e.toString());
        }
        finally
        {
            try
            {
                if (bis != null) bis.close();
                if (outstream != null) outstream.close();
            }
            catch(IOException e)
            {
                (new File(localFileName)).delete();

                throw new STAFException(
                    STAFResult.FileWriteError,
                    "Error setting current contents (when closing " +
                    "streams).\n" + e.toString());
            }
        }

        if ((fileName == null) && (totalRead <= MAX_STRING_SIZE))
        {
            // Contents stored in a string

            if (HTTP.DEBUG)
                System.out.println("Contents stored in string");

            currentContents = output.toString();
            
            // Make sure the local temporary file is deleted

            (new File(localFileName)).delete();
        }
        else
        {
            // Contents stored in a file
            
            if (HTTP.DEBUG)
                System.out.println("Contents stored in file " + localFileName);

            currentContentsFileName = localFileName;
            output = new StringBuffer("");
        }
                    
        // Determine if should parse the content as HTML or not

        boolean parseContent = false;

        if (tempSession)
        {
            // If temporary session, don't parse the content as HTML (since it
            // is of no value since won't be able to access the parsed HTML
            // content)

            if (HTTP.DEBUG)
                System.out.println(
                    "Don't parse HTML content since temporary session");

            // Return since don't need to set content to EMPTY_DOC
            return;
        }

        if (fParseContent.equals(AUTODETECT))
        {
            // Auto-detect whether should parse the content as HTML by checking
            // if the "Content-Type" header set by the Web Server contains
            // "text/html".  If so, then parse the content as HTML.

            Header contentTypeHeader = lastRequest.getResponseHeader(
                "Content-Type");

            if (contentTypeHeader != null)
            {
                String contentType = contentTypeHeader.getValue();

                if (HTTP.DEBUG)
                    System.out.println(
                        "Autodetect whether to parse content based on " +
                        "content type.  Content type: " + contentType);

                if ((contentType != null) &&
                    (contentType.indexOf("text/html") >= 0))
                    parseContent = true;
            }
        }
        else if (fParseContent.equals(ENABLED))
        {
            parseContent = true;
        }
        
        if (HTTP.DEBUG)
            System.out.println("Parse HTML content: " + parseContent);

        // If should parse the content as HTML, set the parser content to the
        // current contents; otherwise, set the parser content to EMPTY_DOC

        try
        {
            if (parseContent)
            {
                if (!currentContents.equals(""))
                    parser.setContent(currentContents);
                else
                    parser.setContent(
                        new FileInputStream(currentContentsFileName));
            }
            else
            {
                parser.setContent(HTMLParser.EMPTY_DOC);
            }
        }
        catch (Exception e)
        {
            try
            {
                parser.setContent(HTMLParser.EMPTY_DOC);
            }
            catch (Exception ex)
            {
                // Do nothing. EMPTY_DOC is error free
            }
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: writeContentToFile                                                */
/* Description: utility method to write the current content to a file        */
/* Parameters: filename    - string representation of a file                 */
/*             machineName - name of the machine to write the file to        */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/

public void writeContentToFile(String toFile, String toMachine)
    throws IOException, STAFException
{
    if (HTTP.DEBUG)
        System.out.println("WebSession.writeContentToFile()");

    // Store name for file redirect
    String fName = toFile;

    if (currentContentsFileName.equals(""))
    {
        // Current contents reside in a string.  Write contents to a file.

        String fileName = toFile;

        if (toMachine != null)
        {
            // Create a temporary file name to use to store the content in
            fileName = httpService.getTempDir() + "file" +
                httpService.getTempFileCount() + ".tmp";
        }

        File outFile = new File(fileName);
        FileWriter out = new FileWriter(outFile);
        out.write(currentContents);
        out.close();

        if (toMachine == null)
        {
            // The contents have already been written to the specified file
            // on the local machine
            return;
        }
        
        // Copy the temporary file on the local machine to the specified file
        // on the specified machine.

        String request = "COPY FILE " + fileName +
            " TOFILE " + fName + " TOMACHINE " + toMachine;

        if (HTTP.DEBUG)
            System.out.println("STAF local FS " + request);

        STAFResult res = httpService.getServiceHandle().submit2(
            "local", "FS", request);

        // Delete the temporary local file

        (new File(fileName)).delete();

        if (res.rc != 0)
        {
            String errMsg = "FS " + request + " failed with RC=" + res.rc +
                " Result=" + res.result;
            throw new STAFException(res.rc, errMsg);
        }
    }
    else
    {
        // Current contents reside in a file on the local service machine

        String request = "";

        if (toMachine == null)
        {
            // Copy the contents file to a file on the local service machine
            request = "COPY FILE " + currentContentsFileName +
                " TOFILE " + fName;
        }
        else
        {
            // Copy the contents file to a file on the specified machine
            request = "COPY FILE " + currentContentsFileName +
                " TOFILE " + fName + " TOMACHINE " + toMachine;
        }

        if (HTTP.DEBUG)
            System.out.println("STAF local FS " + request);

        STAFResult res = httpService.getServiceHandle().submit2(
            "local", "FS", request);

        if (res.rc != 0)
        {
            String errMsg = "FS " + request + " failed with RC=" + res.rc +
                " Result=" + res.result;
            throw new STAFException(res.rc, errMsg);
        }
    }
    
    return;
}

/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentTitle                                                   */
/* Description: get the title of the current page of the session.            */
/* Parameters: none                                                          */
/* Returns: the title of the current page of the session.  If there is no    */
/*          title, an empty string is returned.                              */
/*                                                                           */
/*****************************************************************************/    

    public String getCurrentTitle()
    {
        try
        {
            return parser.getTitle();
        }
        catch (Exception e)
        {
            // Ignore errors (like DOMExceptions, NullPointerException) when
            // getting the title to so that a QUERY SESSION or LIST SESSIONS
            // request won't fail just because could not get the title for a
            // session.
            return "";
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentUrl                                                     */
/* Description: get the url of the current page of the session.              */
/* Parameters: none                                                          */
/* Returns: the current url                                                  */
/*                                                                           */
/*****************************************************************************/    
    
    public String getCurrentUrl() 
    {
        try{
            String url = lastRequest.getURI().getURI();
            return url;
        }catch (NullPointerException e){
            // no previous requests
        }catch (URIException e){
        }
        
        return "";
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentStatusCode                                              */
/* Description: get the status code of the last request of the session.      */
/* Parameters: none                                                          */
/* Returns: the current status code                                          */
/*                                                                           */
/*****************************************************************************/    
    
    public int getCurrentStatusCode()
    {    
        try{
            return lastRequest.getStatusCode();
        }catch (NullPointerException e){
            // no previous requests
        }
        return -1;
    }    
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentStatusText                                              */
/* Description: get the status message of the last request of the session.   */
/* Parameters: none                                                          */
/* Returns: the current status message                                       */
/*                                                                           */
/*****************************************************************************/    
    
    public String getCurrentStatusText()
    {    
        try{
            return lastRequest.getStatusText();
        }catch (NullPointerException e){
            // no previous requests
        }
        return "";
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCurrentStatus                                                  */
/* Description: get the status of the last request of the session.           */
/* Parameters: none                                                          */
/* Returns: the current status                                               */
/*                                                                           */
/*****************************************************************************/    
    
    public String getCurrentStatus() 
    {
        
        int rc = getCurrentStatusCode();
        String rm = getCurrentStatusText();
        return rc + "\n" + rm;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCookiePolicy                                                   */
/* Description: get the cookie handling policy for this session.             */
/* Parameters: none                                                          */
/* Returns: the cookie handling policy                                       */
/*                                                                           */
/*****************************************************************************/    
    
    public String getCookiePolicy()
    {
        return session.getParams().getCookiePolicy();
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: setCookiePolicy                                                   */
/* Description: set the cookie handling policy for this session.  If an      */
/*              invalid policy is selected the DEFAULT policy is set.        */
/*              Valid types: NETSCAPE, RFC2109, BROWSER, IGNORE              */
/*              Default policy is RFC2109.                                   */
/* Parameters: policy - the new cookie handling policy for the session.      */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/
        
    public void setCookiePolicy(String policy)
    {
        CookieAccess.setCookiePolicy(policy, session);
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCookieNames                                                    */
/* Description: get a list of cookies associated with this session.          */
/* Parameters: none                                                          */
/* Returns: a list of cookies associated with this session                   */
/*                                                                           */
/*****************************************************************************/    

    public Vector getCookieNames()
    {
        return CookieAccess.getCookieNames(session);
    }
        
/*****************************************************************************/
/*                                                                           */
/* Method: getCookieValue                                                    */
/* Description: get the value of the cookie with the specified name.         */
/*              This may need to change to get cookie summary to obtain      */
/*              domain and path information about a specific cookie.         */
/* Parameters: name - the name of the cookie                                 */
/* Returns: the value of the specified cookie.                               */
/*                                                                           */
/*****************************************************************************/    

    public String getCookieValue(String name) throws InvalidCookieIDException 
    {
        return CookieAccess.getCookieValue(name, session);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setCookieValue                                                    */
/* Description: sets the value of the specifed cookie to the specified value */
/* Parameters: name - name of the cookie                                     */
/*             value - new value for the cookie                              */
/* Returns: void                                                             */
/* Throws: InvalidCookieIDException if the cookie does not exist             */
/*                                                                           */
/*****************************************************************************/    
    
    public void setCookieValue (String name, String value) 
                                throws InvalidCookieIDException
    {
        CookieAccess.setCookieValue(name, value, session);
                                    
    }

/*****************************************************************************/
/*                                                                           */
/* Method: addCookie                                                         */
/* Description: Add a new cookie to the session.  This may need more         */
/*              parameters if path and domain are to be specified.           */
/* Parameters: name - name of the cookie                                     */
/*             value - new value for the cookie                              */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public void addCookie(String name, String value) 
    {
        CookieAccess.addCookie(CookieAccess.createCookie(name,value), session);                           
    }

/*****************************************************************************/
/*                                                                           */
/* Method: deleteCookie                                                      */
/* Description: Delete the specified cookie from the session.  If the cookie */
/*              does not exist, no error is declared.                        */
/* Parameters: name - name of the cookie                                     */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public void deleteCookie(String name) throws InvalidCookieIDException
    {
        CookieAccess.deleteCookie(name, session);
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: summarizeCookie                                                   */
/* Description: Get the information about a cookie.  If the cookie is not in */
/*              the session throw an InvalidElementException.                */
/* Parameters: name - name of the cookie                                     */
/* Returns: description of the cookie                                        */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap summarizeCookie(String name) throws
                            InvalidCookieIDException
    {    
        return CookieAccess.getCookieSummary(name, session);
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: listCookies                                                       */
/* Description: return a summary of all cookies in the current page          */
/* Parameters: None                                                          */
/* Returns: the summary of all cookies in the current page                   */
/*                                                                           */
/*****************************************************************************/
    public HashMap[] listCookies()
    {    
        return CookieAccess.listCookies(session);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: findLink                                                          */
/* Description: Find the specified link in the current page.  If the link is */
/*              not in the page or an invalid id type or an invalid value    */
/*              was specified throw an InvalidElementException.              */
/* Parameters: linkID - the identified for the link                          */
/*             idType - the type of id used to identify the link             */
/*                      ID_ID_TYPE and NAME_ID_TYPE are accepted             */
/* Returns: the link                                                         */
/*                                                                           */
/*****************************************************************************/    

    protected WebLink findLink(String linkID, int idType)
                                throws InvalidElementIDException
    {
        if (idType == ID_ID_TYPE)
            return parser.getLinkByID(linkID);

        if (idType == NAME_ID_TYPE)
            return parser.getLinkByName(linkID);

        if (idType == INDEX_ID_TYPE)
        {
            // Make sure specified index is numeric
            try
            {
                int index = (new Integer(linkID)).intValue();
            }catch (NumberFormatException e)
            {
                throw new InvalidElementIDException(
                    linkID, "LINK INDEX " + linkID + " is not an integer");
            }

            return parser.getLinkByIndex(Integer.parseInt(linkID));
        }
        
        throw new InvalidElementIDException(linkID, "Bad ID Type.");
    }

/*****************************************************************************/
/*                                                                           */
/* Method: summarizeLink                                                     */
/* Description: Get the information about a link.  If the link is not in the */
/*              page or an invalid id type was specified throw an            */
/*              InvalidElementException.                                     */
/* Parameters: linkID - the identified for the link                          */
/*             idType - the type of id used to identify the link             */
/*                      ID_ID_TYPE and NAME_ID_TYPE are accepted             */
/* Returns: description of the link.                                         */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap summarizeLink(String linkID, int idType) throws
                            InvalidElementIDException
    {    
        WebLink link = findLink(linkID, idType);
        
        try 
        {
            return link.getSummary();
                
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (linkID, "Link " + linkID);
        }
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: listLinks                                                         */
/* Description: return a summary of all links in the current page            */
/* Parameters: None                                                          */
/* Returns: the summary of all links in the current page                     */
/*                                                                           */
/*****************************************************************************/
    public HashMap[] listLinks()
    {    
        return parser.listLinks();
    }

/*****************************************************************************/
/*                                                                           */
/* Method: followLink                                                        */
/* Description: Send the session to the target of the specified link in the  */
/*              current page.  If the link is not in the page or an invalid  */
/*              id type was specified throw an InvalidElementException.      */
/* Parameters: linkID - the identified for the link                          */
/*             idType - the type of id used to identify the link             */
/*                      ID_ID_TYPE and NAME_ID_TYPE are accepted             */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void followLink(String linkID, int idType, Boolean redirect) throws
        InvalidElementIDException, MalformedURLException, IOException,
        STAFException
    {    
        WebLink link = findLink(linkID, idType);
        
        try 
        {
            HashMap request = link.getRequest();
            request.put(REQUEST_REDIRECT, redirect);
            
            requestMethod(request);
                
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (linkID, "Link " + linkID);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: findForm                                                          */
/* Description: Find the specified form in the current page.  If the form is */
/*              not in the page or an invalid id type was specified throw an */
/*              InvalidElementException.                                     */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/* Returns: the form                                                         */
/*                                                                           */
/*****************************************************************************/    

    protected WebForm findForm(String formID, int idType)
                                throws InvalidElementIDException
    {
        if (idType == NAME_ID_TYPE)
            return parser.getFormByName(formID);
            
        if (idType == ID_ID_TYPE)
            return parser.getFormByID(formID);
    
        if (idType == INDEX_ID_TYPE)
        {
            // Make sure specified index is numeric
            try
            {
                int index = (new Integer(formID)).intValue();
            }catch (NumberFormatException e)
            {
                throw new InvalidElementIDException(
                    formID, "FORM INDEX " + formID + " is not an integer");
            }

            return parser.getFormByIndex(Integer.parseInt(formID));
        }

        throw new InvalidElementIDException(formID, "Bad ID Type.");
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getFormControlIDs                                                 */
/* Description: Get the control ids in the specified form in the current     */
/*              page.  If the form is not in the page or an invalid id type  */
/*              was specified throw an InvalidElementException.              */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/* Returns: a list of control ID's                                           */
/*                                                                           */
/*****************************************************************************/    

    public String[] getFormControlIDs(String formID, int idType)
                                   throws InvalidElementIDException
    {
        WebForm form = findForm(formID, idType);
        return form.getParameterKeyList();
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setFormElement                                                    */
/* Description: Set the value of the specified control in the specified form */
/*              in the current page.  If the form is not in the page or an   */
/*              invalid id type was specified or an invalid control name was */
/*              specified throw an InvalidElementException.                  */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/*             elementID - the id of the control                             */
/*             value - the value ot assign to the control                    */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public void setFormElement(String formID, int idType,    String elementID,
                                String value) throws InvalidElementIDException,
                                                InvalidParameterValueException
    {
        WebForm form = findForm(formID, idType);
        
        try 
        {
            form.setParameterValue(elementID, value);
                
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (formID, "Form " + formID);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: summarizeForm                                                     */
/* Description: Get the information about a form.  If the form is not in the */
/*              page or an invalid id type was specified throw an            */
/*              InvalidElementException.                                     */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/* Returns: description of the form.                                         */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap summarizeForm(String formID, int idType) throws
                            InvalidElementIDException
    {    
        WebForm form = findForm (formID, idType);
        
        try 
        {
            return form.getSummary();
                
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (formID, "Form " + formID);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: summarizeFormElement                                              */
/* Description: Get the information about a form control.  If the form is not*/
/*              in the page or an invalid id type was specified throw an     */
/*              InvalidElementException.                                     */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/*             formControlKey - the key used to identify the form contorl    */
/* Returns: description of the form.                                         */
/*                                                                           */
/*****************************************************************************/    
    
    public HashMap summarizeFormControl(String formID, int idType, 
                                        String formControlKey) throws
                            InvalidElementIDException
    {    
        WebForm form = findForm (formID, idType);
        
        try 
        {
            return form.getParameterSummary(formControlKey);
                
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (formID, "Form " + formID);
        }
    }    
/*****************************************************************************/
/*                                                                           */
/* Method: listForms                                                         */
/* Description: return a summary of all forms in the current page            */
/* Parameters: None                                                          */
/* Returns: the summary of all forms in the current page                     */
/*                                                                           */
/*****************************************************************************/
    public HashMap[] listForms()
    {    
        return parser.listForms();
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getFormElement                                                    */
/* Description: Get the value of the specified control in the specified form */
/*              in the current page.  If the form is not in the page or an   */
/*              invalid id type was specified or an invalid control name was */
/*              specified throw an InvalidElementException.                  */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/*             elementID - the id of the control                             */
/* Returns: the value of the specified control                               */
/*                                                                           */
/*****************************************************************************/    
    
    public String getFormElement(String formID, int idType,    String elementID) 
                                throws InvalidElementIDException
    {
        String value = null;
        
        WebForm form = findForm(formID, idType);
        
        try 
        {
            value = form.getParameterValue(elementID);
            
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (formID, "Form " + formID);
        }
        
        return value;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: submitForm                                                        */
/* Description: Submit the specified form in the current page.  If the form  */
/*              is not in the page or an invalid id type was specified       */
/*              specified throw an InvalidElementException.                  */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void submitForm(String formID, int idType, Boolean redirect) throws
        MalformedURLException, IOException, FileNotFoundException,
        InvalidElementIDException, STAFException
    {
        WebForm form = findForm(formID, idType);
        
        try 
        {
            HashMap request = form.getRequest();
            request.put(REQUEST_REDIRECT, redirect);
            
            requestMethod(request);
             
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (formID, "Form " + formID);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: resetForm                                                         */
/* Description: Reset the specified form in the current page.  If the form   */
/*              is not in the page or an invalid id type was specified       */
/*              specified throw an InvalidElementException.                  */
/* Parameters: formID - the identified for the form                          */
/*             idType - the type of id used to identify the form             */
/*                      ID_ID_TYPE INDEX_ID_TYPE and NAME_ID_TYPE are        */
/*                      accepted                                             */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public void resetForm(String formID, int idType) 
                            throws InvalidElementIDException
    {            
        WebForm form = findForm(formID, idType);
        
        try 
        {
            form.reset();
            
        }catch (NullPointerException e)
        {
            throw new InvalidElementIDException (formID, "Form " + formID);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setRequestHeader                                                  */
/* Description: Set the request to include the header specified.  If the     */
/*              header is already part of the request it is replaced by the  */
/*              new value.  If it is not present it is added.                */
/* Parameters: request - this is the request for the header                  */
/*             header - this is the header to be set                         */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    protected void setRequestHeader(HttpMethodBase request, 
                                      Header header) 
    {
        // this is a case insensitive match
        Header h = request.getRequestHeader(header.getName());
        
        if (h == null)
            request.addRequestHeader(header);
        else
            request.setRequestHeader(header);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: requestMethod                                                     */
/* Description: submit the http request described                            */
/* Parameters: data - hash map containing the components of the request      */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public void requestMethod(HashMap data)throws MalformedURLException, 
        IOException, FileNotFoundException, STAFException
                                                       
    {
        requestMethod((String)data.get(REQUEST_URL), 
                      (String)data.get(REQUEST_METHOD),
                      (HashMap)data.get(REQUEST_HEADERS), 
                      (Vector)data.get(REQUEST_PARAMETERS),
                      (Vector)data.get(REQUEST_FILES), 
                      (String)data.get(REQUEST_CONTENT), 
                      (Boolean)data.get(REQUEST_REDIRECT),
                      null, null, false);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: requestMethod                                                     */
/* Description: submit the http request described                            */
/* Parameters: targetURL - the target url for this request                   */
/*             method - the method of the http request                       */
/*             headers - the headers to submit with the http request         */
/*             params - the parameters to submit with the http request       */
/*             files - a list of file names and associed parameter names to  */
/*                     submit with the http request                          */
/*             content - the body of the http request if no files or         */
/*                       parameters were specified                           */
/*             redirect - indicates whether to follow redirects              */
/*             toFile - specifies the fully-qualified name of a file where   */
/*                      the response should be stored (or null if not        */
/*                      specified)                                           */
/*             toMachine - specifies the machine that should be used to      */
/*                         store the toFile (or null if not specified)       */
/*             urlEncoded - specifies whether the targetURL is already       */
/*                          escape-encoded (aka percent-encoded)             */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void requestMethod(String targetURL, String method, 
                                      HashMap headers, Vector params,
                                      Vector files, String content,
                                      Boolean redirect,
                                      String toFile, String toMachine,
                                      boolean urlEncoded)
    throws MalformedURLException, IOException, FileNotFoundException,
        STAFException
    {
        // clean up the url and make it absolute
        targetURL = resolveUrl(targetURL, urlEncoded);
        
        // check to see if pre-emptive authentication is possible.
        session.getParams().setAuthenticationPreemptive(false);
        // strip off the protocol
        String host = targetURL.substring(targetURL.indexOf("://") + 3);
        
        Iterator it = authenticationSets.keySet().iterator();
        
        // match the current host with one in the list of automatic authenticators
        //  if it exists
        while(it.hasNext())
        {
            String key = (String) it.next();
            if (host.startsWith(key))
            try
            {
                session.getParams().setAuthenticationPreemptive(true);
                String [] values = getAuthenticationSetValues(key);
                Credentials creds = new NTCredentials
                                        (values[0], values[1], key, values[2]);

                //session.getState().setCredentials(null, key, creds);    
                session.getState().setCredentials(new AuthScope(
                    key, AuthScope.ANY_PORT, AuthScope.ANY_REALM), creds);
            }
            catch (InvalidElementIDException e)
            {
                /* do nothing this won't be thrown since key is gotten from a list of
                existing keys */
            }
        }
        
        releaseConnection();
        
        // purge expired cookies
        java.util.Date now = new java.util.Date();
        CookieAccess.purgeExpiredCookies(now, session);
        
        // create request
        try{
            if (method.equalsIgnoreCase("GET"))
            {
                lastRequest = new GetMethod (targetURL);
                // allow for parameters
                if (content == null)
                    content = "";
                content = "?" + content;
            }
            else if (method.equalsIgnoreCase("HEAD"))
                lastRequest = new HeadMethod (targetURL);
            else if (method.equalsIgnoreCase("TRACE"))
                lastRequest = new TraceMethod (targetURL);
            else if (method.equalsIgnoreCase("OPTIONS"))
                lastRequest = new OptionsMethod (targetURL);
            else if (method.equalsIgnoreCase("DELETE"))
                lastRequest = new DeleteMethod (targetURL);
            else if (method.equalsIgnoreCase("PUT"))
                lastRequest = new PutMethod (targetURL);
            else if (method.equalsIgnoreCase("POST"))
                lastRequest = new PostMethod (targetURL);
        }catch (java.lang.IllegalArgumentException e)
        {
            // - when URI is invalid
            throw new MalformedURLException("Invalid URI " + targetURL);
        } 
        catch(java.lang.IllegalStateException e)
        {
            //- when protocol of the absolute URI is not recognised
            throw new MalformedURLException("Invalid protocol " + targetURL);
        }        
        
        if (lastRequest == null)
            throw new MalformedURLException("Bad Method");
        
        // set default headers
        it = defaultHeaders.keySet().iterator();
        while(it.hasNext()) 
        {
            String key = (String) it.next();
            String value = (String) defaultHeaders.get(key);
            setRequestHeader(lastRequest, new Header(key, value));
        }
            
        // set requested headers
        if (headers != null)
        {    
            it = headers.keySet().iterator();
            
            while(it.hasNext()) 
            {
                String key = (String) it.next();
                String value = (String) headers.get(key);
                setRequestHeader(lastRequest, new Header(key, value));
            }
        }    
        
        // Add parameters

        NameValuePair[] pairs = null;

        if (params != null)
        {
            int numParams = params.size();
            pairs = new NameValuePair[numParams];

            for (int i = 0; i < numParams; i++)
            {
                Vector pair = (Vector)params.elementAt(i);
                String key = (String)pair.elementAt(0);
                String value = STAFUtil.removePrivacyDelimiters(
                    (String)pair.elementAt(1));

                if (PostMethod.class == lastRequest.getClass())
                {
                    ((PostMethod)lastRequest).addParameter(key, value);
                }
                else
                {
                    // Need to url encode key and value
                    pairs[i] = new NameValuePair (key, value);     
                }
            }
        }

        // Add files, parameters, or content

        try
        {
            if (files != null)
            {
                // This indicates a Multipart POST method

                // The Multipart POST method uses a different request body
                // format (e.g. MultipartRequestEntity) than a POST method.

                int numFiles = files.size();
                Part[] parts = new Part[numFiles];

                // Add files to parts array

                for (int i = 0; i < numFiles; i++)
                {
                    Vector pair = (Vector)files.elementAt(i);
                    String key = (String)pair.elementAt(0);
                    String value = (String)pair.elementAt(1);

                    // Note: The key is not currently used.
                    // I think Blake used a key in case support for
                    // CONTENTFILEMACHINE was added in the future so that the
                    // key could be used to match a CONTENTFILE option to a
                    // CONTENTFILEMACHINE option.
                    
                    try
                    {
                        parts[i] = new FilePart(value, new File(value));
                    }
                    catch (FileNotFoundException e)
                    {
                        // Error in getting file to be added to a post

                        String errMsg = " specified for INPUT FILE form " +
                            "control:\n";

                        if (value.equals(""))
                            errMsg = "No value" + errMsg;
                        else
                            errMsg = "Invalid value" + errMsg;
                        
                        throw new FileNotFoundException(
                            errMsg + "  <INPUT type=\"file\" name=\"" + key +
                            "\" value=\"" + value + "\" ...>" +
                            "\nFileNotFoundException message: " +
                            e.getMessage());
                    }        
                }

                ((PostMethod)lastRequest).setRequestEntity(
                    new MultipartRequestEntity(parts, lastRequest.getParams()));
            }
            else if ((params != null) &&
                     (GetMethod.class == lastRequest.getClass()))
            {
                try
                {
                    // XXX: May need to change the encoding based on codepage
                    content += EncodingUtil.formUrlEncode(
                        pairs, "ISO-8859-1") +
                        " "; // Pad with a space to counter space stripping
                }
                catch (HttpClientError uee)
                {
                    throw new MalformedURLException(uee.getMessage());
                }
            }    
            else if (content != null)
            {
                if (method.equalsIgnoreCase("GET"))
                {
                    if (! content.equals("?"))
                    // pad content and don't try to include it in the body
                        content += " ";
                }
                else
                {
                    ((EntityEnclosingMethod)lastRequest).setRequestEntity(
                        new StringRequestEntity(content));
                }
            }
        }
        catch (ClassCastException e)
        {
            //  the error indicates that invalid options were specified on the 
            //   requested method, a request of this type cannot include a body
            
            String msg = " Methods of type " + method + " cannot send " ;
            
            if (files != null)
                msg += "files";
            else if (params != null)
                msg += "parameters";
            else if (content != null)
                msg += "body content";
                
            throw new MalformedURLException(msg);
        }
        
        if (method.equalsIgnoreCase("GET"))
        {
            content = content.substring(0, content.length() - 1);

            if (!content.equals(""))
                lastRequest = new GetMethod(targetURL + content);
        }
        
        // set auto follow redirects
        // do not set redirect if type is put or post - causes httpclient error
        boolean doRedirect = followRedirect;
        
        // override the default setting
        if (redirect != null)
            doRedirect = redirect.booleanValue();
        
        if (doRedirect &&
            ! method.equalsIgnoreCase("POST") && 
            ! method.equalsIgnoreCase("PUT") ) 
            
            lastRequest.setFollowRedirects(true);
            
        else
            lastRequest.setFollowRedirects(false);
        
        int statusCode = -1;

        // Retry up to 5 times for GET method requests

        int numTries = 5;

        if (method.equalsIgnoreCase("GET"))
        {
            // Create a new DefaultHttpMethodRetryHandler that retries up to
            // 5 times (instead of the default of 3 times) but does not retry
            // methods that have successfully sent their requests.

            DefaultHttpMethodRetryHandler retryHandler =
                new DefaultHttpMethodRetryHandler(numTries, false);

            session.getParams().setParameter(HttpMethodParams.RETRY_HANDLER,
                                             retryHandler);
        }

        // Submit request
        try
        {
            if (HTTP.DEBUG)
                System.out.println(
                    "WebSession.requestMethod(): Before " +
                    "session.executeMethod(lastRequest)");

            statusCode = session.executeMethod(lastRequest);

            if (HTTP.DEBUG)
                System.out.println(
                    "WebSession: requestMethod(): After " +
                    "session.executeMethod(lastRequest)");
        }
        catch (URIException e)
        {
            // Indicates bad URL
            // Is this redundant with the constructor exception ?
            throw new MalformedURLException("Invalid URI " + targetURL);
        }
        catch (HttpException e)
        {
            // Indicates the request failed
            throw e;
        }
        catch (IOException e)
        {
            // Indicates page failed to download
            throw e;
        }
        catch (java.lang.IllegalArgumentException e)
        {
            // When URI is invalid
            throw new MalformedURLException("Invalid URI " + targetURL);
        } 
        catch (java.lang.IllegalStateException e)
        {
            // When protocol of the absolute URI is not recognized
            throw new MalformedURLException("Invalid protocol " + targetURL);
        }
        
        if (statusCode == -1)
            throw new HttpException ("Request Failed: Unknown Reason");
        
        if (doRedirect && (statusCode > 299) && (statusCode < 400))
        {
            // this was a post or put request
            // redirect manually
            Header locationHeader = lastRequest.getResponseHeader("location");
            if (locationHeader != null) 
            {
                String redirectLocation = locationHeader.getValue();
                requestMethod(redirectLocation, "GET", 
                              null, null, null, null, 
                              new Boolean(true), toFile, toMachine, false);
            }
            // should not reach here because 3xx return codes should have
            // a location header specifying the redirect url
        }
           
        try
        {
            if (HTTP.DEBUG)
                System.out.println(
                    "WebSession.requestMethod(): Before setCurrentContents(" +
                    "lastRequest.getResponseBodyAsStream(), " + toFile +
                    ", " + toMachine + ")");
            
            setCurrentContents(lastRequest.getResponseBodyAsStream(),
                               toFile, toMachine);

            if (HTTP.DEBUG)
                System.out.println(
                    "WebSession.requestMethod(): After setCurrentContents()");
        }
        catch(IOException e)
        {
            throw e;
        }
        catch(STAFException e)
        {
            throw e;
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getHttpResponse                                                   */
/* Description: Generate a string containing all information generated by the*/
/*              reply from the last interaction.  It corresponds to the info */
/*              returned by the HTTP service 1.0.6 and earlier               */
/* Parameters: none                                                          */
/* Returns: a string containing the specified content from the last          */
/*          interaction                                                      */
/*                                                                           */
/*****************************************************************************/    

    public HashMap getHttpResponse ()
    {
        HashMap response = new HashMap();
        
        if (lastRequest == null) return response;
        
        response.put(RETURN_STATUS_CODE, Integer.toString(getCurrentStatusCode()));
        response.put(RETURN_STATUS_MESSAGE, getCurrentStatusText());
        
        
        Header heads [] = lastRequest.getResponseHeaders() ;
        Map headers = new HashMap();

        for (int i = 0; i < heads.length; i++)
        {
            // This may need to be changed to handle headers with multiple
            // values.
            headers.put(heads[i].getName(), heads[i].getValue());
        }
        
        response.put(RETURN_HEADERS, headers);
                        
        return response;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: isOwner                                                           */
/* Description: Determines if requester is the owner of the session by       */
/* Parameters: Requester's machine, process name, and handle                 */
/* Returns: true if Owner and false if not owner                             */
/*                                                                           */
/*****************************************************************************/    

    public boolean isOwner(String reqInstanceUUID)
    {
        if (reqInstanceUUID.equals(ownerInstanceUUID))
        {
            return true;
        }
        else
        {
            return false;
        }
    }    

/*****************************************************************************/
/*                                                                           */
/* Method: getSummary                                                        */
/* Description: get a summary of the session.                                */
/* Parameters: none                                                          */
/* Returns: description of a session                                         */
/*                                                                           */
/*****************************************************************************/    

    public HashMap getSummary()
    {
        HashMap summary = new HashMap();
        
        summary.put(ID,Integer.toString(id));
        summary.put(TITLE, getCurrentTitle());
        summary.put(URL, getCurrentUrl());
        summary.put(RETURN_STATUS_CODE, Integer.toString(getCurrentStatusCode()));
        summary.put(RETURN_STATUS_MESSAGE, getCurrentStatusText());
        summary.put(POLICY, getCookiePolicy());
        if (followRedirect)
            summary.put(REQUEST_REDIRECT, "Enabled");
        else
            summary.put(REQUEST_REDIRECT, "Disabled");
        summary.put(PARSE_CONTENT, fParseContent);
        summary.put(HTTP_PROXY_HOST,
                    session.getHostConfiguration().getProxyHost());
        summary.put(HTTP_PROXY_PORT,
                    new Integer(session.getHostConfiguration().getProxyPort()));
        summary.put(OWNER_INSTANCE_UUID, ownerInstanceUUID);
        summary.put(OWNER_MACHINE, ownerMachine);
        summary.put(OWNER_HANDLE_NAME, ownerHandleName);
        summary.put(OWNER_HANDLE, Integer.toString(ownerHandle));
                
        return summary;
    }    

/*****************************************************************************/
/*                                                                           */
/* Method: setDefaultHeader                                                  */
/* Description: sets the value of the specifed default header to the         */
/*              specified value                                              */
/* Parameters: name - key of the header                                      */
/*             value - new value for the header                              */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void setDefaultHeader(String name, String value)
    {
        try
        {
            String key = getDefaultHeaderKey(name);
            defaultHeaders.put(key, value);
        } catch (InvalidElementIDException e)
        {
            defaultHeaders.put(name, value);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: deleteDefaultHeader                                               */
/* Description: remove the specifed default header                           */
/* Parameters: name - key of the header                                      */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void deleteDefaultHeader(String name) 
                                    throws InvalidElementIDException
    {
        String key = getDefaultHeaderKey(name);
        defaultHeaders.remove(key);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getDefaultHeaderKey                                               */
/* Description: get the key object of the specifed default header            */
/* Parameters: name - key of the header                                      */
/* Returns:  key object to access the defaultHeader HashMap                  */
/*                                                                           */
/*****************************************************************************/    
    
    public String getDefaultHeaderKey(String name)
                                    throws InvalidElementIDException
    {
        Iterator it = defaultHeaders.keySet().iterator();
        
        while(it.hasNext())
        {
            String key = (String) it.next();
            if (name.equalsIgnoreCase(key))
                return key;    
        }
        throw new InvalidElementIDException (name, "Header key " + name);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getDefaultHeaderValue                                             */
/* Description: get the value associated with the specifed default header    */
/* Parameters: name - key of the header                                      */
/* Returns:  value assoicated with the defaultHeader                         */
/*                                                                           */
/*****************************************************************************/    
    
    public String getDefaultHeaderValue(String name)
                                    throws InvalidElementIDException
    {
        String key = getDefaultHeaderKey(name);
        
        return (String)defaultHeaders.get(key);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getDefaultHeaders                                                 */
/* Description: get the HashMap storing the default header values            */
/* Parameters: none                                                          */
/* Returns: the default header HashMap                                       */
/*                                                                           */
/*****************************************************************************/    
        
    public HashMap getDefaultHeaders()
    {
        return defaultHeaders;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getDefaultFollowRedirect                                          */
/* Description: get if the session follows 3XX redirects by default or not   */
/* Parameters: none                                                          */
/* Returns: if the session follows 3XX redirects by default or not           */
/*                                                                           */
/*****************************************************************************/    
    
    public boolean getDefaultFollowRedirect()
    {
        return followRedirect;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: setDefaultFollowRedirect                                          */
/* Description: get if the session follows 3XX redirects by default or not   */
/* Parameters: redirect - if the session follows 3XX redirects by default or */
/*                        not                                                */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void setDefaultFollowRedirect(boolean redirect)
    {
        followRedirect = redirect;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getParseContent                                                   */
/* Description: Returns the value of fParseContent for the session           */
/* Parameters: none                                                          */
/* Returns:  A string containing Enabled, Disabled, or AutoDetect            */
/*                                                                           */
/*****************************************************************************/    
    
    public String getParseContent()
    {
        return fParseContent;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: setParseContent                                                   */
/* Description: Sets if/how the session should parse content as HTML         */
/* Parameters: A string containing Enabled, Disabled, or AutoDetect, or an   */
/*             invalid value                                                 */
/*                                                                           */
/* Returns:  STAFResult instance with RC == 0 if a valid parameter was input */
/*           STAFResult instance with RC != 0 and an error message in the    */
/*           result if an invalid parameter was input                        */
/*                                                                           */
/*****************************************************************************/    
    
    public STAFResult setParseContent(String parseContent)
    {
        // Verify that Enabled, Disabled, or AutoDetect was specified,
        // case-insensitive

        if (parseContent.equalsIgnoreCase(ENABLED))
        {
            fParseContent = ENABLED;
        }
        else if (parseContent.equalsIgnoreCase(DISABLED))
        {
            fParseContent = DISABLED;
        }
        else if (parseContent.equalsIgnoreCase(AUTODETECT))
        {
            fParseContent = AUTODETECT;
        }
        else
        {
            return new STAFResult(
                STAFResult.InvalidValue,
                "PARSECONTENT option value must be Enabled, Disabled," +
                " or AutoDetect, case-insensitive. Invalid value: " +
                parseContent);
        }

        return new STAFResult(STAFResult.Ok);
    }

 /*****************************************************************************/
 /*                                                                           */
 /* Method: setHttpProxy                                                      */
 /* Description: sets the HTTP Proxy host and port for the session,           */
 /* Parameters: proxyHost - the HTTP Proxy host                               */
 /*             proxyPort - the HTTP Proxy port (-1 indicates to use the      */
 /*                         default port)                                     */
 /* Returns: void                                                             */
 /*                                                                           */
 /*****************************************************************************/    
    public void setHttpProxy(String proxyHost, int proxyPort)
    {
        session.getHostConfiguration().setProxy(proxyHost, proxyPort);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: setAuthenticationSet                                              */
/* Description: sets the user and password for preemptive challenges to the  */
/*              host                                                         */
/* Parameters: host - the host to associate with this user and password      */
/*             user - the user to assciate with this host                    */
/*             pwd  - the password to assciate with this host                */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public void setAuthenticationSet(String host, String user, String pwd,
                                      String domain)
    {
        String [] values = new String[3];
        values[0] = user;
        values[1] = pwd;
        values[2] = domain;
        try
        {
            String key = getAuthenticationSetKey(host);
            authenticationSets.put(key, values);
        } catch (InvalidElementIDException e)
        {
            authenticationSets.put(host, values);
        }
    }

/*****************************************************************************/
/*                                                                           */
/* Method: deleteAuthenticationSet                                           */
/* Description: remove the specifed host                                     */
/* Parameters: host - the host to be removed                                 */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
        
    public void deleteAuthenticationSet(String host) 
                                    throws InvalidElementIDException
    {
        String key = getAuthenticationSetKey(host);
        authenticationSets.remove(key);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getAuthenticationSetKey                                           */
/* Description: get the key object of the specifed host                      */
/* Parameters: host - host string                                            */
/* Returns: key object to access the authenticationSet HashMap               */
/*                                                                           */
/*****************************************************************************/    
        
    public String getAuthenticationSetKey(String host)
                                    throws InvalidElementIDException
    {
        Iterator it = authenticationSets.keySet().iterator();
        
        while(it.hasNext())
        {
            String key = (String) it.next();
            if (host.equalsIgnoreCase(key))
                return key;    
        }
        throw new InvalidElementIDException (host, "Authentication host key " + host);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getAuthenticationSetValues                                        */
/* Description: get the value associated with the specifed host              */
/* Parameters: host - host assciated with the values                         */
/* Returns:  values assoicated with the host, [0] is user, [1] is password   */
/*                                                                           */
/*****************************************************************************/    
    
    public String[] getAuthenticationSetValues(String host)
                                    throws InvalidElementIDException
    {
        String key = getAuthenticationSetKey(host);
        
        return (String[]) authenticationSets.get(key);
    }

/*****************************************************************************/
/*                                                                           */
/* Method: getAuthenticationSets                                             */
/* Description: get the HashMap storing the preemptive authentication values */
/* Parameters: none                                                          */
/* Returns: the authentication sets HashMap                                  */
/*                                                                           */
/*****************************************************************************/    
            
    public HashMap getAuthenticationSets()
    {
        return authenticationSets;
    }

/*****************************************************************************/
/*                                                                           */
/* Method: resolveUrl                                                        */
/* Description: clean up the url so that it is a full and absolute url       */
/* Parameters: targetURL - the url to be cleaned up                          */
/*             encoded - indicates whether the url is already encoded        */
/*                       (aka percent-encoded or percent-escaped)            */
/* Returns: the updated URL string                                           */
/*                                                                           */
/*****************************************************************************/
    protected String resolveUrl(String targetURL, boolean encoded)
              throws URIException
    {
        // An instance of the URI class is always in an "escaped" (aka
        // "encoded") form, since escaping or unescaping a completed URI might
        // change its semantics.  Thus, we need to be careful not to escape or
        // unescape the same string more than once, since unescaping an
        // already unescaped string might lead to misinterpreting a percent
        // data character as another escaped character, or vice versa in the
        // case of escaping an already escaped string. 
        
        URI target = new URI(targetURL, encoded);
        URI absolute = null;
        String baseURL = getCurrentUrl();
        
        if (target.isAbsoluteURI() || baseURL.equals(""))
        {
            // Set the absolute URL
            absolute = new URI(targetURL, encoded);
        }
        else
        {
            // Convert relative URL to an absolute URL
            URI base = new URI(baseURL, true);
            absolute = new URI(base, target);
        }
        
        String absoluteURL = absolute.toString();

        // Assume http protocol if no protocol specified
        if (absoluteURL.indexOf("://") == -1)
            absoluteURL = "http://" + absoluteURL;

        return absoluteURL;
    }

}

