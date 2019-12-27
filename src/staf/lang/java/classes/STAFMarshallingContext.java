/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf;

import java.util.*;

/**
 * This class is used to create and/or access a STAF marshalling context which
 * is used by STAF to help in marshalling and unmarshalling data. A
 * marshalling context is simply a container for map class definitions and a
 * data structure that uses (or is defined in terms of) them.
 * <p>
 * In order to use a map class when marshalling data, you must add the map
 * class definition to the marshalling context, set the root object of the
 * marshalling context to the object you want to marshall, and then marshall
 * the marshalling context itself. When you unmarshall a data structure, you
 * will always receive a marshalling context. Any map class definitions
 * referenced by map classes within the data structure will be present in the
 * marshalling context.
 * <p>
 * The primary use of this class is to represent multi-valued results that
 * consist of a data structure (e.g. results from a QUERY/LIST service
 * request, etc.) as a string that can also be converted back into the data
 * structure. This string can be assigned to the string result buffer returned
 * from the service request.
 * 
 * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_STAFMC">
 *      Section "3.2.2 Class STAFMarshallingContext" in the STAF Java User's
 *      Guide</a>
 */
public class STAFMarshallingContext
{
    /**
     * The default flag used when unmarshalling a marshalling context which
     * indicates to also unmarshall indirect objects (that is, any nested
     * marshalled data) in the root object  
     */ 
    public static final int UNMARSHALLING_DEFAULTS = 0;

    /**
     * A flag that specifies to ignore indirect objects (that is, any nested
     * marshalled data) in the root object when unmarshalling a marshalling
     * context
     */ 
    public static final int IGNORE_INDIRECT_OBJECTS = 1;

    /**
     * This method tests if the argument <code>someData</code> is a string-
     * based marshalled representation.
     * 
     * @param  someData a string to be tested if it is a marshalled string
     * @return          Returns a true value if <code>someData<code> is a
     *                  marshalled string and returns a false value if it is
     *                  not a marshalled string.
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_isMarshalled">
     * Section "3.2.2.1 Static Method isMarshalledData" in the STAF Java User's
     * Guide</a>
     */ 
    public static boolean isMarshalledData(String someData)
    {
        return someData.startsWith("@SDT/");
    }

    /**
     * This constructs a marshalling context with a <code>null</code> root
     * object and an empty map class definitions map.
     */ 
    public STAFMarshallingContext()
    { /* Do nothing */ }

    /**
     * This constructs a marshalling context with the specified root object
     * and an empty map class definitions map.
     * 
     * @param  obj   the root object to be marshalled.
     */ 
    public STAFMarshallingContext(Object obj)
    {
        rootObject = obj;
    }

    /**
     * This constructs a marshalling context with the specified root object
     * and map class definitions map.
     * 
     * @param  obj          the root object to be marshalled.
     * @param  mapClassMap  a map containing the map class definitions for
     *                      this marshalling context
     */ 
    STAFMarshallingContext(Object obj, Map mapClassMap)
    {
        rootObject = obj;
        this.mapClassMap = mapClassMap;
    }

    /**
     * This method adds a map class definition to the marshalling context.
     * You may call this method any number of times to set multiple
     * <code>STAFMapClassDefinition</code> objects for the marshalling
     * context.
     * 
     * @param  mapClassDef  a map class definition object that can be used
     *                      when marshalling the object
     */ 
    public void setMapClassDefinition(STAFMapClassDefinition mapClassDef)
    {
        mapClassMap.put(mapClassDef.name(),
                        mapClassDef.getMapClassDefinitionObject());
    }

    /**
     * This method returns the map class definition for the specified map
     * class name.
     * 
     * @param  mapClassName  the name of the <code>STAFMapClassDefinition</code>
     *                       object that you want to return.
     * @return               the map class definition object for the specified
     *                       map class name
     */ 
    public STAFMapClassDefinition getMapClassDefinition(String mapClassName)
    {
        return new STAFMapClassDefinition((Map)mapClassMap.get(mapClassName));
    }

    /**
     * This method determine whether the marshalling context contains the
     * specified map class definition. 
     * 
     * @param  mapClassName  the name of the <tt>STAFMapClassDefinition</tt>
     *                       object that you want to check if it exists in the
     *                       marshalling context's list of map class
     *                       definitions.
     * @return               a <code>true</code> value if the marshalling
     *                       context contains the specified map class
     *                       definition or a <code>false</code> value if it
     *                       doesn't
     */ 
    public boolean hasMapClassDefinition(String mapClassName)
    {
        return mapClassMap.containsKey(mapClassName);
    }

