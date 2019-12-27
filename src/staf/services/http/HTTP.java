package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Class: HTTP                                                               */
/* Description: This class provides the STAFService Interface and implements */
/*              the majority of the service function                         */
/*                                                                           */
/*****************************************************************************/

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import com.ibm.staf.service.utility.ServiceUtilities;
import com.ibm.staf.service.http.html.InvalidParameterValueException;

import java.io.File;
import java.io.FileOutputStream;
import java.io.DataOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.net.UnknownHostException;
import java.net.ConnectException;
import java.net.MalformedURLException;

import com.ibm.staf.service.http.html.*;

import org.apache.commons.httpclient.HttpException;

public class HTTP implements STAFServiceInterfaceLevel30
{
    public static final boolean DEBUG = false;

    private static String sHelpMsg;
    private static String sLineSep;

    private STAFHandle sHandle;
    private String fServiceName;
    private String fLocalMachineName = "";
    private STAFCommandParser requestParser;
    private STAFCommandParser infoParser;
    private STAFCommandParser getParser;
    private STAFCommandParser htmlActionParser;
    private STAFCommandParser setParser;
    private STAFCommandParser delCloseParser;

    private STAFMapClassDefinition fListSessionMapClass;
    private STAFMapClassDefinition fSessionMapClass;
    private STAFMapClassDefinition fSessionOwnerMapClass;
    private STAFMapClassDefinition fHttpResultMapClass;
    private STAFMapClassDefinition fAuthHostMapClass;
    private STAFMapClassDefinition fCookieMapClass;
    private STAFMapClassDefinition fListFormMapClass;
    private STAFMapClassDefinition fFormMapClass;
    private STAFMapClassDefinition fFormControlMapClass;
    private STAFMapClassDefinition fListLinkMapClass;
    private STAFMapClassDefinition fLinkMapClass;

    private SessionList sessionList;
    private String fTempDir;
    private int fTempFileCount = 0;
    
    /* String Constants */

    private static final String VERSIONINFO = "3.0.4";
    
    // Version of STAF (or later) required for this service
    // STAF Version 3.1.0 or later is required so that the privacy methods
    // in STAFUtil are available.
    private static final String REQUIREDSTAFVERSION = "3.4.0";

    private final static SimpleDateFormat sTimestampFormat = 
        new SimpleDateFormat("yyyyMMdd-HH:mm:ss");

    private static final String REQUEST = "REQUEST";
    private static final String DOPOST = "DOPOST";
    private static final String DOGET = "DOGET";
    private static final String METHOD = "METHOD";
    private static final String URL = "URL";
    private static final String ENCODED = "ENCODED";
    private static final String CONTENT = "CONTENT";
    private static final String HEADER = "HEADER";
    private static final String PARAMETER = "PARAMETER";
    private static final String CONTENTFILE ="CONTENTFILE";
    private static final String FILE = "FILE";
    private static final String TOMACHINE = "TOMACHINE";
    private static final String IGNOREERRORS = "IGNOREERRORS";
    private static final String RETURNHEADERS = "RETURNHEADERS";
    private static final String NOSTATUS = "NOSTATUS";
    private static final String NOCONTENT = "RETURNNOCONTENT";
    private static final String SESSION = "SESSION";
    private static final String SESSIONS = "SESSIONS";
    private static final String OPEN = "OPEN";
    private static final String CLOSE = "CLOSE";
    private static final String COOKIE = "COOKIE";
    private static final String COOKIES = "COOKIES";
    private static final String NAME = "NAME";
    private static final String GET = "GET";
    private static final String SET = "SET";
    private static final String VALUE = "VALUE";
    private static final String DELETE = "DELETE";
    private static final String POLICY = "POLICY";
    private static final String SHOW = "SHOW";
    private static final String PAGE = "PAGE";
    private static final String QUERY = "QUERY";
    private static final String LIST = "LIST";
    private static final String LINK = "LINK";
    private static final String LINKS = "LINKS";
    private static final String FORM = "FORM";
    private static final String FORMS = "FORMS";
    private static final String ID = "ID";
    private static final String INDEX = "INDEX";
    private static final String FOLLOW = "FOLLOW";
    private static final String SUBMIT = "SUBMIT";
    private static final String RESET = "RESET";
    private static final String FORMCONTROLS = "CONTROLNAMES";
    private static final String CONTROLNAME = "CONTROLNAME";
    private static final String AUTOREDIRECT = "FOLLOWREDIRECT";
    private static final String NOAUTOREDIRECT = "DONOTFOLLOWREDIRECT";
    private static final String DEFAULTHEADER = "DEFAULTHEADER";
    private static final String DEFAULTHEADERS = "DEFAULTHEADERS";
    private static final String AUTHENTICATIONHOSTS = "AUTHENTICATIONHOSTS";
    private static final String AUTHENTICATIONHOST = "AUTHENTICATIONHOST";
    private static final String AUTHENTICATIONUSER = "AUTHENTICATIONUSER";
    private static final String AUTHENTICATIONPWD = "AUTHENTICATIONPASSWORD";
    private static final String AUTHENTICATIONDOMAIN = "AUTHENTICATIONDOMAIN";
    private static final String HELP = "HELP";
    private static final String VERSION = "VERSION";

