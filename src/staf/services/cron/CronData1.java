/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.cron;

import java.util.*;

public class CronData1 implements java.io.Serializable
{
    // CAUTION!  Adding fields to this class will make older versions of the 
    // serialized class incompatible with the new version.  If a new field
    // is required for this class, then create a new class with the new field,
    // called CronData2, and if the serialized file being read in during 
    // service startup is of object type CronData1, then read it in and convert
    // it into object type CronData2.

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

    public CronData1()
    {
     super();
    }

    public CronData1(String description, String originMachine, String machine, 
                    boolean pythonMachine, String service, 
                    boolean pythonService, String request, 
                    boolean pythonRequest, String prepare,
                    Vector<String> minute, Vector<String> hour,
                    Vector<String> day, Vector<String> month,
                    Vector<String> weekday, boolean once)
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
    }
}