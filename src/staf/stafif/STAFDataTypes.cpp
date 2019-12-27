/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFUtil.h"
#include "STAFRefPtr.h"
#include "STAFException.h"
#include <deque>
#include <map>

typedef std::deque<STAFObject_t> STAFObjectList;
typedef std::map<STAFString, STAFObject_t> STAFObjectMap;

enum STAFObjectIteratorType_t
{
    kSTAFObjectListIterator     = 0,
    kSTAFObjectMapKeyIterator   = 1,
    kSTAFObjectMapValueIterator = 2
};

struct STAFObjectListIterator
{
    STAFObjectList::iterator iter;
    STAFObjectList *listObject;
};

struct STAFObjectMapIterator
{
    STAFObjectMap::iterator iter;
    STAFObjectMap *mapObject;
};

struct STAFObjectIteratorImpl
{
    STAFObjectIteratorType_t type;

    union
    {
        STAFObjectListIterator *listIterator;
        STAFObjectMapIterator *mapIterator;
    };
};


struct STAFObjectMarshallingContext
{
    STAFObject_t mapClassMap;
    STAFObject_t rootObject;
};


struct STAFObjectImpl
{
    STAFObjectType_t type;
    bool isRef;

    union
    {
        STAFObjectList               *listValue;
        STAFObjectMap                *mapValue;
        STAFObjectMarshallingContext *contextValue;
        STAFString                   *scalarStringValue;
    };
};


static STAFObjectImpl sNoneObj = { kSTAFNoneObject, true };
static STAFObject_t sNoneObjT = &sNoneObj;
static STAFString sColon(kUTF8_COLON);

inline unsigned int incrementNChars(const STAFString &theString,
                                    const unsigned int beginIndex,
                                    const unsigned int incrementLength)
{
    // This is the only viable way to increment characters as we have a length
    // in  "chars", but we really want to be working in bytes.

    unsigned int endOfString = beginIndex;

    for (unsigned int i = 0; i < incrementLength; ++i)
        endOfString += theString.sizeOfChar(endOfString);

    return endOfString;
}


inline unsigned int getCLCLength(const STAFString &theString, unsigned int &index)
{
    unsigned int colonIndex1 = theString.find(sColon, index);

    if (colonIndex1 == STAFString::kNPos)
        throw STAFException("No first colon found");

    unsigned int colonIndex2 = theString.find(sColon, colonIndex1 + 1);

    if (colonIndex2 == STAFString::kNPos)
        throw STAFException("No second colon found");

    index = colonIndex2 + 1;

    return theString.subString(
        colonIndex1 + 1, colonIndex2 - colonIndex1 - 1).asUInt();
}


inline STAFString getCLCString(const STAFString &theString, unsigned int &index)
{
    unsigned int colonIndex1 = theString.find(sColon, index);

    if (colonIndex1 == STAFString::kNPos)
        throw STAFException("No first colon found");

    unsigned int colonIndex2 = theString.find(sColon, colonIndex1 + 1);

    if (colonIndex2 == STAFString::kNPos)
        throw STAFException("No second colon found");

    unsigned int stringLength = theString.subString(
        colonIndex1 + 1, colonIndex2 - colonIndex1 - 1).asUInt();

    index = incrementNChars(theString, colonIndex2 + 1, stringLength);

    return theString.subString(colonIndex2 + 1, index - colonIndex2 - 1);
}


inline STAFObject_t adoptPrimaryObject(STAFObject_t &context)
{
    // Initialize the output object to be the context itself.  If the context
    // doesn't have any map classes, then we set the output object to be the
    // adopted root object and then destruct the context itself.

    STAFObject_t obj = context;

    if (context->contextValue->mapClassMap->mapValue->size() == 0)
    {
        STAFObjectMarshallingContextAdoptRootObject(context, &obj);
        STAFObjectDestruct(&context);
    }

    return obj;
}

inline STAFObject_t unmarshallToPrimaryObject(STAFString &theString,
                                              STAFObject_t context,
                                              unsigned int beginIndex,
                                              unsigned int endIndex,
                                              unsigned int flags)
{
                                   
    STAFObject_t newContext = 0;

    STAFObjectUnmarshallFromString(
        &newContext,
        theString.subString(beginIndex, endIndex - beginIndex).getImpl(),
        context,
        flags);

    return adoptPrimaryObject(newContext);
}

inline STAFObject_t unmarshallObject(STAFString &theString, STAFObject_t context,
                                     unsigned int &index, unsigned int flags)
{
    unsigned int beginIndex = index;
    unsigned int colonIndex1 = theString.find(sColon, beginIndex);
    unsigned int colonIndex2 = theString.find(sColon, colonIndex1 + 1);
    unsigned int objectLength = theString.subString(colonIndex1 + 1,
                                                    colonIndex2 - colonIndex1 -
                                                    1).asUInt();
    index = incrementNChars(theString, colonIndex2 + 1, objectLength);

    return unmarshallToPrimaryObject(theString, context, beginIndex, index,
                                     flags);
}