    private static final int BASEERROR = 4000;
    public static final int UNKNOWNHOST = 4001;
    private static final String UNKNOWNHOSTInfo =
       "Unknown Host Error";
    private static final String UNKNOWNHOSTDesc =
       "The specified host could not be found in the naming service.";
    public static final int CONNECTERROR = 4002;
    private static final String CONNECTERRORInfo = "Connect Error";
    private static final String CONNECTERRORDesc =
       "The host was located, but a connection could not be made to " +
       "the specified port.";
    public static final int INVALIDMETHOD = 4003;
    private static final String INVALIDMETHODInfo =
        "Invalid Method";
    private static final String INVALIDMETHODDesc =
       "The specified method is not allowed. Allowed methods: POST, PUT, " +
       "GET, HEAD, OPTIONS, DELETE, and TRACE.";
    public static final int IOEXCEPTION = 4004;
    private static final String IOEXCEPTIONInfo = "IO Exception";
    private static final String IOEXCEPTIONDesc =
       "The host was located, but an IO exception occurred opening a " +
       "connection to the URL.";
    public static final int CONTENTTOOLARGE = 4005;
    private static final String CONTENTTOOLARGEInfo = "Out of Memory Error";
    private static final String CONTENTTOOLARGEDesc =
       "Trying to get content that is too large. Recommend " +
       "redirecting the content to a file using the FILE option or, " +
       "if the content is not needed, you may be able to specify the " +
       "RETURNNOCONTENT option depending on the request.";

/*****************************************************************************/
/*                                                                           */
/* Method: Constructor                                                       */
/* Description: Constructor method                                           */
/*                                                                           */
/*****************************************************************************/

public HTTP()
{
    /* do nothing */
}

/*****************************************************************************/
/*                                                                           */
/* Method: acceptRequest                                                     */
/* Description: required by interface STAFServiceInterfaceLevel30            */
/*              performs first parse of request                              */
/*                                                                           */
/*****************************************************************************/

public STAFResult acceptRequest(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Try block is here to catch any unexpected errors/exceptions

    try
    {
        // Determine the command request (the first word in the request)

        String action;
        int spaceIndex = info.request.indexOf(" ");

        if (spaceIndex != -1)
            action = info.request.substring(0, spaceIndex);
        else
            action = info.request;

        String actionUC = action.toUpperCase();

        // Call the appropriate method to handle the command request

        if ((actionUC.equals(DOPOST)) || (actionUC.equals(DOGET)) ||
            (actionUC.equals(REQUEST)))
        {
            return handleMethod(info);
        }
        else if (actionUC.equals(OPEN))
            return handleOpen(info);
        else if (actionUC.equals(LIST))
            return handleList(info);
        else if (actionUC.equals(SET))
            return handleSet(info);
        else if (actionUC.equals(GET))
            return handleGetPage(info);
        else if (actionUC.equals(QUERY))
            return handleQuery(info);
        else if (actionUC.equals(CLOSE))
            return handleClose(info);
        else if (actionUC.equals(FOLLOW))
            return handleFollow(info);
        else if (actionUC.equals(SUBMIT))
            return handleFormAction(info);
        else if (actionUC.equals(DELETE))
            return handleDelete(info);
        else if (actionUC.equals(RESET))
            return handleFormAction(info);
        else if (actionUC.equals(HELP))
            return help(info);
        else if (actionUC.equals(VERSION))
            return version(info);
        else
        {
            return new STAFResult(
                STAFResult.InvalidRequestString,
                "'" + action + "' is not a valid command request for the " +
                fServiceName + " service" + sLineSep + sLineSep + sHelpMsg);
        }
    }
    catch (STAFException se)
    {
        return new STAFResult(se.rc, se.getMessage());
    }
    catch (Throwable t)
    {
        // Write the Java stack trace to the JVM log for the service
        
        System.out.println(
            sTimestampFormat.format(Calendar.getInstance().getTime()) +
            " ERROR: Exception on " + fServiceName + " service request:" +
            sLineSep + sLineSep + info.request + sLineSep);

        t.printStackTrace();

        // And also return the Java stack trace in the result

        StringWriter sw = new StringWriter();
        t.printStackTrace(new PrintWriter(sw));

        if (t.getMessage() != null)
        {
            return new STAFResult(
                STAFResult.JavaError,
                t.getMessage() + sLineSep + sw.toString());
        }
        else
        {
            return new STAFResult(
                STAFResult.JavaError, sw.toString());
        }
    }
}


// enhanced functionality

/*****************************************************************************/
/*                                                                           */
/* Method: handleOpen                                                        */
/* Description: Creates a new web session and goes to the specified url.     */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: session id of new session                                        */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleOpen(STAFServiceInterfaceLevel30.RequestInfo info)
                              throws STAFException
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "OPEN", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    if (! info.request.equalsIgnoreCase("OPEN SESSION"))
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              info.request);
    }

    String result;

    WebSession newSession = new WebSession(sessionList, info, this);

    result = Integer.toString(newSession.getID());

    return new STAFResult(STAFResult.Ok, result);
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleClose                                                       */
/* Description: ends a web session and removes it from the session list.     */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleClose(STAFServiceInterfaceLevel30.RequestInfo info)
                               throws STAFException
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "CLOSE", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = delCloseParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);

    // Check if the requester is the owner of the session

    WebSession session;

    try
    {
        session = sessionList.getSession(sessionId);
    }catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    if (!session.isOwner(info.stafInstanceUUID))
    {
        // Verify the requester has at least trust level 5 if not the owner
        // of the session

        trustResult = STAFUtil.validateTrust(
            5, fServiceName, "CLOSE", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
    }

    // Release the resources held by the session and delete the session

    try
    {
        session.releaseConnection();
        sessionList.deleteSession(sessionId);

    }catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist,e.getMessage());
    }

    return new STAFResult(STAFResult.Ok, "");
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleQuery                                                       */
/* Description: gets information about a specific entity.                    */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult containing detailed information on a specific entity  */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleQuery(STAFServiceInterfaceLevel30.RequestInfo info)
                               throws STAFException
{
    // Verify the requester has at least trust level 2

    STAFResult trustResult = STAFUtil.validateTrust(
        2, fServiceName, "QUERY", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = infoParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);

    // Get the session

    WebSession session;

    try
    {
        session = sessionList.getSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    STAFMarshallingContext mc = new STAFMarshallingContext();

    String id = "";
    int idType = -1;

    if (pResult.optionTimes(ID) > 0)
    {
        // Resolve the ID option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(ID), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.ID_ID_TYPE;
    }
    else if (pResult.optionTimes(NAME) > 0)
    {
        // Resolve the NAME option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(NAME), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.NAME_ID_TYPE;
    }
    else if (pResult.optionTimes(INDEX) > 0)
    {
        // Resolve the INDEX option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(INDEX), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.INDEX_ID_TYPE;
    }

    if (pResult.optionTimes(DEFAULTHEADER) > 0)
    {
        // QUERY DEFAULTHEADER
        
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(DEFAULTHEADER), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String key = res.result;

        try
        {
            return new STAFResult(STAFResult.Ok, 
                                  session.getDefaultHeaderValue(key));
        }
        catch (InvalidElementIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }
    }
    else if (pResult.optionTimes(AUTHENTICATIONHOST) > 0)
    {
        // QUERY AUTHENTICATIONHOST

        mc.setMapClassDefinition(fAuthHostMapClass);
        Map authHostMap = fAuthHostMapClass.createInstance();

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(AUTHENTICATIONHOST), sHandle,
            info.requestNumber);

        if (res.rc != 0) return res;

        String key = res.result;

        try
        {
            String[] values = session.getAuthenticationSetValues(key);
            String user = values[0];
            String password = values[1];
            String domain = values[2];
            
            if (user != null && !user.equals(""))
                authHostMap.put("user", user);

            if (password != null && !password.equals(""))
                authHostMap.put("password", "*******");

            if (domain != null && !domain.equals(""))
                authHostMap.put("domain", domain);

            mc.setRootObject(authHostMap);
        }
        catch (InvalidElementIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }
    }
    else if (pResult.optionTimes(FORM) > 0)
    {
        try
        {
            if (id.equals("") && idType == -1)
            {
                id = "1";
                idType = WebSession.INDEX_ID_TYPE;
            }
            
            if (pResult.optionTimes(CONTROLNAME) > 0)
            {
                // QUERY FORM CONTROLNAME

                mc.setMapClassDefinition(fFormControlMapClass);
                Map formMap = fFormControlMapClass.createInstance();

                res = STAFUtil.resolveRequestVar(
                    pResult.optionValue(CONTROLNAME), sHandle,
                    info.requestNumber);

                if (res.rc != 0) return res;

                String controlName = res.result;
                
                HashMap data = session.summarizeFormControl(
                    id, idType, controlName);
                
                String type     = (String)data.get(Parameter.TYPE);
                String disabled = (String)data.get(Parameter.DISABLED);
                String readOnly = (String)data.get(Parameter.READONLY);
                String value    = (String)data.get(Parameter.VALUE);

                if (type != null && !type.equals(""))
                    formMap.put("type", type);
                if (disabled != null && !disabled.equals(""))
                    formMap.put("disabled", disabled);
                if (readOnly != null && !readOnly.equals(""))
                    formMap.put("readOnly", readOnly);
                if (value != null && !value.equals(""))
                    formMap.put("value", STAFUtil.maskPrivateData(value));

                formMap.put("possibleValueList",
                            (List)data.get(Parameter.POSSIBLEVALUES));

                mc.setRootObject(formMap);
            }
            else
            {
                // QUERY FORM      (Summarize the form)

                mc.setMapClassDefinition(fFormMapClass);
                Map formMap = fFormMapClass.createInstance();

                HashMap data = session.summarizeForm(id, idType);

                String formIndex = data.get(WebElement.INDEX).toString();
                String formName  = (String)data.get(WebElement.NAME);
                String formID    = (String)data.get(WebElement.ID);
                String method    = (String)data.get(WebSession.REQUEST_METHOD);
                String action    = (String)data.get(WebSession.REQUEST_URL);

                if (formIndex != null && !formIndex.equals(""))
                    formMap.put("formIndex", formIndex);
                if (formName != null && !formName.equals(""))
                    formMap.put("formName", formName);
                if (formID != null && !formID.equals(""))
                    formMap.put("formID", formID);
                if (method != null && !method.equals(""))
                    formMap.put("method", method);
                if (action != null && !action.equals(""))
                    formMap.put("targetUrl", action);
                
                formMap.put("headerMap",
                            (HashMap)data.get(WebSession.REQUEST_HEADERS));

                mc.setRootObject(formMap);
            }
        }
        catch (InvalidElementIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }
    }
    else if (pResult.optionTimes(COOKIE) > 0)
    {
        // QUERY COOKIE

        mc.setMapClassDefinition(fCookieMapClass);
        Map cookieMap = fCookieMapClass.createInstance();

        try
        {
            HashMap data = session.summarizeCookie(id);

            String value = (String)data.get(CookieAccess.VALUE);
            String domain = (String)data.get(CookieAccess.DOMAIN);
            String path = (String)data.get(CookieAccess.PATH);
            String expirationDate = null;
             
            if (data.get(CookieAccess.EXPIRATION) != null)
                expirationDate = data.get(CookieAccess.EXPIRATION).toString();

            if (value != null && !value.equals(""))
                cookieMap.put("value", value);

            if (domain != null && !domain.equals(""))
                cookieMap.put("domain", domain);

            if (path != null && !path.equals(""))
                cookieMap.put("path", path);

            if (expirationDate != null && !expirationDate.equals(""))
                cookieMap.put("expirationDate", expirationDate);

            mc.setRootObject(cookieMap);
        }
        catch (InvalidCookieIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());

        }
    }
    else if (pResult.optionTimes(LINK) > 0)
    {
        // QUERY LINK
        
        mc.setMapClassDefinition(fLinkMapClass);
        Map linkMap = fLinkMapClass.createInstance();

        try
        {
            HashMap data = session.summarizeLink(id, idType);

            String linkName = null;
            String linkID = null;
            String href = null;

            if (data.get(WebElement.NAME) != null)
                linkName = (String)data.get(WebElement.NAME);

            if (data.get(WebElement.ID) != null)
                linkID = (String)data.get(WebElement.ID);

            if (data.get(WebLink.HREF) != null)
                href = (String)data.get(WebLink.HREF);

            linkMap.put("linkIndex", data.get(WebElement.INDEX));

            if (linkID != null && !linkID.equals(""))
                linkMap.put("linkID", linkID);
            if (linkName != null && !linkName.equals(""))
                linkMap.put("linkName", linkName);
            if (href != null && !href.equals(""))
                linkMap.put("href", href);

            mc.setRootObject(linkMap);
        }
        catch (InvalidElementIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }
    }
    else
    {
        // QUERY SESSION <Session>  (Summarize the session)

        mc.setMapClassDefinition(fSessionMapClass);
        mc.setMapClassDefinition(fSessionOwnerMapClass);
        Map sessionMap = fSessionMapClass.createInstance();
        Map ownerMap = fSessionOwnerMapClass.createInstance();
        
        HashMap data = session.getSummary();
        
        String url              = (String)data.get(WebSession.URL);
        String title            = (String)data.get(WebSession.TITLE);
        String statusCode       = (String)data.get(
            WebSession.RETURN_STATUS_CODE);
        String statusMessage    = (String)data.get(
            WebSession.RETURN_STATUS_MESSAGE);
        String cookiePolicy     = (String)data.get(WebSession.POLICY);
        String followsRedirects = (String)data.get(WebSession.REQUEST_REDIRECT);
        String parseContent     = (String)data.get(WebSession.PARSE_CONTENT);
        String httpProxyHost    = (String)data.get(WebSession.HTTP_PROXY_HOST);
        Integer httpProxyPort    = (Integer)data.get(WebSession.HTTP_PROXY_PORT);
        String ownerInstanceUUID= (String)data.get(
            WebSession.OWNER_INSTANCE_UUID);
        String ownerMachine     = (String)data.get(WebSession.OWNER_MACHINE);
        String ownerHandleName  = (String)data.get(
            WebSession.OWNER_HANDLE_NAME);
        String ownerHandle      = (String)data.get(WebSession.OWNER_HANDLE);

        if (url != null && !url.equals(""))
            sessionMap.put("url", url);
            
        if (title != null && !title.equals(""))
            sessionMap.put("title", title);
        
        if (statusCode != null && !statusCode.equals(""))
            sessionMap.put("statusCode", statusCode);

        if (statusMessage != null && !statusMessage.equals(""))
            sessionMap.put("statusMessage", statusMessage);

        if (cookiePolicy != null && !cookiePolicy.equals(""))
            sessionMap.put("cookiePolicy", cookiePolicy);

        if (followsRedirects != null && !followsRedirects.equals(""))
            sessionMap.put("followsRedirects", followsRedirects);

        if (parseContent != null && !parseContent.equals(""))
            sessionMap.put("parseContent", parseContent);

        if (httpProxyHost != null && !httpProxyHost.equals(""))
            sessionMap.put("httpProxyHost", httpProxyHost);

        if (httpProxyPort.intValue() != -1)
            sessionMap.put("httpProxyPort", httpProxyPort.toString());

        ownerMap.put("instanceUUID", ownerInstanceUUID);
        ownerMap.put("machine", ownerMachine);
        ownerMap.put("handle", ownerHandle);

        if (ownerHandleName != null & !ownerHandleName.equals(""))
            ownerMap.put("handleName", ownerHandleName);

        sessionMap.put("owner", ownerMap);
            
        mc.setRootObject(sessionMap);
    }

    return new STAFResult(STAFResult.Ok, mc.marshall());
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleList                                                        */
/* Description: gets a list of the requested entities.            .          */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult listing the requested entities                        */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleList(STAFServiceInterfaceLevel30.RequestInfo info)
                              throws STAFException
{
    // Verify the requester has at least trust level 2

    STAFResult trustResult = STAFUtil.validateTrust(
        2, fServiceName, "LIST", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the result

    STAFCommandParseResult pResult = infoParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    STAFMarshallingContext mc = new STAFMarshallingContext();
    List resultList = new ArrayList();

    if (pResult.optionTimes(SESSION) > 0)
    {
        // Get resolved value for SESSION option and make sure it's an integer

        STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
            SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        int sessionId = Integer.parseInt(res.result);

        WebSession session;

        // Get the session
        try
        {
            session = sessionList.getSession(sessionId);
        }
        catch (InvalidSessionIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }

        if (pResult.optionTimes(DEFAULTHEADERS) > 0)
        {
            // LIST DEFAULTHEADERS

            HashMap headers = session.getDefaultHeaders();

            mc.setRootObject(headers);
            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else if (pResult.optionTimes(AUTHENTICATIONHOSTS) > 0)
        {
            // LIST AUTHENTICATIONHOSTS

            HashMap authenticationSets = session.getAuthenticationSets();
            Iterator it = authenticationSets.keySet().iterator();

            while(it.hasNext())
                resultList.add((String)it.next());
        }
        else if (pResult.optionTimes(FORMS) > 0)
        {
            // LIST FORMS

            mc.setMapClassDefinition(fListFormMapClass);

            HashMap[] list = session.listForms();

            for (int i = 0; i < list.length; i++)
            {
                Map resultMap = fListFormMapClass.createInstance();

                resultMap.put("formIndex", list[i].get(WebElement.INDEX));

                String id = (String)list[i].get(WebElement.ID);
                String name = (String)list[i].get(WebElement.NAME);

                if (id != null && !id.equals(""))
                    resultMap.put("formID", id);

                if (name != null && !name.equals(""))
                    resultMap.put("formName", name);

                resultList.add(resultMap);
            }
        }
        else if (pResult.optionTimes(LINKS) > 0)
        {
            // LIST LINKS

            mc.setMapClassDefinition(fListLinkMapClass);

            HashMap[] list = session.listLinks();

            for (int i = 0; i < list.length; i++)
            {
                Map resultMap = fListLinkMapClass.createInstance();

                resultMap.put("linkIndex", list[i].get(WebElement.INDEX));

                String id = (String)list[i].get(WebElement.ID);
                String name = (String)list[i].get(WebElement.NAME);

                if (id != null && !id.equals(""))
                    resultMap.put("linkID", id);

                if (name != null && !name.equals(""))
                    resultMap.put("linkName", name);

                resultList.add(resultMap);
            }
        }
        else if (pResult.optionTimes(COOKIES) > 0)
        {
            // LIST COOKIES

            HashMap[] list = session.listCookies();

            for (int i = 0; i < list.length; i++)
                resultList.add(list[i].get(CookieAccess.NAME));
        }
        else if (pResult.optionTimes(FORMCONTROLS) > 0)
        {
            // LIST FORMCONTROLS

            String id = "";
            int idType = -1;

            if (pResult.optionTimes(ID) > 0)
            {
                // Resolve the ID option value

                res = STAFUtil.resolveRequestVar(
                    pResult.optionValue(ID), sHandle, info.requestNumber);

                if (res.rc != 0) return res;

                id = res.result;
                idType = WebSession.ID_ID_TYPE;
            }
            else if (pResult.optionTimes(NAME) > 0)
            {
                // Resolve the NAME option value

                res = STAFUtil.resolveRequestVar(
                    pResult.optionValue(NAME), sHandle, info.requestNumber);

                if (res.rc != 0) return res;

                id = res.result;
                idType = WebSession.NAME_ID_TYPE;
            }
            else if (pResult.optionTimes(INDEX) > 0)
            {
                // Resolve the INDEX option value

                res = STAFUtil.resolveRequestVar(
                    pResult.optionValue(INDEX), sHandle, info.requestNumber);

                if (res.rc != 0) return res;

                id = res.result;
                idType = WebSession.INDEX_ID_TYPE;
            }

            if (id.equals("") && idType == -1)
            {
                id = "1";
                idType = WebSession.INDEX_ID_TYPE;
            }
            try
            {
                HashMap data = session.summarizeForm(id, idType);
                List controls = (List) data.get(WebForm.CONTROLS);

                for (int i = 0; i < controls.size(); i++)
                    resultList.add(((Parameter)controls.get(i)).getKey());
            }
            catch (InvalidElementIDException e)
            {
                return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
            }
        }
    }
    else
    {
        // LIST SESSIONS

        mc.setMapClassDefinition(fListSessionMapClass);

        HashMap[] list = sessionList.listSessions();
        
        for (int i = 0; i < list.length; i++)
        {
            Map sessionMap = fListSessionMapClass.createInstance();

            sessionMap.put("sessionID", list[i].get(WebSession.ID));

            String url = (String)list[i].get(WebSession.URL);
            String title = (String)list[i].get(WebSession.TITLE);

            if (url != null && !url.equals(""))
                sessionMap.put("url", url);
            
            if (title != null && !title.equals(""))
                sessionMap.put("title", title);
            
            resultList.add(sessionMap);
        }
    }

    mc.setRootObject(resultList);

    return new STAFResult(STAFResult.Ok, mc.marshall());
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleSet                                                         */
/* Description: performs the desired set interaction                         */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleSet(STAFServiceInterfaceLevel30.RequestInfo info)
    throws STAFException
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "SET", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the result

    STAFCommandParseResult pResult = setParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);

    WebSession session;

    // Get the session

    try
    {
        session = sessionList.getSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    if (!session.isOwner(info.stafInstanceUUID))
    {
        // Verify the requester has at least trust level 4 since not the
        // session owner

        trustResult = STAFUtil.validateTrust(
            4, fServiceName, "SET", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
    }

    String result = "";

    if (pResult.optionTimes(DEFAULTHEADER) > 0)
    {
        // Resolve the DEFAULTHEADER option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(DEFAULTHEADER), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String key = res.result;

        // Resolve the VALUE option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(VALUE), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String value = res.result;

        session.setDefaultHeader(key, value);

    }
    else if (pResult.optionTimes(AUTHENTICATIONHOST) > 0)
    {
        // Resolve the AUTHENTICATIONHOST option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(AUTHENTICATIONHOST), sHandle,
            info.requestNumber);

        if (res.rc != 0) return res;

        String key = res.result;

        // Resolve the AUTHENTICATIONUSER option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(AUTHENTICATIONUSER), sHandle,
            info.requestNumber);

        if (res.rc != 0) return res;

        String user = res.result;

        String pwd = "";
        if (pResult.optionTimes(AUTHENTICATIONPWD) > 0)
        {
            // Resolve the AUTHENTICATIONPWD option value

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(AUTHENTICATIONPWD), sHandle,
                info.requestNumber);

            if (res.rc != 0) return res;

            // Remove privacy delimiters.  Can remove since never show
            // true value of a authentication password on a query request.
            pwd = STAFUtil.removePrivacyDelimiters(res.result);
        }

        String domain = "";

        if (pResult.optionTimes(AUTHENTICATIONDOMAIN) > 0)
        {
            // Resolve the AUTHENTICATIONDOMAIN option value

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(AUTHENTICATIONDOMAIN), sHandle,
                info.requestNumber);

            if (res.rc != 0) return res;

            domain = res.result;
        }

        session.setAuthenticationSet(key, user, pwd, domain);

    }
    else if (pResult.optionTimes(NOAUTOREDIRECT) > 0)
    {
        session.setDefaultFollowRedirect(false);

    }
    else if (pResult.optionTimes(AUTOREDIRECT) > 0)
    {
        session.setDefaultFollowRedirect(true);

    }
    else if (pResult.optionTimes("PARSECONTENT") > 0)
    {
        // Resolve the PARSECONTENT option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue("PARSECONTENT"),
            sHandle, info.requestNumber);

        if (res.rc != 0) return res;
        
        res = session.setParseContent(res.result);
        
        if (res.rc != 0) return res;
    }
    else if (pResult.optionTimes("HTTPPROXYHOST") > 0)
    {
        // Resolve the HTTPPROXYHOST option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue("HTTPPROXYHOST"),
            sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String proxyHost = res.result;

        if (proxyHost.length() == 0)
        {
            return new STAFResult(
                STAFResult.InvalidValue,
                "Invalid HTTPPROXYHOST value.  Cannot be blank");
        }
        
        int proxyPort = -1;  // Indicates to use default proxy port

        if (pResult.optionTimes("HTTPPROXYPORT") > 0)
        {
            // Resolve the HTTPPROXYPORT option value and check if the
            // value is an integer

            res = STAFUtil.resolveRequestVarAndCheckInt(
                "HTTPPROXYPORT", pResult.optionValue("HTTPPROXYPORT"),
                sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            proxyPort = Integer.parseInt(res.result);

            if (proxyPort < -1 || proxyPort > 65535)
            {
                return new STAFResult(
                    STAFResult.InvalidValue,
                    "Invalid HTTPPROXYPORT value (" + proxyPort +
                    "). Must be in range -1 to 65535.");
            }
        }

        session.setHttpProxy(proxyHost, proxyPort);
    }
    else if (pResult.optionTimes(COOKIE) > 0)
    {
        if (pResult.optionTimes(POLICY) > 0)
        {
            // Resolve the POLICY option value

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(POLICY), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            String policy = res.result;

            session.setCookiePolicy(policy);
        }

        // Resolve the NAME option value

        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(NAME), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String name = res.result;

        if (pResult.optionTimes(VALUE) > 0)
        {
            // Resolve the VALUE option value

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(VALUE), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            String value = res.result;

            try
            {
                session.setCookieValue(name, value);
            }
            catch (InvalidCookieIDException e)
            {
                session.addCookie(name, value);
            }
        }
    }
    else if (pResult.optionTimes(FORM) > 0)
    {
        String id = "";
        int idType = -1;

        if (pResult.optionTimes(ID) > 0)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(ID), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            id = res.result;
            idType = WebSession.ID_ID_TYPE;
        }
        else if (pResult.optionTimes(NAME) > 0)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(NAME), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            id = res.result;
            idType = WebSession.NAME_ID_TYPE;
        }
        else if (pResult.optionTimes(INDEX) > 0)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(INDEX), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            id = res.result;
            idType = WebSession.INDEX_ID_TYPE;
        }
        else
        {
            // default to the 1st form on the page
            id = "1";
            idType = WebSession.INDEX_ID_TYPE;
        }
        
        if (pResult.optionTimes(CONTROLNAME) > 0)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(CONTROLNAME), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            String control = res.result;

            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(VALUE), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            String value = res.result;

            try
            {
                session.setFormElement(id, idType, control, value);
            }
            catch (InvalidElementIDException e)
            {
                return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
            }
            catch(InvalidParameterValueException e)
            {
                return new STAFResult(STAFResult.InvalidValue, e.getMessage());
            }
        }
    }

    return new STAFResult(STAFResult.Ok, result);
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleMethod                                                      */
/* Description: creates the specified web request submits it, and return the */
/*              output.                                                      */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult with the result of the requested method               */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleMethod(
    STAFServiceInterfaceLevel30.RequestInfo info) throws STAFException
{
    STAFCommandParseResult pResult = requestParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    if (pResult.optionTimes(SESSION) == 0)
        return handleNonSessionMethod(info);

    return handleSessionMethod(info);

}

