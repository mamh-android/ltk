/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service;

import com.ibm.staf.*;
import java.util.jar.*;
import java.util.Map;
import java.util.HashMap;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.File;
import java.util.Collections;

// STAFServiceHelper - This class is used to help load and call Java services
//                     from jar files, as well as from classes in the CLASSPATH.
//
// Most of its methods are called from JNI code.  Essentially the static main
// method is invoked by STAFProc (via a standard java invocation).  The main
// method creates an instance of the STAFServiceHelper.  The service helper
// then calls into native code to listen on an IPC interface.  This native
// method doesn't return until it is told to by STAFProc.  The native method
// creates threads for each service request.  These threads call back into the
// helper object in order load/initialize/call/terminate the service.

public class STAFServiceHelper
{
    static final String STAF_ENTRY = new String("staf/service/info");
    static final String STAF_ENTRY3 = new String("staf/service3/info");
    static final String STAF_SERVICE_CLASS = new String("Service-Class");
    static final String STAF_SERVICE_JARS = new String("Packaged-Jars");

    static public class ServiceInit
    {
        public ServiceInit(String name, String parms, String writeLocation)
        {
            this.name = name;
            this.parms = parms;
            this.writeLocation = writeLocation;
        }

        public String name;
        public String parms;
        public String writeLocation;
    }

    static public class ServiceRequest
    {
        public ServiceRequest(String stafInstanceUUID, String machine,
                              String machineNickname,
                              String handleName, int handle,
                              int trustLevel, boolean isLocalRequest,
                              int diagEnabled,
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
    }

    static public class ServiceData
    {
        public Object service;
        public JarFile serviceJar;
        public ClassLoader serviceClassLoader;
        public int serviceType;

        public ServiceData(Object serviceObject, JarFile serviceJar,
                           ClassLoader serviceClassLoader, int serviceType)
        {
            this.service = serviceObject;
            this.serviceJar = serviceJar;
            this.serviceClassLoader = serviceClassLoader;
            this.serviceType = serviceType;
        }
    }

    public static void main(String [] argv)
    {
        if (argv.length != 1)
        {
            System.out.println(
                "Usage: java com.ibm.staf.service.STAFServiceHelper <JVM Name>");
            System.exit(1);
        }

        STAFServiceHelper helper = new STAFServiceHelper(argv[0]);

        // We will only return from this call once there are no loaded services
        // left.

        helper.listen();

        System.exit(0);
    }

    public STAFServiceHelper(String jvmName)
    {
        fJVMName = jvmName;
    }

    // loadService - This method is reponsible for loading and constructing the
    //               implementation class for a given service.  It supports
    //               raw class files, as well as jar-based services.

    private STAFResult loadService(String service, String serviceImpl,
                                   String stafTemp, int serviceType)
    {
        try
        {
            String serviceClassName = serviceImpl;
            ClassLoader loader = this.getClass().getClassLoader();
            Class serviceClass = null;
            Object serviceObj = null;
            JarFile jarFile = null;

            if (serviceImpl.toLowerCase().endsWith(".jar"))
            {
                try
                {
                    jarFile = new JarFile(serviceImpl);
                }
                catch (IOException e)
                {
                    return new STAFResult(STAFResult.JavaError,
                                          "Unable to open jar file: " +
                                          serviceImpl);
                }

                Manifest manifest = null;

                try
                {
                    manifest = jarFile.getManifest();
                }
                catch (IOException e)
                {
                    return new STAFResult(STAFResult.JavaError,
                                          "Unable to load jar file manifest: " +
                                          serviceImpl);
                }

                Attributes attrs;

                if (manifest.getEntries().containsKey(STAF_ENTRY3))
                {
                    attrs = manifest.getAttributes(STAF_ENTRY3);
                }
                else if (manifest.getEntries().containsKey(STAF_ENTRY))
                {
                    attrs = manifest.getAttributes(STAF_ENTRY);
                }
                else
                {
                    return new STAFResult(STAFResult.JavaError,
                                          "Invalid manifest for STAF service: " +
                                          "No " + STAF_ENTRY + " entry");
                }

                if (!attrs.containsKey(new Attributes.Name(STAF_SERVICE_CLASS)))
                {
                    return new STAFResult(STAFResult.JavaError,
                                          "Invalid manifest for STAF service: " +
                                          "No " + STAF_SERVICE_CLASS +
                                          " attribute");
                }

                serviceClassName = attrs.getValue(STAF_SERVICE_CLASS);

                String serviceDir = stafTemp + "/lang/java/service/" + service +
                    "/jars";

                File serviceDirFile = new File(serviceDir);

                if (!serviceDirFile.exists() && !serviceDirFile.mkdirs())
                {
                    return new STAFResult(STAFResult.JavaError,
                                          "Error creating directory: " +
                                          serviceDir);
                }

                loader = new STAFServiceJarClassLoader(jarFile, serviceDir);
            }

            try
            {
                serviceClass = loader.loadClass(serviceClassName);

                // set context class loader

                Thread.currentThread().setContextClassLoader(loader);

                serviceObj = serviceClass.newInstance();
            }
            catch (ClassNotFoundException e)
            {
                return new STAFResult(STAFResult.JavaError,
                                      "ClassNotFound: " + serviceClassName);
            }
            catch (InstantiationException e)
            {
                return new STAFResult(STAFResult.JavaError,
                                      "Could not instantiate class: " +
                                      serviceClassName);
            }
            catch (IllegalAccessException e)
            {
                return new STAFResult(STAFResult.JavaError,
                                      "Illegal access to class: " +
                                      serviceClassName);
            }

            if (!((serviceObj instanceof
                   com.ibm.staf.service.STAFServiceInterfaceLevel30)))
            {
                return new STAFResult(STAFResult.JavaError,
                                      "Not a valid interface level");
            }

            fServiceMap.put(service, new ServiceData(serviceObj, jarFile,
                                                     loader, serviceType));
        }
        catch (Throwable t)
        {
            // Java error loading service - Print stack trace and return
            // in the error result

            System.out.println(
                "Error loading Java service " + service + ". Verify you " +
                "are using a valid version of Java (e.g. Sun or IBM Java).");
            t.printStackTrace();

            StringWriter stringWriter = new StringWriter();
            t.printStackTrace(new PrintWriter(stringWriter));
            return new STAFResult(
                STAFResult.JavaError, "JSTAFSH.loadService(): Error loading" +
                " the Java service. Verify you are using a valid version of" +
                " Java (e.g. Sun or IBM Java).\n" +
                stringWriter.toString());
        }

        return new STAFResult(STAFResult.Ok);
    }

