/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2006                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.eventmanager;

public class EventManagerData1 implements java.io.Serializable
{
    // CAUTION!  Adding fields to this class will make older versions of the 
    // serialized class incompatible with the new version.  If a new field
    // is required for this class, then create a new class with the new field,
    // called EventManagerData2, and if the serialized file being read in
    // during service startup is of object type EventManagerData1, then read it
    // in and convert it into object type EventManagerData2.

    String fOriginMachine;
    String fMachine;
    boolean fPythonMachine = true;
    String fService;
    boolean fPythonService = true;
    String fRequest;
    boolean fPythonRequest = true;
    String fType;
    String fSubtype;
    String fPrepare;
    String fDescription; // Added on EventManagerData1

    public EventManagerData1()
    {
     super();
    }

    public EventManagerData1(String description, String originMachine,
                              String machine, boolean pythonMachine,
                              String service, boolean pythonService,
                              String request, boolean pythonRequest,
                              String type, String subtype, String prepare)
    {
        this.fDescription = description;
        this.fOriginMachine = originMachine;
        this.fMachine = machine;
        this.fPythonMachine = pythonMachine;
        this.fService = service;
        this.fPythonService = pythonService;
        this.fRequest = request;
        this.fPythonRequest = pythonRequest;
        this.fType = type;
        this.fSubtype = subtype;
        this.fPrepare = prepare;
    }
}