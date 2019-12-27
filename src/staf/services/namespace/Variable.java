/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2005                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.namespace;

/**
 * Represents a variable in a namespace
 */ 
public class Variable
{
    private String fKey;
    private String fValue;

    /**
     * Creates a new Variable instance
     * @param key The key for the variable
     * @param value The value for the variable
     */ 
    public Variable(String key, String value)
    {
        fKey = key;
        fValue = value;
    }

    /**
     * Gets the key for the variable
     * @return The key for the variable
     */ 
    public String getKey() { return fKey; }

    /**
     * Gets the value for the variable
     * @return The value for the variable
     */ 
    public String getValue() { return fValue; }

    /**
     * Sets the key for a variable
     * @param key The key for a variable
     */ 
    public void setKey(String key) { fKey = key; }

    /**
     * Sets the value for a variable
     * @param value The value for a variable
     */ 
    public void setValue(String value) { fValue = value; }
}