    private STAFResult initService(String service, ServiceInit initInfo)
    {
        try
        {
            Object serviceDataObj = fServiceMap.get(service);
            int rc = STAFResult.UnknownError;
            STAFResult res = new STAFResult(rc);

            if (serviceDataObj == null)
            {
                return new STAFResult(STAFResult.JavaError,
                                      "No such service implementation: " +
                                      service);
            }

            ServiceData serviceData = (ServiceData)serviceDataObj;

            // set context class loader

            Thread.currentThread().setContextClassLoader(serviceData.serviceClassLoader);

            if (serviceData.service instanceof STAFServiceInterfaceLevel30)
            {
                STAFServiceInterfaceLevel30 serviceL30 =
                    (STAFServiceInterfaceLevel30)serviceData.service;

                res = serviceL30.init(
                     new STAFServiceInterfaceLevel30.InitInfo(service,
                         initInfo.parms, serviceData.serviceJar,
                         serviceData.serviceType, initInfo.writeLocation));
            }
            else
            {
                return new STAFResult(STAFResult.JavaError,
                                      "Unknown service interface type");
            }

            return res;
        }
        catch (Throwable t)
        {
            StringWriter stringWriter = new StringWriter();
            t.printStackTrace(new PrintWriter(stringWriter));
            return new STAFResult(STAFResult.JavaError, stringWriter.toString());
        }
    }

    private STAFResult callService(String service, ServiceRequest reqInfo)
    {
        try
        {
            Object serviceDataObj = fServiceMap.get(service);
            STAFResult result = new STAFResult(STAFResult.UnknownError);

            if (serviceDataObj == null)
            {
                return new STAFResult(STAFResult.JavaError,
                                      "No such service implementation: " +
                                      service);
            }

            ServiceData serviceData = (ServiceData)serviceDataObj;

            // set context class loader

            Thread.currentThread().setContextClassLoader(serviceData.serviceClassLoader);

            if (serviceData.service instanceof STAFServiceInterfaceLevel30)
            {
                STAFServiceInterfaceLevel30 serviceL30 =
                    (STAFServiceInterfaceLevel30)serviceData.service;

                result = serviceL30.acceptRequest(
                    new STAFServiceInterfaceLevel30.RequestInfo(
                        reqInfo.stafInstanceUUID, reqInfo.machine,
                        reqInfo.machineNickname,
                        reqInfo.handleName, reqInfo.handle,
                        reqInfo.trustLevel, reqInfo.isLocalRequest,
                        reqInfo.diagEnabled,
                        reqInfo.request, reqInfo.requestNumber,
                        reqInfo.user,
                        reqInfo.endpoint,
                        reqInfo.physicalInterfaceID));
            }
            else
            {
                result = new STAFResult(STAFResult.JavaError,
                                        "Unknown service interface type");
            }

            return result;
        }
        catch (Throwable t)
        {
            StringWriter stringWriter = new StringWriter();
            t.printStackTrace(new PrintWriter(stringWriter));
            return new STAFResult(STAFResult.JavaError, stringWriter.toString());
        }
    }

    private STAFResult termService(String service)
    {
        try
        {
            Object serviceDataObj = fServiceMap.get(service);
            int rc = STAFResult.UnknownError;
            STAFResult res = new STAFResult(rc);

            if (serviceDataObj == null)
            {
                return new STAFResult(STAFResult.JavaError,
                                      "No such service implementation: " +
                                      service);
            }

            fServiceMap.remove(service);

            ServiceData serviceData = (ServiceData)serviceDataObj;

            // set context class loader

            Thread.currentThread().setContextClassLoader(serviceData.serviceClassLoader);

            if (serviceData.service instanceof STAFServiceInterfaceLevel30)
            {
                STAFServiceInterfaceLevel30 serviceL30 =
                    (STAFServiceInterfaceLevel30)serviceData.service;

                res = serviceL30.term();
            }
            else
            {
                serviceData.serviceJar.close();

                return new STAFResult(STAFResult.JavaError,
                                      "Unknown service interface type");
            }

            serviceData.serviceJar.close();

            return res;
        }
        catch (Throwable t)
        {
            StringWriter stringWriter = new StringWriter();
            t.printStackTrace(new PrintWriter(stringWriter));
            return new STAFResult(STAFResult.JavaError, stringWriter.toString());
        }
    }

    private String fJVMName;
    private Map fServiceMap = Collections.synchronizedMap(new HashMap());

    /************************/
    /* All the native stuff */
    /************************/

    private static native void initialize();
    private native void listen();

    // Static initializer - called first time class is loaded.
    static
    {
        if (System.getProperty("os.name").toLowerCase().indexOf("aix") == 0)
            System.loadLibrary("STAF");

        System.loadLibrary("JSTAFSH");
        initialize();
    }
}
