/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2003                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import java.util.HashMap;


// Helper class for processing the Extension Jar List

public class STAXExtension
{
    public STAXExtension()
    { /* Do Nothing */ }

    // Constructor used by old EXTENSIONFILE parameter (not XML)
    public STAXExtension(String jarFileName,
                         HashMap<String, String> includeElementMap)
    {
        fJarFileName = jarFileName;
        fIncludeElementMap = includeElementMap;
    }

    public String getJarFileName() { return fJarFileName; }
    public String getExtVersion() { return fExtVersion; }
    public String getExtDescription() { return fExtDescription; }
    public String getRequiredServiceVersion()
    { return fRequiredServiceVersion; }
    public String getRequiredMonitorVersion()
    { return fRequiredMonitorVersion; }
    
    public HashMap<String, String> getParmMap()
    {
        return fParmMap;
    }
    
    public HashMap<String, String> getMonitorExtensionMap()
    {
        return fMonitorExtensionMap;
    }
    
    // Contain any excluded elements via a <exclude-element> in an
    // EXTENSIONXMLFILE
    public HashMap<String, String> getExcludeElementMap()
    {
        return fExcludeElementMap;
    }

    // Contains any included elements via a <include-element> in an
    // EXTENSIONXMLFILE or via a comment (#) in a EXTENSIONFILE
    public HashMap<String, String> getIncludeElementMap()
    {
        return fIncludeElementMap;
    }

    // Complete map of all supported elements
    public HashMap<String, String> getSupportedElementMap()
    {
        return fSupportedElementMap;
    }

    // Complete map of all unsupported elements
    public HashMap<String, String> getUnsupportedElementMap()
    {
        return fUnsupportedElementMap;
    }

    public void setJarFileName(String name)
    {
        fJarFileName = name;
    }

    public void setExtVersion(String version)
    {
        fExtVersion = version;
    }

    public void setExtDescription(String description)
    {
        fExtDescription = description;
    }

    public void setRequiredServiceVersion(String version)
    {
        fRequiredServiceVersion = version;
    }

    public void setRequiredMonitorVersion(String version)
    {
        fRequiredMonitorVersion = version;
    }

    public boolean setParm(String name, String value)
    {
        if (fParmMap.containsKey(name))
        {
            return false;
        }

        fParmMap.put(name, value);
        return true;
    }

    public void setMonitorExtension(String name, String className)
    {
        fMonitorExtensionMap.put(name, className);
    }

    public void setSupportedElement(String elementName, String factoryClassName)
    {
        fSupportedElementMap.put(elementName, factoryClassName);
    }

    public void setUnsupportedElement(String elementName)
    {
        fUnsupportedElementMap.put(elementName, null);
    }

    public boolean setExcludeElement(String name)
    {
        if (fExcludeElementMap.containsKey(name))
        {
            return false;
        }

        fExcludeElementMap.put(name, null);
        return true;
    }

    public boolean setIncludeElement(String name)
    {
        if (fIncludeElementMap.containsKey(name))
        {
            return false;
        }

        fIncludeElementMap.put(name, null);
        return true;
    }
    
    private String  fJarFileName = new String();
    private String  fExtVersion     = new String();
    private String  fExtDescription = new String();
    private String  fRequiredServiceVersion = new String("<None>");
    private String  fRequiredMonitorVersion = new String("<None>");

    private HashMap<String, String> fParmMap =
        new HashMap<String, String>();
    private HashMap<String, String> fSupportedElementMap =
        new HashMap<String, String>();
    private HashMap<String, String> fUnsupportedElementMap =
        new HashMap<String, String>();
    private HashMap<String, String> fIncludeElementMap =
        new HashMap<String, String>();
    private HashMap<String, String> fExcludeElementMap =
        new HashMap<String, String>();
    private HashMap<String, String> fMonitorExtensionMap =
        new HashMap<String, String>();
}