/*****************************************************************************/
/*                                                                           */
/* Method: handleNonSessionMethod                                            */
/* Description: creates the specified web request submits it, and return the */
/*              output.                                                      */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult with the result of the requested method               */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleNonSessionMethod(
    STAFServiceInterfaceLevel30.RequestInfo info) throws STAFException
{
    // Create a temporary new session (only used for this request)
    WebSession newSession = new WebSession(sessionList, info, this, true);

    int sessionId = newSession.getID();

    // add SESSION <sessionId> to info
    info.request += " SESSION " + sessionId;

    STAFResult res;

    try
    {
        // execute request
        res = handleSessionMethod(info);
    }
    catch (STAFException e)
    {
        // Perform clean-up for the temporary session
        cleanupNonSessionMethod(sessionId);
        throw e;
    }

    // Perform clean-up for the temporary session
    cleanupNonSessionMethod(sessionId);

    return res;
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleSessionMethod                                               */
/* Description: creates the specified web request submits it, and return the */
/*              output for an existing session.                              */
/* Parameters: info - request info passed to acceptRequest                   */
/* Returns: STAFResult with the result of the requested method               */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleSessionMethod(
    STAFServiceInterfaceLevel30.RequestInfo info) throws STAFException
{
    // Parse the request

    STAFCommandParseResult pResult = requestParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }
    
    // Get method type

    String method = "";
    String action = "";
    STAFResult res = new STAFResult();

    if (pResult.optionTimes(METHOD) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(METHOD), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        method = res.result;
        action = "REQUEST METHOD";
    }
    else if (pResult.optionTimes(DOPOST) > 0)
    {
        method = "POST";
        action = "DOPOST";
    }
    else if (pResult.optionTimes(DOGET) > 0)
    {
        method = "GET";
        action = "DOGET";
    }

    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, action, fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Get resolved value for SESSION option and make sure it's an integer

    res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    int sessionId = Integer.parseInt(res.result);

    // Resolve the URL option value

    res = STAFUtil.resolveRequestVar(
        pResult.optionValue(URL), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    String url = res.result;

    WebSession session;

    // Get the session
    try
    {
        session = sessionList.getSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    if (!session.isOwner(info.stafInstanceUUID))
    {
        // Verify the requester has at least trust level 4 since not the
        // session owner

        trustResult = STAFUtil.validateTrust(
            4, fServiceName, action, fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
    }

    // Get headers

    int numHeaders = pResult.optionTimes(HEADER);
    HashMap headers = null;

    if (numHeaders > 0)
    {
        headers = new HashMap();

        for (int i = 1; i <= numHeaders; i++)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(HEADER, i), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            String h = res.result;
            int equalPos = h.indexOf("=");

            if (equalPos == -1)
            {
                return new STAFResult(STAFResult.InvalidValue,
                                      "Invalid HEADER: " + h);
            }

            String key = h.substring(0, equalPos);
            String value = h.substring(equalPos + 1);
            headers.put(key, value);
        }
    }

    // Get POST parameters

    Vector params = new Vector();
    int numParams = pResult.optionTimes(PARAMETER);

    if (numParams > 0)
    {
        for (int i = 1; i <= numParams; i++)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(PARAMETER, i), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            String h = res.result;
            int equalPos = h.indexOf("=");

            if (equalPos == -1)
            {
                return new STAFResult(STAFResult.InvalidValue,
                                      "Invalid PARAMETER: " + h);
            }

            String key = h.substring(0, equalPos);
            String value = h.substring(equalPos + 1);

            Vector pair = new Vector(2);
            pair.addElement(key);
            pair.addElement(value);

            params.addElement(pair);
        }
    }
    else
        params = null;

    // Files to be added to the request
    Vector files = new Vector();
    int numFiles = pResult.optionTimes(CONTENTFILE);

    if (numFiles > 0)
    {
        for (int i = 1; i <= numFiles; i++)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(CONTENTFILE, i), sHandle,
                info.requestNumber);

            if (res.rc != 0) return res;

            String h = res.result;
            int equalPos = h.indexOf("=");

            if (equalPos == -1)
            {
                return new STAFResult(STAFResult.InvalidValue,
                                      "Invalid CONTENTFILE: " + h);
            }

            String key = h.substring(0, equalPos);
            String value = h.substring(equalPos + 1);

            // if CONTENTFILEMACHINE option enabled
            //   get ContentFileMachine with same key / parameter name
            //   copy file from contentFileMachine to
            //    service temp dir
            //   change value to temp file path

            Vector pair = new Vector(2);
            pair.addElement(key);
            pair.addElement(value);

            files.addElement(pair);
       }
    }
    else
        files = null;

    // Get content

    String content = null;

    if (pResult.optionTimes(CONTENT) > 0)
    {
        // Ignore any errors resolving variables in the CONTENT option's value
        // as it could contain "{" characters that don't denote STAF variables

        res = resolveRequestVarIgnoreErrors(
            pResult.optionValue(CONTENT), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        content = res.result;
    }

    // Resolve auto redirection on 3XX returns

    Boolean redirect = null;
    if (pResult.optionTimes(AUTOREDIRECT) > 0)
        redirect = new Boolean(true);
    if (pResult.optionTimes(NOAUTOREDIRECT) > 0)
        redirect = new Boolean(false);

    // Check if the toFile option was specified

    String toFile = null;

    if (pResult.optionTimes(FILE) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(FILE), sHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;

        toFile = res.result;
    }

    String toMachine = null;

    if (pResult.optionTimes(TOMACHINE) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(TOMACHINE), sHandle, info.requestNumber);

        if (res.rc != STAFResult.Ok) return res;

        toMachine = res.result;
    }

    // Assign the url encoded option (defaults to false unless the ENCODED
    // option is specified.  This option says whether the URL is already
    // escaped-encoded

    boolean encoded = false;

    if (pResult.optionTimes(ENCODED) > 0)
        encoded = true;

    // Send request

    try
    {
        session.requestMethod(url, method, headers, params, files, content,
                              redirect, toFile, toMachine, encoded);
    }
    catch (MalformedURLException e)
    {
        if (e.getMessage() != null)
        {
            if (e.getMessage().equals("Bad Method"))
                return new STAFResult(INVALIDMETHOD, method);

            if (e.getMessage().startsWith("Invalid URI"))
                return new STAFResult(UNKNOWNHOST, e.getMessage());

            if (e.getMessage().startsWith("Invalid protocol"))
                return new STAFResult(IOEXCEPTION, e.getMessage());
        }

        return new STAFResult(STAFResult.InvalidRequestString, e.getMessage());

    }
    catch (HttpException e)
    {
        return new STAFResult(STAFResult.UserDefined,
            e.getMessage());

    }
    catch(UnknownHostException uhe)
    {
        return new STAFResult(UNKNOWNHOST, "Unknown Host: " + url);

    }
    catch(ConnectException ce)
    {
        return new STAFResult(CONNECTERROR, "Connection Refused: " + url);

    }
    catch (FileNotFoundException e)
    {
        // error in getting files to be added to a post
        return new STAFResult(STAFResult.FileOpenError, e.getMessage());

    }
    catch (IOException e)
    {
        // error getting page

        // XXX this is a hack
        //  invalid url error not coming back correctly
        if ((e.getMessage() != null) && (e.getMessage().equals(url)))
            return new STAFResult(UNKNOWNHOST, "Unknown Host: " + url);

        // Return the Java stack trace in the result

        StringWriter sr = new StringWriter();
        e.printStackTrace(new PrintWriter(sr));

        return new STAFResult(
            IOEXCEPTION, "Error getting target url\n" + e + "\n" +
            sr.toString());
    }
    catch (STAFException e)
    {
        return new STAFResult(e.rc, e.getMessage());
    }
    catch (Exception e)
    {
        // unexpected error
        java.io.StringWriter sr = new java.io.StringWriter();
        e.printStackTrace(new java.io.PrintWriter(sr));
        System.err.println(sr.toString());

        return new STAFResult(STAFResult.UserDefined,
            "HttpClient Error\n" + e + "\n" + e.getMessage());
    }

    return getHttpResponse(info, pResult);
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleFormAction                                                  */
/* Description: performs the desired form interaction                        */
/* Parameters: info - request info passed to acceptRequest                */
/* Returns: STAFResult.OK or the http result of submitting the form          */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleFormAction(STAFServiceInterfaceLevel30.RequestInfo
                                   info) throws STAFException
{
    // Parse the request

    STAFCommandParseResult pResult = htmlActionParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    String formAction = "SUBMIT FORM";

    if (pResult.optionTimes(RESET) > 0)
        formAction = "RESET FORM";

    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, formAction, fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;

    int sessionId = Integer.parseInt(res.result);

    WebSession session;

    // Get the session

    try
    {
        session = sessionList.getSession(sessionId);

    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    if (!session.isOwner(info.stafInstanceUUID))
    {
        // Verify the requester has at least trust level 4 if not session owner

        trustResult = STAFUtil.validateTrust(
            4, fServiceName, formAction, fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
    }

    // Resolve auto redirection on 3XX returns

    Boolean redirect = null;

    if (pResult.optionTimes(AUTOREDIRECT) > 0)
        redirect = new Boolean(true);

    if (pResult.optionTimes(NOAUTOREDIRECT) > 0)
        redirect = new Boolean(false);

    String id = "";
    int idType = -1;

    if (pResult.optionTimes(ID) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(ID), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.ID_ID_TYPE;

    }
    else if (pResult.optionTimes(NAME) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(NAME), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.NAME_ID_TYPE;

    }
    else if (pResult.optionTimes(INDEX) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(INDEX), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.INDEX_ID_TYPE;

    }
    else
    {
        // default to the 1st form on the page
        id = "1";
        idType = WebSession.INDEX_ID_TYPE;
    }

    String result = "";

    try
    {
        if (pResult.optionTimes(RESET) > 0)
        {
            session.resetForm(id, idType);

        }else if (pResult.optionTimes(SUBMIT) > 0)
        {
            session.submitForm(id, idType, redirect);

            return getHttpResponse(info, pResult);
        }

    }catch (InvalidElementIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());

    }catch (MalformedURLException e)
    {
        if (e.getMessage() != null)
        {
            if (e.getMessage().equals("Bad Method"))
                return new STAFResult(INVALIDMETHOD, e.getMessage());

            if (e.getMessage().startsWith("Invalid URI"))
                return new STAFResult(UNKNOWNHOST, e.getMessage());

            if (e.getMessage().startsWith("Invalid protocol"))
                return new STAFResult(IOEXCEPTION, e.getMessage());
        }

        return new STAFResult(STAFResult.InvalidRequestString, e.getMessage());

    }catch (HttpException e)
    {
        return new STAFResult(STAFResult.UserDefined,
            e.getMessage());

    }catch(UnknownHostException uhe)
    {
        return new STAFResult(UNKNOWNHOST, uhe.getMessage());

    }catch(ConnectException ce)
    {
        return new STAFResult(CONNECTERROR, ce.getMessage());

    }catch (FileNotFoundException e)
    {
        // error in getting files to be added to a post
        return new STAFResult(STAFResult.FileOpenError, e.getMessage());

    }catch (IOException e)
    {
        // error getting page
        return new STAFResult(IOEXCEPTION,
            "Error getting target url\n" + e + "\n" + e.getMessage());

    }catch (Exception e)
    {
        // unexpected error

        return new STAFResult(STAFResult.UserDefined,
            "HttpClient Error\n" + e.getMessage());
    }

    return new STAFResult(STAFResult.Ok, result);
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleFollow                                                      */
/* Description: follows the specified link                                   */
/* Parameters: info - request info passed to acceptRequest                */
/* Returns: STAFResult with the http result of following the link            */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleFollow(STAFServiceInterfaceLevel30.RequestInfo
                                   info) throws STAFException
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "FOLLOW", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = htmlActionParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);

    WebSession session;

    // Get the session

    try
    {
        session = sessionList.getSession(sessionId);

    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    if (!session.isOwner(info.stafInstanceUUID))
    {
        // Verify the requester has at least trust level 4 if not session owner

        trustResult = STAFUtil.validateTrust(
            4, fServiceName, "FOLLOW", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
    }

    // Resolve auto redirection on 3XX returns

    Boolean redirect = null;

    if (pResult.optionTimes(AUTOREDIRECT) > 0)
        redirect = new Boolean(true);

    if (pResult.optionTimes(NOAUTOREDIRECT) > 0)
        redirect = new Boolean(false);

    String id = "";
    int idType = -1;

    if (pResult.optionTimes(ID) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(ID), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.ID_ID_TYPE;
    }
    else if (pResult.optionTimes(NAME) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(NAME), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.NAME_ID_TYPE;
    }
    else if (pResult.optionTimes(INDEX) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(INDEX), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        id = res.result;
        idType = WebSession.INDEX_ID_TYPE;
    }
    else
        return new STAFResult(STAFResult.DoesNotExist,"No Link Specified.");

    try
    {
        session.followLink(id, idType, redirect);
    }
    catch (InvalidElementIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());

    }
    catch (MalformedURLException e)
    {
        if (e.getMessage() != null)
        {
            if (e.getMessage().startsWith("Invalid URI"))
                return new STAFResult(UNKNOWNHOST, e.getMessage());

            if (e.getMessage().startsWith("Invalid protocol"))
                return new STAFResult(IOEXCEPTION, e.getMessage());
        }

        return new STAFResult(STAFResult.InvalidRequestString, e.getMessage());

    }
    catch (HttpException e)
    {
        return new STAFResult(STAFResult.UserDefined, e.getMessage());

    }
    catch(UnknownHostException uhe)
    {
        return new STAFResult(UNKNOWNHOST, uhe.getMessage());

    }
    catch(ConnectException ce)
    {
        return new STAFResult(CONNECTERROR, ce.getMessage());

    }
    catch (IOException e)
    {
        // error getting page

        return new STAFResult(IOEXCEPTION,
            "Error getting target url\n" + e + "\n" + e.getMessage());

    }
    catch (STAFException e)
    {
        return new STAFResult(e.rc, e.getMessage());
    }
    catch (Exception e)
    {
        // unexpected error

        return new STAFResult(
            STAFResult.UserDefined, "HttpClient Error\n" + e.getMessage());
    }

    return getHttpResponse(info, pResult);
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleDelete                                                      */
/* Description: Deltes a cookie from the session.                            */
/* Parameters: info - request info passed to acceptRequest                */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleDelete(STAFServiceInterfaceLevel30.RequestInfo info)
                               throws STAFException
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "DELETE", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = delCloseParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);
    
    WebSession session;

    // Get the session

    try
    {
        session = sessionList.getSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    if (!session.isOwner(info.stafInstanceUUID))
    {
        // Verify the requester has at least trust level 4 since not
        // session owner

        trustResult = STAFUtil.validateTrust(
            4, fServiceName, "DELETE", fLocalMachineName, info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;
    }

    if (pResult.optionTimes(DEFAULTHEADER) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(DEFAULTHEADER), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String key = res.result;

        try
        {
            session.deleteDefaultHeader(key);
        }
        catch (InvalidElementIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }

    }
    else if (pResult.optionTimes(AUTHENTICATIONHOST) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(AUTHENTICATIONHOST), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String key = res.result;
        
        try
        {
            session.deleteAuthenticationSet(key);
        }
        catch (InvalidElementIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }

    }
    else if(pResult.optionTimes(COOKIE) > 0)
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(NAME), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String name = res.result;

        try
        {
            session.deleteCookie(name);
        }
        catch (InvalidCookieIDException e)
        {
            return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
        }
    }

    return new STAFResult(STAFResult.Ok,"");
}

/*****************************************************************************/
/*                                                                           */
/* Method: handleGetPage                                                     */
/* Description: gets the contents of the current page of the specified       */
/*              session                                                      */
/* Parameters: info - request info passed to acceptRequest                */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult handleGetPage(STAFServiceInterfaceLevel30.RequestInfo
                                   info) throws STAFException
{
    // Verify the requester has at least trust level 3

    STAFResult trustResult = STAFUtil.validateTrust(
        3, fServiceName, "GET", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Parse the request

    STAFCommandParseResult pResult = getParser.parse(info.request);

    if (pResult.rc != 0)
    {
        return new STAFResult(STAFResult.InvalidRequestString,
                              pResult.errorBuffer);
    }

    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);

    WebSession session;

    // Get the session

    try
    {
        session = sessionList.getSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    String result = "";

    // Get the content

    if (pResult.optionTimes(FILE) == 0)
    {
        // Get the content as a string

        try
        {
            result = session.getCurrentContents();
        }
        catch (STAFException e)
        {
            return new STAFResult(e.rc, e.getMessage());
        }
        catch (ContentTooLargeException e)
        {
            return new STAFResult(CONTENTTOOLARGE, e.toString());
        }
    }
    else // pResult.optionTimes(FILE) > 0
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(FILE), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String fileName = res.result;

        String machineName = null;

        if (pResult.optionTimes(TOMACHINE) > 0)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(TOMACHINE), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            machineName = res.result;
        }

        // Write the result content to the specified file
        try
        {
            session.writeContentToFile(fileName, machineName);
        }
        catch(IOException e)
        {
            return new STAFResult(
                STAFResult.FileWriteError,
                "Error writing the content to a file.\n" + e.toString());
        }
        catch(STAFException e)
        {
            return new STAFResult(
                e.rc, "Error writing the content to a file.\n" +
                e.getMessage());
        }
        catch(Exception e)
        {
            return new STAFResult(
                STAFResult.FileWriteError,
                "Unexpected exception writing the content to a file.\n" +
                e.toString());
        }
    }

    return new STAFResult(STAFResult.Ok, result);
}

