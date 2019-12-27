/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;
import java.util.Map;
import java.util.TreeMap;
import java.util.Iterator;

/**
 * Represents a namespace which holds a set of variables and can contain
 * child namespaces which allows for a namespace hierarchy.
 */
public class Namespace
{
    /** Indicates that a namespace has no parent;  it is a root namespace */
    public final static String sNONE = "NONE";

    private String fName;
    private String fDescription;
    private String fParent;
    private Map fChildren = new TreeMap();
    private VariableManager fVariableManager = new VariableManager();

    /**
     * Creates a new Namespace instance with the specified name and
     * description and with no parent.
     * 
     * @param name The name of the namespace to create
     * @param description The description of the namespace
     */ 
    public Namespace(String name, String description)
    {
        fName = name;
        fDescription = description;
        fParent = sNONE;
    }
     
    /**
     * Creates a new Namespace instance with the specified name, description,
     * and parent namespace name.
     * 
     * @param name The name of the namespace to create
     * @param description The description of the namespace
     * @param parent The name of the parent namespace
     */ 
    public Namespace(String name, String description, String parent)
    {
        fName = name;
        fDescription = description;
        fParent = parent;
    }
     
    /**
     * Gets the name of the namespace
     * @return The name of the namespace
     */ 
    public String getName() { return fName; }
    
    /**
     * Sets the name of the namespace
     * @param name The name of the namespace
     */ 
    public void setName(String name)
    {
        fName = name;
    }
    
    /**
     * Gets the description of the namespace
     * @return the description of the namespace
     */ 
    public String getDescription() { return fDescription; }

    /**
     * Sets the description of the namespace
     * @param description The description of the namespace
     */ 
    public void setDescription(String description)
    {
        fDescription = description;
    }

    /**
     * Gets the name of the parent namespace
     * @return the name of the parent namespace
     */ 
    public String getParent() { return fParent; }

    /**
     * Sets the name of the parent namespace
     * @param parent The name of the parent namespace
     */ 
    public void setParent(String parent)
    {
        fParent = parent;
    }

    /**
     * Get a map of the namespaces that are children of the namespace.
     * The map's key will be the name of each child namespace in upper-case
     * and the value for the map will be a Namespace instance.
     * @return an instance of a Map containing the namespaces that are
     * children of this namepsace
     */ 
    public Map getChildren() { return fChildren; }

    /**
     * Add a map of namespaces to the children map for the namespace.
     * @param children An instance of a Map of namespaces to be added as
     * children of this namespace
     */ 
    public void addChildren(Map children)
    {
        fChildren.putAll(children);
    }

    /**
     * Adds a namespace to the children map for the namespace.
     * @param name The name of the child namespace to add
     * @param ns An instance of the child namespace to add
     */ 
    public void addChild(String name, Namespace ns)
    {
        fChildren.put(name, ns);
    }
    
    /**
     * Removes a namespace from the children map for the namespace.
     * @param name The name of the child namespace to remove
     */ 
    public void removeChild(String name)
    {
        fChildren.remove(name);
    }

    /**
     * Returns a copy of the map containing the variables defined for the
     * namespace. The map's key will contain the key for each variable in
     * upper-case and the value for the map will be a Variable instance.
     * @return an instance of a Map containing the variables defined for
     * the namespace
     */ 
    public Map getVariables()
    {
        return fVariableManager.getVariableMapCopy();
    }

    /**
     * Adds variables to the variable map for a namespace
     * @param variables An instance of a Map contain variables to add.
     * The map's key will contain the key for each variable in upper-case
     * and the value for the map will be a Variable instance.
     */ 
    public void addVariables(Map variables)
    {
        Iterator iter = variables.keySet().iterator();

        while (iter.hasNext())
        {
            String key = (String)iter.next();
            setVariable(key, (String)variables.get(key));
        }
    }

    /**
     * Sets the key and value for a variable in the variable map for a
     * namespace.
     * @param key The key for the variable
     * @param value The value for the variable
     */ 
    public void setVariable(String key, String value)
    {
        fVariableManager.set(key, value);
    }
     
    /**
     * Checks if the key specified (case-insensitive) exists
     * in the variable map
     * 
     * @param key The key of the variable
     * @return true if the key is in the variable map or false if the
     * key is not in the variable map
     */ 
    public boolean hasVariable(String key)
    {
        return fVariableManager.hasKey(key);
    }
     
    /**
     * Gets the value for the variable with the specified key
     * @param key The key of the variable whose value you want returned
     * @return The value of the variable
     */ 
    public String getVariable(String key)
    {
        return fVariableManager.getValue(key);
    }

    /**
     * Gets the key for the variable (in the actual case) for the variable
     * with the specified key
     * @param key The key of the variable 
     * @return The key for the variable in the actual case
     */ 
    public String getVariableKey(String key)
    {
        return fVariableManager.getKey(key);
    }

    /**
     * Removes a variable with the specified key from the variable map
     * 
     * @param key The key of the variable to remove
     * @return An Object containing the Variable instance that was removed
     */ 
    public Object removeVariable(String key)
    {
        return fVariableManager.delete(key);
    }
}