    /**
     * This method returns the map class definitions map for the marshalling
     * context.
     * 
     * @return a map of the map class definitions for the marshalling context
     */ 
    Map getMapClassMap()
    {
        return Collections.unmodifiableMap(mapClassMap);
    }

    /**
     * This method returns an <code>Iterator</code> object for the names of
     * the map class definitions contained in the marshalling context.
     * Note that the name of a map class definition is a <code>String</code>.
     * 
     * @return  an <code>Iterator</code> object for the names of the map
     *          class definitions contained in the marshalling context
     */ 
    public Iterator mapClassDefinitionIterator()
    {
        return mapClassMap.keySet().iterator();
    }

    /**
     * This method sets the root object for the marshalling context.
     * 
     * @param  rootObject  the root object (can be any <code>Object</code>)
     */ 
    public void setRootObject(Object rootObject)
    {
        this.rootObject = rootObject;
    }

    /**
     * This method returns the root object for the marshalling context.
     * 
     * @return  the root object for the marshalling context
     */ 
    public Object getRootObject()
    {
        return rootObject;
    }

    /**
     * This method returns the primary object for the marshalling context.
     * 
     * @return  If the marshalling context contains one or more map class
     *          definitions, it returns the marshalling context object itself.
     *          Otherwise, it returns the root object for the marshalling
     *          context.
     */ 
    public Object getPrimaryObject()
    {
        if (mapClassMap.size() == 0) return rootObject;

        return this;
    }

    /**
     * This method creates a marshalled data string for the marshalling
     * context.
     * 
     * @return  a string containing the marshalled data
     */ 
    public String marshall()
    {
        return marshall(this, this);
    }

    /**
     * This method creates a string-based marshalled representation of the
     * object specified by argument <code>object</code.
     * 
     * @param  object   can be any <code>Object</code (e.g. a List, Map,
     *                  <code>STAFMarshallingContext</code>, String)
     * @param  context  the <code>STAFMarshallingContext</code> object that
     *                  should be used when creating the marshalled string.
     *                  You can specify <code>null</code> if you don't need to
     *                  specify a marshalling context.
     * @return          a string containing the marshalled data
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_marshall">
     * Section "3.2.2.2 Static Method marshall" in the STAF Java User's Guide</a>
     */ 
    public static String marshall(Object object, STAFMarshallingContext context)
    {
        if (object == null)
        {
            return NONE_MARKER;
        }
        if (object instanceof List)
        {
            List list = (List)object;
            Iterator iter = list.iterator();
            StringBuffer listData = new StringBuffer();

            while (iter.hasNext())
                listData.append(marshall(iter.next(), context));

            return LIST_MARKER + list.size() + ":" + listData.length() + ":" +
                   listData.toString();
        }
        else if (object instanceof Map)
        {
            Map map = (Map)object;

            // If a staf-map-class-name key exists in the map, make sure that
            // it's map class definition is provided in the marshalling context.
            // If it's not, then treat the map as a plain map object.

            boolean isMapClass = false;
            String mapClassName = "";

            if ((context != null) &&
                (context instanceof STAFMarshallingContext) &&
                (map.containsKey(MAP_CLASS_NAME_KEY)))
            {
                mapClassName = (String)map.get(MAP_CLASS_NAME_KEY);

                if (context.hasMapClassDefinition(mapClassName))
                {
                    isMapClass = true;
                }
            }

            if (isMapClass)
            {
                STAFMapClassDefinition mapClass =
                    context.getMapClassDefinition(mapClassName);
                Iterator iter = mapClass.keyIterator();
                StringBuffer result = new StringBuffer(
                    ":" + mapClassName.length() + ":" + mapClassName);

                while (iter.hasNext())
                {
                    Map key = (Map)iter.next();
                    result.append(marshall(map.get(key.get("key")), context));
                }

                return MC_INSTANCE_MARKER + ":" + result.length() + ":" +
                       result.toString();
            }
            else
            {
                Iterator iter = map.keySet().iterator();
                StringBuffer mapData = new StringBuffer();

                while (iter.hasNext())
                {
                    Object key = iter.next();
                    mapData.append(":" + key.toString().length() + ":" +
                        key.toString() + marshall(map.get(key), context));
                }

                return MAP_MARKER + ":" + mapData.length() + ":" +
                       mapData.toString();
            }
        }
        else if (object instanceof STAFMarshallingContext)
        {
            STAFMarshallingContext mc = (STAFMarshallingContext)object;
            Map classMap = (Map)mc.getMapClassMap();

            if (classMap.size() == 0)
            {
                return marshall(mc.getRootObject(), context);
            }
            else
            {
                Map contextMap = new HashMap();

                contextMap.put(MAP_CLASS_MAP_KEY, classMap);

                // Note: We can't simply put the root object as a map key like
                //       "root-object" and then marshall the whole map, as in
                //       the unmarshalling routines, we need to be able to
                //       unmarshall the root object in the context of the
                //       map-class-map.

                String data = marshall(contextMap, context) +
                              marshall(mc.getRootObject(),
                                       (STAFMarshallingContext)object);

                return CONTEXT_MARKER + ":" + data.length() + ":" + data;
            }
        }
        // else if (object has method "stafMarshall")

        String objString = object.toString();

        return "@SDT/$S:" + objString.length() + ":" + objString;
    }

