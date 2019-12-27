/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2007                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

/**
 * A STAXDocument is a parsed STAX element and contains all of the functions
 * belonging to it. This allows multiple jobs to use the same STAX file without
 * reparsing.
 * 
 * @author David Hawkey
 *
 */
public class STAXDocument
{
    private HashMap<String, STAXFunctionAction> fFunctionMap =
        new HashMap<String, STAXFunctionAction>();

    private String fStartFunction;
    private String fStartFuncArgs;

    private LinkedList<STAXAction> fDefaultActions =
        new LinkedList<STAXAction>();

    /**
     * Creates an empty STAX document with no functions.
     */
    public STAXDocument()
    {
        fStartFunction = new String();
        fStartFuncArgs =  "None";
    }
    
    /**
     * Creates a STAX document.
     * 
     * @param functions a collection of STAXFunctionAction objects that belong
     *  to the document.
     * @param startFunction the name of the start function or null if none
     *  exists.
     * @param startFunctionArgs the start function arguments or null if none
     *  exists.
     * @param defaultActions a list of the default STAXAction items.
     */
    public STAXDocument(Collection<STAXFunctionAction> functions,
                        String startFunction,
                        String startFunctionArgs,
                        List<STAXAction> defaultActions)
    {
        // Add the functions from the collection to the function map
        
        for (STAXFunctionAction function : functions)
        {
            fFunctionMap.put(function.getName(), function);
        }
        
        if (startFunction == null)
        {
            fStartFunction = new String();
        }
        else
        {
            fStartFunction = startFunction;
        }
        
        if (startFunctionArgs == null)
        {
            fStartFuncArgs = "None";
        }
        else
        {
            fStartFuncArgs = startFunctionArgs;
        }
        
        // Add the default actions

        for (STAXAction action : defaultActions)
        {
            fDefaultActions.add(action);
        }
    }

    /**
     * Gets the default actions.
     * 
     * @return the default actions.
     */
    public LinkedList<STAXAction> getDefaultActions()
    {
        synchronized(fDefaultActions)
        {
            return new LinkedList<STAXAction>(fDefaultActions);
        }
    }

    /**
     * Gets the start function.
     * 
     * @return the start function.
     */
    public String getStartFunction()
    {
        return fStartFunction;
    }

    /**
     * Gets the start function arguments.
     * 
     * @return the start function arguments.
     */
    public String getStartFunctionArgs()
    {
        return fStartFuncArgs;
    }

    /**
     * Gets a function by name.
     * 
     * @param name the name of the function.
     * @return the function with the given name or null if no function exists
     *  with that name.
     */
    public STAXAction getFunction(String name)
    {
        synchronized(fFunctionMap)
        {
            return fFunctionMap.get(name);
        }
    }

    /**
     * Gets a map of all the functions in the document.
     * 
     * @return a HashMap of all functions keyed by the function names.
     */
    public HashMap<String, STAXFunctionAction> getFunctionMap()
    {
        synchronized(fFunctionMap)
        {
            return new HashMap<String, STAXFunctionAction>(fFunctionMap);
        }
    }

    /**
     * Gets a sorted map of all the functions in the document,
     * 
     * @return a TreeMap of all functions keyed by the function names.
     */
    public TreeMap<String, STAXFunctionAction> getSortedFunctionMap()
    {
        synchronized(fFunctionMap)
        {
            return new TreeMap<String, STAXFunctionAction>(fFunctionMap);
        }
    }
    
    /**
     * Gets a collection of all functions in the document.
     * 
     * @return a collection of STAXFunctionAction items.
     */
    public Collection<STAXFunctionAction> getFunctions()
    {
        synchronized(fFunctionMap)
        {
            return new LinkedList<STAXFunctionAction>(fFunctionMap.values());
        }
    }

    /**
     * Checks if a function exists.
     * 
     * @param name the name of the function.
     * @return true if the function exists and false if it doesn't.
     */
    public boolean functionExists(String name)
    {
        synchronized(fFunctionMap)
        {
            return fFunctionMap.containsKey(name);
        }
    }

    /**
     * Adds a function to the document.
     * 
     * @param function the function action to add.
     */
    protected void addFunction(STAXFunctionAction function)
    {
        synchronized (fFunctionMap)
        {
            fFunctionMap.put(function.getName(), function);
        }
    }

    /**
     * Adds a default action.
     * 
     * @param action the default action to add.
     */
    protected void addDefaultAction(STAXAction action)
    {
        synchronized(fDefaultActions)
        {
            fDefaultActions.addFirst(action);
        }
    }

    /**
     * Sets the start function for the document.
     * 
     * @param startFunction the start function.
     */
    protected void setStartFunction(String startFunction)
    {
        fStartFunction = startFunction;
    }

    /**
     * Sets the start function arguments.
     * 
     * @param startFuncArgs the start function arguments.
     */
    protected void setStartFuncArgs(String startFuncArgs)
    {
        this.fStartFuncArgs = startFuncArgs;
    }

    /**
     * Creates a clone of this document.
     * 
     * @return a cloned copy of the document.
     */
    public synchronized STAXDocument cloneDocument()
    {
        STAXDocument clone = new STAXDocument();
        
        clone.fStartFunction = this.fStartFunction;
        clone.fStartFuncArgs = this.fStartFuncArgs;
        
        // Clone the default actions

        synchronized (fDefaultActions)
        {
            for (STAXAction action : fDefaultActions)
            {
                clone.fDefaultActions.add(action.cloneAction());
            }
        }
        
        // Clone the function map

        synchronized(fFunctionMap)
        {
            for (Map.Entry<String, STAXFunctionAction> entry :
                 fFunctionMap.entrySet())
            {
                clone.fFunctionMap.put(
                    entry.getKey(),
                    (STAXFunctionAction)entry.getValue().cloneAction());
            }
        }
        
        return clone;
    }
    
}