/*****************************************************************************/
/*                                                                           */
/* Method: getHttpResponse                                                   */
/* Description: format the output of a http request.                         */
/* Parameters: info - request info passed to acceptRequest                   */
/*             pResult - parsed result of the accepted request               */
/* Returns: STAFResult with the result of the last http request              */
/*                                                                           */
/*****************************************************************************/

private STAFResult getHttpResponse(STAFServiceInterfaceLevel30.RequestInfo
                                   info, STAFCommandParseResult pResult)
                                   throws STAFException
{
    // Get resolved value for SESSION option and make sure it's an integer

    STAFResult res = STAFUtil.resolveRequestVarAndCheckInt(
        SESSION, pResult.optionValue(SESSION), sHandle, info.requestNumber);

    if (res.rc != 0) return res;
    
    int sessionId = Integer.parseInt(res.result);

    WebSession session;

    // Get the session

    try
    {
        session = sessionList.getSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        return new STAFResult(STAFResult.DoesNotExist, e.getMessage());
    }

    boolean showContent = (pResult.optionTimes(NOCONTENT) == 0);
    boolean showHeaders = (pResult.optionTimes(RETURNHEADERS) > 0);
    boolean status = (pResult.optionTimes(NOSTATUS) == 0);
    boolean ignoreErrors = (pResult.optionTimes(IGNOREERRORS) > 0);

    int rc = STAFResult.Ok;
    
    STAFMarshallingContext mc = new STAFMarshallingContext();
    mc.setMapClassDefinition(fHttpResultMapClass);
    Map resultMap = fHttpResultMapClass.createInstance();
    
    int httpRc = session.getCurrentStatusCode();

    if ((!ignoreErrors) && (httpRc > 399) && (httpRc < 600))
    {
        rc = httpRc + STAFResult.UserDefined;
    }

    HashMap response = session.getHttpResponse();
 
    if (status)
    {
        resultMap.put("statusCode",
                      response.get(WebSession.RETURN_STATUS_CODE));
        resultMap.put("statusMessage",
                      response.get(WebSession.RETURN_STATUS_MESSAGE));
    }

    if (showHeaders)
    {
        resultMap.put("headers", response.get(WebSession.RETURN_HEADERS));
    }

    boolean wroteToFile = false;

    if (pResult.optionTimes(FILE) > 0 )
    {
        res = STAFUtil.resolveRequestVar(
            pResult.optionValue(FILE), sHandle, info.requestNumber);

        if (res.rc != 0) return res;

        String fileName = res.result;

        String machineName = null;

        if (pResult.optionTimes(TOMACHINE) > 0)
        {
            res = STAFUtil.resolveRequestVar(
                pResult.optionValue(TOMACHINE), sHandle, info.requestNumber);

            if (res.rc != 0) return res;

            machineName = res.result;
        }

        try
        {
            // Write the result content to the specified file
            session.writeContentToFile(fileName, machineName);

            if (HTTP.DEBUG)
            {
                System.out.println(
                    "Completed writing content to file " + fileName +
                    " on machine " + machineName);
            }

            // Successfully wrote to file so don't return content in result
            wroteToFile = true;
        }
        catch(IOException ioe)
        {
            String result = "Error writing the content to a file.\n" +
                ioe.toString();

            if (resultMap.get("statusCode") == null)
                result += "\nStatus Code   : <None>";
            else
                result += "\nStatus Code   : " + resultMap.get("statusCode");

            if (resultMap.get("statusMessage") == null)
                result += "\nStatus Message: <None>";
            else
                result += "\nStatus Message: " + resultMap.get("statusMessage");

            if (resultMap.get("content") == null)
                result += "\nHeaders       : <None>";
            else
                result += "\nHeaders       : " + resultMap.get("headers");

            result += "\nContent       : <None>";

            return new STAFResult(STAFResult.FileWriteError, result);
        }
        catch (STAFException se)
        {
            String result = "Error writing the content to a file.\n" +
                se.getMessage();

            if (resultMap.get("statusCode") == null)
                result += "\nStatus Code   : <None>";
            else
                result += "\nStatus Code   : " + resultMap.get("statusCode");

            if (resultMap.get("statusMessage") == null)
                result += "\nStatus Message: <None>";
            else
                result += "\nStatus Message: " + resultMap.get("statusMessage");

            if (resultMap.get("content") == null)
                result += "\nHeaders       : <None>";
            else
                result += "\nHeaders       : " + resultMap.get("headers");

            result += "\nContent       : <None>";

            return new STAFResult(STAFResult.FileWriteError, result);
        }
    }
    
    if (showContent && !wroteToFile)
    {
        try
        {
            resultMap.put("content", session.getCurrentContents());
        }
        catch (STAFException e)
        {
            return new STAFResult(e.rc, e.getMessage());
        }
        catch (ContentTooLargeException e)
        {
            return new STAFResult(CONTENTTOOLARGE, e.toString());
        }
    }

    mc.setRootObject(resultMap);

    return new STAFResult(rc, mc.marshall());
}

