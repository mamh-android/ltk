/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import com.ibm.staf.*;
import com.ibm.staf.service.*;

public class STAXBlockActionFactory implements STAXActionFactory,
                                               STAXListRequestHandler,
                                               STAXQueryRequestHandler,
                                               STAXGenericRequestHandler,
                                               STAXJobManagementHandler
{
    static final String STAX_BLOCK_EVENT = new String("Block");

    static final String sBlockInfoMapClassName = new String(
        "STAF/Service/STAX/BlockInfo");
    static final String sQueryBlockMapClassName = new String(
        "STAF/Service/STAX/QueryBlock");

    // STAX Service Error Codes
    static final int BlockNotHeld = 4002;
    static final int BlockAlreadyHeld = 4003;

    private static String fDTDInfo =
"\n" +
"<!--================= The Block Element ============================ -->\n" +
"<!--\n" +
"     Defines a task block that can be held, released, or terminated.\n" +
"     Used in conjunction with the hold/terminate/release elements to\n" +
"     define a task block that can be held, terminated, or released.\n" +
"     The name attribute value is evaluated via Python.\n" +
"-->\n" +
"<!ELEMENT block      (%task;)>\n" +
"<!ATTLIST block\n" +
"          name       CDATA    #REQUIRED\n" +
">\n";

    public STAXBlockActionFactory(STAX staxService)
    {
        staxService.registerListHandler("BLOCKS", this);
        staxService.registerQueryHandler("BLOCK", "Block Name", this);
        staxService.registerQueryJobHandler(this);

        staxService.registerJobManagementHandler(this);

        // Set up Hold, Release, Terminate Block Parser

        fHRTParser.addOption("HOLD", 1, STAFCommandParser.VALUENOTALLOWED);
        fHRTParser.addOption("RELEASE", 1, STAFCommandParser.VALUENOTALLOWED);
        fHRTParser.addOption("TERMINATE", 1,STAFCommandParser.VALUENOTALLOWED);
        fHRTParser.addOption("JOB", 1, STAFCommandParser.VALUEREQUIRED);
        fHRTParser.addOption("BLOCK", 1, STAFCommandParser.VALUEREQUIRED);
        fHRTParser.addOption("TIMEOUT", 1, STAFCommandParser.VALUEALLOWED);

        fHRTParser.addOptionGroup("HOLD RELEASE TERMINATE", 1, 1);
        fHRTParser.addOptionNeed("HOLD RELEASE TERMINATE", "JOB");
        fHRTParser.addOptionNeed("TIMEOUT", "HOLD");

        // Register as a GenericRequestHandler
        try
        {
            // Assign STAFServiceInterfaceLevel class that this handler uses

            Class serviceInterfaceClass = Class.forName(STAX.INTERFACE_LEVEL_30);

            int rc = staxService.registerGenericRequestHandler(this,
                                                      serviceInterfaceClass);
            if (rc != 0)
            {
                System.out.println("STAXBlockActionFactory." +
                                   "registerGenericRequestHandler() failed");
            }
        }
        catch (ClassNotFoundException e)
        {
            System.out.println("STAXBlockActionFactory: " +
                               "registerGenericRequestHandler: " + e);
        }

        // Construct map-class for block information

        fBlockInfoMapClass = new STAFMapClassDefinition(sBlockInfoMapClassName);

        fBlockInfoMapClass.addKey("blockName", "Block Name");
        fBlockInfoMapClass.addKey("state",     "State");

        // Construct map-class for query block information

        fQueryBlockMapClass = new STAFMapClassDefinition(sQueryBlockMapClassName);

        fQueryBlockMapClass.addKey("blockName", "Block Name");
        fQueryBlockMapClass.addKey("state",     "State");
        fQueryBlockMapClass.addKey("threadID", "Thread ID");
        fQueryBlockMapClass.addKey("startTimestamp", "Start Date-Time");
    }

    public void initJob(STAXJob job)
    {
        if (!job.setData("blockMap", new TreeMap<String, STAXBlockAction>()))
        {
            String msg = "STAXBlockActionFactory.initJob: setData for " +
                         "blockMap failed.";
            job.log(STAXJob.JOB_LOG, "error", msg);
        }
    }

    public void terminateJob(STAXJob job)
    { /* Do Nothing */ }

    public STAFResult handleListRequest(String type, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("blocks"))
        {
            // LIST JOB <Job ID> BLOCKS

            // Create the marshalling context and set its map class definitions
            // and create an empty list to contain the block map entries

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fBlockInfoMapClass);
            List<Map<String, Object>> blockOutputList =
                new ArrayList<Map<String, Object>>();

            // Iterate through the block map, generating the output list

            TreeMap blockMap = (TreeMap)job.getData("blockMap");

            if (blockMap != null)
            {
                synchronized (blockMap)
                {
                    Iterator iter = blockMap.values().iterator();
                    
                    while (iter.hasNext())
                    {
                        STAXBlockAction block = (STAXBlockAction)iter.next();

                        Map<String, Object> blockOutputMap =
                            new TreeMap<String, Object>();
                        blockOutputMap.put("staf-map-class-name",
                                           fBlockInfoMapClass.name());
                        blockOutputMap.put("blockName", block.getName());
                        blockOutputMap.put(
                            "state", block.getBlockStateAsString());

                        blockOutputList.add(blockOutputMap);
                    }
                }
            }

            mc.setRootObject(blockOutputList);

            return new STAFResult(STAFResult.Ok, mc.marshall());
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryRequest(String type, String key, STAXJob job,
                                        STAXRequestSettings settings)
    {
        if (type.equalsIgnoreCase("block"))
        {
            // QUERY JOB <Job ID> BLOCK <Block name>

            // Create the marshalling context and set its map class definitions

            STAFMarshallingContext mc = new STAFMarshallingContext();
            mc.setMapClassDefinition(fQueryBlockMapClass);

            TreeMap blockMap = (TreeMap)job.getData("blockMap");

            if (blockMap == null)
                return new STAFResult(STAFResult.DoesNotExist, key);

            synchronized (blockMap)
            {
                if (!blockMap.containsKey(key))
                    return new STAFResult(STAFResult.DoesNotExist, key);
                else
                {
                    STAXBlockAction block = (STAXBlockAction)blockMap.get(key);

                    Map<String, Object> blockOutputMap =
                        new TreeMap<String, Object>();
                    blockOutputMap.put("staf-map-class-name",
                                       fQueryBlockMapClass.name());
                    blockOutputMap.put("blockName", block.getName());
                    blockOutputMap.put("state", block.getBlockStateAsString());
                    blockOutputMap.put("threadID", "" + block.getOwningThread().
                                       getThreadNumber());
                    blockOutputMap.put("startTimestamp",
                                       block.getStartTimestamp().
                                       getTimestampString());

                    mc.setRootObject(blockOutputMap);

                    return new STAFResult(STAFResult.Ok, mc.marshall());
                }
            }
        }
        else
            return new STAFResult(STAFResult.DoesNotExist, type);
    }

    public STAFResult handleQueryJobRequest(STAXJob job,
                                            STAXRequestSettings settings)
    {
        // Provide additional information for the QUERY JOB <Job ID> result

        int blocksRunningState = 0;
        int blocksHeldState = 0;
        int blocksUnknownState = 0;

        TreeMap blockMap = (TreeMap)job.getData("blockMap");

        if (blockMap != null)
        {
            synchronized (blockMap)
            {
                Iterator iter = blockMap.values().iterator();

                while (iter.hasNext())
                {
                    STAXBlockAction block = (STAXBlockAction)iter.next();

                    if (block.getBlockState() == STAXBlockAction.BLOCK_RUNNING)
                        blocksRunningState += 1;
                    else if (block.getBlockState() ==
                             STAXBlockAction.BLOCK_HELD)
                        blocksHeldState += 1;
                    else
                        blocksUnknownState += 1;
                }
            }
        }

        // XXX: Change to pass in the marshalling context 
        STAFMarshallingContext mc = new STAFMarshallingContext();

        Map<String, Object> blockOutputMap = new HashMap<String, Object>();
        blockOutputMap.put("numBlocksRunning", "" + blocksRunningState);
        blockOutputMap.put("numBlocksHeld", "" + blocksHeldState);
        blockOutputMap.put("numBlocksUnknown", "" + blocksUnknownState);

        mc.setRootObject(blockOutputMap);

        return new STAFResult(STAFResult.Ok, mc.marshall()); 
    }

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "block";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        STAXBlockAction action = new STAXBlockAction();

        action.setLineNumber(root);
        action.setXmlFile(job.getXmlFile());
        action.setXmlMachine(job.getXmlMachine());

        STAXAction blockAction = null;

        NamedNodeMap attrs = root.getAttributes();

        for (int i = 0; i < attrs.getLength(); ++i)
        {
            Node thisAttr = attrs.item(i);

            action.setElementInfo(new STAXElementInfo(
                root.getNodeName(), thisAttr.getNodeName()));

            if (thisAttr.getNodeName().equals("name"))
            {
                action.setName(STAXUtil.parseAndCompileForPython(
                    thisAttr.getNodeValue(), action));
            }
        }

        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.ELEMENT_NODE)
            {
                if (blockAction != null)
                {
                    action.setElementInfo(
                        new STAXElementInfo(
                            root.getNodeName(),
                            STAXElementInfo.NO_ATTRIBUTE_NAME,
                            STAXElementInfo.LAST_ELEMENT_INDEX,
                            thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementCountException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                STAXActionFactory factory = staxService.getActionFactory(
                    thisChild.getNodeName());

                if (factory == null)
                {
                    action.setElementInfo(new STAXElementInfo(
                        root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                        STAXElementInfo.LAST_ELEMENT_INDEX,
                        "No action factory for element type \"" +
                        thisChild.getNodeName() + "\""));

                    throw new STAXInvalidXMLElementException(
                        STAXUtil.formatErrorMessage(action), action);
                }

                blockAction = factory.parseAction(
                    staxService, job, thisChild);
            }
            else
            {
               action.setElementInfo(new STAXElementInfo(
                    root.getNodeName(), STAXElementInfo.NO_ATTRIBUTE_NAME,
                    STAXElementInfo.LAST_ELEMENT_INDEX,
                    "Contains invalid node type: " +
                    Integer.toString(thisChild.getNodeType())));

                throw new STAXInvalidXMLNodeTypeException(
                    STAXUtil.formatErrorMessage(action), action);
            }
        }

        action.setBlockAction(blockAction);

        return action;
    }


    // STAXGenericRequestHandler Interface Methods

    public STAFResult handleRequest(Object infoObject, STAX staxService)
    {
        STAFServiceInterfaceLevel30.RequestInfo info =
            (STAFServiceInterfaceLevel30.RequestInfo)infoObject;

        // Parse the request

        STAFCommandParseResult parseResult= fHRTParser.parse(info.request);

        if (parseResult.rc != STAFResult.Ok)
        {
            if ((parseResult.numInstances() >= 1) &&
                ((parseResult.instanceName(1).equalsIgnoreCase("HOLD")) ||
                 (parseResult.instanceName(1).equalsIgnoreCase("RELEASE")) ||
                 (parseResult.instanceName(1).equalsIgnoreCase("TERMINATE"))))
            {
                return new STAFResult(STAFResult.InvalidRequestString,
                                      parseResult.errorBuffer);
            }
            else
            {   // Returning nothing in the result indicates that this parser
                // does not support this request.
                return new STAFResult(STAFResult.InvalidRequestString, "");
            }
        }

        // Verify the requesting machine/user has at least trust level 4

        String request = "";

        if (parseResult.optionTimes("HOLD") != 0)
            request = "HOLD";
        else if (parseResult.optionTimes("RELEASE") != 0)
            request = "RELEASE";
        else if (parseResult.optionTimes("TERMINATE") != 0)
            request = "TERMINATE";

        STAFResult trustResult = STAFUtil.validateTrust(
            4, staxService.getServiceName(), request,
            staxService.getLocalMachineName(), info);

        if (trustResult.rc != STAFResult.Ok) return trustResult;

        STAXJob job = null;
        
        try
        {
            job = staxService.getJobMap().get(
                new Integer(parseResult.optionValue("JOB")));

            if (job == null)
            {
                return new STAFResult(
                    STAFResult.DoesNotExist,
                    "Job " +  parseResult.optionValue("JOB") +
                    " does not exist");
            }
        }
        catch (NumberFormatException e)
        {
            return new STAFResult(STAFResult.InvalidValue,
                                  parseResult.optionValue("JOB"));
        }
        
        String blockName = new String("main");

        if (parseResult.optionTimes("BLOCK") != 0)
            blockName = parseResult.optionValue("BLOCK");

        // Check if the job is in a pending state and if the main block
        // was specified as need to handle differently since the block
        // will not be in the job's blockMap 

        if ((job.getState() == STAXJob.PENDING_STATE) &&
             blockName.equals("main"))
        {
            String action = "terminate, hold, or release";

            if (parseResult.optionTimes("TERMINATE") != 0)
                action = "terminate";
            else if (parseResult.optionTimes("HOLD") != 0)
                action = "hold";
            else if (parseResult.optionTimes("RELEASE") != 0)
                action = "release";
            
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Cannot " + action + " a job that is in the process of " +
                "being submitted (e.g. in a Pending state) because the " +
                "'main' block does not exist yet.  Try again later.");
        }

        TreeMap blockMap = (TreeMap)job.getData("blockMap");
        
        if ((blockMap == null) || !blockMap.containsKey(blockName))
        {
            return new STAFResult(
                STAFResult.DoesNotExist,
                "Block '" + blockName + "' does not exist");
        }

        STAXBlockAction theBlock = (STAXBlockAction)blockMap.get(blockName);

        if (parseResult.optionTimes("HOLD") != 0)
        {
            if (theBlock.getBlockState() == STAXBlockAction.BLOCK_HELD)
            {
                return new STAFResult(BlockAlreadyHeld,
                                      "Block already held");
            }

            String msg = "Received HOLD BLOCK " + blockName;

            long timeout = 0;

            if (parseResult.optionTimes("TIMEOUT") != 0)
            {
                // A timeout was specified for the HOLD request
                
                String timeoutString = parseResult.optionValue("TIMEOUT");
                
                if (timeoutString.length() != 0)
                {
                    // Resolve the TIMEOUT value, verify that it is a valid
                    // timeout value, and convert it to milliseconds if needed

                    STAFResult resolvedValue =
                        STAFUtil.resolveRequestVarAndConvertDuration(
                            "TIMEOUT", timeoutString,
                            staxService.getSTAFHandle(), info.requestNumber);

                    if (resolvedValue.rc != STAFResult.Ok)
                        return resolvedValue;
                    
                    timeoutString = resolvedValue.result;

                    msg += " TIMEOUT " + timeoutString;

                    // Convert timeout string to a number

                    try
                    {
                        timeout = Long.parseLong(timeoutString);
                    }
                    catch (NumberFormatException nfe)
                    {
                        // Should never happen because already check by
                        // STAFUtil.resolveRequestVarAndConvertDuration()

                        return new STAFResult(
                            STAFResult.InvalidValue,
                            "Invalid value for the TIMEOUT option: " +
                            timeout + "\nNumberFormatException: " +
                            nfe.getMessage());
                    }
                }
            }

            msg += " request";

            job.log(STAXJob.JOB_LOG, "info", msg);

            theBlock.holdBlock(timeout);
        }
        else if (parseResult.optionTimes("RELEASE") != 0)
        {
            if (theBlock.getBlockState() != STAXBlockAction.BLOCK_HELD)
                return new STAFResult(BlockNotHeld, "Block not held");

            String msg = "Received RELEASE BLOCK " + blockName + " request";
            job.log(STAXJob.JOB_LOG, "info", msg);

            theBlock.releaseBlock();
        }
        else
        {
            String msg = "Received TERMINATE BLOCK "  + blockName + " request";
            job.log(STAXJob.JOB_LOG, "info", msg);

            theBlock.terminateBlock();
        }

        return new STAFResult(STAFResult.Ok);
    }

    public String getHelpInfo(String lineSep)
    {
        return "HOLD      JOB <Job ID> [BLOCK <Block Name>]" +
               " [TIMEOUT [<Number>[s|m|h|d|w]]]" +
               lineSep + lineSep +
               "RELEASE   JOB <Job ID> [BLOCK <Block Name>]" +
               lineSep + lineSep +
               "TERMINATE JOB <Job ID> [BLOCK <Block Name>]";
    }

    private STAFCommandParser fHRTParser = new STAFCommandParser();
    private STAFMapClassDefinition fBlockInfoMapClass;
    private STAFMapClassDefinition fQueryBlockMapClass;
}
