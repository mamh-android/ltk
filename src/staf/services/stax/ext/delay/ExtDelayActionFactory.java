/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.extension.samples.extdelay;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import com.ibm.staf.service.stax.*;

public class ExtDelayActionFactory implements STAXActionFactory,
                                              STAXJobManagementHandler
{
    public static final String EXT_DELAY = new String("ext-delay");
    
    private static Map fParameterMap = new HashMap();

    private static String fDTDInfo =
"\n" +
"<!--================= The Ext-Delay Element ============================ -->\n" +
"<!--\n" +
"     Delays for the specified number of seconds and generates and event\n" +
"     every iteration.\n" +
"-->\n" +
"<!ELEMENT ext-delay     (#PCDATA)>\n";

    public ExtDelayActionFactory() { }

    public ExtDelayActionFactory(STAX staxService)
    {
        // Note that in order for it's initJob method to be run at the start
        // of a job, you must register the factory as a Job Management handler.

        staxService.registerJobManagementHandler(this);
    }

    public ExtDelayActionFactory(STAX staxService, Map parmMap)
        throws STAXExtensionInitException
    {
        // Note that in order for it's initJob method to be run at the start
        // of a job, you must register the factory as a Job Management handler.

        staxService.registerJobManagementHandler(this);

        // If an invalid parameter name or an invalid value for a parameter is
        // specified, raise a STAXExtensionInitException.

        Iterator iter = parmMap.keySet().iterator();

        while (iter.hasNext())
        {
            // Check if the parameter name is supported

            String name = (String)iter.next();

            if (!name.equals("delay"))
            {
                throw new STAXExtensionInitException(
                    "Unsupported parameter name " + name);
            }

            // Make sure that the value specified for the "delay"
            // parameter is an integer > 0.
                
            String delay = (String)parmMap.get(name);

            try
            {
                Integer delayInt = new Integer(delay);
             
                if (delayInt.intValue() <= 0)
                {
                    throw new STAXExtensionInitException(
                        "Value specified for parameter delay is not > 0.  " +
                        "Value=" + delay);
                }
            }
            catch (NumberFormatException e)
            {
                throw new STAXExtensionInitException(
                    "Value specified for parameter delay is not an integer.\n" +
                    e.toString());
            }

            // Add to the parameter map

            fParameterMap.put(name, delay);
        }
    }

    public String getParameter(String name)
    {
        return (String)fParameterMap.get(name);
    }

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "ext-delay";
    }

    public STAXAction parseAction(STAX staxService, STAXJob job,
                                  org.w3c.dom.Node root) throws STAXException
    {
        ExtDelayAction delay = new ExtDelayAction();

        delay.setActionFactory(this);
        delay.setLineNumber(root);
        delay.setXmlFile(job.getXmlFile());
        delay.setXmlMachine(job.getXmlMachine());
        
        NodeList children = root.getChildNodes();

        for (int i = 0; i < children.getLength(); ++i)
        {
            Node thisChild = children.item(i);

            if (thisChild.getNodeType() == Node.COMMENT_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.CDATA_SECTION_NODE)
            {
                /* Do nothing */
            }
            else if (thisChild.getNodeType() == Node.TEXT_NODE)
            {
                delay.setElementInfo(new STAXElementInfo(root.getNodeName()));

                delay.setDelayValue(
                    STAXUtil.parseAndCompileForPython(
                        thisChild.getNodeValue(), delay));
            }
        }

        return delay;
    }

    // STAXJobManagement methods

    public void initJob(STAXJob job)
    {
        // Create a default signal handler for signal ExtDelayInvalidValue
        // that logs and displays a message and terminates the job.

        ArrayList signalHandlerActionList = new ArrayList();

        // Note the message is in a Python form so it can be evaluated.
        String msg = "'ExtDelayInvalidValue signal raised.  " +
                     "Terminating job. %s' % ExtDelayInvalidValueMsg";
        
        signalHandlerActionList.add(new STAXMessageAction(msg));
        signalHandlerActionList.add(new STAXLogAction(msg, "'error'",
                                                      STAXJob.JOB_LOG));
        signalHandlerActionList.add(new STAXTerminateAction("'main'"));

        // Add the default signal handler to the job's default action list.
        job.addDefaultAction(new STAXSignalHandlerAction(
            "'ExtDelayInvalidValue'",
            new STAXSequenceAction(signalHandlerActionList)));

        // Create a map that contains a key named "extDelayNum" whose
        // value is initialized to 0.  This number is used in creating a 
        // unique name for each delay element within a job.

        HashMap extDelayMap = new HashMap();
        extDelayMap.put("extDelayNum", new Integer(0));

        boolean result = job.setData("ext-delay-map", extDelayMap); 
    }

    public void terminateJob(STAXJob job)
    { /* Do Nothing */ }
}
