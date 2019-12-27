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

public class RegistrationManager
implements Serializable
{
    static final long serialVersionUID = 1;
    transient Hashtable fRegistrationTable = new Hashtable();
    transient double version = 0.9;
    transient double fileVersion;

    public class Type
    {
        transient String key = null;
        transient Vector clientList = new Vector();
        transient Hashtable subtypeTable = new Hashtable();

        public Type(String name)
        {
            key = name;
        }
         
        public List getSubtypesAsList()
        {
            List subtypeList = new ArrayList();

            for (Enumeration e = subtypeTable.keys();
                 e.hasMoreElements();)
            {
                subtypeList.add((String)e.nextElement());
            }

            return subtypeList;
        }
    }

    public class SubType
    {
        transient String key = null;
        transient Vector clientList = new Vector();

        public SubType(String name)
        {
            key = name;
        }
    }

    private void writeObject(ObjectOutputStream stream) throws IOException
    {   
        Type type; 
        SubType subtype; 

        stream.writeDouble(version);
        stream.writeInt(fRegistrationTable.size());

        for (Enumeration e = fRegistrationTable.elements();
             e.hasMoreElements();)
        {
            type = (Type)e.nextElement();
            stream.writeObject(type.key);
            stream.writeObject(type.clientList);
            stream.writeInt(type.subtypeTable.size());

            for (Enumeration subTypeEnum = type.subtypeTable.elements();
                subTypeEnum.hasMoreElements();)
            {
                subtype = (SubType)subTypeEnum.nextElement();
                stream.writeObject(subtype.key);
                stream.writeObject(subtype.clientList);                 
            }                               
        }               
    } 

    private void readObject(ObjectInputStream stream)
    throws IOException, ClassNotFoundException
    {
        fRegistrationTable = new Hashtable(); 
        version = 1.0;
        fileVersion = stream.readDouble();

        int tableSize = stream.readInt();
        int subtypeTableSize; 
        Type type;
        SubType subtype; 
         
        for (int i = 0; i < tableSize; i++)
        {
            type = new Type((String)stream.readObject());
            type.clientList = (Vector)stream.readObject();
            subtypeTableSize = stream.readInt();

            for (int j = 0; j < subtypeTableSize; j++)
            {
                subtype = new SubType((String)stream.readObject());
                subtype.clientList = (Vector)stream.readObject(); 
                // XXX CHANGED!!!
                type.subtypeTable.put(subtype.key.toLowerCase(), subtype);
            } 
      
            fRegistrationTable.put(type.key.toLowerCase(), type); 
        }     
    } 

    synchronized public void registerClient(Client c, String t, String[] s)
    {
        Type type = (Type)fRegistrationTable.get(t.toLowerCase());

        // if no such type, create one
        if (type == null)
        {
            type = new Type(t);
            fRegistrationTable.put(t.toLowerCase(), type);
        }

        // if no subtypes specified, add client to type list
        if (s.length == 0)
        {
            boolean found = false;
                        
            for (Enumeration e = type.clientList.elements(); 
                 e.hasMoreElements();)
            {
                Client nextClient = (Client)e.nextElement();

                if (nextClient.equals(c))
                {
                    int index = type.clientList.indexOf(nextClient);

                    type.clientList.setElementAt(c, index);
                    found = true;

                    break;  
                }
            }
            
            if (!found) type.clientList.addElement(c);
        }
        else
        {
            // for each specified subtype, add client to subtype list
            for (int i = 0; i < s.length; i++)
            {
                boolean found = false;
                SubType subtype =
                        (SubType)type.subtypeTable.get(s[i].toLowerCase());

                // if no such subtype create one
                if (subtype == null)
                {
                    subtype = new SubType(s[i]);
                    type.subtypeTable.put(s[i].toLowerCase(), subtype);
                }

                for (Enumeration e = subtype.clientList.elements(); 
                     e.hasMoreElements();)
                {
                    Client nextClient = (Client)e.nextElement();

                    if (nextClient.equals(c))
                    {
                        int index = subtype.clientList.indexOf(nextClient);

                        subtype.clientList.setElementAt(c, index);
                        found = true;

                        break;  
                    }
                }
            
                if (!found) subtype.clientList.addElement(c);
            }
        }
    }

    // Returns all registrations for the specified type (if only registered
    // for that type and not registered explicitly for any subtypes)

    synchronized public Vector clientsRegisteredFor(String t)
    {
        Type type = (Type)fRegistrationTable.get(t.toLowerCase());

        if (type == null) return null;

        Vector v = (Vector)type.clientList.clone();

        return v;
    }

    // Returns all registrations for the specified subtype of the specified
    // type

    synchronized public Vector clientsRegisteredFor(String t, String s) 
    {
        Type type = (Type)fRegistrationTable.get(t.toLowerCase());

        if (type == null) return null;

        SubType subtype = (SubType)type.subtypeTable.get(s.toLowerCase());

        if (subtype == null) return null;

        return (Vector)subtype.clientList.clone();
    }

    // Returns all clients that are registered either for the specified
    // type or for the specified subtype of the specified type

    synchronized public Vector clientsFor(String t, String s)
    {
        Vector typeClientList = clientsRegisteredFor(t);

        if (typeClientList == null) return null;

        Vector subtypeClientList = clientsRegisteredFor(t, s);

        if (subtypeClientList == null) return typeClientList;

        // do the actual merge on a new vector and return it

        for (Enumeration e = typeClientList.elements();
             e.hasMoreElements();)
        {
            Client client = (Client)e.nextElement();

            if (!subtypeClientList.contains(client))
                subtypeClientList.addElement(client);
        }

        return subtypeClientList;
    }

    // Returns all registrations for the specified type or any of its subtypes
    
    synchronized public Vector allClientsFor(String t)
    {
        Type type = (Type)fRegistrationTable.get(t.toLowerCase());

        if (type == null) return null;

        Vector allClients = new Vector();
 
        for (Enumeration e = type.clientList.elements();
             e.hasMoreElements();)
        {
            allClients.addElement(e.nextElement());    
        } 

        if (type.subtypeTable.size() == 0)
            return allClients;

        SubType sub = null;

        for (Enumeration e = type.subtypeTable.elements();
             e.hasMoreElements();)
        {
            sub = ((SubType)e.nextElement());

            for (Enumeration enum2 = sub.clientList.elements();
                 enum2.hasMoreElements();)
            {
                allClients.addElement(enum2.nextElement()); 
            }  
        }

        return allClients;  
    }     

    public List getTypes(boolean listSubtypes)
    {
        // Return a list of the types and their subtypes

        List typeList = new ArrayList();

        if (fRegistrationTable != null) 
        {
            for (Enumeration e = fRegistrationTable.keys();
                 e.hasMoreElements();)
            {
                String type = (String)e.nextElement();

                if (listSubtypes)
                {
                    Map typeMap = EventService.fTypeMapClass.createInstance();

                    typeMap.put("type", type);

                    List subtypeList = ((Type)fRegistrationTable.get(
                        type.toLowerCase())).getSubtypesAsList();

                    typeMap.put("subtypeList", subtypeList);

                    typeList.add(typeMap);
                }
                else
                {
                    typeList.add(type);
                }
            }
        }

        return typeList;
    }

    public List getSubtypes(String type)
    {
        // Return a list of the subtypes for the specified type

        List subtypeList = new ArrayList();

        if (fRegistrationTable != null)
        {
            if (fRegistrationTable.get(type.toLowerCase()) != null)
            {
                subtypeList = ((Type)fRegistrationTable.get(
                    type.toLowerCase())).getSubtypesAsList();
            }
            else
            {
                // Return null if no processes are registered to be
                // notified about event of this type
                return null;
            }
        }

        return subtypeList;
    }

    public STAFResult unRegisterClient(String machine, String handleName,
                                       int handle, String t, String[] s)
    {
        boolean clientMatch = false;

        // Check if any registrations exist for the specified type

        Type type = (Type)fRegistrationTable.get(t.toLowerCase());

        if (type == null)
        {
            return new STAFResult(
                EventService.kNotRegisteredForType,
                "No registrations exist for type " + t);
        }

        // Check if one or more SUBTYPE options were specified on the
        // UNREGISTER request

        if (s.length == 0)
        {
            // No subtypes were specified so remove the client from the
            // type table if it exists

            for (Enumeration e = type.clientList.elements(); 
                 e.hasMoreElements();)
            {
                Client nextClient = (Client)e.nextElement();
                int nHandle = nextClient.getHandle();
                String nName = nextClient.getProcessName();
                String nMachine = nextClient.getMachineName();

                if ((((nHandle == 0) && nName.equalsIgnoreCase(handleName)) ||
                     ((nHandle != 0) && (nHandle == handle))) &&
                    nMachine.equalsIgnoreCase(machine)) 
                {
                    clientMatch = true;

                    int index = type.clientList.indexOf(nextClient);
                    type.clientList.removeElementAt(index);
                        
                    if ((type.clientList.size() == 0) &&
                        (type.subtypeTable.size() == 0))
                    {
                        fRegistrationTable.remove(t.toLowerCase());
                    }

                    break;
                }
            }

            if (clientMatch)
            {
                return new STAFResult(STAFResult.Ok);
            }
            else
            {
                String msg = "No registration exists for type \"" + t + "\"" +
                    " with no subtype ";

                if ((handle == 0) || !handleName.equals(""))
                {
                    msg += "for a process running on machine " + machine +
                        " registered to be notified by handle name " +
                        handleName;
                }
                else
                {
                    msg += "for a process running on machine " + machine +
                        " registered to be notified by handle number " +
                        handle;
                }

                return new STAFResult(
                    EventService.kNotRegisteredForType, msg);
            }
        }

        // Subtypes were specified on the unregister request
        
        int sLength = s.length;
        int index = 0;
        SubType subtype = null;   

        for (int count = 0; count < sLength; count++)
        { 
            subtype = (SubType)type.subtypeTable.get(s[count].toLowerCase());

            if (subtype == null) continue;

            for (Enumeration e = subtype.clientList.elements();
                 e.hasMoreElements();)
            {
                Client nextClient = (Client)e.nextElement();
                int nHandle = nextClient.getHandle();
                String nName = nextClient.getProcessName();
                String nMachine = nextClient.getMachineName();

                if ((((nHandle == 0) && nName.equalsIgnoreCase(handleName)) ||
                     ((nHandle != 0) && (nHandle == handle))) &&
                    nMachine.equalsIgnoreCase(machine))
                {
                    clientMatch = true;

                    index = subtype.clientList.indexOf(nextClient);
                        
                    subtype.clientList.removeElementAt(index);
                            
                    if (subtype.clientList.size() == 0)
                    {
                        type.subtypeTable.remove(s[count].toLowerCase());
                    }

                    if ((type.clientList.size() == 0) &&
                        (type.subtypeTable.size() == 0))
                    {
                        fRegistrationTable.remove(t.toLowerCase());
                    }

                    break;  
                }
            }
        }

        if (clientMatch)
        {
            return new STAFResult(STAFResult.Ok);
        }
        else
        {
            String subtypesString = "\"" + s[0] + "\"";

            for (int count = 1; count < sLength; count++)
            {
                subtypesString += ", \"" + s[count] + "\"";
            }

            String msg = "No registration exists for type \"" + t +
                "\" with subtype(s) " + subtypesString + " ";

            if ((handle == 0) || !handleName.equals(""))
            {
                msg += "for a process running on machine " + machine +
                    " registered to be notified by handle name " +
                    handleName;
            }
            else
            {
                msg += "for a process running on machine " + machine +
                    " registered to be notified by handle number " +
                    handle;
            }

            return new STAFResult(
                EventService.kNotRegisteredForSubtype, msg);
        }
    }
    
    public void handleReset()
    {
        fRegistrationTable.clear();
    }    
}