    /**
     * This method converts a string-based marshalled representation specified
     * by argument <code>marshalledObject</code> back into a data structure
     * (recursively unmarshalling any nested marshalled data if present) and
     * returns a <code>STAFMarshallingContext</code> containing this data
     * structure as its root object.
     * 
     * @param  marshalledObject  a string to be unmarshalled
     * @return                   a marshalling context (from which you can get
     *                           the data structure via the STAFMarshallingContext
     *                           class's <code>getRootObject</code> function)
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_unmarshall">
     * Section "3.2.2.3 Static Method unmarshall" in the STAF Java User's Guide</a>
     */ 
    public static STAFMarshallingContext unmarshall(String marshalledObject)
    {
        return unmarshall(marshalledObject, new STAFMarshallingContext(),
                          UNMARSHALLING_DEFAULTS);
    }

    /**
     * This method converts a string-based marshalled representation specified
     * by argument <code>marshalledObject</code> back into a data structure
     * (using the <code>flags</code> argument to control how to unmarshall the
     * string) and returns a <code>STAFMarshallingContext</code> containing
     * this data structure as its root object.
     * 
     * @param  marshalledObject  a string to be unmarshalled
     * @param  flags             the flags used to control how to unmarshall
     *                           the string.  When a string is unmarshalled
     *                           into a data structure, it is possible that
     *                           one of the string objects that is unmarshalled
     *                           is itself the string form of another
     *                           marshalled data structure. Use
     *                           <code>STAFMarshallingContext.UNMARSHALLING_DEFAULTS</code>
     *                           to recursively unmarshall these nested objects.
     *                           Use <code>STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS</code>
     *                           to disable this additional processing.
     * @return                   a marshalling context (from which you can get
     *                           the data structure via the STAFMarshallingContext
     *                           class's <code>getRootObject</code> function)
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_unmarshall">
     * Section "3.2.2.3 Static Method unmarshall" in the STAF Java User's Guide</a>
     */ 
    public static STAFMarshallingContext unmarshall(String marshalledObject,
                                                    int flags)
    {
        return unmarshall(marshalledObject, new STAFMarshallingContext(), flags);
    }

    /**
     * This method converts a string-based marshalled representation specified
     * by argument <code>data</code> back into a data structure (recursively
     * unmarshalling any nested marshalled data if present) and returns a
     * <code>STAFMarshallingContext</code> containing this data structure as
     * its root object.
     * 
     * @param  data     a string to be unmarshalled   
     * @param  context  the <code>STAFMarshallingContext</code> object that
     *                  should be used when unmarshalling the string.  You can
     *                  specify <code>null</code> for this argument.
     * @return          a marshalling context (from which you can get the data
     *                  structure via the STAFMarshallingContext class's
     *                  <code>getRootObject</code> function)
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_unmarshall">
     * Section "3.2.2.3 Static Method unmarshall" in the STAF Java User's Guide</a>
     */ 
    public static STAFMarshallingContext unmarshall(
        String data, STAFMarshallingContext context)
    {
        return unmarshall(data, context, UNMARSHALLING_DEFAULTS);
    }

