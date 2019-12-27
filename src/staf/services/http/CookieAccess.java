package com.ibm.staf.service.http;

/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import org.apache.commons.httpclient.*;
import org.apache.commons.httpclient.cookie.CookiePolicy;

import java.util.Vector;
import java.util.HashMap;
import java.util.Date;

/*****************************************************************************/
/*                                                                           */
/* Class: CookieAccess                                                       */
/* Description: This class gives the HTTP service access to package          */
/*              restricted functionality of HTTPUnit for cookie manipulation.*/
/*                                                                           */
/*****************************************************************************/

public class CookieAccess 
{
    public static final String NAME="NAME";
    public static final String VALUE="VALUE";
    public static final String DOMAIN="DOMAIN";
    public static final String EXPIRATION="EXPIRATION";
    public static final String PATH="PATH";

/*****************************************************************************/
/*                                                                           */
/* Method: addCookie                                                         */
/* Description: Add a new cookie to the cookie jar.                          */
/* Parameters: theCookie - the cookie to be added                            */
/*             session - the session that contains this cookie               */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public static void addCookie ( Cookie theCookie, HttpClient session)
    {
        session.getState().addCookie(theCookie);
    }
    
    public static int findCookie (String name, Cookie[] cookies)
                                   throws InvalidCookieIDException
    {
        for (int i = 0; i < cookies.length; i++)
            if (cookies[i].getName().equals(name))
                return i;
        
        throw new InvalidCookieIDException(name, "");        
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: deleteCookie                                                      */
/* Description: Delete the specified cookie from the session.  If the cookie */
/*              does not exist, no error is declared.                        */
/* Parameters: name - name of the cookie                                     */
/*             session - the session that contains this cookie              */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public static void deleteCookie(String name, HttpClient session)
                         throws InvalidCookieIDException
    {
        Cookie [] cookies = session.getState().getCookies();
        
        int indx = findCookie(name, cookies);    
        Date purgeDate = new Date (10);
        cookies[indx].setExpiryDate(purgeDate);
        
        session.getState().purgeExpiredCookies(purgeDate);    
        
    }

/*****************************************************************************/
/*                                                                           */
/* Method: createCookie                                                      */
/* Description: create a new cookie                                          */
/* Parameters: name - name of the new cookie                                 */
/*             value - value for the new cookie                              */
/* Returns: a new Cookie                                                     */
/*                                                                           */
/*****************************************************************************/    
        
    public static Cookie createCookie (String name, String value)
    {
        
        Cookie newCookie = new Cookie ();
        newCookie.setName(name);
        newCookie.setValue(value);
        
        return newCookie;
    }
        
/*****************************************************************************/
/*                                                                           */
/* Method: cookieSummary                                                     */
/* Description: summarizes the state of a cookie                             */
/* Parameters: theCookie - the cookie to be summarized                       */
/* Returns: a summary of the cookie: name value path domain                  */
/*                                                                           */
/*****************************************************************************/    

    public static HashMap cookieSummary (Cookie theCookie)
    {
        HashMap summary = new HashMap();
        
        summary.put(NAME,theCookie.getName());
        summary.put(VALUE,theCookie.getValue());
        summary.put(PATH,theCookie.getPath());
        summary.put(DOMAIN,theCookie.getDomain());
        summary.put(EXPIRATION,theCookie.getExpiryDate()/*.toString()*/);
                         
        return summary;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: setCookieValue                                                    */
/* Description: sets the value of the specifed cookie to the specified value */
/* Parameters: name - name of the cookie                                     */
/*             value - new value for the cookie                              */
/*             session - the session that contains this cookie               */
/* Returns: void                                                             */
/* Throws: InvalidCookieIDException if the cookie does not exist             */
/*                                                                           */
/*****************************************************************************/    

    public static void setCookieValue(String name, String value, 
                                        HttpClient session)
                         throws InvalidCookieIDException
    {

        Cookie [] cookies = session.getState().getCookies();
        
        int indx = findCookie(name, cookies);        
        
        cookies[indx].setValue(value);                
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCookieValue                                                    */
/* Description: gets the value of the specifed cookie                        */
/* Parameters: name - name of the cookie                                     */
/*             session - the session that contains this cookie               */
/* Returns: cookie value                                                     */
/* Throws: InvalidCookieIDException if the cookie does not exist             */
/*                                                                           */
/*****************************************************************************/    

    public static String getCookieValue(String name, HttpClient session)
                         throws InvalidCookieIDException
    {

        Cookie [] cookies = session.getState().getCookies();
        
        int indx = findCookie(name, cookies);        
        
        return cookies[indx].getValue();                
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCookieSummary                                                  */
/* Description: gets the summary of the specifed cookie                      */
/* Parameters: name - name of the cookie                                     */
/*             session - the session that contains this cookie               */
/* Returns: summary of a cookie                                              */
/* Throws: InvalidCookieIDException if the cookie does not exist             */
/*                                                                           */
/*****************************************************************************/    

    public static HashMap getCookieSummary(String name, HttpClient session)
                         throws InvalidCookieIDException
    {

        Cookie [] cookies = session.getState().getCookies();
        
        int indx = findCookie(name, cookies);        
        
        return cookieSummary(cookies[indx]);                
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: getCookieNames                                                    */
/* Description: get a list of cookies associated with the specified session. */
/* Parameters: session - the session to get the cookielist from              */
/* Returns: a list of cookies associated with this session                   */
/*                                                                           */
/*****************************************************************************/    
    
    public static Vector getCookieNames(HttpClient session) 
    {

        Cookie [] cookies = session.getState().getCookies();
        Vector names = new Vector();
        for (int i = 0; i < cookies.length; i++)
            names.addElement(cookies[i].getName());
            
        return names;
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: setCookiePolicy                                                   */
/* Description: sets the cookie policy for the specified session.            */
/* Parameters: session - the session to get the cookielist from              */
/*             policy - the cookie policy for the session.  This must be a   */
/*                      valid type for HttpClient                            */
/*                      NETSCAPE, RFC2109, COMPATIBILITY, IGNORE             */
/*                      a request for another policy type will result in use */
/*                      of HttpClient's default cookie handling policy       */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    
    
    public static void setCookiePolicy(String policy, HttpClient session) 
    {
        String cp = null;
        if (policy.equalsIgnoreCase("NETSCAPE"))
            cp = CookiePolicy.NETSCAPE;
        else if (policy.equalsIgnoreCase("IGNORE"))
            cp = CookiePolicy.IGNORE_COOKIES;
        else if (policy.equalsIgnoreCase("RFC2109"))
            cp = CookiePolicy.RFC_2109;
        else if (policy.equalsIgnoreCase("COMPATIBILITY"))
            cp = CookiePolicy.BROWSER_COMPATIBILITY;
        else
            cp = CookiePolicy.DEFAULT;
            
        session.getParams().setCookiePolicy(cp);        
    }    
    
/*****************************************************************************/
/*                                                                           */
/* Method: purgeExpiredCookies                                               */
/* Description: Purge all cookies in a session which expire before a given   */
/*              date.                                                        */
/* Parameters: purgeDate - the date before which to expire cookies           */
/*             session - the session that contains this cookies              */
/* Returns: void                                                             */
/*                                                                           */
/*****************************************************************************/    

    public static void purgeExpiredCookies(Date purgeDate, HttpClient session)
    {
        session.getState().purgeExpiredCookies(purgeDate);        
    }
    
/*****************************************************************************/
/*                                                                           */
/* Method: listCookies                                                       */
/* Description: get a list of cookies in the session.                        */
/* Parameters: none                                                          */
/* Returns: a list of link details                                           */
/*                                                                           */
/*****************************************************************************/    
    
    public static HashMap[] listCookies(HttpClient session)
    {
        Cookie [] cookies = session.getState().getCookies();
        
        HashMap [] list = new HashMap[cookies.length];    
        
        for (int i = 0; i < list.length; i++)
            list[i] = cookieSummary(cookies[i]);
            
        return list;
    }    
        
}