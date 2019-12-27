/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

import java.util.*;

/**
 * This class provides the metadata associated with a map class used to
 * marshall a "well-defined" map. In particular, it defines the keys
 * associated with the map class. This class is used to create and/or access
 * a STAF map class definition which can be useful if you want to generate a
 * STAF marshalling context with map classes. The map class definition is used
 * to reduce the size of a marshalling map class in comparison to a map
 * containing the same data. It also contains information about how to display
 * instances of the map class, such as the order in which to display the keys
 * and the display names to use for the keys. You get and set map class
 * definitions using the <code>setMapClassDefinition</code> and
 * <code>getMapClassDefinition</code> functions in the
 * <code>STAFMarshallingContext</code> class.
 * 
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFMCDef">
 *      Section "3.2.1 Class STAFMapClassDefinition" in the STAF Java User's
 *      Guide</a>
 */
public class STAFMapClassDefinition
{
    /**
     * This constructs a map class definition with the specified name.
     */ 
    public STAFMapClassDefinition(String name)
    {
        fMapClassDef.put("name", name);
        fMapClassDef.put("keys", new LinkedList());
    }

    /**
     * This constructs a copy of the specified map class definition.
     * 
     * @param   mapClassDef  a Map object containing the map class definition.
     */ 
    STAFMapClassDefinition(Map mapClassDef)
    {
        if (mapClassDef == null)
        {
            fMapClassDef.put("name", new String());
            fMapClassDef.put("keys", new LinkedList());
        }
        else
        {
            fMapClassDef = mapClassDef;
        }
    }

    /**
     * This method returns a Map containing one entry with a key name of
     * <code>staf-map-class-name</code> with a value set to the name of the
     * map class definition.
     * 
     * @return  Returns a Map containing one entry with a key name of
     *          <code>staf-map-class-name</code> with a value set to the name
     *          of the map class definition.
     */ 
    public Map createInstance()
    {
        Map mapInstance = new TreeMap();

        mapInstance.put("staf-map-class-name", fMapClassDef.get("name"));

        return mapInstance;
    }

    /**
     * This method adds a key to the map class definition.
     * 
     * @param keyName  A String specifying the name of the key.
     */ 
    public void addKey(String keyName)
    {
        Map aKey = new TreeMap();

        aKey.put("key", keyName);

        List keyList = (List)fMapClassDef.get("keys");

        keyList.add(aKey);
    }

    /**
     * This method adds a key to the map class definition and specifies the
     * display name for the key.
     * 
     * @param keyName      A String that specifies the name of the key.
     * @param displayName  A String that specifies the display name for the key.
     *                     The default is null which indicates to use the
     *                     actual key name when displaying the key. 
     */ 
    public void addKey(String keyName, String displayName)
    {
        Map aKey = new TreeMap();

        aKey.put("key", keyName);
        aKey.put("display-name", displayName);

        List keyList = (List)fMapClassDef.get("keys");

        keyList.add(aKey);
    }

    /**
     * This method sets a property such as a short display name
     * (<code>display-short-name</code>) for a key in the map class definition. 
     * 
     * @param keyName   A String that specifies the name of a key for which
     *                  this property is being set.
     * @param property  A String that specifies the name of the property being
     *                  set. The only property name currently recognized is
     *                  <code>display-short-name</code> which is used by the
     *                  STAF executable when displaying a result in a tabular
     *                  format when the length of the values for the fields is
     *                  less than the length of the key's
     *                  <code>display-name</code>
     * @param value     A String that specifies the value for the property
     *                  being set.
     */ 
    public void setKeyProperty(String keyName, String property, String value)
    {
        for (Iterator iter = keyIterator(); iter.hasNext();)
        {
            Map thisKey = (Map)iter.next();

            if (thisKey.get("key").equals(keyName))
                thisKey.put(property, value);
        }
    }

    /**
     * This method returns an Iterator object for the list of all the keys
     * defined in the map class definition. Each entry in the list is a map
     * containing a key named <code>key</code>, and optionally, a key named
     * <code>display-name</code>, and optionally, any key property names such
     * as <code>display-short-name</code>. 
     * 
     * @return An Iterator object for the list of all the keys defined in the
     *         map class definition.
     */ 
    public Iterator keyIterator()
    {
        List keyList = (List)fMapClassDef.get("keys");

        return keyList.iterator();
    }

    /**
     * This method returns a String containing the name for the map class
     * definition.
     * 
     * @return  A String containing the name of the map class definition.
     */ 
    public String name() { return (String)fMapClassDef.get("name"); }

    Object getMapClassDefinitionObject() { return fMapClassDef; }

    private Map fMapClassDef = new TreeMap();
}