    /**
     * This method converts a string-based marshalled representation specified
     * by argument <code>data</code> back into a data structure (using the
     * <code>flags</code> argument to control how to unmarshall the string)
     * and returns a <code>STAFMarshallingContext</code> containing this data
     * structure as its root object.
     * 
     * @param  data     a string to be unmarshalled   
     * @param  context  the <code>STAFMarshallingContext</code> object that
     *                  should be used when unmarshalling the string.  You can
     *                  specify <code>null</code> for this argument.
     * @param  flags    the flags used to control how to unmarshall the string.
     *                  When a string is unmarshalled into a data structure,
     *                  it is possible that one of the string objects that is
     *                  unmarshalled is itself the string form of another
     *                  marshalled data structure. Use
     *                  <code>STAFMarshallingContext.UNMARSHALLING_DEFAULTS</code>
     *                  to recursively unmarshall these nested objects.  Use
     *                  <code>STAFMarshallingContext.IGNORE_INDIRECT_OBJECTS</code>
     *                  to disable this additional processing.
     * @return          a marshalling context (from which you can get the data
     *                  structure via the STAFMarshallingContext class's
     *                  <code>getRootObject</code> function)
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_unmarshall">
     * Section "3.2.2.3 Static Method unmarshall" in the STAF Java User's Guide</a>
     */ 
    public static STAFMarshallingContext unmarshall(
        String data, STAFMarshallingContext context, int flags)
    {
        try
        {
            if (data.startsWith(NONE_MARKER))
            {
                return new STAFMarshallingContext();
            }
            else if (data.startsWith(SCALAR_MARKER))
            {
                // @SDT/$S:<string-length>:<String>

                int colonIndex = data.indexOf(':', SCALAR_MARKER.length());
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int dataIndex = colonIndex + 1;

                colonIndex = data.indexOf(':', dataIndex);

                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int stringLength = Integer.parseInt(
                    data.substring(dataIndex, colonIndex));

                dataIndex = colonIndex + 1;

                if (stringLength != (data.length() - dataIndex))
                    return new STAFMarshallingContext(data);

                String theString = data.substring(dataIndex);

                if (theString.startsWith(MARSHALLED_DATA_MARKER) &&
                    ((flags & IGNORE_INDIRECT_OBJECTS) !=
                     IGNORE_INDIRECT_OBJECTS))
                {
                    return unmarshall(theString, context, flags);
                }
                else
                {
                    return new STAFMarshallingContext(theString);
                }
            }
            else if (data.startsWith(LIST_MARKER))
            {
                // @SDT/[<number-of-items>:<array-length>:<SDT-Any-1>...
                //                                        <SDT-Any-n>

                // Get number-of-items in the list

                int colonIndex = data.indexOf(':', LIST_MARKER.length());
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int numItems = Integer.parseInt(
                    data.substring(LIST_MARKER.length(), colonIndex));

                // Get array-length

                int dataIndex = colonIndex + 1;

                colonIndex = data.indexOf(':', dataIndex);
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int arrayLength = Integer.parseInt(
                    data.substring(dataIndex, colonIndex));

                dataIndex = colonIndex + 1;

                if (arrayLength != (data.length() - dataIndex))
                    return new STAFMarshallingContext(data);

                // Create a list of the data

                List list = new LinkedList();

                for (int i = 0; i < numItems; ++i)
                {
                    // Get the next item in the list and unmarshall it and add
                    // it to the list

                    int colonIndex1 = data.indexOf(':', dataIndex);

                    if (colonIndex1 == -1)
                        return new STAFMarshallingContext(data);
                    
                    int colonIndex2 = data.indexOf(':', colonIndex1 + 1);

                    if (colonIndex2 == -1)
                        return new STAFMarshallingContext(data);

                    int itemLength = Integer.parseInt(
                        data.substring(colonIndex1 + 1, colonIndex2));

                    list.add(unmarshall(
                        data.substring(
                            dataIndex, colonIndex2 + itemLength + 1),
                        context, flags).getPrimaryObject());
                    
                    dataIndex = colonIndex2 + itemLength + 1;
                }

                return new STAFMarshallingContext(list);
            }
            else if (data.startsWith(MAP_MARKER))
            {
                // @SDT/{:<map-length>::<key-1-length>:<key-1><SDT-Any>
                //                     ...
                //                     :<key-n-length>:<key-1><SDT-Any>

                // Get map-length

                int colonIndex = data.indexOf(':', MAP_MARKER.length());
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int dataIndex = colonIndex + 1;

                colonIndex = data.indexOf(':', dataIndex);
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int mapLength = Integer.parseInt(
                    data.substring(dataIndex, colonIndex));

                dataIndex = colonIndex + 1;

                if (mapLength != (data.length() - dataIndex))
                {
                    return new STAFMarshallingContext(data);
                }

                // Create the map of data

                Map map = new HashMap();

                while (dataIndex < data.length())
                {
                    // Get key first

                    int keyColonIndex1 = data.indexOf(':', dataIndex);
                    
                    if (keyColonIndex1 == -1)
                        return new STAFMarshallingContext(data);

                    int keyColonIndex2 = data.indexOf(':', keyColonIndex1 + 1);
                    
                    if (keyColonIndex2 == -1)
                        return new STAFMarshallingContext(data);

                    int keyLength = Integer.parseInt(
                        data.substring(keyColonIndex1 + 1, keyColonIndex2));

                    String key = data.substring(keyColonIndex2 + 1,
                                                keyColonIndex2 + 1 + keyLength);

                    dataIndex = keyColonIndex2 + 1 + keyLength;

                    // Now, get the object

                    int colonIndex1 = data.indexOf(':', dataIndex);

                    if (colonIndex1 == -1)
                        return new STAFMarshallingContext(data);

                    int colonIndex2 = data.indexOf(':', colonIndex1 + 1);

                    if (colonIndex2 == -1)
                        return new STAFMarshallingContext(data);

                    int itemLength = Integer.parseInt(
                        data.substring(colonIndex1 + 1, colonIndex2));

                    map.put(key, unmarshall(
                        data.substring(
                            dataIndex, colonIndex2 + itemLength + 1),
                        context, flags).getPrimaryObject());

                    dataIndex = colonIndex2 + itemLength + 1;
                }

                return new STAFMarshallingContext(map);
            }
            else if (data.startsWith(MC_INSTANCE_MARKER))
            {
                // @SDT/%:<map-class-instance-length>::<map-class-name-length>:
                //      <map-class-name><SDT-Any-value-1>...<SDT-Any-value-n>

                // Get the map-class-instance-length

                int colonIndex = data.indexOf(
                    ':', MC_INSTANCE_MARKER.length());
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int dataIndex = colonIndex + 1;

                colonIndex = data.indexOf(':', dataIndex);
                
                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int mapClassInstanceLength = Integer.parseInt(
                    data.substring(dataIndex, colonIndex));

                dataIndex = colonIndex + 1;

                if (mapClassInstanceLength != (data.length() - dataIndex))
                    return new STAFMarshallingContext(data);

                // Get map-class-name-length

                colonIndex = data.indexOf(':', dataIndex);

                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                dataIndex = colonIndex + 1;

                colonIndex = data.indexOf(':', dataIndex);

                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int mapClassNameLength = Integer.parseInt(
                    data.substring(dataIndex, colonIndex));

                // Get map-class-name

                dataIndex = colonIndex + 1;

                String mapClassName = data.substring(
                    dataIndex, dataIndex + mapClassNameLength);

                dataIndex = dataIndex + mapClassNameLength;

                // Create a map and add the the staf-map-class-name key and
                // value to the map

                Map map = new HashMap();

                map.put(MAP_CLASS_NAME_KEY, mapClassName);

                // Unmarshall all of the actual keys and add to the map

                STAFMapClassDefinition mapClass =
                    context.getMapClassDefinition(mapClassName);
                Iterator iter = mapClass.keyIterator();

                while (dataIndex < data.length())
                {
                    colonIndex = data.indexOf(':', dataIndex);
                    
                    if (colonIndex == -1)
                        return new STAFMarshallingContext(data);

                    int colonIndex2 = data.indexOf(':', colonIndex + 1);

                    if (colonIndex2 == -1)
                        return new STAFMarshallingContext(data);

                    int itemLength = Integer.parseInt(
                        data.substring(colonIndex + 1, colonIndex2));

                    map.put(((Map)iter.next()).get("key"), 
                            unmarshall(data.substring(
                                dataIndex, colonIndex2 + itemLength + 1),
                                       context, flags).getPrimaryObject());

                    dataIndex = colonIndex2 + itemLength + 1;
                }

                return new STAFMarshallingContext(map);
            }
            else if (data.startsWith(CONTEXT_MARKER))
            {
                // @SDT/*:<context-length>:
                //       @SDT/{:<mapClassLength>:<mapClassData><rootObject>

                // Get context-length

                int colonIndex = data.indexOf(':', CONTEXT_MARKER.length());

                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int contextIndex = data.indexOf(':', colonIndex + 1);

                if (contextIndex == -1)
                    return new STAFMarshallingContext(data);

                int contextLength = Integer.parseInt(
                    data.substring(colonIndex + 1, contextIndex));

                contextIndex = contextIndex + 1;

                if (contextLength != (data.length() - contextIndex))
                    return new STAFMarshallingContext(data);

                // Get mapClassLength

                colonIndex = data.indexOf(':', contextIndex);

                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int mapIndex = contextIndex;
                int mapDataIndex = data.indexOf(':', colonIndex + 1);
                
                if (mapDataIndex == -1)
                    return new STAFMarshallingContext(data);

                int mapLength = Integer.parseInt(
                    data.substring(colonIndex + 1, mapDataIndex));

                mapDataIndex++;

                if (mapLength > (data.length() - mapDataIndex))
                    return new STAFMarshallingContext(data);

                // Create a new marshalling context with the map classes and
                // root object

                Map contextMap = (Map)unmarshall(
                    data.substring(mapIndex, mapDataIndex + mapLength),
                    context, flags).getPrimaryObject();

                Map mapClassMap = (Map)contextMap.get(MAP_CLASS_MAP_KEY);

                STAFMarshallingContext newContext = new STAFMarshallingContext(
                    null, mapClassMap);

                colonIndex = data.indexOf(':', mapDataIndex + mapLength);

                if (colonIndex == -1)
                    return new STAFMarshallingContext(data);

                int rootObjIndex = mapDataIndex + mapLength;
                int rootObjDataIndex = data.indexOf(':', colonIndex + 1);

                if (rootObjDataIndex == -1)
                    return new STAFMarshallingContext(data);

                int rootObjLength = Integer.parseInt(data.substring(
                    colonIndex + 1, rootObjDataIndex));

                rootObjDataIndex++;

                if (rootObjLength > (data.length() - rootObjDataIndex))
                    return new STAFMarshallingContext(data);

                newContext.setRootObject(
                    unmarshall(data.substring(
                        rootObjIndex, rootObjDataIndex + rootObjLength),
                               newContext, flags).getPrimaryObject());

                return newContext;
            }
            else if (data.startsWith(MARSHALLED_DATA_MARKER))
            {
                // Here, we don't know what the type is

                return new STAFMarshallingContext(data);
            }
            else
            {
                return new STAFMarshallingContext(data);
            }
        }
        catch (Exception e)
        {
            // An exception occurred processing the marshalling data.
            // This means its probably invalid marshalled data, so just fall
            // through to return a marshalling context of the string
            // containing the invalid marshalled data.
        }

        return new STAFMarshallingContext(data);
    }