STAFRC_t STAFObjectConstructReference(STAFObject_t *pObject, STAFObject_t source)
{
    if (pObject == 0) return kSTAFInvalidObject;
    if (source  == 0) return kSTAFInvalidObject;

    *pObject = new STAFObjectImpl;
    STAFObjectImpl &obj = **pObject;

    obj = *source;
    obj.isRef = true;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructNone(STAFObject_t *pObject)
{
    if (pObject == 0) return kSTAFInvalidObject;

    *pObject = new STAFObjectImpl;
    STAFObjectImpl &obj = **pObject;

    obj.type = kSTAFNoneObject;
    obj.isRef = false;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructScalarString(STAFObject_t *pObject,
                                         STAFStringConst_t aString)
{
    if (pObject == 0) return kSTAFInvalidObject;

    *pObject = new STAFObjectImpl;
    STAFObjectImpl &obj = **pObject;

    obj.type = kSTAFScalarStringObject;
    obj.isRef = false;

    // XXX: Used to be this
    // STAFStringConstructCopy(&obj.scalarStringValue, aString, 0);
    obj.scalarStringValue = new STAFString(aString);

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructList(STAFObject_t *pObject)
{
    if (pObject == 0) return kSTAFInvalidObject;

    *pObject = new STAFObjectImpl;
    STAFObjectImpl &obj = **pObject;

    obj.type = kSTAFListObject;
    obj.isRef = false;

    obj.listValue = new STAFObjectList;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructListIterator(STAFObjectIterator_t *pIter,
                                         STAFObject_t list)
{
    if (pIter == 0) return kSTAFInvalidObject;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (list->type != kSTAFListObject) return kSTAFInvalidObject;

    *pIter = new STAFObjectIteratorImpl;
    STAFObjectIteratorImpl &iter = **pIter;

    iter.type = kSTAFObjectListIterator;

    iter.listIterator = new STAFObjectListIterator;
    iter.listIterator->iter = list->listValue->begin();
    iter.listIterator->listObject = list->listValue;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructMap(STAFObject_t *pObject)
{
    if (pObject == 0) return kSTAFInvalidObject;

    *pObject = new STAFObjectImpl;
    STAFObjectImpl &obj = **pObject;

    obj.type = kSTAFMapObject;
    obj.isRef = false;

    obj.mapValue = new STAFObjectMap;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructMapKeyIterator(STAFObjectIterator_t *pIter,
                                           STAFObject_t map)
{
    if (pIter == 0) return kSTAFInvalidObject;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (map->type != kSTAFMapObject) return kSTAFInvalidObject;

    *pIter = new STAFObjectIteratorImpl;
    STAFObjectIteratorImpl &iter = **pIter;

    iter.type = kSTAFObjectMapKeyIterator;

    iter.mapIterator = new STAFObjectMapIterator;
    iter.mapIterator->iter = map->mapValue->begin();
    iter.mapIterator->mapObject = map->mapValue;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructMapValueIterator(STAFObjectIterator_t *pIter,
                                             STAFObject_t map)
{
    if (pIter == 0) return kSTAFInvalidObject;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (map->type != kSTAFMapObject) return kSTAFInvalidObject;

    *pIter = new STAFObjectIteratorImpl;
    STAFObjectIteratorImpl &iter = **pIter;

    iter.type = kSTAFObjectMapValueIterator;

    iter.mapIterator = new STAFObjectMapIterator;
    iter.mapIterator->iter = map->mapValue->begin();
    iter.mapIterator->mapObject = map->mapValue;

    return kSTAFOk;
}


STAFRC_t STAFObjectConstructMapClassDefinitionIterator(
    STAFObjectIterator_t *pIter, STAFObject_t context)
{
    if (pIter == 0) return kSTAFInvalidObject;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    return STAFObjectConstructMapKeyIterator(
               pIter, context->contextValue->mapClassMap);
}


STAFRC_t STAFObjectConstructMarshallingContext(STAFObject_t *pObject)
{
    if (pObject == 0) return kSTAFInvalidObject;

    *pObject = new STAFObjectImpl;
    STAFObjectImpl &obj = **pObject;

    obj.type = kSTAFMarshallingContextObject;
    obj.isRef = false;

    obj.contextValue = new STAFObjectMarshallingContext;
    STAFObjectConstructNone(&obj.contextValue->rootObject);
    STAFObjectConstructMap(&obj.contextValue->mapClassMap);

    return kSTAFOk;
}

STAFRC_t STAFObjectDestruct(STAFObject_t *pObject)
{
    if (pObject  == 0) return kSTAFInvalidObject;
    if (*pObject == 0) return kSTAFInvalidObject;

    STAFObjectImpl &obj = **pObject;

    if (!obj.isRef)
    {
        if (obj.type == kSTAFScalarStringObject)
        {
            delete obj.scalarStringValue;
        }
        else if (obj.type == kSTAFListObject)
        {
            // First destruct everything in the list

            for (STAFObjectList::iterator iter = obj.listValue->begin();
                 iter != obj.listValue->end();
                 ++iter)
            {
                STAFObjectDestruct(&(*iter));
            }

            // Then, get rid of the list

            delete obj.listValue;
        }
        else if (obj.type == kSTAFMapObject)
        {
            // First get rid of all the object values in the map.
            // The keys will be deleted automatically.

            for (STAFObjectMap::iterator iter = obj.mapValue->begin();
                 iter != obj.mapValue->end();
                 ++iter)
            {
                STAFObjectDestruct(&iter->second);
            }

            // Then, get rid of the map

            delete obj.mapValue;
        }
        else if (obj.type == kSTAFMarshallingContextObject)
        {
            STAFObjectDestruct(&obj.contextValue->mapClassMap);
            STAFObjectDestruct(&obj.contextValue->rootObject);

            delete obj.contextValue;
        }
    }

    delete *pObject;

    *pObject = 0;

    return kSTAFOk;
}


STAFRC_t STAFObjectIsStringMarshalledData(STAFStringConst_t string,
                                          unsigned int *isMarshalledData)
{
    if (string == 0) return kSTAFInvalidObject;
    if (isMarshalledData == 0) return kSTAFInvalidParm;

    static STAFString marshalledDataMarker("@SDT/");

    return STAFStringStartsWith(string, marshalledDataMarker.getImpl(),
                                isMarshalledData, 0);
}



STAFRC_t STAFObjectGetType(STAFObject_t object, STAFObjectType_t *type)
{
    if (object == 0) return kSTAFInvalidObject;
    if (type   == 0) return kSTAFInvalidParm;

    *type = object->type;

    return kSTAFOk;
}


STAFRC_t STAFObjectGetSize(STAFObject_t object, unsigned int *size)
{
    if (object == 0) return kSTAFInvalidObject;
    if (size   == 0) return kSTAFInvalidParm;

    if (object->type == kSTAFNoneObject)
        *size = 0;
    else if (object->type == kSTAFScalarStringObject)
        *size = object->scalarStringValue->length();
    else if (object->type == kSTAFMapObject)
        *size = object->mapValue->size();
    else if (object->type == kSTAFListObject)
        *size = object->listValue->size();
    else if (object->type == kSTAFMarshallingContextObject)
        return STAFObjectGetSize(object->contextValue->mapClassMap, size);
    else
      *size = 0;

    return kSTAFOk;
}


STAFRC_t STAFObjectIsReference(STAFObject_t object, unsigned int *isRef)
{
    if (object == 0) return kSTAFInvalidObject;
    if (isRef  == 0) return kSTAFInvalidParm;

    *isRef = object->isRef ? 1 : 0;

    return kSTAFOk;
}


STAFRC_t STAFObjectGetStringValue(STAFObject_t object, STAFString_t *pString)
{
    if (object  == 0) return kSTAFInvalidObject;
    if (pString == 0) return kSTAFInvalidParm;
    
    if (object->type == kSTAFNoneObject)
    {
        static STAFString sNoneString("<None>");
        STAFStringConstructCopy(pString, sNoneString.getImpl(), 0);
    }
    else if (object->type == kSTAFScalarStringObject)
    {
        STAFStringConstructCopy(pString,
                                object->scalarStringValue->getImpl(), 0);
    }
    else if (object->type == kSTAFListObject)
    {
        unsigned int size = 0;

        STAFObjectGetSize(object, &size);

        STAFString sListString = STAFString("<List>[") + size + "]";
        STAFStringConstructCopy(pString, sListString.getImpl(), 0);
    }
    else if (object->type == kSTAFMapObject)
    {
        static STAFString sMapClassKey("staf-map-class-name");
        unsigned int size = 0;

        STAFObjectGetSize(object, &size);

        STAFObjectMap::iterator iter =
            object->mapValue->find(sMapClassKey);

        if (iter != object->mapValue->end())
        {
            STAFString_t mapClassNameT = 0;

            STAFObjectGetStringValue(iter->second, &mapClassNameT);

            STAFString sMapString = STAFString("<Map:") +
                STAFString(mapClassNameT, STAFString::kShallow) +
                ">[" + size + "]";

            STAFStringConstructCopy(pString, sMapString.getImpl(), 0);
        }
        else
        {
            STAFString sMapString = STAFString("<Map>[") + size + "]";
            STAFStringConstructCopy(pString, sMapString.getImpl(), 0);
        }
    }
    else if (object->type == kSTAFMarshallingContextObject)
    {
        unsigned int size = 0;

        STAFObjectGetSize(object, &size);

        STAFString sContextString =
            STAFString("<MarshalingContext>[") + size + "]";
        STAFStringConstructCopy(pString, sContextString.getImpl(), 0);
    }
    else *pString = STAFString("<Unknown>").adoptImpl();

    return kSTAFOk;
}


STAFString ISTAFQuoteString(const STAFString &input)
{
    static STAFString sSingleQuote(kUTF8_SQUOTE);
    static STAFString sDoubleQuote(kUTF8_DQUOTE);
    static STAFString sEscapedSingleQuote("\\'");

    if (input.find(sSingleQuote) == STAFString::kNPos)
        return STAFString(sSingleQuote) + input + STAFString(sSingleQuote);

    if (input.find(sDoubleQuote) == STAFString::kNPos)
        return STAFString(sDoubleQuote) + input + STAFString(sDoubleQuote);

    return STAFString(sSingleQuote) +
           input.replace(sSingleQuote, sEscapedSingleQuote) +
           STAFString(sSingleQuote);
}


STAFString ISTAFGetLineSep()
{
    struct STAFConfigInfo configInfo = { 0 };

    STAFUtilGetConfigInfo(&configInfo, 0, 0);

    return configInfo.lineSeparator;
}


void ISTAFObjectGetFormattedStringValue(STAFString &output,
                                        STAFObjectPtr &objPtr,
                                        STAFObjectPtr &context,
                                        unsigned int indentLevel)
{
    static unsigned int sIndentDelta = 2;
    static STAFString sSpaces("                                        "
                              "                                        ");
    static STAFString sListIndent("[");
    static STAFString sListOutdent("]");
    static STAFString sMapIndent("{");
    static STAFString sMapOutdent("}");
    static STAFString sEntrySeparator("");
    static STAFString sMapKeySeparator(": ");
    static STAFString sKey("key");
    static STAFString sMapClassKey("staf-map-class-name");
    static STAFString sDisplayName("display-name");
    static STAFString sLineSep = ISTAFGetLineSep();

    switch (objPtr->type())
    {
        case kSTAFListObject:
        {
            output += sListIndent;

            ++indentLevel;

            if (objPtr->size() > 0) output += sLineSep;

            // Print out each object
            
            // Get the size of the list and create an array with that size.
            // Add a string representing each element in the list to the array
            // and then join the strings in the array.
            // Using an array and then doing a join is done for performance
            // reasons as it's faster than concatenating strings using +=.

            unsigned int size = objPtr->size();
            STAFString *stringArray = new STAFString[size];
            unsigned int i = 0;

            for (STAFObjectIteratorPtr iterPtr = objPtr->iterate();
                 iterPtr->hasNext();)
            {
                STAFString entryOutput;
                STAFObjectPtr thisObj = iterPtr->next();

                if ((thisObj->type() == kSTAFListObject) ||
                    (thisObj->type() == kSTAFMapObject) ||
                    (thisObj->type() == kSTAFMarshallingContextObject))
                {
                    entryOutput += sSpaces.subString(0, indentLevel * sIndentDelta);

                    ISTAFObjectGetFormattedStringValue(
                        entryOutput, thisObj, context, indentLevel);
                }
                else
                {
                    entryOutput += sSpaces.subString(0, indentLevel * sIndentDelta);

                    if (thisObj->type() == kSTAFNoneObject)
                        entryOutput += thisObj->asString();
                    else
                        entryOutput += thisObj->asString();
                }

                if (iterPtr->hasNext()) entryOutput += sEntrySeparator;

                entryOutput += sLineSep;

                stringArray[i++].replaceImpl(entryOutput.adoptImpl());
            }
            
            output.join(stringArray, size);

            delete [] stringArray;

            --indentLevel;

            if (objPtr->size() > 0)
                output += sSpaces.subString(0, indentLevel * sIndentDelta);

            output += sListOutdent;

            break;
        }
        case kSTAFMapObject:
        {
            output += sMapIndent;

            ++indentLevel;

            if (objPtr->size() > 0) output += sLineSep;

            // Check if the map object has a map class key and if the context
            // is valid and contains a map class definition for this map class.
            // If not, treat as a plain map class.

            if (objPtr->hasKey(sMapClassKey) &&
                context && (context->type() == kSTAFMarshallingContextObject) &&
                context->hasMapClassDefinition(objPtr->get(sMapClassKey)->asString()))
            {
                // Map object has a map class definition in the context

                STAFMapClassDefinitionPtr mapClass =
                    context->getMapClassDefinition(
                        objPtr->get(sMapClassKey)->asString());

                // Determine maximum key length

                STAFObjectIteratorPtr iterPtr;
                unsigned int maxKeyLength = 0;
                unsigned int size = 0;  // Number of keys

                for (iterPtr = mapClass->keyIterator(); iterPtr->hasNext();)
                {
                    size++;
                    STAFObjectPtr theKey = iterPtr->next();
                    STAFString theKeyString;

                    if (theKey->hasKey(sDisplayName))
                        theKeyString = theKey->get(sDisplayName)->asString();
                    else
                        theKeyString = theKey->get(sKey)->asString();

                    if (theKeyString.length(STAFString::kChar) > maxKeyLength)
                        maxKeyLength = theKeyString.length(STAFString::kChar);
                }

                // Now print each object in the map

                // Get the size of the map and create an array with that size.
                // Add a string representing each element in the map to the array
                // and then join the strings in the array.
                // Using an array and then doing a join is done for performance
                // reasons as it's faster than concatenating strings using +=.

                STAFString *stringArray = new STAFString[size];
                unsigned int i = 0;

                for (iterPtr = mapClass->keyIterator(); iterPtr->hasNext();)
                {
                    STAFObjectPtr theKey = iterPtr->next();
                    STAFString theKeyString;

                    if (theKey->hasKey(sDisplayName))
                        theKeyString = theKey->get(sDisplayName)->asString();
                    else
                        theKeyString = theKey->get(sKey)->asString();

                    STAFString entryOutput = sSpaces.subString(
                        0, indentLevel * sIndentDelta);
                    entryOutput += theKeyString;
                    entryOutput += sSpaces.subString(0, maxKeyLength -
                                   theKeyString.length(STAFString::kChar));
                    entryOutput += sMapKeySeparator;

                    STAFObjectPtr thisObj =
                        objPtr->get(theKey->get(sKey)->asString());

                    if ((thisObj->type() == kSTAFListObject) ||
                        (thisObj->type() == kSTAFMapObject) ||
                        (thisObj->type() == kSTAFMarshallingContextObject))
                    {
                        ISTAFObjectGetFormattedStringValue(
                            entryOutput, thisObj, context, indentLevel);
                    }
                    else if (thisObj->type() == kSTAFNoneObject)
                    {
                        entryOutput += thisObj->asString();
                    }
                    else
                    {
                        entryOutput += thisObj->asString();
                    }

                    if (iterPtr->hasNext()) entryOutput += sEntrySeparator;

                    entryOutput += sLineSep;

                    stringArray[i++].replaceImpl(entryOutput.adoptImpl());
                }
                
                output.join(stringArray, size);

                delete [] stringArray;
            }
            else
            {
                // The map does not have a map class key or the map class
                // definition is not provided in the context

                // Determine maximum key length

                STAFObjectIteratorPtr iterPtr;
                unsigned int maxKeyLength = 0;

                for (iterPtr = objPtr->keyIterator(); iterPtr->hasNext();)
                {
                    STAFString theKeyString = iterPtr->next()->asString();

                    if (theKeyString.length(STAFString::kChar) > maxKeyLength)
                        maxKeyLength = theKeyString.length(STAFString::kChar);
                }

                // Now print each object in the map

                // Get the size of the map and create an array with that size.
                // Add a string representing each element in the map to the array
                // and then join the strings in the array.
                // Using an array and then doing a join is done for performance
                // reasons as it's faster than concatenating strings using +=.

                unsigned int size = objPtr->size();
                STAFString *stringArray = new STAFString[size];
                unsigned int i = 0;

                for (iterPtr = objPtr->keyIterator(); iterPtr->hasNext();)
                {
                    STAFString theKeyString = iterPtr->next()->asString();

                    STAFString entryOutput = sSpaces.subString(
                        0, indentLevel * sIndentDelta);
                    entryOutput += theKeyString;
                    entryOutput += sSpaces.subString(
                        0, maxKeyLength - theKeyString.length(STAFString::kChar));
                    entryOutput += sMapKeySeparator;

                    STAFObjectPtr thisObj = objPtr->get(theKeyString);

                    if ((thisObj->type() == kSTAFListObject) ||
                        (thisObj->type() == kSTAFMapObject) ||
                        (thisObj->type() == kSTAFMarshallingContextObject))
                    {
                        ISTAFObjectGetFormattedStringValue(
                            entryOutput, thisObj, context, indentLevel);
                    }
                    else if (thisObj->type() == kSTAFNoneObject)
                    {
                        entryOutput += thisObj->asString();
                    }
                    else
                    {
                        entryOutput += thisObj->asString();
                    }

                    if (iterPtr->hasNext()) entryOutput += sEntrySeparator;

                    entryOutput += sLineSep;

                    stringArray[i++].replaceImpl(entryOutput.adoptImpl());
                }
                

                output.join(stringArray, size);

                delete [] stringArray;
            }

            --indentLevel;

            if (objPtr->size() > 0)
                output += sSpaces.subString(0, indentLevel * sIndentDelta);

            output += sMapOutdent;

            break;
        }
        case kSTAFMarshallingContextObject:
        {
            STAFObjectPtr thisObj = objPtr->getRootObject();

            ISTAFObjectGetFormattedStringValue(output, thisObj, objPtr,
                                               indentLevel);
            break;
        }
        default:
        {
            output += sSpaces.subString(0, indentLevel * sIndentDelta);
            output += objPtr->asString();
            break;
        }
    }
}


STAFRC_t STAFObjectGetFormattedStringValue(STAFObject_t object,
                                           STAFString_t *pString,
                                           unsigned int flags)
{
    if (object  == 0) return kSTAFInvalidObject;
    if (pString == 0) return kSTAFInvalidParm;

    if (object->type == kSTAFNoneObject)
    {
        static STAFString sNoneString("<None>");
        STAFStringConstructCopy(pString, sNoneString.getImpl(), 0);
    }
    else if (object->type == kSTAFScalarStringObject)
    {
        STAFStringConstructCopy(pString,
                                object->scalarStringValue->getImpl(), 0);
    }
    else
    {
        STAFObjectPtr theObj = STAFObject::createReference(object);
        STAFObjectPtr context = STAFObject::createNone();
        STAFString output;

        ISTAFObjectGetFormattedStringValue(output, theObj, context, 0);

        *pString = output.adoptImpl();
    }

    return kSTAFOk;
}


void STAFObjectFreeSTAFStringTArray(STAFString_t *theArray, unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i)
        STAFStringDestruct(&theArray[i], 0);

    delete [] theArray;
}


STAFRC_t STAFObjectMarshallToString(STAFObject_t object, STAFObject_t context,
                                    STAFString_t *pString, unsigned int flags)
{
    if (object  == 0) return kSTAFInvalidObject;
    if (pString == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if ((context != 0) && (context->type != kSTAFMarshallingContextObject))
        return kSTAFInvalidParm;

    static STAFString sMapClassKey("staf-map-class-name");

    STAFString outString;

    if (object->type == kSTAFScalarStringObject)
    {
        outString += "@SDT/$S:";
        outString += object->scalarStringValue->length(STAFString::kChar);
        outString += sColon;
        outString += *object->scalarStringValue;
    }
    else if (object->type == kSTAFNoneObject)
    {
        outString += "@SDT/$0:0:";
    }
    else if (object->type == kSTAFListObject)
    {
        // Get the size of the list and create an array with that size.
        // Instead of creating an array using:
        //   STAFString_t *stringArray = new STAFString_t[size];
        // we're using a STAFRefPtr<STAFString_t> custom array type so that
        // the array and it's contents will get destructed (even if an
        // exception occurs).
        // For each entry in the list, add a string representation of the entry
        // to the array and then join the strings in the array.
        // Using an array and then doing a join is done for performance
        // reasons as it's faster than concatenating strings using +=.

        unsigned int size = 0;
        STAFObjectGetSize(object, &size);
        
        STAFRefPtr<STAFString_t> stringArray = STAFRefPtr<STAFString_t>
            (new STAFString_t[size],
             STAFRefPtr<STAFString_t>::INIT,
             size, STAFObjectFreeSTAFStringTArray);
        
        unsigned int i = 0;

        for (STAFObjectList::iterator iter = object->listValue->begin();
             iter != object->listValue->end();
             ++iter)
        {
            STAFString_t thisItemStringT = 0;
            // XXX: Check RC?
            STAFObjectMarshallToString(*iter, context, &thisItemStringT, flags);
            stringArray[i++] = thisItemStringT;
        }

        STAFString_t joinedStringT = 0;
        unsigned int osRC;

        // XXX: Check RC from STAFStringConstructJoin?
        STAFStringConstructJoin(&joinedStringT, stringArray, size, &osRC);

        STAFString dataString = STAFString(joinedStringT, STAFString::kShallow);

        outString += "@SDT/[";
        outString += object->listValue->size();
        outString += sColon;
        outString += dataString.length(STAFString::kChar);
        outString += sColon;
        outString += dataString;
    }
    else if (object->type == kSTAFMapObject)
    {
        // If a staf-map-class-name key exists in the map, make sure that it's
        // map class definition is provided in the marshalling context.
        // If it's not, then treat the map as a plain map object.

        bool isMapClass = false;
        STAFString mapClassName;

        if (context &&
            (object->mapValue->find(sMapClassKey) != object->mapValue->end()))
        {
            mapClassName = *(*object->mapValue)[sMapClassKey]->
                scalarStringValue;

            unsigned int hasMapClassDefinition = 0;

            STAFRC_t rc = STAFObjectMarshallingContextHasMapClassDefinition(
                context, mapClassName.getImpl(), &hasMapClassDefinition);

            if ((rc == kSTAFOk) && (hasMapClassDefinition)) isMapClass = true;
        }

        if (isMapClass)
        {
            // Note: Not much checking is done here as all the checking is done
            //       in STAFObjectMarshallingContextSetMapClassDefinition

            // XXX: This whole block could use some rework to simplify all
            //      the dereferences

            STAFString dataString;
            dataString += sColon;
            dataString += mapClassName.length(STAFString::kChar);
            dataString += sColon;
            dataString += mapClassName;

            STAFObject_t keyList = (*(*context->contextValue->
                                    mapClassMap->mapValue)[mapClassName]->
                                    mapValue)["keys"];

            // Get the number of keys in the map class definition and create an
            // array with that size.
            // Instead of creating an array using:
            //   STAFString_t *stringArray = new STAFString_t[size];
            // we're using a STAFRefPtr<STAFString_t> custom array type so that
            // the array and it's contents will get destructed (even if an
            // exception occurs).
            // For each key in the map class, add a string representation to
            // the array and then join the strings in the array.
            // Using an array and then doing a join is done for performance
            // reasons as it's faster than concatenating strings using +=.

            unsigned int size = 0;
            STAFObjectGetSize(keyList, &size);

            STAFRefPtr<STAFString_t> stringArray = STAFRefPtr<STAFString_t>
                (new STAFString_t[size],
                 STAFRefPtr<STAFString_t>::INIT,
                 size, STAFObjectFreeSTAFStringTArray);

            unsigned int i = 0;

            for (STAFObjectList::iterator iter = keyList->listValue->begin();
                 iter != keyList->listValue->end();
                 ++iter)
            {
                STAFString &keyName =
                    *(*(*iter)->mapValue)["key"]->scalarStringValue;
                STAFString_t thisItemStringT = 0;
                STAFObject_t thisItem = (*object->mapValue)[keyName];

                if (thisItem == 0) thisItem = sNoneObjT;

                STAFObjectMarshallToString(thisItem, context, &thisItemStringT,
                                           flags);

                stringArray[i++] = thisItemStringT;
            }

            STAFString_t joinedStringT = 0;
            unsigned int osRC;

            STAFStringConstructJoin(&joinedStringT, stringArray, size, &osRC);

            dataString += STAFString(joinedStringT, STAFString::kShallow);

            outString += "@SDT/%:";
            outString += dataString.length(STAFString::kChar);
            outString += sColon;
            outString += dataString;
        }
        else
        {
            // Get the size of the map and create an array with that size.
            // Instead of creating an array using:
            //   STAFString_t *stringArray = new STAFString_t[size];
            // we're using a STAFRefPtr<STAFString_t> custom array type so that
            // the array and it's contents will get destructed (even if an
            // exception occurs).
            // For each key/value in the map, add a string representation to
            // the array and then join the strings in the array.
            // Using an array and then doing a join is done for performance
            // reasons as it's faster than concatenating strings using +=.

            unsigned int size = 0;
            STAFObjectGetSize(object, &size);
            
            STAFRefPtr<STAFString_t> stringArray = STAFRefPtr<STAFString_t>
                (new STAFString_t[size],
                 STAFRefPtr<STAFString_t>::INIT,
                 size, STAFObjectFreeSTAFStringTArray);

            unsigned int i = 0;

            for (STAFObjectMap::iterator iter = object->mapValue->begin();
                 iter != object->mapValue->end(); ++iter)
            {
                // First add the key

                STAFString entryString = sColon;
                entryString += iter->first.length(STAFString::kChar);
                entryString += sColon;
                entryString += iter->first;

                // Next add the value

                STAFString_t thisItemStringT = 0;
                // XXX: Check RC?
                STAFObjectMarshallToString(iter->second, context,
                                           &thisItemStringT, flags);

                entryString += STAFString(thisItemStringT, STAFString::kShallow);

                stringArray[i++] = entryString.adoptImpl();
            }

            STAFString_t joinedStringT = 0;
            unsigned int osRC;

            STAFStringConstructJoin(&joinedStringT, stringArray, size, &osRC);
            
            STAFString dataString = STAFString(joinedStringT, STAFString::kShallow);

            outString += "@SDT/{:";
            outString += dataString.length(STAFString::kChar);
            outString += sColon;
            outString += dataString;
        }
    }
    else if (object->type == kSTAFMarshallingContextObject)
    {
        STAFString dataString;
        unsigned int size = 0;
        STAFObjectGetSize(object->contextValue->mapClassMap, &size);
        
        if (size != 0)
        {
            // Get a reference to the context's map class map

            STAFObject_t mapClassMap = 0;

            STAFObjectConstructReference(&mapClassMap,
                                         object->contextValue->mapClassMap);

            // Now create the context map for marshalling and add the context's
            // map class map as the "map-class-map" object

            STAFObject_t contextMap = 0;

            STAFObjectConstructMap(&contextMap);
            STAFObjectMapPut(contextMap, STAFString("map-class-map").getImpl(),
                             mapClassMap);
            STAFObjectDestruct(&mapClassMap);

            // Marshall the context map itself and then destruct it
            //
            // Note, the context map is not marshalled within any context.  If it
            // were, we run the risk of recursive map class definitions, which
            // wouldn't work when unmarshalling.

            STAFString_t mapClassMapStringT = 0;

            STAFObjectMarshallToString(contextMap, 0, &mapClassMapStringT, flags);

            STAFObjectDestruct(&contextMap);

            // Generate the marshalled string, and destruct the string
            // containing the intermediate value

            STAFStringConcatenate(dataString.getImpl(), mapClassMapStringT, 0);
            STAFStringDestruct(&mapClassMapStringT, 0);
        }

        // Marshall the root object
        //
        // Note: We marshall the root object in the context of the context
        //       we are marshalling (not the context that was passed in, which
        //       might be different.

        STAFString_t rootObjectStringT = 0;

        STAFObjectMarshallToString(object->contextValue->rootObject, object,
                                   &rootObjectStringT, flags);

        // Finally, generate the marshalled string, and destruct the string
        // containing the intermediate value

        STAFStringConcatenate(dataString.getImpl(), rootObjectStringT, 0);
        STAFStringDestruct(&rootObjectStringT, 0);
        
        if (size != 0)
        {
            outString += "@SDT/*";
            outString += sColon;
            outString += dataString.length(STAFString::kChar);
            outString += sColon;
        }

        outString += dataString;
    }
    else
    {
        // Do what?
    }

    *pString = outString.adoptImpl();

    return kSTAFOk;
}


STAFRC_t STAFObjectUnmarshallFromString(STAFObject_t *newContext,
                                        STAFStringConst_t marshalledObject,
                                        STAFObject_t context,
                                        unsigned int flags)
{
    if (newContext       == 0) return kSTAFInvalidObject;
    if (marshalledObject == 0) return kSTAFInvalidParm;

    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if ((context != 0) && (context->type != kSTAFMarshallingContextObject))
        return kSTAFInvalidParm;

    static STAFString marshalledDataMarker("@SDT/");
    static STAFString noneMarker("@SDT/$0:0:");
    static STAFString scalarMarker("@SDT/$");
    static STAFString mapMarker("@SDT/{");
    static STAFString mcInstanceMarker("@SDT/%");
    static STAFString listMarker("@SDT/[");
    static STAFString contextMarker("@SDT/*");
    static STAFString sMapClassKey("staf-map-class-name");

    STAFObjectConstructMarshallingContext(newContext);

    // XXX: Probably shouldn't make a copy of the string like this, but I'm
    //      going to make sure things work first, and then I'll optimize

    STAFString data(marshalledObject);

    try
    {
        if (data.startsWith(noneMarker))
        {
            // XXX: This is all probably redundant since the marshalling
            // context already has a None in it.

            STAFObject_t noneObj = 0;

            STAFObjectConstructNone(&noneObj);
            STAFObjectMarshallingContextSetRootObject(*newContext, noneObj);
            STAFObjectDestruct(&noneObj);
        }
        else if (data.startsWith(scalarMarker))
        {
            // @SDT/$S:<string-length>:<String>

            unsigned int dataIndex = scalarMarker.length(STAFString::kChar);

            // Get String length

            unsigned int stringLength = getCLCLength(data, dataIndex);

            if (stringLength != (data.length(STAFString::kChar) - dataIndex))
                throw STAFException("Invalid marshalled scalar data");
            
            STAFObject_t stringObj = 0;
            STAFObjectConstructScalarString(
                &stringObj, data.subString(dataIndex).getImpl());

            // Unless told otherwise we will try to unmarshall a nested object
            // reference

            if ((flags & kSTAFIgnoreIndirectObjects) ||
                !stringObj->scalarStringValue->startsWith(
                    marshalledDataMarker))
            {
                STAFObjectMarshallingContextSetRootObject(
                    *newContext, stringObj);
            }
            else
            {
                STAFObject_t nestedContext = 0;

                STAFObjectUnmarshallFromString(
                    &nestedContext, stringObj->scalarStringValue->getImpl(),
                    context, flags);

                STAFObject_t primaryObject = adoptPrimaryObject(
                    nestedContext);

                STAFObjectMarshallingContextSetRootObject(
                    *newContext, primaryObject);
                STAFObjectDestruct(&primaryObject);
            }

            STAFObjectDestruct(&stringObj);
        }
        else if (data.startsWith(listMarker))
        {
            // @SDT/[<number-of-items>:<array-length>:<SDT-Any-1>...<SDT-Any-n>

            // Get the number of items in the list

            unsigned int colonIndex = data.find(sColon);

            if (colonIndex == STAFString::kNPos)
                throw STAFException("Invalid marshalled list data");

            unsigned int numItems = data.subString(6, colonIndex - 6).asUInt();

            unsigned int dataIndex = colonIndex;
            
            // Get the array length
            
            unsigned int arrayLength = getCLCLength(data, dataIndex);

            if (arrayLength != (data.length(STAFString::kChar) - dataIndex))
                throw STAFException("Invalid marshalled list data");

            STAFObject_t listObj = 0;

            STAFObjectConstructList(&listObj);

            for (unsigned int i = 0; i < numItems; ++i)
            {
                STAFObject_t obj = unmarshallObject(
                    data, context, dataIndex, flags);

                STAFObjectListAppend(listObj, obj);
                STAFObjectDestruct(&obj);
            }

            STAFObjectMarshallingContextSetRootObject(*newContext, listObj);
            STAFObjectDestruct(&listObj);
        }
        else if (data.startsWith(mapMarker))
        {
            // @SDT/{:<map-length>::<key-1-length>:<key-1><SDT-Any>
            //                     ...
            //                     :<key-n-length>:<key-1><SDT-Any>

            unsigned int dataIndex = mapMarker.length(STAFString::kChar);

            // Get the map-length and increment dataIndex

            unsigned int mapLength = getCLCLength(data, dataIndex);

            if (mapLength != (data.length(STAFString::kChar) - dataIndex))
                throw STAFException("Invalid marshalled map data");

            STAFObject_t mapObj = 0;

            STAFObjectConstructMap(&mapObj);

            while (dataIndex < data.length(STAFString::kChar))
            {
                STAFString key = getCLCString(data, dataIndex);
                STAFObject_t obj = unmarshallObject(
                    data, context, dataIndex, flags);

                STAFObjectMapPut(mapObj, key.getImpl(), obj);
                STAFObjectDestruct(&obj);
            }

            STAFObjectMarshallingContextSetRootObject(*newContext, mapObj);
            STAFObjectDestruct(&mapObj);
        }
        else if (data.startsWith(mcInstanceMarker))
        {
            // @SDT/%:<map-class-instance-length>::<map-class-name-length>:<map-class-name>
            //    <SDT-Any-value-1>
            //    ...
            //    <SDT-Any-value-n>

            STAFObject_t mapObj = 0;

            STAFObjectConstructMap(&mapObj);

            unsigned int dataIndex = mcInstanceMarker.length(
                STAFString::kChar);

            // Get the map-class-instance-length and increment dataIndex

            unsigned int mapClassInstanceLength = getCLCLength(
                data, dataIndex);

            if (mapClassInstanceLength != (data.length(STAFString::kChar) -
                                           dataIndex))
                throw STAFException("Invalid marshalled map data");

            // Get the map-class-name

            STAFString mapClassName = getCLCString(data, dataIndex);

            // Now add the map class name to the map

            STAFObject_t mapClassNameObj = 0;

            STAFObjectConstructScalarString(&mapClassNameObj,
                                            mapClassName.getImpl());

            STAFObjectMapPut(mapObj, sMapClassKey.getImpl(), mapClassNameObj);
            STAFObjectDestruct(&mapClassNameObj);

            // Now unmarshall all the actual keys

            STAFObject_t mapClassObj =
                (*context->contextValue->mapClassMap->mapValue)[mapClassName];
            STAFObject_t keyList = (*mapClassObj->mapValue)["keys"];
            STAFObjectList::iterator iter = keyList->listValue->begin();

            while (dataIndex < data.length(STAFString::kChar))
            {
                STAFObject_t keyStringObj =  (*(*iter)->mapValue)["key"];
                STAFString_t keyImpl = keyStringObj->scalarStringValue->
                    getImpl();
                STAFObject_t obj = unmarshallObject(
                    data, context, dataIndex, flags);

                STAFObjectMapPut(mapObj, keyImpl, obj);
                STAFObjectDestruct(&obj);

                ++iter;
            }

            STAFObjectMarshallingContextSetRootObject(*newContext, mapObj);
            STAFObjectDestruct(&mapObj);
        }
        else if (data.startsWith(contextMarker))
        {
            // @SDT/*:<context-length>:@SDT/{:<mapClassLength>:<mapClassData>
            //                         <rootObject>

            // Get context-length
            
            unsigned int dataIndex = contextMarker.length(STAFString::kChar);

            unsigned int contextLength = getCLCLength(data, dataIndex);

            if (contextLength != (data.length(STAFString::kChar) - dataIndex))
                throw STAFException("Invalid marshalled context");

            STAFObject_t contextMap = unmarshallObject(
                data, 0, dataIndex, flags);

            // Now we need to take ownership of the underlying map class map.
            // 1) Destruct the existing map class map in the new context
            // 2) "Adopt" the map class map from the context map
            // 3) Construct a None in the place of the "adopted" map class map
            //    in the context map
            // 4) Destruct the context map since we don't need it any more

            STAFObjectDestruct(&(*newContext)->contextValue->mapClassMap);

            (*newContext)->contextValue->mapClassMap =
                (*contextMap->mapValue)["map-class-map"];

            STAFObjectConstructNone(
                &(*contextMap->mapValue)["map-class-map"]);

            STAFObjectDestruct(&contextMap);

            // Now, get and set the root object

            STAFObject_t rootObject = unmarshallObject(
                data, *newContext, dataIndex, flags);

            STAFObjectMarshallingContextSetRootObject(
                *newContext, rootObject);
            STAFObjectDestruct(&rootObject);
        }
        else
        {
            // We don't know what this data is, so just return a Scalar String
            // object containing the whole string

            STAFObject_t stringObj = 0;

            STAFObjectConstructScalarString(&stringObj, data.getImpl());
            STAFObjectMarshallingContextSetRootObject(*newContext, stringObj);
            STAFObjectDestruct(&stringObj);
        }
    }
    catch (...)
    {
        // An exception occurred processing the marshalling data.
        // This means its probably invalid marshalled data, so just return
        // a Scalar String object containing the invalid marshalled data.

        STAFObject_t stringObj = 0;

        STAFObjectConstructScalarString(&stringObj, data.getImpl());
        STAFObjectMarshallingContextSetRootObject(*newContext, stringObj);
        STAFObjectDestruct(&stringObj);
    }

    return kSTAFOk;
}


STAFRC_t STAFObjectListAppend(STAFObject_t list, STAFObject_t obj)
{
    if (list == 0) return kSTAFInvalidObject;
    if (obj  == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (list->type != kSTAFListObject) return kSTAFInvalidObject;

    // Note: The object being passed in gets turned into a reference

    STAFObject_t newObj = new STAFObjectImpl(*obj);

    obj->isRef = true;

    list->listValue->push_back(newObj);

    return kSTAFOk;
}


STAFRC_t STAFObjectIteratorHasNext(STAFObjectIterator_t iter,
                                   unsigned int *hasNext)
{
    if (iter    == 0) return kSTAFInvalidObject;
    if (hasNext == 0) return kSTAFInvalidParm;

    if (iter->type == kSTAFObjectListIterator)
    {
        *hasNext = (iter->listIterator->iter ==
                    iter->listIterator->listObject->end()) ? 0 : 1;
    }
    else if ((iter->type == kSTAFObjectMapKeyIterator) ||
             (iter->type == kSTAFObjectMapValueIterator))
    {
        *hasNext = (iter->mapIterator->iter ==
                    iter->mapIterator->mapObject->end()) ? 0 : 1;
    }
    else
    {
        // XXX: Return an error?
        *hasNext = 0;
    }

    return kSTAFOk;
}


STAFRC_t STAFObjectIteratorGetNext(STAFObjectIterator_t iter,
                                   STAFObject_t *pObject)
{
    if (iter    == 0) return kSTAFInvalidObject;
    if (pObject == 0) return kSTAFInvalidParm;

    if (iter->type == kSTAFObjectListIterator)
    {
        STAFObjectConstructReference(pObject, *iter->listIterator->iter);
        ++iter->listIterator->iter;
    }
    else if (iter->type == kSTAFObjectMapKeyIterator)
    {
        STAFObjectConstructScalarString(
            pObject, iter->mapIterator->iter->first.getImpl());
        ++iter->mapIterator->iter;
    }
    else if (iter->type == kSTAFObjectMapValueIterator)
    {
        STAFObjectConstructReference(pObject, iter->mapIterator->iter->second);
        ++iter->mapIterator->iter;
    }
    else return kSTAFInvalidObject;

    return kSTAFOk;
}


STAFRC_t STAFObjectIteratorDestruct(STAFObjectIterator_t *iter)
{
    if (iter  == 0) return kSTAFInvalidObject;
    if (*iter == 0) return kSTAFInvalidObject;

    if ((*iter)->type == kSTAFObjectListIterator)
        delete (*iter)->listIterator;
    else
        delete (*iter)->mapIterator;

    delete *iter;

    *iter = 0;

    return kSTAFOk;
}


STAFRC_t STAFObjectMapHasKey(STAFObject_t map, STAFStringConst_t key,
                             unsigned int *pHasKey)
{
    if (map     == 0) return kSTAFInvalidObject;
    if (key     == 0) return kSTAFInvalidParm;
    if (pHasKey == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (map->type != kSTAFMapObject) return kSTAFInvalidObject;

    STAFObjectMap::iterator iter = map->mapValue->find(key);

    if (iter == map->mapValue->end())
        *pHasKey = 0;
    else
        *pHasKey = 1;

    return kSTAFOk;
}


STAFRC_t STAFObjectMapGet(STAFObject_t map, STAFStringConst_t key,
                          STAFObject_t *pObject)
{
    if (map     == 0) return kSTAFInvalidObject;
    if (key     == 0) return kSTAFInvalidParm;
    if (pObject == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (map->type != kSTAFMapObject) return kSTAFInvalidObject;

    STAFObjectMap::iterator iter = map->mapValue->find(key);

    if (iter == map->mapValue->end())
        STAFObjectConstructNone(pObject);
    else
        STAFObjectConstructReference(pObject, iter->second);

    return kSTAFOk;
}


STAFRC_t STAFObjectMapPut(STAFObject_t map, STAFStringConst_t key,
                          STAFObject_t obj)
{
    if (map == 0) return kSTAFInvalidObject;
    if (key == 0) return kSTAFInvalidParm;
    if (obj == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (map->type != kSTAFMapObject) return kSTAFInvalidObject;

    // If there is already something associated with this key, we have to
    // get rid of it first

    STAFObjectMap::iterator iter = map->mapValue->find(key);

    if (iter != map->mapValue->end())
    {
        STAFObjectDestruct(&iter->second);
    }

    // Now add the new object
    // Note: The object being passed in gets turned into a reference

    STAFObject_t newObj = new STAFObjectImpl(*obj);

    obj->isRef = true;

    (*map->mapValue)[key] = newObj;

    return kSTAFOk;
}


STAFRC_t STAFObjectMarshallingContextSetMapClassDefinition(
    STAFObject_t context,
    STAFStringConst_t name,
    STAFObject_t obj)
{
    if (context == 0) return kSTAFInvalidObject;
    if (name    == 0) return kSTAFInvalidParm;
    if (obj     == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    // XXX: We need to do a good bit of checking to ensure that the map class
    //      definition is valid

    // If there is already something associated with this key, we have to
    // get rid of it first

    STAFObjectMap::iterator iter =
        context->contextValue->mapClassMap->mapValue->find(name);

    if (iter != context->contextValue->mapClassMap->mapValue->end())
    {
        STAFObjectDestruct(&iter->second);
    }

    // Now add the new object
    // Note: The object being passed in gets turned into a reference

    STAFObject_t newObj = new STAFObjectImpl(*obj);

    obj->isRef = true;

    (*context->contextValue->mapClassMap->mapValue)[name] = newObj;

    return kSTAFOk;
}


STAFRC_t STAFObjectMarshallingContextGetMapClassDefinition(
    STAFObject_t context,
    STAFStringConst_t name,
    STAFObject_t *pObject)
{
    if (context == 0) return kSTAFInvalidObject;
    if (name    == 0) return kSTAFInvalidParm;
    if (pObject == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    STAFObjectMap::iterator iter =
        context->contextValue->mapClassMap->mapValue->find(name);

    if (iter == context->contextValue->mapClassMap->mapValue->end())
        STAFObjectConstructNone(pObject);
    else
        STAFObjectConstructReference(pObject, iter->second);

    return kSTAFOk;
}


STAFRC_t STAFObjectMarshallingContextHasMapClassDefinition(
    STAFObject_t context,
    STAFStringConst_t name,
    unsigned int *pHasMapClassDefinition)
{
    if (context                == 0) return kSTAFInvalidObject;
    if (name                   == 0) return kSTAFInvalidParm;
    if (pHasMapClassDefinition == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    return STAFObjectMapHasKey(context->contextValue->mapClassMap, name,
                               pHasMapClassDefinition);
}


STAFRC_t STAFObjectMarshallingContextSetRootObject(STAFObject_t context,
                                                   STAFObject_t obj)
{
    if (context == 0) return kSTAFInvalidObject;
    if (obj     == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    // Get rid of the old root object

    STAFObjectDestruct(&context->contextValue->rootObject);

    // Now set the new root object
    // Note: The object being passed in gets turned into a reference

    context->contextValue->rootObject = new STAFObjectImpl(*obj);

    obj->isRef = true;

    return kSTAFOk;
}


STAFRC_t STAFObjectMarshallingContextGetRootObject(STAFObject_t context,
                                                   STAFObject_t *pObject)
{
    if (context == 0) return kSTAFInvalidObject;
    if (pObject == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    STAFObjectConstructReference(pObject, context->contextValue->rootObject);

    return kSTAFOk;
}


STAFRC_t STAFObjectMarshallingContextAdoptRootObject(STAFObject_t context,
                                                     STAFObject_t *pObject)
{
    if (context == 0) return kSTAFInvalidObject;
    if (pObject == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    // Copy our root object into the destination
    *pObject = new STAFObjectImpl(*context->contextValue->rootObject);

    // Note: The marshalling context root object is now a reference
    context->contextValue->rootObject->isRef = true;

    return kSTAFOk;
}


STAFRC_t STAFObjectMarshallingContextGetPrimaryObject(STAFObject_t context,
                                                      STAFObject_t *pObject)
{
    if (context == 0) return kSTAFInvalidObject;
    if (pObject == 0) return kSTAFInvalidParm;
    // XXX: Might this want to be a new kSTAFInvalidOperation ?
    if (context->type != kSTAFMarshallingContextObject)
        return kSTAFInvalidObject;

    if (context->contextValue->mapClassMap->mapValue->size() != 0)
        STAFObjectConstructReference(pObject, context);
    else
        STAFObjectConstructReference(pObject, context->contextValue->rootObject);

    return kSTAFOk;
}
