/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax.extension.samples.extdelay;
import java.util.Map;
import com.ibm.staf.service.stax.*;

public class ExtSleepActionFactory extends ExtDelayActionFactory
{
    private static String fDTDInfo =
"\n" +
"<!--================= The Ext-Sleep Element ============================ -->\n" +
"<!--\n" +
"     Delays for the specified number of seconds and generates and event\n" +
"     every iteration.\n" +
"-->\n" +
"<!ELEMENT ext-sleep     (#PCDATA)>\n";

    public String getDTDInfo()
    {
        return fDTDInfo;
    }

    public String getDTDTaskName()
    {
        return "ext-sleep";
    }
    
    public ExtSleepActionFactory()
    {
        /* Do Nothing */
    }
    
    public ExtSleepActionFactory(STAX staxService)
    {
        super(staxService);
    }

    public ExtSleepActionFactory(STAX staxService, Map parmMap)
        throws STAXExtensionInitException
    {
        super(staxService, parmMap);
    }
}
