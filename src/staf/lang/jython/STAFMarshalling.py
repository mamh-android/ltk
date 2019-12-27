#############################################################################
# Software Testing Automation Framework (STAF)                              #
# (C) Copyright IBM Corp. 2004, 2005                                        #
#                                                                           #
# This software is licensed under the Eclipse Public License (EPL) V1.0.    #
#############################################################################

# Marshalling constants and imports

import types

UNMARSHALLING_DEFAULTS  = 0
IGNORE_INDIRECT_OBJECTS = 1

MARSHALLED_DATA_MARKER = '@SDT/'
NONE_MARKER            = '@SDT/$0:0:'
SCALAR_MARKER          = '@SDT/$'
SCALAR_STRING_MARKER   = '@SDT/$S'
LIST_MARKER            = '@SDT/['
MAP_MARKER             = '@SDT/{'
MC_INSTANCE_MARKER     = '@SDT/%'
CONTEXT_MARKER         = '@SDT/*'

# Formatting constants and imports

import os

NONE_STRING            = '<None>'
DISPLAY_NAME_KEY       = 'display-name'
MAP_CLASS_MAP_KEY      = 'map-class-map'
MAP_CLASS_NAME_KEY     = 'staf-map-class-name'
ENTRY_SEPARATOR        = ''
INDENT_DELTA           = 2
# 80 spaces
SPACES = ('                                         ' + 
          '                                         ')

# STAFMapClassDefinitionClass

class STAFMapClassDefinition:
    # Constructors
    def __init__(self, name = None, mapClassDef = None):
        if (mapClassDef is None) and (name is None):
            self.mapClassDef = { 'name': '', 'keys': [] }
        elif (name is not None):
            self.mapClassDef = { 'name': name, 'keys': [] }
        else:
            self.mapClassDef = mapClassDef

    def createInstance(self):
        return { 'staf-map-class-name' : self.mapClassDef['name'] }

    def addKey(self, keyName, displayName = None):
        theKey = { 'key': keyName }
        if displayName is not None:
            theKey['display-name'] = displayName
        self.mapClassDef['keys'].append(theKey)

    def setKeyProperty(self, keyName, property, value):
        for key in self.mapClassDef['keys']:
            if key['key'] == keyName:
                key[property] = value

    def keys(self):
        return self.mapClassDef['keys']

    def name(self):
        return self.mapClassDef['name']

    def getMapClassDefinitionObject(self):
        return self.mapClassDef

    def __str__(self):
        return formatObject(self.mapClassDef)

    def __repr__(self):
        return formatObject(self.mapClassDef)

# STAFMarshallingContext class

class STAFMarshallingContext:

    def isMarshalledData(self, someData):
        return someData.startswith('@SDT/')

    def __init__(self, obj = None, mapClassMap = None):
        if mapClassMap is None:
            self.mapClassMap = {}
        else:
            self.mapClassMap = mapClassMap
        self.rootObject = obj

    def setMapClassDefinition(self,  mapClassDef):
        self.mapClassMap[mapClassDef.name()] = mapClassDef.getMapClassDefinitionObject()

    def getMapClassDefinition(self, mapClassName):
        return STAFMapClassDefinition(mapClassDef =
                                      self.mapClassMap.get(mapClassName, None))

    def hasMapClassDefinition(self, mapClassName):
        return self.mapClassMap.has_key(mapClassName)

    def getMapClassMap(self):
        return self.mapClassMap

    def mapClassDefinitionIterator(self):
        return self.mapClassMap.keys()

    def setRootObject(self, rootObject):
        self.rootObject = rootObject

    def getRootObject(self):
        return self.rootObject

    def getPrimaryObject(self):
        if  len(self.mapClassMap.keys()) == 0:
            return self.rootObject
        else:
            return self

    def marshall(self):
        return marshall(self, self)

    def __str__(self):
        return formatObject(self.rootObject, self)

    # XXX: Change to show the key map class in addition?
    def __repr__(self):
        return formatObject(self.rootObject, self)

# Function that tests if a string is marshalled data

def isMarshalledData(someData):
    return someData.startswith('@SDT/')

# General marshalling function