    private static String quoteString(String input)
    {
        if (input.indexOf("'") == -1)
            return "'" + input + "'";

        if (input.indexOf("\"") == -1)
            return "\"" + input + "\"";

        StringTokenizer tokens = new StringTokenizer(input, "'");

        String output = "'" + tokens.nextToken();

        while (tokens.hasMoreTokens())
            output = output + "\'" + tokens.nextToken();

        return output + "'";
    }

    /**
     * This method converts the marshalling context into a verbose formatted
     * hierarchical string that can be used when you want a "pretty print"
     * representation of a <code>STAFMarshallingContext</code> object. 
     * 
     * @return              a string containing a "pretty print" representation
     *                      of the marshalling context
     */ 
    public String toString()
    {
        return formatObject(rootObject, this, 0, 0);
    }

    /**
     * This method converts the marshalling context into a verbose formatted
     * hierarchical string that can be used when you want a "pretty print"
     * representation of a <code>STAFMarshallingContext</code> object. 
     * 
     * @param  flags        reserved for future use (just specify 0)
     * @return              a string containing a "pretty print" representation
     *                      of the marshalling context
     */ 
    public String toString(int flags)
    {
        return formatObject(rootObject, this, 0, flags);
    }

    /**
     * This method converts a data structure into a verbose formatted
     * hierarchical string that can be used when you want a "pretty print"
     * representation of an object.
     * 
     * @param  obj    any <code>Object</code> to be formatted in a verbose,
     *                more readable format.
     * @return        a string containing a "pretty print" representation
     *                of an object
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_formatObj">
     * Section "3.2.2.4 Static Method formatObject" in the STAF Java User's
     * Guide</a>
     */ 
    public static String formatObject(Object obj)
    {
        return formatObject(obj, null, 0, 0);
    }

