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
 * Manages a map of variables in a namespace.
 * <p>
 * The key is the upper-case key and the value is a Variable instance
 * that contains the key (in the case specified) and the value of the variable.
 * This allows variable keys to be case-insensitive.
 */ 
public class VariableManager
{
    private Map fVarMap = new TreeMap();

    /**
     * Creates a new VariableManager instance.
     */
    public VariableManager()
    { /* Do Nothing */ }

    /**
     * Adds or modifies a value for a variable in the variable map.
     * 
     * @param key The key of a variable to set
     * @param value The value of the variable to set
     */
    public void set(String key, String value)
    {
        fVarMap.put(key.toUpperCase(), new Variable(key, value));
    }
     
    /**
     * Gets the value for a variable in the variable map.
     * 
     * @param key The key for the variable
     * @return The value for the variable or null if the specified key for 
     * the variable does not exist
     */ 
    public String getValue(String key)
    {
        return ((Variable)fVarMap.get(key.toUpperCase())).getValue();
    }
     
    /**
     * Gets the key (in the actual case it was set) for the variable from
     * the variable map.
     * 
     * @param key The key for the variable
     * @return The key for the variable or null if the specified key for
     * the variable does not exist
     */ 
    public String getKey(String key)
    {
        return ((Variable)fVarMap.get(key.toUpperCase())).getKey();
    }

    /**
     * Deletes a variable from the map of variables.
     *  
     * @param key The key of the variable to remove
     * @return An Object containing the Variable instance that was removed
     */ 
    public Object delete(String key)
    {
        return fVarMap.remove(key.toUpperCase());
    }
     
    /**
     * Checks if the key specified (case-insensitive) exists in the
     * variable map.
     * 
     * @param key The key for the variable
     * @return true if the key is in the variable map or false if the
     * key is not in the variable map
     */ 
    public boolean hasKey(String key)
    {
        return fVarMap.containsKey(key.toUpperCase());
    }

    /**
     * Returns a copy of the variablee map
     * @return an instance of the Map of the variables
     */ 
    public Map getVariableMapCopy()
    {
        return fVarMap;
    }
}