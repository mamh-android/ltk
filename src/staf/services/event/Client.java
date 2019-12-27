/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.event;

import com.ibm.staf.*;
import java.util.*;
import java.io.*;

public class Client
implements Serializable
{
    static final long serialVersionUID = 1;
    transient double version = 0.90;
    transient double fileVersion;
    transient Who fWho;
    transient How fHow;

    public class Who
    {
        transient String machineName = "";
        transient String handleName = "";
        transient int handle = 0;

        public Who()
        { /* Default */ }

        public Who(String machineName, String handleName)
        {
            this.machineName = machineName;
            this.handleName = handleName;
            this.handle = 0;
        }

        public Who(String machineName, int handle)
        {
            this.machineName = machineName;
            this.handleName = "";
            this.handle = handle;
        }
    }

    public class How
    {
        transient int maxAttempts = 1;
        transient long timeout = 60000;
        transient int priority = 5;
        transient int priorityDelta = 1;

        public How()
        { /* Default */ }

        public How(int maxAttempts, long timeout, int priority, int delta)
        {
            this.maxAttempts = maxAttempts;
            this.timeout = timeout;
            this.priority = priority;
            this.priorityDelta = delta;
        }
    }

    public Client(Client c)
    { 
        fWho = c.fWho;
        fHow = new How(c.fHow.maxAttempts, c.fHow.timeout, c.fHow.priority, 
                       c.fHow.priorityDelta);
    }

    public Client(String machineName, String handleName)
    {
        fWho = new Who(machineName, handleName);
        fHow = new How();
    }

    public Client(String machineName, int handle)
    {
        fWho = new Who(machineName, handle);
        fHow = new How();
    }

    public Client(String machineName, String handleName, int maxAttempts,
                  long timeout, int priority, int delta)
    {
        fWho = new Who(machineName, handleName);
        fHow = new How(maxAttempts, timeout, priority, delta);
    }

    public Client(String machineName, int handle, int maxAttempts,
                  long timeout, int priority, int delta)
    {
        fWho = new Who(machineName, handle);
        fHow = new How(maxAttempts, timeout, priority, delta);
    }

    private void writeObject(ObjectOutputStream stream) throws IOException
    {
        stream.writeDouble(version);
        stream.writeObject(fWho.machineName);
        stream.writeObject(fWho.handleName);
        stream.writeInt(fWho.handle);

        stream.writeInt(fHow.maxAttempts);
        stream.writeLong(fHow.timeout);
        stream.writeInt(fHow.priority);
        stream.writeInt(fHow.priorityDelta);
    }

    private void readObject(ObjectInputStream stream)
    throws IOException, ClassNotFoundException
    {   
        fWho = new Who();
        fHow = new How();
        
        version = 0.90;
        fileVersion = stream.readDouble();
        fWho.machineName = (String)stream.readObject();
        fWho.handleName = (String)stream.readObject();
        fWho.handle = stream.readInt();

        fHow.maxAttempts = stream.readInt();
        fHow.timeout = stream.readLong();
        fHow.priority = stream.readInt();
        fHow.priorityDelta = stream.readInt();
    }      
	
    public long getTimeout() { return fHow.timeout; }
    public String getMachineName() { return fWho.machineName; }
    public int getHandle() { return fWho.handle; }
    public String getProcessName() { return fWho.handleName; }
    public int getMaxAttempts() { return fHow.maxAttempts; }
    public int getPriority() { return fHow.priority; }

    public Map getClientMap(STAFMapClassDefinition mapClass)
    {
        Map clientMap = mapClass.createInstance();

        if (fWho.handle == 0)
        {
            clientMap.put("notifyBy", "Name");
            clientMap.put("notifiee", fWho.handleName);
        }
        else
        {
            clientMap.put("notifyBy", "Handle");
            clientMap.put("notifiee", "" + fWho.handle);
        }
        
        clientMap.put("machine", fWho.machineName);
        clientMap.put("attempts", "" + fHow.maxAttempts);
        clientMap.put("timeout", "" + fHow.timeout);
        clientMap.put("priority", "" + fHow.priority);
        clientMap.put("priorityDelta", "" + fHow.priorityDelta);

        return clientMap;
    }

    public boolean equals(Object o)
    {
        Client c = (Client)o;

        // Compare "fWho" data, ignoring "fHow" data since this can be
        // different for a client.
        // Compare machine name and if matches, check if c is registered by
        // name (i.e. handle = 0, handleName != null) then only compare
        // handleName (ignoring case); otherwise only compare handle number.

        if (c == this) return true;
        if (!(c instanceof Client)) return false;
        if (!(c.fWho.machineName.equals(this.fWho.machineName))) return false;

        if ((fWho.handle == 0) &&
            (fWho.handleName.equalsIgnoreCase(c.fWho.handleName)))
        {
            return true;
        }

        if ((fWho.handle != 0) && (fWho.handle == c.fWho.handle)) return true;

        return false;
    }
}
