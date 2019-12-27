import com.ibm.staf.*;
import com.ibm.staf.service.*;

public class SampleService1 implements STAFServiceInterfaceLevel3
{
    private STAFHandle fHandle;
    private final String kVersion = "1.0.0";
    private STAFCommandParser fListParser;
    private STAFCommandParser fQueryParser;
    private STAFCommandParser fAddParser;
    private STAFCommandParser fDeleteParser;
    private String fLineSep;
    
    public SampleService1() {}    
    
    public int init(STAFServiceInterfaceLevel3.InitInfo info)
    {
        int rc = STAFResult.Ok;

        try
        {
            fHandle = new STAFHandle("STAF/SERVICE/" + info.name);
        }
        catch (STAFException e)
        {
            return STAFResult.STAFRegistrationError;
        }      
       
            // instantiate parsers as not case sensitive
        fListParser = new STAFCommandParser(0, false);
        fQueryParser = new STAFCommandParser(0, false);
        fAddParser = new STAFCommandParser(0, false);
        fDeleteParser = new STAFCommandParser(0, false);

        fListParser.addOption("LIST", 1, 
                              STAFCommandParser.VALUENOTALLOWED);

        fListParser.addOption("PRINTERS", 1, 
                              STAFCommandParser.VALUENOTALLOWED);
        
        fListParser.addOption("MODEMS", 1, 
                              STAFCommandParser.VALUENOTALLOWED);
        
        fQueryParser.addOption("QUERY", 1, 
                               STAFCommandParser.VALUENOTALLOWED);

        fQueryParser.addOption("PRINTER", 1, 
                               STAFCommandParser.VALUEREQUIRED);

        fQueryParser.addOption("MODEM", 1, 
                               STAFCommandParser.VALUEREQUIRED);
                               
        fAddParser.addOption("ADD", 1, 
                             STAFCommandParser.VALUENOTALLOWED);

        fAddParser.addOption("PRINTER", 1, 
                             STAFCommandParser.VALUEREQUIRED);

        fAddParser.addOption("MODEM", 1, 
                             STAFCommandParser.VALUEREQUIRED);
                            
        fDeleteParser.addOption("DELETE", 1, 
                               STAFCommandParser.VALUENOTALLOWED);

        fDeleteParser.addOption("PRINTER", 1, 
                                STAFCommandParser.VALUEREQUIRED);

        fDeleteParser.addOption("MODEM", 1, 
                                STAFCommandParser.VALUEREQUIRED);

            // this means you must have PRINTER or MODEM, but not both
        fQueryParser.addOptionGroup("PRINTER MODEM", 1, 1);
        fAddParser.addOptionGroup("PRINTER MODEM", 1, 1);
        fDeleteParser.addOptionGroup("PRINTER MODEM", 1, 1);
                               
        fLineSep = (fHandle.submit2("local", "var", 
                    "resolve {STAF/Config/Sep/Line}")).result;       

        return rc;
    }  
        
    public STAFResult acceptRequest(STAFServiceInterfaceLevel3.RequestInfo info)
    {               
        String lowerRequest = info.request.toLowerCase();

        // call the appropriate method to handle the command
        if (lowerRequest.startsWith("list"))
        {
            return handleList(info);
        }
        else if (lowerRequest.startsWith("query"))
        {
            return handleQuery(info);
        }
        else if (lowerRequest.startsWith("add"))
        {
            return handleAdd(info);
        }
        else if (lowerRequest.startsWith("delete"))
        {
            return handleDelete(info);
        }
        else if (lowerRequest.startsWith("help"))
        {
            return handleHelp();
        }
        else if (lowerRequest.startsWith("version"))
        {
            return handleVersion();
        }
        else
        {
            return new STAFResult(STAFResult.InvalidRequestString,
                                  "Unknown SampleMonitor Request: " + 
                                  lowerRequest);
        }
    }

    private STAFResult handleHelp()
    {
        return new STAFResult(STAFResult.Ok,
         "SampleService Service Help"          
         + fLineSep + "LIST    [PRINTERS] [MODEMS]"
         + fLineSep + "QUERY   PRINTER <printerName> | MODEM <modemName>"
         + fLineSep + "ADD PRINTER <printerName> | MODEM <modemName>"
         + fLineSep + "DELETE PRINTER <printerName> | MODEM <modemName>"
         + fLineSep + "VERSION" + fLineSep + "HELP");
    }
    
    private STAFResult handleVersion()
    {
        return new STAFResult(STAFResult.Ok, kVersion);
    }

    private STAFResult handleList(STAFServiceInterfaceLevel3.RequestInfo info)
    {
        // Check whether Trust level is sufficient for this command.

        if (info.trustLevel < 2)
        {   
            return new STAFResult(STAFResult.AccessDenied, 
                "Trust level 2 required for LIST request. Requesting " +
                "machine's trust level: " +  info.trustLevel); 
        }    

        STAFResult result = new STAFResult(STAFResult.Ok, "");
        String resultString = "";
        STAFCommandParseResult parsedRequest = fListParser.parse(info.request);
        int printersOption;
        int modemsOption;

        try
        { 
            if (parsedRequest.rc != STAFResult.Ok)
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parsedRequest.errorBuffer);
            }

            
            printersOption = parsedRequest.optionTimes("printers");
            
