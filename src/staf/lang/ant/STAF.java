/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.ant.taskdef;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;
import com.ibm.staf.*;

public class STAF extends Task
{
    private String location;
    private String service;
    private String request;
    private STAFHandle handle;
    private String resultPrefix;
    private String throwBuildException = "";

    public void execute() throws BuildException
    {
        System.out.println(location + " " + service + " " + request);

        try
        {
            handle = new STAFHandle("STAF_Ant_Extension");
        }
        catch(STAFException e)
        {
            throw new BuildException(e);
        }

        STAFResult result = handle.submit2(location, service, request);
        String outputResult = "";

        if (STAFMarshallingContext.isMarshalledData(result.result))
        {
            STAFMarshallingContext mc = STAFMarshallingContext.unmarshall(
                result.result);

            outputResult = System.getProperty("line.separator") +
                STAFMarshallingContext.formatObject(mc);
        }
        else
        {
            outputResult = result.result;
        }

        System.out.println("RC=" + result.rc + ", Result=" + outputResult);

        getProject().setNewProperty(resultPrefix + ".rc",
                                    (new Integer(result.rc)).toString());
        getProject().setNewProperty(resultPrefix + ".result", outputResult);

        if ((result.rc != 0) && !(throwBuildException.equals("")))
        {
            throw new BuildException("RC=" + result.rc +
                                      ", Result=" + outputResult);
        }

        throwBuildException = "";
    }

    public void setLocation(String location)
    {
        this.location = location;
    }

    public void setService(String service)
    {
        this.service = service;
    }

    public void setRequest(String request)
    {
        this.request = request;
    }

    public void setResultPrefix(String resultPrefix)
    {
        this.resultPrefix = resultPrefix;
    }

    public void setThrowBuildException(String throwBuildException)
    {
        this.throwBuildException = throwBuildException;
    }
}

