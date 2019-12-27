/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.cron;

import java.util.*;

public class CronData2 implements java.io.Serializable
{
    // CAUTION!  Adding fields to this class will make older versions of the 
    // serialized class incompatible with the new version.  If a new field
    // is required for this class, then create a new class with the new field,
    // called CronData3, and if the serialized file being read in during 
    // service startup is of object type CronData2, then read it in and convert
    // it into object type CronData3.

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
    boolean fOnce = false; // Added on CronData1
    String fDescription; // Added on CronData1
    String fState; // Added on CronData2

    public CronData2()
    {
     super();
    }

    public CronData2(String description, String originMachine, String machine, 
                    boolean pythonMachine, String service, 
                    boolean pythonService, String request, 
                    boolean pythonRequest, String prepare,
                    Vector<String> minute, Vector<String> hour,
                    Vector<String> day, Vector<String> month,
                    Vector<String> weekday, boolean once,
                    String state)
    {
        this.fDescription = description;
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
        this.fOnce = once;
        this.fState = state;
    }
}