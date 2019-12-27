/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.cron;

import java.util.*;

public class CronData implements java.io.Serializable
{
    // This class is only used for registration data that was saved using
    // versions of the Cron service prior to 3.2.0.  CronData1 will be used
    // by Cron V3.2.0+.

    String fOriginMachine;
    String fMachine;
    boolean fPythonMachine = true;
    String fService;
    boolean fPythonService = true;
    String fRequest;
    boolean fPythonRequest = true;
    String fPrepare;
    Vector<String> fMinute = new Vector<String>();
    Vector<String> fHour = new Vector<String>();
    Vector<String> fDay = new Vector<String>();
    Vector<String> fMonth = new Vector<String>();
    Vector<String> fWeekday = new Vector<String>();

    public CronData()
    {
     super();
    }

    public CronData(String originMachine, String machine, 
                    boolean pythonMachine, String service, 
                    boolean pythonService, String request, 
                    boolean pythonRequest, String prepare,
                    Vector<String> minute, Vector<String> hour,
                    Vector<String> day, Vector<String> month,
                    Vector<String> weekday)
    {
        this.fOriginMachine = originMachine;
        this.fMachine = machine;
        this.fPythonMachine = pythonMachine;
        this.fService = service;
        this.fPythonService = pythonService;
        this.fRequest = request;
        this.fPythonRequest = pythonRequest;
        this.fPrepare = prepare;
        this.fMinute = minute;
        this.fHour = hour;
        this.fDay = day;
        this.fMonth = month;
        this.fWeekday = weekday;
    }
}