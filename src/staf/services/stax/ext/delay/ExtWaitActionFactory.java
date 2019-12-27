/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.extension.samples.extdelay;
import java.util.Map;
import com.ibm.staf.service.stax.*;

public class ExtWaitActionFactory extends ExtDelayActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Ext-Wait Element ============================= -->\n" +
"<!--\n" +
"     Delays for the specified number of seconds and generates and event\n" +
"     every iteration.\n" +
"-->\n" +
"<!ELEMENT ext-wait     (#PCDATA)>\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "ext-wait";
    }
    
    public ExtWaitActionFactory()
    {
        /* Do Nothing */
    }
    
    public ExtWaitActionFactory(STAX staxService)
    {
        super(staxService);
    }

    public ExtWaitActionFactory(STAX staxService, Map parmMap)
        throws STAXExtensionInitException
    {
        super(staxService, parmMap);
    }
}