            modemsOption = parsedRequest.optionTimes("modems");
                    
        }
        catch (Exception e)
        {            
            return new STAFResult(STAFResult.JavaError, 
                                  "Internal Java error.");
        }
        
        boolean printers = (printersOption > 0);
        boolean modems = (modemsOption > 0);
        
        if (!printers && !modems)
        {
            // list everything
            resultString = "List of Everything";
        }
        else if (printers && modems)
        {            
            resultString = "List of Printers and Modems";
        }            
        else if (printers)
        {            
            resultString = "List of Printers";
         
        }
        else if (modems)
        {
            resultString = "List of Modems";
        }

        return new STAFResult(STAFResult.Ok, resultString);
    }
    
    private STAFResult handleQuery(STAFServiceInterfaceLevel3.RequestInfo info)
    {
        // Check whether Trust level is sufficient for this command.

        if (info.trustLevel < 2)
        {   
            return new STAFResult(STAFResult.AccessDenied, 
                "Trust level 2 required for QUERY request. Requesting " +
                "machine's trust level: " +  info.trustLevel); 
        }        

        STAFResult result = new STAFResult(STAFResult.Ok, "");
        String resultString = "";
        STAFCommandParseResult parsedRequest = fQueryParser.parse(info.request);
        String printerValue;
        String modemValue;

        try
        { 
            if (parsedRequest.rc != STAFResult.Ok)
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parsedRequest.errorBuffer);
            }

            
            printerValue = (resolveVar(info.localMachine, 
                                       parsedRequest.optionValue("printer"),
                                       info.handle)).result;
            
            modemValue = (resolveVar(info.localMachine, 
                                     parsedRequest.optionValue("modem"),
                                     info.handle)).result;
                    
        }
        catch (Exception e)
        {            
            return new STAFResult(STAFResult.JavaError, 
                                  "Internal Java error.");
        }
        
        boolean printer = !(printerValue.equals(""));
        boolean modem = !(modemValue.equals(""));
        
        if (!(printerValue.equals("")))
        {
            resultString = "Query of Printer " + printerValue;
        }
        else if (!(modemValue.equals("")))
        {
            resultString = "Query of Modem " + modemValue;
        }

        return new STAFResult(STAFResult.Ok, resultString);
    }
    
    private STAFResult handleAdd(STAFServiceInterfaceLevel3.RequestInfo info)
    {
        // Check whether Trust level is sufficient for this command.

        if (info.trustLevel < 3)
        {   
            return new STAFResult(STAFResult.AccessDenied, 
                "Trust level 3 required for ADD request. Requesting " +
                "machine's trust level: " +  info.trustLevel); 
        }
    
        STAFResult result = new STAFResult(STAFResult.Ok, "");
        String resultString = "";
        STAFCommandParseResult parsedRequest = fAddParser.parse(info.request);
        String printerValue;
        String modemValue;

        try
        { 
            if (parsedRequest.rc != STAFResult.Ok)
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parsedRequest.errorBuffer);
            }

            
            printerValue = (resolveVar(info.localMachine, 
                                       parsedRequest.optionValue("printer"),
                                       info.handle)).result;
            
            modemValue = (resolveVar(info.localMachine, 
                                     parsedRequest.optionValue("modem"),
                                     info.handle)).result;
                    
        }
        catch (Exception e)
        {            
            return new STAFResult(STAFResult.JavaError, 
                                  "Internal Java error.");
        }
        
        boolean printer = !(printerValue.equals(""));
        boolean modem = !(modemValue.equals(""));
        
        if (!(printerValue.equals("")))
        {
            resultString = "Adding Printer " + printerValue;
        }
        else if (!(modemValue.equals("")))
        {
            resultString = "Adding Modem " + modemValue;
        }

        return new STAFResult(STAFResult.Ok, resultString);
    }
    
    private STAFResult handleDelete(STAFServiceInterfaceLevel3.RequestInfo info)
    {
        // Check whether Trust level is sufficient for this command.

        if (info.trustLevel < 4)
        {   
            return new STAFResult(STAFResult.AccessDenied, 
                "Trust level 4 required for DELETE request. Requesting " +
                "machine's trust level: " +  info.trustLevel); 
        }    
                  
        STAFResult result = new STAFResult(STAFResult.Ok, "");
        String resultString = "";
        STAFCommandParseResult parsedRequest = fDeleteParser.parse(info.request);
        String printerValue;
        String modemValue;

        try
        { 
            if (parsedRequest.rc != STAFResult.Ok)
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parsedRequest.errorBuffer);
            }

            
            printerValue = (resolveVar(info.localMachine, 
                                       parsedRequest.optionValue("printer"),
                                       info.handle)).result;
            
            modemValue = (resolveVar(info.localMachine, 
                                     parsedRequest.optionValue("modem"),
                                     info.handle)).result;
                    
        }
        catch (Exception e)
        {            
            return new STAFResult(STAFResult.JavaError, 
                                  "Internal Java error.");
        }
        
        boolean printer = !(printerValue.equals(""));
        boolean modem = !(modemValue.equals(""));
        
        if (!(printerValue.equals("")))
        {
            resultString = "Deleting Printer " + printerValue;
        }
        else if (!(modemValue.equals("")))
        {
            resultString = "Deleting Modem " + modemValue;
        }

        return new STAFResult(STAFResult.Ok, resultString);
    }
    
    public int term()
    {      
        try
        {
            fHandle.unRegister();
        }
        catch (STAFException ex)
        {            
            return STAFResult.STAFRegistrationError;
        }
        
        return STAFResult.Ok;
    }

        // this method will resolve any STAF variables that
        // are contained within the Option Value
    private STAFResult resolveVar(String machine, String optionValue, 
                                  int handle)
    {
        String value = "";
        STAFResult resolvedResult = null;

        if (optionValue.startsWith("{"))
        {
            resolvedResult =
                fHandle.submit2(machine, "var", "handle " + handle + 
                                " resolve " + optionValue);

            if (resolvedResult.rc != 0)
            {
                return resolvedResult;
            }

            value = resolvedResult.result;
        }
        else
        {
            value = optionValue;
        }

        return new STAFResult(STAFResult.Ok, value);
    }
}
