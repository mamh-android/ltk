/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;
import java.util.Map;
import java.util.TreeMap;

/**
 * Manages a map of namespaces
 * <p>
 * The key is the upper-case name of the namespace and the value is a
 * Namespace instance that contains the name (in the case specified),
 * variables definied in the namespace, the parent namespace, and
 * children namespaces.
 * This allows namespace names to be case-insensitive.
 */
public class NamespaceManager
{
    private Map fNamespaceMap = new TreeMap();

    /**
     * Creates a new NamespaceManager instance.
     */
    public NamespaceManager()
    { /* Do Nothing */ }
    
    /**
     * Adds a namespace to the map of namespaces.
     * 
     * @param name The name of the namespace to add
     * @param namespace An instance of a Namespace to add
     */
    public void create(String name, Namespace namespace)
    {
        fNamespaceMap.put(name.toUpperCase(), namespace);
    }
     
    /**
     * Returns the Namespace instance for the specified name.
     */ 
    public Namespace get(String name)
    {
        return (Namespace)fNamespaceMap.get(name.toUpperCase());
    }

    /**
     * Deletes a namespace from the map of namespaces.
     *  
     * @param name The name of the namespace to remove
     * @return An Object containing the Namespace instance that was removed
     */ 
    public Object delete(String name)
    {
        return fNamespaceMap.remove(name.toUpperCase());
    }
     
    /**
     * Checks if the namespace name specified (case-insensitive) exists
     * in the namespace map
     * 
     * @param name The name of the namespace
     * @return true if the namespace is in the map or false if the
     * namespace is not in the map
     */ 
    public boolean contains(String name)
    {
        return fNamespaceMap.containsKey(name.toUpperCase());
    }
     
    /**
     * Returns a copy of the namespace map
     * @return an instance of the Map of the namespaces
     */ 
    public Map getNamespaceMapCopy()
    {
        return fNamespaceMap;
    }

    /**
     * Delete all the namespaces in the map
     */ 
    public void deleteAll()
    {
        fNamespaceMap = new TreeMap();
    }
}