def marshall(object, context = None):
    if object is None:
        return NONE_MARKER

    if type(object) == types.ListType:

        # Build a list of strings and join them for performance reasons

        listDataList = []

        for item in object:
            listDataList.append(marshall(item, context))

        listData = ''.join(listDataList)

        return "%s%s:%s:%s" % (LIST_MARKER, len(object), len(listData), listData)

    if type(object) == types.DictType:

        # If a staf-map-class-name key exists in the map, make sure that
        # it's map class definition is provided in the marshalling context.
        # If it's not, then treat the map as a plain map object.

        isMapClass = 0
        mapClassName = ''

        if ((context is not None) and
            (isinstance(context, STAFMarshallingContext)) and
            (object.has_key('staf-map-class-name'))):

            mapClassName = object['staf-map-class-name']

            if context.hasMapClassDefinition(mapClassName):
                isMapClass = 1

        if isMapClass:

            mapClass = context.getMapClassDefinition(mapClassName)
            
            # Build a list of strings and join them for performance reasons

            mapDataList = []
            mapDataList.append(":%s:%s" % (len(mapClassName), mapClassName))

            for key in mapClass.keys():

                if object.has_key(key['key']):
                    thisObj = object[key['key']]
                else:
                    thisObj = None

                mapDataList.append(marshall(thisObj, context))

            mapData = ''.join(mapDataList)

            return "%s:%s:%s" % (MC_INSTANCE_MARKER, len(mapData), mapData)
 
        else:

            # Build a list of strings and join them for performance reasons
            
            mapDataList = []

            for key in object.keys():
                mapDataList.append(
                    ":%s:%s%s" % (len(str(key)), str(key),
                                  marshall(object[key], context)))
            
            mapData = ''.join(mapDataList)

            return "%s:%s:%s" % (MAP_MARKER, len(mapData), mapData)

    if isinstance(object, STAFMarshallingContext):

        if len(object.mapClassMap.keys()) == 0:
            return marshall(object.getRootObject(), context)
        else:
            contextMap = { 'map-class-map': object.mapClassMap }

            # Note: We can't simply put the root object as a map key like
            #       "root-object" and then marshall the whole map, as in
            #       the unmarshalling routines, we need to be able to
            #       unmarshall the root object in the context of the
            #       map-class-map.

            mcData = marshall(contextMap, context) + \
                     marshall(object.getRootObject(), object)

            return "%s:%s:%s" % (CONTEXT_MARKER, len(mcData), mcData)

    # Check if object (such as a STAXGlobal object) has a callable method
    # named stafMarshall.

    if (hasattr(object, 'stafMarshall') and
        callable(getattr(object, 'stafMarshall'))):

        # Return the result from calling the stafMarshall method
        return "%s" % (object.stafMarshall(context))

    # Otherwise, return a string scalar

    return "%s:%s:%s" % (SCALAR_STRING_MARKER, len(str(object)), str(object))


# General unmarshalling function (catches all exceptions)
#   Unmarshalls the input data string and returns a marshalling context.
#   If an exception occurs, it returns a marshalling context of the
#   input data string

