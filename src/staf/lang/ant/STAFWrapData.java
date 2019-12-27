/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2008                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.ant.taskdef;

import org.apache.tools.ant.Task;
import com.ibm.staf.*;

public class STAFWrapData extends Task
{
    private String data;
    private String result;

    public void execute()
    {
        getProject().setNewProperty(result, STAFUtil.wrapData(data));
    }

    public void setData(String data)
    {
        this.data = data;
    }

    public void setResult(String result)
    {
        this.result = result;
    }
}

