/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

import com.installshield.wizard.*;

public class STAFSelectedComponents extends WizardAction
{
    private String selectedComponents = "";
    private String blankSpace = 
        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

    public void execute(WizardBeanEvent event)
    {
        selectedComponents = blankSpace + "Base STAF components<br>";

        // External Services
        if (resolveString("$P(externalServices.active)").equals("true"))
        {
            boolean firstService = true;        

            selectedComponents += blankSpace;
            
            selectedComponents += 
                resolveString("$P(externalServices.description)") + ":  ";
                
            if (resolveString("$P(logService.active)").equals("true"))
            {
                if (!firstService) selectedComponents += ", ";
                firstService = false;         
                
                selectedComponents += 
                    resolveString("$P(logService.description)");
            }
            
            if (resolveString("$P(monitorService.active)").equals("true"))
            {
                if (!firstService) selectedComponents += ", ";
                firstService = false;
                
                selectedComponents += 
                    resolveString("$P(monitorService.description)");
            }
            
            if (resolveString("$P(resourcePoolService.active)").equals("true"))
            {
                if (!firstService) selectedComponents += ", ";
                firstService = false;
                
                selectedComponents += 
                    resolveString("$P(resourcePoolService.description)");
            }

            if (resolveString("$P(zipService.active)").equals("true"))
            {
                if (!firstService) selectedComponents += ", ";
                firstService = false;
                
                selectedComponents += 
                    resolveString("$P(zipService.description)");
            }           
            
            selectedComponents += "<br>";
        }
            
        // Language Support
        if (resolveString("$P(languageSupport.active)").equals("true"))
        {
            boolean firstLanguage = true;
            
            selectedComponents += blankSpace;            
           
            selectedComponents += 
                resolveString("$P(languageSupport.description)") + ":  ";
             
            if (resolveString("$P(cSupport.active)").equals("true"))
            {
                if (!firstLanguage) selectedComponents += ", ";
                firstLanguage = false;         
                
                selectedComponents += 
                    resolveString("$P(cSupport.description)");
            }
   
            if (resolveString("$P(javaSupport.active)").equals("true"))
            {
                if (!firstLanguage) selectedComponents += ", ";
                firstLanguage = false;         
                
                selectedComponents += 
                    resolveString("$P(javaSupport.description)");
            }
            
            if (resolveString("$P(rexxSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinuxOrAix)").equals("true"))
                {
                    if (!firstLanguage) selectedComponents += ", ";
                    firstLanguage = false;         
                
                    selectedComponents += 
                        resolveString("$P(rexxSupport.description)");
                }
            }
            
            if (resolveString("$P(tclSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinux)").equals("true"))                
                {
                    if (!firstLanguage) selectedComponents += ", ";
                    firstLanguage = false;         
                
                    selectedComponents += 
                        resolveString("$P(tclSupport.description)");
                }
            }

            if (resolveString("$P(pythonSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinux)").equals("true"))                
                {
                    if (!firstLanguage) selectedComponents += ", ";
                    firstLanguage = false;         
                
                    selectedComponents += 
                        resolveString("$P(pythonSupport.description)");
                }
            }
            
            if (resolveString("$P(perlSupport.active)").equals("true"))
            {
                if (resolveString(
                        "$W(stafPlatform.winOrLinux)").equals("true"))                
                {
                    if (!firstLanguage) selectedComponents += ", ";
                    firstLanguage = false;         
                
                    selectedComponents += 
                        resolveString("$P(perlSupport.description)");
                }
            }
           
            selectedComponents += "<br>"; 
        }
        
        // Documentation
        if (resolveString("$P(documentation.active)").equals("true"))
        {
            selectedComponents += blankSpace;            
           
            selectedComponents += 
                resolveString("$P(documentation.description)");
                
            selectedComponents += "<br>";
        }        
        
        // Samples and Demos
        if (resolveString("$P(samplesDemos.active)").equals("true"))
        {
            selectedComponents += blankSpace;            
           
            selectedComponents += 
                resolveString("$P(samplesDemos.description)");
                
            selectedComponents += "<br>";
        }
        
        // Service developer support
        if (resolveString("$P(serviceDeveloper.active)").equals("true"))
        {
            selectedComponents += blankSpace;            
           
            selectedComponents += 
                resolveString("$P(serviceDeveloper.description)");
                
            selectedComponents += "<br>";
        }
        
        // Full codepage support
        if (resolveString("$P(optionalCodepageSupport.active)").equals("true"))
        {
            selectedComponents += blankSpace;            
           
            selectedComponents += 
                resolveString("$P(optionalCodepageSupport.description)");
                
            selectedComponents += "<br>";
        }
    }
    
    public String getSelectedComponents()
    {
        return selectedComponents;
    }
    
    public void setSelectedComponents(String components)
    {
        selectedComponents = components;
    }    
    
}