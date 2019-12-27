/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;
import com.ibm.staf.*;
import java.util.*;

public class TestProcess implements Runnable
{    
    private STAFHandle handle = null;    
    private int counter;
    private int loopCounter;
    private int incrementSeconds;
    private int returnCode;
            
    public static void main(String[] args)
    {
        if (args.length < 3)
        {
            System.out.println("Usage: java TestProcess loopCount" +
                               " incrementSeconds returnCode");
            System.exit(1);
        }
        
        try
        {
            int loopCounter = (new Integer(args[0])).intValue();
            int incrementSeconds = (new Integer(args[1])).intValue();
            int returnCode = (new Integer(args[2])).intValue();
            TestProcess testProcess = new TestProcess(loopCounter,
                                                      incrementSeconds,
                                                      returnCode);
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    public TestProcess(int loopCounter, int incrementSeconds, 
                       int returnCode)    
    {
        this.loopCounter = loopCounter;
        this.incrementSeconds = incrementSeconds;
        this.returnCode = returnCode;
        this.run();
    }

    public void run()
    {
        try
        {
            // register with STAF
            handle = new STAFHandle("TestProcess");
        }
        catch(STAFException e)
        {
            System.out.println("Error registering with STAF");
            terminate();
        }        
        
        for (int i=0; i < loopCounter; i++)
        {
            STAFResult result = handle.submit2("local", "monitor", 
                                           "log message " + 
                                           STAFUtil.wrapData("Loop #"
                                           + String.valueOf(i)));
                   
            // System.out.println("Loop #" + String.valueOf(i));
                       
            try
            {
                Thread.sleep(incrementSeconds * 1000);
            }
            catch(InterruptedException e)
            {
                e.printStackTrace();
            }                    
        }
        
        terminate();
    }      

    public void terminate()
    {
        try
        {
            if (handle != null)
            {                
                handle.submit2("local", "monitor", "log message " +
                               STAFUtil.wrapData("Terminating "));                

                // unregister
                handle.unRegister();
            }
        }
        catch(STAFException e)
        {
            /* do nothing */
        }
        finally
        {
            System.exit(returnCode);
        }
    }
}