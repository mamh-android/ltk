/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.event;

import com.ibm.staf.*;
import com.ibm.staf.service.*;
import java.util.*;
import java.io.*;

public class GenerationManager
implements Serializable, Runnable
{
    static final long serialVersionUID = 1; 
    transient double version = 0.9;
    transient double fileVersion;
    transient static int fEventID = 0;
    transient Hashtable fEventTable = new Hashtable();  
    transient PriorityQueue fQueue = new PriorityQueue();
    transient Thread fNotifierThread;
    transient Thread fQueueThread;
    transient STAFHandle eventSTAFHandle = null;
    transient String eventServiceName;
    transient RegistrationManager eventRegManager = null;
    transient String eventGenManagerFileName = null;
    transient HashMap fRequestsAndNotifications = new HashMap();

    public class EventID
    {
        transient int id;
        transient int generatingHandle;
        transient String generatingMachine;
        transient String generatingProcess;
        transient String type;
        transient String subType;
        transient String[] properties;
        transient boolean notify;  // Flag indicating if the event generator
                                   // requested to be notified when all
                                   // registered processes have acknowledged
                                   // receiving the event.
        transient Vector notificationList;

        EventID(int theID, int theGenHandle, String theGenMachine, 
                String theGenProcess, String theType, String theSubType,
                String[] theProperties, boolean theNotify,
                Vector theNotificationList)
        {
            id                = theID;
            generatingHandle  = theGenHandle;
            generatingMachine = theGenMachine;
            generatingProcess = theGenProcess;
            type              = theType;
            subType           = theSubType;
            properties        = theProperties;
            notify            = theNotify;
            notificationList  = theNotificationList;
        }
                
        public int getEventId()
        {
            return id;
        }

        public String getType()
        {
            return type;
        }

        public String getSubtype()
        {
            return subType;
        }

        public Map getPropertyMap()
        {
            return getPropertyMap(false);
        }

        public Map getPropertyMap(boolean maskPrivateData)
        {
            Map propertyMap = new HashMap();

            for (int i = 0; i < properties.length; i++)
            {
                int index = properties[i].indexOf('=');

                if (index >= 0)
                {
                    // Contains a "="
                    String name = properties[i].substring(0, index);
                    String value = properties[i].substring(index + 1);

                    if (!maskPrivateData)
                        propertyMap.put(name, value);
                    else
                        propertyMap.put(name, STAFUtil.maskPrivateData(value));
                }
                else
                {
                    // Does not contain an "=", so assign value = null
                    String name = properties[i];
                    String value = null;
                    propertyMap.put(name, value);
                }
            }

            return propertyMap;
        }

        public List getNotificationList()
        {
            List theNotificationList = new ArrayList();

            for (int i = 0; i < notificationList.size(); i++)
            {
                Notification notification =
                    (Notification)notificationList.get(i);
                theNotificationList.add(notification.getClientMap());
            }
            
            return theNotificationList;
        }

        public Map getEventGeneratorMap()
        {
            Map eventGeneratorMap = EventService.fEventGeneratorMapClass.
                createInstance();
            eventGeneratorMap.put("machine", generatingMachine);
            eventGeneratorMap.put("handleName", generatingProcess);
            eventGeneratorMap.put("handle", "" + generatingHandle);

            return eventGeneratorMap;
        }
    }   

    public class Notification
    {
        Client client;
        EventID eventID;
        boolean async;
        
        public Notification(Client client, EventID eventID, boolean async)
        {
            this.client = client;
            this.eventID = eventID;
            this.async = async;
        }
                
        synchronized public Map getNotificationMap()
        {
            Map notificationMap =
                EventService.fEventIDLongMapClass.createInstance();

            notificationMap.put("eventID", "" + eventID.getEventId());
            notificationMap.put("type", eventID.getType());
            notificationMap.put("subtype", eventID.getSubtype());

            // Currently, getNotificationMap() is only used when listing or
            // querying Event IDs, so always get the properties with any
            // private data masked, as indicated by passing true for the
            // maskPrivateData argument.
            notificationMap.put("propertyMap", eventID.getPropertyMap(true));

            notificationMap.put("generatedBy",
                                eventID.getEventGeneratorMap());
            notificationMap.put("notificationList",
                                eventID.getNotificationList());
            
            return notificationMap;
        }

        synchronized public Map getClientMap()
        {
            return client.getClientMap(EventService.fNotifieeMapClass);
        }
    }

    public GenerationManager(STAFHandle theSTAFHandle, String serviceName,
                             RegistrationManager regManager,
                             String genManagerFileName)
    {
        eventSTAFHandle = theSTAFHandle;
        eventServiceName = serviceName;
        eventRegManager = regManager;
        eventGenManagerFileName = genManagerFileName;

        fNotifierThread = new Thread()
        {
            public void run() 
            {
                notificationThread();
            }
        };

        fNotifierThread.start();

        fQueueThread = new Thread(this);
        fQueueThread.start(); // this calls the run() method
    }

    synchronized public static void serializeEventID(ObjectOutputStream os)
    throws IOException 
    {
        os.writeInt(fEventID);
    }         

    synchronized public static void deSerializeEventID(ObjectInputStream os)
    throws IOException
    {
        fEventID = os.readInt();
    }        

    private void writeObject(ObjectOutputStream stream)
    throws IOException
    {
        stream.writeDouble(version);
        serializeEventID(stream);        
    } 

    private void readObject(ObjectInputStream stream)
    throws IOException, ClassNotFoundException
    {
        fileVersion = stream.readDouble();   
        deSerializeEventID(stream);
    }

    synchronized void serialize()
    {
        try
        {  
            ObjectOutputStream out = new ObjectOutputStream(                     
                new FileOutputStream(eventGenManagerFileName));

            out.writeObject(this);
            out.close(); 
        } 
        catch(Exception e)
        {
           if (EventService.DEBUG) e.printStackTrace();
        }
    } 

    synchronized void deSerialize()
    {
        try
        {
            ObjectInputStream in = 
                new ObjectInputStream(
                new FileInputStream(eventGenManagerFileName)); 

                in.readObject();       
        }
        catch(Exception e)
        {
            if (EventService.DEBUG) e.printStackTrace();
        }  
    }

    synchronized public int generateEvent(String theGenMachine,
                                          String theGenProcess, int theGenHandle,
                                          String theType, String theSubtype, 
                                          String[] theProperties,
                                          boolean notify,
                                          boolean async,
                                          Vector mergedClients)
    {
        if ((mergedClients == null) || (mergedClients.size() == 0))
            return EventService.kNoClientsForEvent;
 
        Integer nextID = new Integer(++fEventID);
        Vector notificationList = new Vector();
        EventID eventID = new EventID(nextID.intValue(), theGenHandle,
                                      theGenMachine, theGenProcess, theType,
                                      theSubtype, theProperties, notify,
                                      notificationList);
               
        fEventTable.put(nextID, eventID);
 
        if ((mergedClients != null) && (mergedClients.size() != 0))
        {
            for (Enumeration e = mergedClients.elements(); e.hasMoreElements();)
            {
                Client client = new Client((Client)e.nextElement());
                Notification notification = new Notification(client,
                                                             eventID,
                                                             async);

                notificationList.addElement(notification);
                fQueue.enqueue(System.currentTimeMillis(), notification);
            }
        }
        else return EventService.kNoClientsForEvent;

        synchronized(fNotifierThread)
        {
            fNotifierThread.notify();  
        }           

        return nextID.intValue();
    }

    synchronized private int sendMessage(Map propertyMap,
                                         Notification notification)
    {
        String type = "STAF/Service/Event";
        STAFResult fSTAFResult = null;
        TimeStamp now = new TimeStamp();

        Map messageMap = new HashMap();
        messageMap.put("eventServiceName", eventServiceName);
        messageMap.put("eventID", "" + notification.eventID.id);
        messageMap.put("machine", notification.eventID.generatingMachine);
        messageMap.put("handleName", notification.eventID.generatingProcess);
        messageMap.put("handle", "" + notification.eventID.generatingHandle);
        messageMap.put("timestamp", now.currentDate() + "-" +
                       now.currentTime());
        messageMap.put("type", notification.eventID.type);
        messageMap.put("subtype", notification.eventID.subType);
        messageMap.put("propertyMap", propertyMap);

        STAFMarshallingContext mc = new STAFMarshallingContext();
        mc.setRootObject(messageMap);
        String message = mc.marshall();

        if (notification.client.fWho.handle != 0)
        {
            if (notification.async)
            {
                // ASYNC is an undocumented option,
                // only used by the STAX service
                fSTAFResult = eventSTAFHandle.submit2(STAFHandle.ReqQueue,
                    notification.client.getMachineName(),
                    "QUEUE", "QUEUE HANDLE " + notification.client.fWho.handle +
                    " PRIORITY " + notification.client.fHow.priority +
                    " TYPE " + STAFUtil.wrapData(type) +
                    " MESSAGE " + STAFUtil.wrapData(message));

                String requestNumber = fSTAFResult.result;

                fRequestsAndNotifications.put(requestNumber, notification);
            }
            else
            {
                fSTAFResult = eventSTAFHandle.submit2(
                    notification.client.getMachineName(),
                    "QUEUE", "QUEUE HANDLE " + notification.client.fWho.handle +
                    " PRIORITY " + notification.client.fHow.priority +
                    " TYPE " + STAFUtil.wrapData(type) +
                    " MESSAGE " + STAFUtil.wrapData(message));

                if (fSTAFResult.rc != fSTAFResult.Ok)
                {
                    if (fSTAFResult.rc == STAFResult.HandleDoesNotExist ||
                        fSTAFResult.rc == STAFResult.NoPathToMachine ||
                        fSTAFResult.rc == STAFResult.CommunicationError)
                    {
                        // Unregister the client (which was registered by handle)
                        // since the handle/machine is no longer available

                        String[] subTypes = new String[1];
                        subTypes[0] = notification.eventID.subType;

                        eventRegManager.unRegisterClient(
                            notification.client.getMachineName(),
                            notification.client.fWho.handleName,
                            notification.client.fWho.handle,
                            notification.eventID.type, subTypes);
                    }
                }
            }
        }
        else
        {
            fSTAFResult = eventSTAFHandle.submit2(
                notification.client.getMachineName(),
                "QUEUE", "QUEUE NAME " +
                STAFUtil.wrapData(notification.client.fWho.handleName) +
                " PRIORITY " + notification.client.fHow.priority +
                " TYPE " + STAFUtil.wrapData(type) +
                " MESSAGE " + STAFUtil.wrapData(message));
        }               

        return fSTAFResult.rc;
    }

    synchronized public int ackEvent(int id, String machine,
                                     String handleName, int handle)
    {
        EventID eventID = (EventID)fEventTable.get(new Integer(id));

        if (eventID == null) return EventService.kNoSuchID;
 
        Vector clientList = eventID.notificationList;

        if (clientList != null)
        {
            for (Enumeration e = clientList.elements(); e.hasMoreElements();)
            {
                Notification notification = (Notification)e.nextElement();

                int    nHandle  = notification.client.fWho.handle;
                String nName    = notification.client.fWho.handleName;
                String nMachine = notification.client.fWho.machineName;

                if ((((nHandle == 0) && nName.equalsIgnoreCase(handleName)) ||
                     ((nHandle != 0) && (nHandle == handle))) &&
                    nMachine.equalsIgnoreCase(machine))
                {
                    notification.client.fHow.maxAttempts = 0;
                    clientList.removeElement(notification);

                    if (eventID.notify && clientList.isEmpty())
                    {
                        // Notify the generator that everyone has acknowledged

                        Map messageMap = new HashMap();                         
                        messageMap.put("eventServiceName", eventServiceName);   
                        messageMap.put("eventID", new Integer(id));
                        
                        STAFMarshallingContext mc = new STAFMarshallingContext();
                        mc.setRootObject(messageMap);

                        eventSTAFHandle.submit2(
                            STAFHandle.ReqFireAndForget,
                            eventID.generatingMachine, "QUEUE",
                            "QUEUE HANDLE " + eventID.generatingHandle +
                            " TYPE STAF/Service/Event/AllAcksReceived" +
                            " MESSAGE " + STAFUtil.wrapData(mc.marshall()));
                    }

                    return STAFResult.Ok;  //kAckPending;
                }
            }

            return EventService.kNoAckPending;
        }

        return EventService.kNoSuchID;
    }

    synchronized public Vector getClientsForEvent(int id)
    {
        Vector result = null;
        EventID eventID = (EventID)fEventTable.get(new Integer(id));

        if (eventID != null) result = eventID.notificationList;

        return (result != null ? (Vector)result.clone() : new Vector());
    }
        
    synchronized public Vector getNotifications()
    {
        Vector result = new Vector();
                
        PriorityQueue.PriorityQueueEntry[] entries = fQueue.getQueueCopy();
                
        for (int i = 0; i < fQueue.count(); i++)
             result.addElement(entries[i]);
                
        return result;
    }
        
    void notificationThread()
    {
        while (true)
        {
            try
            {
                if (fQueue.front() == null)
                {
                    synchronized (fNotifierThread)
                    { fNotifierThread.wait(); }
                }
                else
                {
                    long waitTime = fQueue.topPriority() -
                                    System.currentTimeMillis();

                    if (waitTime  > 0)
                    {
                        synchronized (fNotifierThread)
                        { fNotifierThread.wait(waitTime); }
                    }

                    if (System.currentTimeMillis() >= fQueue.topPriority())
                        notify((Notification)fQueue.dequeue());
                }   
            }
            catch(InterruptedException e)
            {
                if (EventService.DEBUG) e.printStackTrace(); 
            } 
            catch(Exception e)
            {
                if (EventService.DEBUG) e.printStackTrace(); 
            } 
        }
    }

    synchronized private int notify(Notification notification)
    {
        Client client = notification.client;
        EventID eventID = notification.eventID;
        int id = eventID.id;
        int fReturnCode = STAFResult.Ok;
        Map propertyMap = eventID.getPropertyMap();

        if (fEventTable.get(new Integer(id)) == null)
            return EventService.kNoSuchID;

        if ((client.getMaxAttempts() <= 0))
        {
            Vector clientList =
                   ((EventID)fEventTable.get(new Integer(id))).notificationList;

            clientList.removeElement(notification);

            if (clientList.isEmpty())
                fEventTable.remove(new Integer(id)); 
        }
        else
        {
            fReturnCode = sendMessage(propertyMap, notification);

            if ((client.fHow.priority > 0) &&
                (client.fHow.priority >= client.fHow.priorityDelta))
            {
                    --client.fHow.priority;
            }
            else client.fHow.priority = 0;

            --client.fHow.maxAttempts;
            fQueue.enqueue(client.getTimeout() + System.currentTimeMillis(),
                           notification);
        }
       
        return fReturnCode;
    }
    
    public void handleReset()
    {
        fEventTable.clear();
    }

    private void handleRequestCompleteMsg(STAFQueueMessage queueMessage)
    {
        Map messageMap = (Map)queueMessage.message;

        String reqNum = (String)messageMap.get("requestNumber");
        String rc = (String)messageMap.get("rc");

        if (!(fRequestsAndNotifications.containsKey(reqNum)))
        {
            return;
        }

        Notification notification =
            (Notification)(fRequestsAndNotifications.get(reqNum));

        int queueRC = (new Integer(rc)).intValue();

        if (queueRC == STAFResult.HandleDoesNotExist ||
            queueRC == STAFResult.NoPathToMachine ||
            queueRC == STAFResult.CommunicationError)
        {
            // Unregister the client (which was registered by handle)
            // since the handle/machine is no longer available

            String[] subTypes = new String[1];
            subTypes[0] = notification.eventID.subType;

            eventRegManager.unRegisterClient(
                notification.client.getMachineName(),
                notification.client.fWho.handleName,
                notification.client.fWho.handle,
                notification.eventID.type, subTypes);
        }

        fRequestsAndNotifications.remove(reqNum);
    }
    
    public void run()
    {
        STAFResult queueGetResult;

        for (;;)
        {
            queueGetResult = eventSTAFHandle.submit2("local",
                "QUEUE", "GET WAIT");

            if (queueGetResult.rc != 0)
            {
                // XXX: Do anything?
                continue;
            }

            // Need a try/catch block so can catch any errors and continue
            // processing messages on the queue

            try
            {
                STAFQueueMessage queueMessage = new STAFQueueMessage(
                    queueGetResult.result);

                String queueType = queueMessage.type;

                if (queueType.equalsIgnoreCase("STAF/RequestComplete"))
                {
                    handleRequestCompleteMsg(queueMessage);
                }
                else if (queueType.equalsIgnoreCase("STAF/Service/Event/End"))
                {
                    return;
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
    }
}