def unmarshall(data, context = None, flags = UNMARSHALLING_DEFAULTS):

    try:

        if context is None:
            context = STAFMarshallingContext()

        if data.startswith(NONE_MARKER):
            return STAFMarshallingContext()

        elif data.startswith(SCALAR_MARKER):

            # @SDT/$S:<string-length>:<String>
            
            colonIndex = data.find(':', len(SCALAR_MARKER))

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            dataIndex = colonIndex + 1

            colonIndex = data.find(':', dataIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)
            
            stringLength = int(data[dataIndex:colonIndex])

            dataIndex = colonIndex + 1

            if stringLength != (len(data) - dataIndex):
                return STAFMarshallingContext(data)

            theString = data[dataIndex:]

            if (theString.startswith(MARSHALLED_DATA_MARKER) and
                ((flags & IGNORE_INDIRECT_OBJECTS) !=
                 IGNORE_INDIRECT_OBJECTS)):
                return unmarshall(theString, context, flags)
            else:
                return STAFMarshallingContext(theString)

        elif data.startswith(LIST_MARKER):

            # @SDT/[<number-of-items>:<array-length>:<SDT-Any-1>...<SDT-Any-n>

            # Get number-of-items in the list

            colonIndex = data.find(':', len(LIST_MARKER))

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            numItems = int(data[len(LIST_MARKER):colonIndex])

            # Get array-length
            
            dataIndex = colonIndex + 1

            colonIndex = data.find(':', dataIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            arrayLength = int(data[dataIndex:colonIndex])
            
            dataIndex = colonIndex + 1

            if arrayLength != (len(data) - dataIndex):
                return STAFMarshallingContext(data)

            # Create a list of the data

            list = []

            for i in range(numItems):

                # Get the next item in the list and unmarshall it and add it
                # to the list

                colonIndex1 = data.find(':', dataIndex)

                if colonIndex1 == -1:
                    return STAFMarshallingContext(data)

                colonIndex2 = data.find(':', colonIndex1 + 1)

                if colonIndex2 == -1:
                    return STAFMarshallingContext(data)

                itemLength = int(data[colonIndex1 + 1:colonIndex2])

                list.append(
                    unmarshall(data[dataIndex:colonIndex2 + itemLength + 1],
                               context, flags).getPrimaryObject())

                dataIndex = colonIndex2 + itemLength + 1

            return STAFMarshallingContext(list)

        elif data.startswith(MAP_MARKER):

            # @SDT/{:<map-length>::<key-1-length>:<key-1><SDT-Any>
            #                     ...
            #                     :<key-n-length>:<key-1><SDT-Any>

            # Get map-length

            colonIndex = data.find(':', len(MAP_MARKER))

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            dataIndex = colonIndex + 1

            colonIndex = data.find(':', dataIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            mapLength = int(data[dataIndex:colonIndex])
            
            dataIndex = colonIndex + 1

            if mapLength != (len(data) - dataIndex):
                return STAFMarshallingContext(data)

            # Create the map (aka dictionary) of data

            map = {}

            while dataIndex < len(data):

                # Get the key first

                keyColonIndex1 = data.find(':', dataIndex)

                if keyColonIndex1 == -1:
                    return STAFMarshallingContext(data)

                keyColonIndex2 = data.find(':', keyColonIndex1 + 1)

                if keyColonIndex2 == -1:
                    return STAFMarshallingContext(data)

                keyLength = int(data[keyColonIndex1 + 1:keyColonIndex2])
                key = data[keyColonIndex2 + 1:keyColonIndex2 + 1 + keyLength]

                dataIndex = keyColonIndex2 + 1 + keyLength

                # Now, get the object

                colonIndex1 = data.find(':', dataIndex)

                if colonIndex1 == -1:
                    return STAFMarshallingContext(data)

                colonIndex2 = data.find(':', colonIndex1 + 1)

                if colonIndex2 == -1:
                    return STAFMarshallingContext(data)

                itemLength = int(data[colonIndex1 + 1:colonIndex2])

                map[key] = unmarshall(
                    data[dataIndex:colonIndex2 + itemLength + 1],
                    context, flags).getPrimaryObject()

                dataIndex = colonIndex2 + itemLength + 1

            return STAFMarshallingContext(map)

        elif data.startswith(MC_INSTANCE_MARKER):
            
            # @SDT/%:<map-class-instance-length>::<map-class-name-length>:
            #     <map-class-name><SDT-Any-value-1>...<SDT-Any-value-n>

            # Get the map-class-instance-length

            colonIndex = data.find(':', len(MC_INSTANCE_MARKER))

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            dataIndex = colonIndex + 1

            colonIndex = data.find(':', dataIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            mapClassInstanceLength = int(data[dataIndex:colonIndex])

            dataIndex = colonIndex + 1

            if mapClassInstanceLength != (len(data) - dataIndex):
                return STAFMarshallingContext(data)

            # Get map-class-name-length

            colonIndex = data.find(':', dataIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            dataIndex = colonIndex + 1

            colonIndex = data.find(':', dataIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            mapClassNameLength = int(data[dataIndex:colonIndex])

            # Get map-class-name

            dataIndex = colonIndex + 1

            mapClassName = data[dataIndex : dataIndex + mapClassNameLength]

            dataIndex = dataIndex + mapClassNameLength

            # Create a map and add the the staf-map-class-name key and value
            # to the map

            map = { 'staf-map-class-name': mapClassName }

            # Unmarshall all of the actual keys and add to the map

            mapClass = context.getMapClassDefinition(mapClassName)
            keys = mapClass.keys()
            keyIndex = 0

            while dataIndex < len(data):
                colonIndex = data.find(':', dataIndex)

                if colonIndex == -1:
                    return STAFMarshallingContext(data)

                colonIndex2 = data.find(':', colonIndex + 1)

                if colonIndex2 == -1:
                    return STAFMarshallingContext(data)

                itemLength = int(data[colonIndex + 1:colonIndex2])

                map[keys[keyIndex]['key']] = unmarshall(
                    data[dataIndex:colonIndex2 + itemLength + 1],
                    context, flags).getPrimaryObject()

                dataIndex = colonIndex2 + itemLength + 1
                keyIndex = keyIndex + 1

            return STAFMarshallingContext(map)

        elif data.startswith(CONTEXT_MARKER):
            
            # @SDT/*:<context-length>:
            #       @SDT/{:<mapClassLength>:<mapClassData><rootObject>

            # Get context-length

            colonIndex = data.find(':', len(CONTEXT_MARKER))

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            contextIndex = data.find(':', colonIndex + 1)

            if contextIndex == -1:
                return STAFMarshallingContext(data)

            contextLength = int(data[colonIndex + 1:contextIndex])

            contextIndex = contextIndex + 1

            if contextLength != (len(data) - contextIndex):
                return STAFMarshallingContext(data)

            # Get mapClassLength

            colonIndex = data.find(':', contextIndex)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            mapIndex = contextIndex
            mapDataIndex = data.find(':', colonIndex + 1)

            if mapDataIndex == -1:
                return STAFMarshallingContext(data)

            mapLength = int(data[colonIndex + 1:mapDataIndex])

            mapDataIndex = mapDataIndex + 1

            if mapLength > (len(data) - mapDataIndex):
                return STAFMarshallingContext(data)

            # Create a new marshalling context with the map classes
            # and root object

            contextMap = unmarshall(data[mapIndex:mapDataIndex + mapLength],
                                    context, flags).getPrimaryObject()
            mapClassMap = contextMap['map-class-map']
            newContext = STAFMarshallingContext(None, mapClassMap)

            colonIndex = data.find(':', mapDataIndex + mapLength)

            if colonIndex == -1:
                return STAFMarshallingContext(data)

            rootObjIndex = mapDataIndex + mapLength
            rootObjDataIndex = data.find(':', colonIndex + 1)

            if rootObjDataIndex == -1:
                return STAFMarshallingContext(data)

            rootObjLength = int(data[colonIndex + 1:rootObjDataIndex])

            rootObjDataIndex = rootObjDataIndex + 1

            if rootObjLength > (len(data) - rootObjDataIndex):
                return STAFMarshallingContext(data)

            newContext.setRootObject(
                unmarshall(data[rootObjIndex:rootObjDataIndex + rootObjLength],
                           newContext, flags).getPrimaryObject())

            return newContext

        elif data.startswith(MARSHALLED_DATA_MARKER):

            # Here, we don't know what the type is
            return STAFMarshallingContext(data)

    except:
        # If any exception occurs unmarshalling the result, assume invalid
        # marshalled data and return a new marshalling context of the input
        # data string
        return STAFMarshallingContext(data)

    return STAFMarshallingContext(data)

# Formatting function
#
# Note: 
#   The flags option is not currently used (it's for future use)

def formatObject(obj, context = None, indentLevel = 0, flags = 0):

    # Build a list of strings to output and join them for performance reasons
    output = []

    if type(obj) == types.ListType:
        list = obj
        output.append('[')
        indentLevel = indentLevel + 1

        if len(list) > 0:
            output.append(os.linesep)

        # Format each object in the list

        i = 0

        for item in list:

            indent = indentLevel * INDENT_DELTA
            output.append(SPACES[: indent])

            if (type(item) == types.ListType or
                type(item) == types.DictType or
                isinstance(item, STAFMarshallingContext)):
                output.append(formatObject(item, context, indentLevel, flags))
            elif item is None:
                output.append(NONE_STRING)
            else:
                output.append(str(item))

            if i < (len(list) - 1):
                output.append(ENTRY_SEPARATOR)

            output.append(os.linesep)

        indentLevel = indentLevel - 1

        if len(list) > 0:
            indent = indentLevel * INDENT_DELTA
            output.append(SPACES[: indent])

        output.append(']')

    elif type(obj) == types.DictType:
        dict = obj
        output.append('{')
        indentLevel = indentLevel + 1

        if len(dict) > 0:
            output.append(os.linesep)
        
        # Check if the map object has a map class key and if the context
        # is valid and contains a map class definition for this map class.
        # If not, treat as a plain map class.

        if (dict.has_key(MAP_CLASS_NAME_KEY) and
            (context is not None) and
            isinstance(context, STAFMarshallingContext) and
            context.hasMapClassDefinition(dict[MAP_CLASS_NAME_KEY])):

            mapClass = context.getMapClassDefinition(dict[MAP_CLASS_NAME_KEY])

            # Determine maximum key length

            maxKeyLength = 0

            for theKey in mapClass.keys():
                theKeyString = theKey['key']

                if theKey.has_key(DISPLAY_NAME_KEY):
                    theKeyString = theKey[DISPLAY_NAME_KEY]
                if len(theKeyString) > maxKeyLength:
                    maxKeyLength = len(theKeyString)

            # Now print each object in the map

            i = 0

            for theKey in mapClass.keys():
                theKeyString = theKey['key']

                if theKey.has_key(DISPLAY_NAME_KEY):
                    theKeyString = theKey[DISPLAY_NAME_KEY]

                indent = indentLevel * INDENT_DELTA
                output.append('%s%s' % (SPACES[: indent], theKeyString))

                indent = maxKeyLength - len(theKeyString)
                output.append('%s: ' % (SPACES[: indent]))

                if dict.has_key(theKey['key']):
                    thisObj = dict[theKey['key']]
                else:
                    thisObj = None

                if (type(thisObj) == types.ListType or
                    type(thisObj) == types.DictType or
                    isinstance(thisObj, STAFMarshallingContext)):
                    output.append(formatObject(thisObj, context, indentLevel, flags))
                elif thisObj is None:
                    output.append(NONE_STRING)
                else:
                    output.append(str(thisObj))

                if i < (len(dict) - 1):
                    output.append(ENTRY_SEPARATOR)
                
                output.append(os.linesep)
                i = i + 1
        
        else:
            # Determine maximum key length

            maxKeyLength = 0

            for theKeyString in dict.keys():
                if len(theKeyString) > maxKeyLength:
                    maxKeyLength = len(theKeyString)

            # Now print each object in the map

            i = 0

            for theKeyString in dict.keys():
                indent = indentLevel * INDENT_DELTA
                output.append('%s%s' % (SPACES[: indent], theKeyString))

                indent = maxKeyLength - len(theKeyString)
                output.append('%s: ' % (SPACES[: indent]))

                thisObj = dict[theKeyString]

                if (type(thisObj) == types.ListType or
                    type(thisObj) == types.DictType or
                    isinstance(thisObj, STAFMarshallingContext)):
                    output.append(formatObject(thisObj, context, indentLevel, flags))
                elif thisObj is None:
                    output.append(NONE_STRING)
                else:
                    output.append(str(thisObj))

                if i < (len(dict) - 1):
                    output.append(ENTRY_SEPARATOR)
                
                output.append(os.linesep)
                i = i + 1
        
        indentLevel = indentLevel - 1

        if len(dict) > 0:
            indent = indentLevel * INDENT_DELTA
            output.append(SPACES[: indent])

        output.append('}')

    elif isinstance(obj, STAFMarshallingContext):
        inputContext = obj
        return formatObject(inputContext.getRootObject(), inputContext,
                            indentLevel, flags)
    elif obj is None:
        return NONE_STRING
    else:
        return str(obj)

    return ''.join(output)