/*****************************************************************************/
/*                                                                           */
/* Method: help                                                              */
/* Description: returns service help information                             */
/* Parameters: none                                                          */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult help(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 1

    STAFResult trustResult = STAFUtil.validateTrust(
        1, fServiceName, "HELP", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Return help text for the service
    
    return new STAFResult(STAFResult.Ok, sHelpMsg);
}


/*****************************************************************************/
/*                                                                           */
/* Method: init                                                              */
/* Description: required by interface STAFServiceInterfaceLevel30            */
/*              creates parsers and registers with STAF                      */
/*                                                                           */
/*****************************************************************************/

public STAFResult init(STAFServiceInterfaceLevel30.InitInfo initInfo)
{
    /* Generate parsers */
    requestParser = new STAFCommandParser();

    requestParser.addOption(REQUEST, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(DOPOST, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(DOGET, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(METHOD, 1, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(URL, 1, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(ENCODED, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(CONTENT, 1, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(PARAMETER, 0, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(CONTENTFILE, 0, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(HEADER, 0, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(TOMACHINE, 1, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(SESSION, 1, STAFCommandParser.VALUEREQUIRED);
    requestParser.addOption(IGNOREERRORS, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(NOSTATUS, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(RETURNHEADERS, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(NOCONTENT, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(AUTOREDIRECT, 1, STAFCommandParser.VALUENOTALLOWED);
    requestParser.addOption(NOAUTOREDIRECT, 1, STAFCommandParser.VALUENOTALLOWED);
   
    requestParser.addOptionGroup(URL, 1, 1);
    requestParser.addOptionGroup(DOGET + " " + CONTENTFILE, 0, 1);
    requestParser.addOptionGroup(NOAUTOREDIRECT + " " + AUTOREDIRECT, 0, 1);
    requestParser.addOptionGroup(REQUEST + " " + DOGET + " " +DOPOST, 1, 1);
    requestParser.addOptionGroup(CONTENT + " " + CONTENTFILE, 0, 1);
    requestParser.addOptionGroup(CONTENT + " " + PARAMETER, 0, 1);

    requestParser.addOptionNeed(TOMACHINE, FILE);

    infoParser = new STAFCommandParser();

    infoParser.addOption(QUERY, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(LIST, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(FORMS, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(LINKS, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(COOKIES, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(DEFAULTHEADERS, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(FORMCONTROLS, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(AUTHENTICATIONHOSTS, 1,
                         STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(FORM, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(LINK, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(COOKIE, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(SESSIONS, 1, STAFCommandParser.VALUENOTALLOWED);
    infoParser.addOption(SESSION, 1, STAFCommandParser.VALUEREQUIRED);
    infoParser.addOption(NAME, 1, STAFCommandParser.VALUEREQUIRED);
    infoParser.addOption(ID, 1, STAFCommandParser.VALUEREQUIRED);
    infoParser.addOption(INDEX, 1, STAFCommandParser.VALUEREQUIRED);
    infoParser.addOption(DEFAULTHEADER, 1, STAFCommandParser.VALUEREQUIRED);
    infoParser.addOption(CONTROLNAME, 1, STAFCommandParser.VALUEREQUIRED);
    infoParser.addOption(AUTHENTICATIONHOST, 1,
                         STAFCommandParser.VALUEREQUIRED);

    infoParser.addOptionGroup(QUERY + " " + LIST, 1, 1);
    infoParser.addOptionGroup(COOKIE + " " + LINK + " " + FORM + " " +
                              DEFAULTHEADER + " " + AUTHENTICATIONHOST, 0, 1);
    infoParser.addOptionGroup(COOKIES + " " + LINKS + " " + FORMS + " "
                             + SESSIONS + " " + DEFAULTHEADERS + " " +
                             AUTHENTICATIONHOSTS, 0, 1);
    infoParser.addOptionGroup(NAME + " " + ID + " " + INDEX, 0, 1);

    infoParser.addOptionNeed(SESSION, QUERY + " " + FORMCONTROLS + " " +
                             COOKIES + " " + LINKS + " " + FORMS  + " " +
                             DEFAULTHEADERS + " " + AUTHENTICATIONHOSTS);
    infoParser.addOptionNeed(QUERY, SESSION);
    infoParser.addOptionNeed(LIST, SESSIONS + " " + SESSION);
    infoParser.addOptionNeed(COOKIES + " " + LINKS + " " + FORMS  + " " +
                             DEFAULTHEADERS + " " + AUTHENTICATIONHOSTS,
                             SESSION);
    infoParser.addOptionNeed(COOKIES + " " + LINKS + " " + FORMS  + " " +
                             DEFAULTHEADERS + " " + AUTHENTICATIONHOSTS +
                             " " + FORMCONTROLS, LIST);
    infoParser.addOptionNeed(COOKIE + " " + LINK + " " +
                             DEFAULTHEADER + " " + AUTHENTICATIONHOST, QUERY);
    infoParser.addOptionNeed(COOKIE, NAME);
    infoParser.addOptionNeed(FORM, SESSION);
    infoParser.addOptionNeed(FORM, FORMCONTROLS + " " + QUERY);
    infoParser.addOptionNeed(FORMCONTROLS + " " + CONTROLNAME, FORM);
    infoParser.addOptionNeed(LINK, NAME + " " + ID + " " + INDEX);
    infoParser.addOptionNeed(NAME, FORM + " " + LINK + " " + COOKIE);
    infoParser.addOptionNeed(ID, FORM + " " + LINK);
    infoParser.addOptionNeed(INDEX, FORM + " " + LINK);

    getParser = new STAFCommandParser();

    getParser.addOption(GET, 1, STAFCommandParser.VALUENOTALLOWED);
    getParser.addOption(CONTENT, 1, STAFCommandParser.VALUENOTALLOWED);
    getParser.addOption(SESSION, 1, STAFCommandParser.VALUEREQUIRED);
    getParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    getParser.addOption(TOMACHINE, 1, STAFCommandParser.VALUEREQUIRED);

    getParser.addOptionGroup(GET, 1, 1);
    getParser.addOptionNeed(GET, CONTENT);
    getParser.addOptionNeed(GET, SESSION);
    getParser.addOptionNeed(TOMACHINE, FILE);

    htmlActionParser = new STAFCommandParser();
    
    htmlActionParser.addOption(FOLLOW, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(SUBMIT, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(RESET, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(FORM, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(LINK, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(SESSION, 1, STAFCommandParser.VALUEREQUIRED);
    htmlActionParser.addOption(NAME, 1, STAFCommandParser.VALUEREQUIRED);
    htmlActionParser.addOption(ID, 1, STAFCommandParser.VALUEREQUIRED);
    htmlActionParser.addOption(INDEX, 1, STAFCommandParser.VALUEREQUIRED);
    htmlActionParser.addOption(FILE, 1, STAFCommandParser.VALUEREQUIRED);
    htmlActionParser.addOption(TOMACHINE, 1, STAFCommandParser.VALUEREQUIRED);
    htmlActionParser.addOption(IGNOREERRORS, 1,
                               STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(NOSTATUS, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(RETURNHEADERS, 1,
                               STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(NOCONTENT, 1, STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(AUTOREDIRECT, 1,
                               STAFCommandParser.VALUENOTALLOWED);
    htmlActionParser.addOption(NOAUTOREDIRECT, 1,
                               STAFCommandParser.VALUENOTALLOWED);

    htmlActionParser.addOptionGroup(FOLLOW + " " + SUBMIT + " " + RESET, 1, 1);
    htmlActionParser.addOptionGroup(NAME + " " + ID + " " + INDEX, 0, 1);
    htmlActionParser.addOptionGroup(AUTOREDIRECT + " " + NOAUTOREDIRECT, 0, 1);

    htmlActionParser.addOptionNeed(FOLLOW, LINK);
    htmlActionParser.addOptionNeed(SUBMIT + " " + RESET, FORM);
    htmlActionParser.addOptionNeed(LINK + " " + FORM, SESSION);
    htmlActionParser.addOptionNeed(LINK, NAME + " " + ID + " " + INDEX);
    htmlActionParser.addOptionNeed(FILE + " " + IGNOREERRORS + " " + NOSTATUS
                                   + " " + RETURNHEADERS + " " + NOCONTENT +
                                   " " + AUTOREDIRECT + " " + NOAUTOREDIRECT,
                                   SUBMIT + " " + FOLLOW);
    htmlActionParser.addOptionNeed(TOMACHINE, FILE);

    setParser = new STAFCommandParser();

    setParser.addOption(SET, 1, STAFCommandParser.VALUENOTALLOWED);
    setParser.addOption(FORM, 1, STAFCommandParser.VALUENOTALLOWED);
    setParser.addOption(COOKIE, 1, STAFCommandParser.VALUENOTALLOWED);
    setParser.addOption(SESSION, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(NAME, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(ID, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(INDEX, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(CONTROLNAME, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(VALUE, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(POLICY, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(DEFAULTHEADER, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(AUTHENTICATIONHOST, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(AUTHENTICATIONUSER, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(AUTHENTICATIONPWD, 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(AUTHENTICATIONDOMAIN, 1,
                        STAFCommandParser.VALUEREQUIRED);
    setParser.addOption(AUTOREDIRECT, 1, STAFCommandParser.VALUENOTALLOWED);
    setParser.addOption(NOAUTOREDIRECT, 1, STAFCommandParser.VALUENOTALLOWED);
    setParser.addOption("PARSECONTENT", 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption("HTTPPROXYHOST", 1, STAFCommandParser.VALUEREQUIRED);
    setParser.addOption("HTTPPROXYPORT", 1, STAFCommandParser.VALUEREQUIRED);

    setParser.addOptionGroup(NAME + " " + ID + " " + INDEX, 0, 1);
    setParser.addOptionGroup(FORM + " " + COOKIE +" " +
                             DEFAULTHEADER + " " + AUTOREDIRECT + " " +
                             NOAUTOREDIRECT + " " + AUTHENTICATIONHOST +
                             " PARSECONTENT HTTPPROXYHOST", 0, 1);

    setParser.addOptionNeed(SET, SESSION);
    setParser.addOptionNeed(INDEX + " " + ID, FORM);
    setParser.addOptionNeed(CONTROLNAME, FORM);
    setParser.addOptionNeed(FORM, CONTROLNAME);
    setParser.addOptionNeed(COOKIE, NAME + " " + POLICY);
    setParser.addOptionNeed(CONTROLNAME + " " + DEFAULTHEADER, VALUE);
    setParser.addOptionNeed(AUTHENTICATIONHOST, AUTHENTICATIONUSER);
    setParser.addOptionNeed(AUTHENTICATIONUSER + " " + AUTHENTICATIONPWD
                            + " " + AUTHENTICATIONDOMAIN, AUTHENTICATIONHOST);
    setParser.addOptionNeed("HTTPPROXYPORT", "HTTPPROXYHOST");

    delCloseParser = new STAFCommandParser();

    delCloseParser.addOption(DELETE, 1, STAFCommandParser.VALUENOTALLOWED);
    delCloseParser.addOption(CLOSE, 1, STAFCommandParser.VALUENOTALLOWED);
    delCloseParser.addOption(COOKIE, 1, STAFCommandParser.VALUENOTALLOWED);
    delCloseParser.addOption(NAME, 1, STAFCommandParser.VALUEREQUIRED);
    delCloseParser.addOption(SESSION, 1, STAFCommandParser.VALUEREQUIRED);
    delCloseParser.addOption(DEFAULTHEADER, 1, STAFCommandParser.VALUEREQUIRED);
    delCloseParser.addOption(AUTHENTICATIONHOST, 1,
                             STAFCommandParser.VALUEREQUIRED);

    delCloseParser.addOptionGroup(DELETE + " " + CLOSE, 1, 1);

    delCloseParser.addOptionNeed(COOKIE, NAME);
    delCloseParser.addOptionNeed(DELETE + " " + CLOSE, SESSION);
    delCloseParser.addOptionNeed(DELETE, DEFAULTHEADER + " " + COOKIE + " "
                                + AUTHENTICATIONHOST);
    delCloseParser.addOptionNeed(DEFAULTHEADER + " " + COOKIE + " " +
                                 AUTHENTICATIONHOST, DELETE);

    // Construct map class for LIST SESSIONS result

    fListSessionMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/ListSession");
    fListSessionMapClass.addKey("sessionID",  "ID");
    fListSessionMapClass.addKey("url", "Url");
    fListSessionMapClass.addKey("title", "Title");

    // Construct map class for QUERY SESSION result

    fSessionMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/Session");
    fSessionMapClass.addKey("url", "Url");
    fSessionMapClass.addKey("title", "Title");
    fSessionMapClass.addKey("statusCode", "Status Code");
    fSessionMapClass.addKey("statusMessage", "Status Message");
    fSessionMapClass.addKey("cookiePolicy", "Cookie Policy");
    fSessionMapClass.addKey("followsRedirects", "Follows Redirects");
    fSessionMapClass.addKey("parseContent", "Parse Content");
    fSessionMapClass.addKey("httpProxyHost", "HTTP Proxy Host");
    fSessionMapClass.addKey("httpProxyPort", "HTTP Proxy Port");
    fSessionMapClass.addKey("owner", "Owner");

    // Construct map class for session owner used in QUERY SESSION result

    fSessionOwnerMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/SessionOwner");
    fSessionOwnerMapClass.addKey("instanceUUID", "Instance UUID");
    fSessionOwnerMapClass.addKey("machine", "Machine");
    fSessionOwnerMapClass.addKey("handleName", "Handle Name");
    fSessionOwnerMapClass.addKey("handle", "Handle");

    // Construct map classes for results from REQUEST/DOPOST/DOGET/FOLLOWLINK/
    // and SUBMIT FORM requests (the result varies based on options specified)

    fHttpResultMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/HttpResult");
    fHttpResultMapClass.addKey("statusCode", "Status Code");
    fHttpResultMapClass.addKey("statusMessage", "Status Message");
    fHttpResultMapClass.addKey("headers", "Headers");
    fHttpResultMapClass.addKey("content", "Content");

    // Construct map class for QUERY AUTHENTICATIONHOST result

    fAuthHostMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/AuthenticationHost");

    fAuthHostMapClass.addKey("user", "User");
    fAuthHostMapClass.addKey("password", "Password");
    fAuthHostMapClass.addKey("domain", "Domain");
    
    // Construct map class for QUERY COOKIE result

    fCookieMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/Cookie");

    fCookieMapClass.addKey("value", "Cookie Value");
    fCookieMapClass.addKey("domain", "Domain");
    fCookieMapClass.addKey("path", "Path");
    fCookieMapClass.addKey("expirationDate", "Expiration Date");

    // Construct map class for LIST FORMS result

    fListFormMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/ListForm");
    fListFormMapClass.addKey("formIndex", "Index");
    fListFormMapClass.addKey("formID", "Form ID");
    fListFormMapClass.addKey("formName", "Form Name");

    // Construct map class for QUERY FORM result

    fFormMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/Form");
    fFormMapClass.addKey("formIndex", "Index");
    fFormMapClass.addKey("formID", "Form ID");
    fFormMapClass.addKey("formName", "Form Name");
    fFormMapClass.addKey("method", "Method");
    fFormMapClass.addKey("targetUrl", "Action");
    fFormMapClass.addKey("headerMap", "Headers");

    // Construct map class for QUERY FORM CONTROLNAME result

    fFormControlMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/FormControlName");
    fFormControlMapClass.addKey("type", "Type");
    fFormControlMapClass.addKey("disabled", "Disabled");
    fFormControlMapClass.addKey("readOnly", "Read Only");
    fFormControlMapClass.addKey("value", "Value");
    fFormControlMapClass.addKey("possibleValueList", "Possible Values");
    
    // Construct map class for LIST LINKS result

    fListLinkMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/ListLink");
    fListLinkMapClass.addKey("linkIndex", "Index");
    fListLinkMapClass.addKey("linkID", "Link ID");
    fListLinkMapClass.addKey("linkName", "Link Name");

    // Construct map class for QUERY LINK result

    fLinkMapClass = new STAFMapClassDefinition(
        "STAF/Service/HTTP/Link");
    fLinkMapClass.addKey("linkIndex", "Index");
    fLinkMapClass.addKey("linkID", "Link ID");
    fLinkMapClass.addKey("linkName", "Link Name");
    fLinkMapClass.addKey("href", "Href");


    /* Set service name */
    
    fServiceName = initInfo.name;

    /* Create STAFHandle */

    try
    {
        sHandle = new STAFHandle("STAF/Service/" + initInfo.name);
    }
    catch(STAFException se)
    {
        se.printStackTrace();
        return new STAFResult(STAFResult.STAFRegistrationError,
                              se.toString());
    }

    STAFResult res;

    // Resolve the line separator variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Sep/Line}", sHandle);

    if (res.rc != STAFResult.Ok) return res;

    sLineSep = res.result;

    // Verify that the required version of STAF is running on the
    // local service machine.  
    // Note:  Method compareSTAFVersion was added in STAF V3.1.0

    try
    {
        res = STAFUtil.compareSTAFVersion(
            "local", sHandle, REQUIREDSTAFVERSION);

        if (res.rc != STAFResult.Ok)
        {
            if (res.rc == STAFResult.InvalidSTAFVersion)
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Minimum required STAF version for this service " +
                    "is not running." + sLineSep + res.result);
            }
            else
            {
                return new STAFResult(
                    STAFResult.ServiceConfigurationError,
                    "Error verifying the STAF version. RC: " + res.rc +
                    ", Additional info: " + res.result);
            }
        }
    }
    catch (Error err)
    {
        return new STAFResult(
            STAFResult.ServiceConfigurationError,
            "This service requires STAF Version " + REQUIREDSTAFVERSION +
            " or later."); 
    }

    // Resolve the file separator variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Sep/File}", sHandle);

    if (res.rc != STAFResult.Ok)
    {
        try
        {
            sHandle.unRegister();
            return res;
        }
        catch (STAFException ex)
        {
            return res;
        }
    }

    String fileSep = res.result;
    
    // Resolve the machine variable for the local machine

    res = STAFUtil.resolveInitVar("{STAF/Config/Machine}", sHandle);

    if (res.rc != STAFResult.Ok)
    {
        try
        {
            sHandle.unRegister();
            return res;
        }
        catch (STAFException ex)
        {
            return res;
        }
    }

    fLocalMachineName = res.result;

    fTempDir = initInfo.writeLocation + fileSep + "tmp" + fileSep +
        "service" + fileSep + fServiceName + fileSep;

    File tempdir = new File (fTempDir);
        
    try
    {
        tempdir.mkdirs();
    }
    catch (SecurityException e)
    {
        return new STAFResult(
            STAFResult.FileOpenError,
            "Access to HTTP service temporary directory denied.  " +
            "Unable to make directory " + fTempDir);
    }
    
    // Assign the help text string for the service

    sHelpMsg = "*** " + fServiceName + " Service Help ***" +
        sLineSep + sLineSep +
        "OPEN    SESSION" +
        sLineSep +
        "CLOSE   SESSION <Session>" +
        sLineSep +
        "LIST    SESSIONS" +
        sLineSep +
        "LIST    <FORMS | LINKS | COOKIES | DEFAULTHEADERS | AUTHENTICATIONHOSTS |" +
        sLineSep +
        "        CONTROLNAMES FORM [NAME <Form Name> | ID <Id> | INDEX <Index>]>" +
        sLineSep +
        "        SESSION <Session>" +
        sLineSep +
        "QUERY   SESSION <Session>" +
        sLineSep +
        "GET     SESSION <Session> CONTENT [FILE <File Name> [TOMACHINE <Machine>]]" +
        sLineSep + sLineSep +
        "REQUEST METHOD <Http Method> URL <Target Url> [ENCODED] [CONTENT <Content>]" +
        sLineSep +
        "        [HEADER <Key>=<Value>]... [PARAMETER <Key>=<Value>]..." +
        sLineSep +
        "        [CONTENTFILE <Key>=<File Name>]..." +
        sLineSep +
        "        [FOLLOWREDIRECT | DONOTFOLLOWREDIRECT] [SESSION <Session>]" +
        sLineSep +
        "        [FILE <Filename> [TOMACHINE <Machine>]]" +
        sLineSep +
        "        [IGNOREERRORS] [NOSTATUS] [RETURNHEADERS] [RETURNNOCONTENT]" +
        sLineSep +
        "DOPOST  URL <Target Url> [ENCODED] [CONTENT <Content>]" +
        sLineSep +
        "        [HEADER <Key>=<Value>]... [PARAMETER <Key>=<Value>]..."+
        sLineSep +
        "        [CONTENTFILE <Key>=<File Name>]..." +
        sLineSep +
        "        [FOLLOWREDIRECT | DONOTFOLLOWREDIRECT] [SESSION <Session>]" +
        sLineSep +
        "        [FILE <Filename> [TOMACHINE <Machine>]]" +
        sLineSep +
        "        [IGNOREERRORS] [NOSTATUS] [RETURNHEADERS] [RETURNNOCONTENT]" +
        sLineSep +
        "DOGET   URL <Target Url> [ENCODED] [CONTENT <Content>]" +
        sLineSep +
        "        [HEADER <Key>=<Value>]... [PARAMETER <Key>=<Value>]..." +
        sLineSep +
        "        [FOLLOWREDIRECT | DONOTFOLLOWREDIRECT] [SESSION <Session>]" +
        sLineSep +
        "        [FILE <Filename> [TOMACHINE <Machine>]]" +
        sLineSep +
        "        [IGNOREERRORS] [NOSTATUS] [RETURNHEADERS] [RETURNNOCONTENT]"+
        sLineSep + sLineSep +
        "SET     <FOLLOWREDIRECT | DONOTFOLLOWREDIRECT> SESSION <Session>" +
        sLineSep +
        "SET     PARSECONTENT <Enabled | Disabled | Autodetect> SESSION <Session>" +
        sLineSep +
        "SET     HTTPPROXYHOST <Host> [HTTPPROXYPORT <Port>] SESSION <Session>" +
        sLineSep + sLineSep +
        "QUERY   AUTHENTICATIONHOST <Host> SESSION <Session>" +
        sLineSep +
        "SET     AUTHENTICATIONHOST <Host> AUTHENTICATIONUSER <User>" +
        sLineSep +
        "        [AUTHENTICATIONPASSWORD <Password>] [AUTHENTICATIONDOMAIN <Domain>]" +
        sLineSep +
        "        SESSION <Session>" +
        sLineSep +
        "DELETE  AUTHENTICATIONHOST <Host> SESSION <Session>" +
        sLineSep + sLineSep +
        "QUERY   DEFAULTHEADER <Key> SESSION <Session>" +
        sLineSep +
        "SET     DEFAULTHEADER <Key> VALUE <Value> SESSION <Session>" +
        sLineSep +
        "DELETE  DEFAULTHEADER <Key> SESSION <Session>" +
        sLineSep + sLineSep +
        "QUERY   COOKIE NAME <Name> SESSION <Session>" +
        sLineSep +
        "SET     COOKIE <NAME <Name> VALUE <Value>> | <POLICY <policy>>" +
        sLineSep +
        "        SESSION <Session>" +
        sLineSep +
        "DELETE  COOKIE NAME <Name> SESSION <Session>" +
        sLineSep + sLineSep +
        "QUERY   FORM [NAME <Form Name> | ID <Id> | INDEX <Index>] [CONTROLNAME <Name>]" +
        sLineSep +
        "        SESSION <Session>" +
        sLineSep +
        "SET     FORM [NAME <Form Name> | ID <Id> | INDEX <Index>]" +
        sLineSep +
        "        CONTROLNAME <Name> VALUE <Value> SESSION <Session>" +
        sLineSep +
        "SUBMIT  FORM [NAME <Form Name> | ID <Id> | INDEX <Index>] SESSION <Session>" +
        sLineSep +
        "        [FOLLOWREDIRECT | DONOTFOLLOWREDIRECT]" +
        sLineSep +
        "        [FILE <Filename> [TOMACHINE <Machine>]]" +
        sLineSep +
        "        [IGNOREERRORS] [NOSTATUS] [RETURNHEADERS] [RETURNNOCONTENT]" +
        sLineSep +
        "RESET   FORM [NAME <Form Name> | ID <Id> | INDEX <Index>] SESSION <Session>" +
        sLineSep + sLineSep +
        "QUERY   LINK <NAME <Name> | ID <Id> | INDEX <Index>> SESSION <Session>" +
        sLineSep +
        "FOLLOW  LINK <NAME <Name> | ID <Id> | INDEX <Index>> SESSION <Session>" +
        sLineSep +
        "        [FOLLOWREDIRECT | DONOTFOLLOWREDIRECT]" +
        sLineSep +
        "        [FILE <Filename> [TOMACHINE <Machine>]]" +
        sLineSep +
        "        [IGNOREERRORS] [NOSTATUS] [RETURNHEADERS] [RETURNNOCONTENT]" +
        sLineSep + sLineSep +
        "HELP" +
        sLineSep + sLineSep +
        "VERSION";

    /* Register RCs with the HELP service */

    int rc = this.registerHelp(initInfo.name);

    if (rc != STAFResult.Ok)
        return new STAFResult(rc, "Error registering RCs with HELP service.");

    /* Initialize the session list */

    sessionList = new SessionList();

    /* Finished */

    return new STAFResult(rc);
}

/*****************************************************************************/
/*                                                                           */
/* Method: registerHelp                                                      */
/* Description: registers all HTTP return codes with the HELP service        */
/* Parameters: serviceName - name which this service is registered as        */
/* Returns: int representation of STAFResult                                 */
/*                                                                           */
/*****************************************************************************/

private int registerHelp(String name)
{
    try
    {
        String request = "REGISTER SERVICE " + name + " ERROR " + UNKNOWNHOST +
            " INFO \"" + UNKNOWNHOSTInfo + "\" DESCRIPTION \"" +
            UNKNOWNHOSTDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);

        request = "REGISTER SERVICE " + name + " ERROR " + CONNECTERROR +
            " INFO \"" + CONNECTERRORInfo + "\" DESCRIPTION \"" +
            CONNECTERRORDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);

        request = "REGISTER SERVICE " + name + " ERROR " + INVALIDMETHOD +
            " INFO \"" + INVALIDMETHODInfo + "\" DESCRIPTION \"" +
            INVALIDMETHODDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);

        request = "REGISTER SERVICE " + name + " ERROR " + IOEXCEPTION +
            " INFO \"" + IOEXCEPTIONInfo + "\" DESCRIPTION \"" +
            IOEXCEPTIONDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);

        request = "REGISTER SERVICE " + name + " ERROR " + CONTENTTOOLARGE +
            " INFO \"" + CONTENTTOOLARGEInfo + "\" DESCRIPTION \"" +
            CONTENTTOOLARGEDesc + "\"";
        sHandle.submit("LOCAL", "HELP", request);

    }catch(STAFException se)
    {
        return se.rc;
    }

    return STAFResult.Ok;

}

/*****************************************************************************/
/*                                                                           */
/* Method: unRegisterHelp                                                    */
/* Description: un-registers all HTTP return codes with the HELP service     */
/* Returns: int representation of STAFResult                                 */
/*                                                                           */
/*****************************************************************************/

private int unRegisterHelp()
{
    try
    {
        String request = "UNREGISTER SERVICE " + fServiceName + " ERROR "
                       + UNKNOWNHOST;
        sHandle.submit("LOCAL", "HELP", request);

        request = "UNREGISTER SERVICE " + fServiceName + " ERROR " + CONNECTERROR;
        sHandle.submit("LOCAL", "HELP", request);

        request = "UNREGISTER SERVICE " + fServiceName + " ERROR " + INVALIDMETHOD;
        sHandle.submit("LOCAL", "HELP", request);

        request = "UNREGISTER SERVICE " + fServiceName + " ERROR " + IOEXCEPTION;
        sHandle.submit("LOCAL", "HELP", request);

        request = "UNREGISTER SERVICE " + fServiceName + " ERROR " +
            CONTENTTOOLARGE;
        sHandle.submit("LOCAL", "HELP", request);

    }
    catch(STAFException se)
    {
        return se.rc;
    }

    return STAFResult.Ok;
}

/*****************************************************************************/
/*                                                                           */
/* Method: term                                                              */
/* Description: required by interface STAFServiceInterfaceLevel30            */
/*                                                                           */
/*****************************************************************************/

public STAFResult term()
{
    // Close any open sessions
    
    HashMap[] list = sessionList.listSessions();
        
    for (int i = 0; i < list.length; i++)
    {
        String sessionIdString = (String)list[i].get(WebSession.ID);
        int sessionId = Integer.parseInt(sessionIdString);
        
        // Close session ID

        try
        {
            WebSession session = sessionList.getSession(sessionId);
            session.releaseConnection();
            sessionList.deleteSession(sessionId);
        }
        catch (Exception e)
        {
            System.out.println(
                "Error closing session " + sessionId +
                " while shutting down the " + fServiceName + " service.\n" +
                e.toString());
        }    
    }

    // Un-register Help Data

    unRegisterHelp();

    // Un-register the service handle

    try
    {
        sHandle.unRegister();
    }
    catch (STAFException ex)
    {
        return new STAFResult(STAFResult.STAFRegistrationError,
                              ex.toString());
    }

    return new STAFResult(STAFResult.Ok);
    
}

/*****************************************************************************/
/*                                                                           */
/* Method: version                                                           */
/* Description: returns service version information                          */
/* Parameters: none                                                          */
/* Returns: STAFResult.OK                                                    */
/*                                                                           */
/*****************************************************************************/

private STAFResult version(STAFServiceInterfaceLevel30.RequestInfo info)
{
    // Verify the requester has at least trust level 1

    STAFResult trustResult = STAFUtil.validateTrust(
        1, fServiceName, "VERSION", fLocalMachineName, info);

    if (trustResult.rc != STAFResult.Ok) return trustResult;

    // Return the service's version

    return new STAFResult(STAFResult.Ok, VERSIONINFO);
}

/*****************************************************************************/
/*                                                                           */
/* Method: cleanupNonSessionMethod                                           */
/* Description: Perform clean-up for the temporary session (used by a        */
/*              non-session method).                                         */
/* Parameters : session id (for the temporary non-session method request)    */
/* Returns    : Nothing                                                      */
/*                                                                           */
/*****************************************************************************/

private void cleanupNonSessionMethod(int sessionId)
{
    // Delete the temporary file used by the temporary session, if exists
    try
    {
        (new File(getTempDir() + "session" + sessionId + ".tmp")).delete();
    }
    catch (Exception e)
    {
        // Ignore any exceptions
    }

    // Delete the temporary session
    try
    {
        sessionList.deleteSession(sessionId);
    }
    catch (InvalidSessionIDException e)
    {
        // Do nothing
    }
}

/***********************************************************************/
/* Description:                                                        */
/*   This method resolves any STAF variables that are contained within */
/*   the string passed in by submitting a                              */
/*       RESOLVE REQUEST <request#> STRING <value> IGNOREERRORS        */
/*   request to the VAR service on the local system.                   */
/*   Note: Can't use STAFUtil.resolveRequestVar() method because it    */
/*         doesn't support specifying the IGNOREERRORS option for a    */
/*         VAR RESOLVE request.                                        */
/*                                                                     */
/* Input:  String that may contain STAF variables to be resolved       */
/*         STAF handle                                                 */
/*         Request number                                              */
/*                                                                     */
/* Returns:                                                            */
/*   STAFResult.rc = the return code (STAFResult.Ok if successful)     */
/*   STAFResult.result = the resolved value if successful              */
/*                       an error message if not successful            */
/***********************************************************************/
private static STAFResult resolveRequestVarIgnoreErrors(
    String value, STAFHandle handle, int requestNumber)
{
    if (value.indexOf("{") != -1)
    {
        // The string may contains STAF variables
            
        STAFResult resolvedResult = handle.submit2(
            "local", "VAR", "RESOLVE REQUEST " + requestNumber +
            " STRING " + STAFUtil.wrapData(value) + " IGNOREERRORS");

        return resolvedResult;
    }

    return new STAFResult(STAFResult.Ok, value);
}

public String getTempDir() { return fTempDir; }
public int getTempFileCount() { return fTempFileCount++; }
public String getLocalMachineName() { return fLocalMachineName; }
public STAFHandle getServiceHandle() { return sHandle; }

} // end class HTTP
