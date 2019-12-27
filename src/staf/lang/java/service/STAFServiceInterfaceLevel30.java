/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service;

import com.ibm.staf.*;
import java.util.jar.JarFile;

// STAFServiceInterfaceLevel30 - This class defines the Level 30 interface for
//                               STAF Java services.
//
// This interface defines the following methods
//
//   init - This method is called once when STAF is in startup mode.  This
//          method allows the service to perform its initialization.
//
//   acceptRequest - This is the method called whenever the service needs to
//                   perform a request.  This is primary method by which
//                   programs/users interact with the service.
//
//   term - This method is called once when STAF is in shutdown mode.  This
//          method allows the service to perform any cleanup.

public interface STAFServiceInterfaceLevel30
{
    static public final int serviceTypeUnknown = 0;
    static public final int serviceTypeService = 1;
    static public final int serviceTypeServiceLoader = 2;
    static public final int serviceTypeAuthenticator = 3;

    static public class InitInfo
    {
        public String name;
        public String parms;
        public JarFile serviceJar;
        public int serviceType;
        public String writeLocation;

        public InitInfo(String name, String parms, JarFile serviceJar,
                        int serviceType, String writeLocation)
        {
            this.name = name;
            this.parms = parms;
            this.serviceJar = serviceJar;
            this.serviceType = serviceType;
            this.writeLocation = writeLocation;
        }
    }

    static public class RequestInfo
    {
        public String  stafInstanceUUID;
        public String  machine;
        public String  machineNickname;
        public String  handleName;
        public int     handle;
        public int     trustLevel;
        public boolean isLocalRequest;
        public int     diagEnabled;
        public String  request;
        public int     requestNumber;
        public String  user;
        public String  endpoint;
        public String  physicalInterfaceID;

        public RequestInfo(String stafInstanceUUID, String machine,
                           String machineNickname, String handleName,
                           int handle, int trustLevel,
                           boolean isLocalRequest, int diagEnabled,
                           String request, int requestNumber,
                           String user, String endpoint,
                           String physicalInterfaceID)
        {
            this.stafInstanceUUID = stafInstanceUUID;
            this.machine = machine;
            this.machineNickname = machineNickname;
            this.handleName = handleName;
            this.handle = handle;
            this.trustLevel = trustLevel;
            this.isLocalRequest = isLocalRequest;
            this.diagEnabled = diagEnabled;
            this.request = request;
            this.requestNumber = requestNumber;
            this.user = user;
            this.endpoint = endpoint;
            this.physicalInterfaceID = physicalInterfaceID;
        }
    }

    STAFResult init(InitInfo initInfo);
    STAFResult acceptRequest(RequestInfo reqInfo);
    STAFResult term();
}