    /**
     * This method converts a data structure into a verbose formatted
     * hierarchical string that can be used when you want a "pretty print"
     * representation of an object.
     * 
     * @param  obj    any <code>Object</code> to be formatted in a verbose,
     *                more readable format.
     * @param  flags  reserved for future use (just specify 0)
     * @return        a string containing a "pretty print" representation
     *                of an object
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_formatObj">
     * Section "3.2.2.4 Static Method formatObject" in the STAF Java User's
     * Guide</a>
     */ 
    public static String formatObject(Object obj, int flags)
    {
        return formatObject(obj, null, 0, flags);
    }

    /**
     * This method converts a data structure into a verbose formatted
     * hierarchical string that can be used when you want a "pretty print"
     * representation of an object.  It allows you to specify the
     * <code>indentLevel</code> argument which lets you indent the formatted
     * output further to the right.
     * 
     * @param  obj          any <code>Object</code> to be formatted in a
     *                      verbose, more readable format.
     * @param  context
     * @param  indentLevel  the indentation level (in case you want the
     *                      formatted output indented further to the right).
     *                      Specify 0 for the default indentation level.
     * @param  flags        reserved for future use (just specify 0)
     * @return              a string containing a "pretty print" representation
     *                      of an object
     * @see <a href="http://staf.sourceforge.net/current/STAFJava.htm#Header_formatObj">
     * Section "3.2.2.4 Static Method formatObject" in the STAF Java User's
     * Guide</a>
     */ 
    static String formatObject(Object obj, STAFMarshallingContext context,
                               int indentLevel, int flags)
    {
        String lineSep = System.getProperty("line.separator");
        StringBuffer output = new StringBuffer();

        if (obj instanceof List)
        {
            List list = (List)obj;

            output.append("[");

            ++indentLevel;

            if (list.size() > 0) output.append(lineSep);

            // Format each object

            for (Iterator iter = list.iterator(); iter.hasNext();)
            {
                Object thisObj = iter.next();

                if ((thisObj instanceof List) ||
                    (thisObj instanceof Map) ||
                    (thisObj instanceof STAFMarshallingContext))
                {
                    output.append(
                        SPACES.substring(0, indentLevel * INDENT_DELTA));

                    output.append(formatObject(thisObj, context, indentLevel,
                                               flags));
                }
                else
                {
                    output.append(
                        SPACES.substring(0, indentLevel * INDENT_DELTA));

                    if (thisObj == null)
                        output.append(NONE_STRING);
                    else
                        output.append(thisObj.toString());
                }

                if (iter.hasNext()) output.append(ENTRY_SEPARATOR);

                output.append(lineSep);
            }

            --indentLevel;

            if (list.size() > 0)
                output.append(SPACES.substring(0, indentLevel * INDENT_DELTA));

            output.append("]");
        }
        else if (obj instanceof Map)
        {
            Map map = (Map)obj;

            output.append("{");

            ++indentLevel;

            if (map.size() > 0) output.append(lineSep);

            // Check if the map object has a map class key and if the context
            // is valid and contains a map class definition for this map class.
            // If not, treat as a plain map class.

            if (map.containsKey(MAP_CLASS_NAME_KEY) &&
                (context != null) &&
                (context instanceof STAFMarshallingContext) &&
                (context.hasMapClassDefinition(
                    (String)map.get(MAP_CLASS_NAME_KEY))))
            {
                STAFMapClassDefinition mapClass =
                    context.getMapClassDefinition(
                        (String)map.get(MAP_CLASS_NAME_KEY));

                // Determine maximum key length

                int maxKeyLength = 0;

                for (Iterator iter = mapClass.keyIterator(); iter.hasNext();)
                {
                    Map theKey = (Map)iter.next();
                    String theKeyString = (String)theKey.get("key");

                    if (theKey.containsKey(DISPLAY_NAME_KEY))
                        theKeyString = (String)theKey.get(DISPLAY_NAME_KEY);

                    if (theKeyString.length() > maxKeyLength)
                        maxKeyLength = theKeyString.length();
                }

                // Now print each object in the map

                for (Iterator iter = mapClass.keyIterator(); iter.hasNext();)
                {
                    Map theKey = (Map)iter.next();
                    String theKeyString = (String)theKey.get("key");

                    if (theKey.containsKey(DISPLAY_NAME_KEY))
                        theKeyString = (String)theKey.get(DISPLAY_NAME_KEY);

                    output.append(SPACES.substring(0,
                                                   indentLevel * INDENT_DELTA))
                          .append(theKeyString)
                          .append(SPACES.substring(0, maxKeyLength -
                                                   theKeyString.length()))
                          .append(": ");

                    Object thisObj = map.get(theKey.get("key"));

                    if ((thisObj instanceof List) ||
                        (thisObj instanceof Map) ||
                        (thisObj instanceof STAFMarshallingContext))
                    {
                        output.append(
                            formatObject(thisObj, context, indentLevel, flags));
                    }
                    else if (thisObj == null)
                    {
                        output.append(NONE_STRING);
                    }
                    else
                    {
                        output.append(thisObj.toString());
                    }

                    if (iter.hasNext()) output.append(ENTRY_SEPARATOR);

                    output.append(lineSep);
                }
            }
            else
            {
                // Determine maximum key length

                int maxKeyLength = 0;

                for (Iterator iter = map.keySet().iterator(); iter.hasNext();)
                {
                    String theKeyString = (String)iter.next();

                    if (theKeyString.length() > maxKeyLength)
                        maxKeyLength = theKeyString.length();
                }

                // Now print each object in the map

                for (Iterator iter = map.keySet().iterator(); iter.hasNext();)
                {
                    String theKeyString = (String)iter.next();

                    output.append(SPACES.substring(0,
                                                   indentLevel * INDENT_DELTA))
                          .append(theKeyString)
                          .append(SPACES.substring(0, maxKeyLength -
                                                   theKeyString.length()))
                          .append(": ");

                    Object thisObj = map.get(theKeyString);

                    if ((thisObj instanceof List) ||
                        (thisObj instanceof Map) ||
                        (thisObj instanceof STAFMarshallingContext))
                    {
                        output.append(
                            formatObject(thisObj, context, indentLevel, flags));
                    }
                    else if (thisObj == null)
                    {
                        output.append(NONE_STRING);
                    }
                    else
                    {
                        output.append(thisObj.toString());
                    }

                    if (iter.hasNext()) output.append(ENTRY_SEPARATOR);

                    output.append(lineSep);
                }
            }

            --indentLevel;

            if (map.size() > 0)
                output.append(SPACES.substring(0, indentLevel * INDENT_DELTA));

            output.append("}");
        }
        else if (obj instanceof STAFMarshallingContext)
        {
            STAFMarshallingContext inputContext = (STAFMarshallingContext)obj;

            return formatObject(inputContext.getRootObject(), inputContext,
                                indentLevel, flags);
        }
        else if (obj == null) return NONE_STRING;
        else return obj.toString();

        return output.toString();
    }

    // Class data

    private static final String MARSHALLED_DATA_MARKER = new String("@SDT/");
    private static final String NONE_MARKER = new String("@SDT/$0:0:");
    private static final String SCALAR_MARKER = new String("@SDT/$");
    private static final String LIST_MARKER = new String("@SDT/[");
    private static final String MAP_MARKER = new String("@SDT/{");
    private static final String MC_INSTANCE_MARKER = new String("@SDT/%");
    private static final String CONTEXT_MARKER = new String("@SDT/*");
    private static final String NONE_STRING = new String("<None>");
    private static final String DISPLAY_NAME_KEY = new String("display-name");
    private static final String MAP_CLASS_MAP_KEY = new String("map-class-map");
    private static final String MAP_CLASS_NAME_KEY =
        new String("staf-map-class-name");
    private static final String ENTRY_SEPARATOR = new String("");
    // 80 spaces
    private static final String SPACES = new String(
        "                                         " + 
        "                                         ");
    private static final int INDENT_DELTA = 2;

    private Map mapClassMap = new HashMap();
    private Object rootObject = null;
}
