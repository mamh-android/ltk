/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DataTypesInlImpl
#define STAF_DataTypesInlImpl

#include "STAF.h"
#include "STAFDataTypes.h"
#include "STAFRefPtr.h"
#include "STAFString.h"

STAF_INLINE STAFObject::STAFObject(const STAFString &aString)
{
    unsigned int osRC = 0;
    STAFRC_t rc = STAFObjectConstructScalarString(&fObject, aString.getImpl());

    STAFException::checkRC(rc, "STAFObjectConstructScalarString", osRC);
}


STAF_INLINE STAFObject::STAFObject(ObjectType objType)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    if (objType == STAF_NONE_OBJECT)
        rc = STAFObjectConstructNone(&fObject);
    else if (objType == STAF_LIST_OBJECT)
        rc = STAFObjectConstructList(&fObject);
    else if (objType == STAF_MAP_OBJECT)
        rc = STAFObjectConstructMap(&fObject);
    else
        rc = STAFObjectConstructMarshallingContext(&fObject);
    
    STAFException::checkRC(rc, "STAFObjectConstruct[None|List|Map]", osRC);
}


STAF_INLINE STAFObject::STAFObject(ObjectRef objRefMarker,
                                   STAFObject_t source)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    rc = STAFObjectConstructReference(&fObject, source);

    STAFException::checkRC(rc, "STAFObjectConstructReference", osRC);
}


STAF_INLINE STAFObject::STAFObject(STAFObject_t obj) : fObject(obj)
{
    /* Do Nothing */
}


STAF_INLINE STAFObjectPtr STAFObject::create(STAFObject_t source)
{
    return STAFObjectPtr(new STAFObject(source), STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createReference(const STAFObject &source)
{
    return STAFObjectPtr(new STAFObject(STAF_REF_OBJECT, source.fObject),
                         STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createReference(
    const STAFObjectPtr &source)
{
    return STAFObjectPtr(new STAFObject(STAF_REF_OBJECT, source->fObject),
                         STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createReference(STAFObject_t source)
{
    return STAFObjectPtr(new STAFObject(STAF_REF_OBJECT, source),
                         STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createNone()
{
    return STAFObjectPtr(new STAFObject(STAF_NONE_OBJECT),
                         STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createScalar(const STAFString &aString)
{
    return STAFObjectPtr(new STAFObject(aString), STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createList()
{
    return STAFObjectPtr(new STAFObject(STAF_LIST_OBJECT),
                         STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createMap()
{
    return STAFObjectPtr(new STAFObject(STAF_MAP_OBJECT),
                         STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFObject::createMarshallingContext()
{
    return STAFObjectPtr(new STAFObject(STAF_MARSHALLING_CONTEXT_OBJECT),
                         STAFObjectPtr::INIT);
}


STAF_INLINE void STAFObject::marshall(STAFString &output, unsigned int flags)
{
    unsigned int osRC = 0;
    STAFString_t outputT = 0;
    STAFRC_t rc = STAFObjectMarshallToString(fObject, 0, &outputT, flags);

    STAFException::checkRC(rc, "STAFObjectMarshallToString", osRC);

    output = STAFString(outputT, STAFString::kShallow);
}


STAF_INLINE STAFString STAFObject::marshall(unsigned int flags)
{
    unsigned int osRC = 0;
    STAFString_t outputT = 0;
    STAFRC_t rc = STAFObjectMarshallToString(fObject, 0, &outputT, flags);

    STAFException::checkRC(rc, "STAFObjectMarshallToString", osRC);

    return STAFString(outputT, STAFString::kShallow);
}


STAF_INLINE STAFObjectPtr STAFObject::unmarshall(const STAFString &input,
                                                 unsigned int flags)
{
    unsigned int osRC = 0;
    STAFObject_t context = 0;
    STAFRC_t rc =
        STAFObjectUnmarshallFromString(&context, input.getImpl(), 0, flags);

    STAFException::checkRC(rc, "STAFObjectUnmarshallFromString", osRC);

    return STAFObjectPtr(new STAFObject(context), STAFObjectPtr::INIT);
}


STAF_INLINE STAFObject_t STAFObject::getImpl()
{
    return fObject;
}


STAF_INLINE STAFObjectPtr STAFObject::reference()
{
    return STAFObjectPtr(new STAFObject(STAF_REF_OBJECT, fObject),
                         STAFObjectPtr::INIT);
}


STAF_INLINE bool STAFObject::isMarshalledData(const STAFString &aString)
{
    unsigned int isMarshalledData = 0;
    STAFRC_t rc = STAFObjectIsStringMarshalledData(aString.getImpl(),
                                                   &isMarshalledData);
    return isMarshalledData ? true : false;
}


STAF_INLINE STAFObjectType_t STAFObject::type()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObjectType_t objType = kSTAFNoneObject;

    rc = STAFObjectGetType(fObject, &objType);

    STAFException::checkRC(rc, "STAFObjectGetType", osRC);

    return objType;
}


STAF_INLINE unsigned int STAFObject::size()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    unsigned int size = 0;

    rc = STAFObjectGetSize(fObject, &size);

    STAFException::checkRC(rc, "STAFObjectGetSize", osRC);

    return size;
}


STAF_INLINE STAFString STAFObject::asString()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFString_t aString = 0;

    rc = STAFObjectGetStringValue(fObject, &aString);

    STAFException::checkRC(rc, "STAFObjectGetStringValue", osRC);

    return STAFString(aString, STAFString::kShallow);
}


STAF_INLINE STAFString STAFObject::asFormattedString()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFString_t aString = 0;

    rc = STAFObjectGetFormattedStringValue(fObject, &aString, 0);

    STAFException::checkRC(rc, "STAFObjectGetFormattedStringValue", osRC);

    return STAFString(aString, STAFString::kShallow);
}


STAF_INLINE void STAFObject::append(const STAFObject &obj)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    rc = STAFObjectListAppend(fObject, obj.fObject);

    STAFException::checkRC(rc, "STAFObjectListAppend", osRC);
}


STAF_INLINE void STAFObject::append(const STAFObjectPtr &objPtr)
{
    append(*objPtr);
}


STAF_INLINE void STAFObject::append(const STAFString &aString)
{
    append(STAFObject(aString));
}


STAF_INLINE STAFObjectIteratorPtr STAFObject::iterate()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObjectIterator_t iter = 0;

    rc = STAFObjectConstructListIterator(&iter, fObject);

    STAFException::checkRC(rc, "STAFObjectConstructListIterator", osRC);

    return STAFObjectIteratorPtr(new STAFObjectIterator(iter),
                                 STAFObjectIteratorPtr::INIT);
}


STAF_INLINE bool STAFObject::hasKey(const STAFString &key)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    unsigned int hasKey = 0;

    rc = STAFObjectMapHasKey(fObject, key.getImpl(), &hasKey);

    STAFException::checkRC(rc, "STAFObjectMapHasKey", osRC);

    return (hasKey == 0) ? false : true;
}


STAF_INLINE STAFObjectPtr STAFObject::get(const STAFString &key)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObject_t retObj = 0;

    rc = STAFObjectMapGet(fObject, key.getImpl(), &retObj);

    STAFException::checkRC(rc, "STAFObjectMapGet", osRC);

    return STAFObjectPtr(new STAFObject(retObj), STAFObjectPtr::INIT);
}


STAF_INLINE void STAFObject::put(const STAFString &key, const STAFObject &obj)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    rc = STAFObjectMapPut(fObject, key.getImpl(), obj.fObject);

    STAFException::checkRC(rc, "STAFObjectMapPut", osRC);
}


STAF_INLINE void STAFObject::put(const STAFString &key,
                                 const STAFObjectPtr &objPtr)
{
    put(key, *objPtr);
}


STAF_INLINE void STAFObject::put(const STAFString &key,
                                 const STAFString &aString)
{
    put(key, STAFObject(aString));
}


STAF_INLINE STAFObjectIteratorPtr STAFObject::keyIterator()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObjectIterator_t iter = 0;

    rc = STAFObjectConstructMapKeyIterator(&iter, fObject);

    STAFException::checkRC(rc, "STAFObjectConstructMapKeyIterator", osRC);

    return STAFObjectIteratorPtr(new STAFObjectIterator(iter),
                                 STAFObjectIteratorPtr::INIT);
}


STAF_INLINE STAFObjectIteratorPtr STAFObject::valueIterator()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObjectIterator_t iter = 0;

    rc = STAFObjectConstructMapValueIterator(&iter, fObject);

    STAFException::checkRC(rc, "STAFObjectConstructMapValueIterator", osRC);

    return STAFObjectIteratorPtr(new STAFObjectIterator(iter),
                                 STAFObjectIteratorPtr::INIT);
}


STAF_INLINE void STAFObject::setMapClassDefinition(
    const STAFMapClassDefinition &def)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    rc = STAFObjectMarshallingContextSetMapClassDefinition(
            fObject, def.name().getImpl(),
            def.fMapClassDefObj->fObject);

    STAFException::checkRC(
        rc, "STAFObjectMarshallingContextSetMapClassDefinition", osRC);
}


STAF_INLINE void STAFObject::setMapClassDefinition(
    const STAFMapClassDefinitionPtr &defPtr)
{
    setMapClassDefinition(*defPtr);
}


STAF_INLINE STAFMapClassDefinitionPtr STAFObject::getMapClassDefinition(
    const STAFString &name)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObject_t obj = 0;

    rc = STAFObjectMarshallingContextGetMapClassDefinition(
             fObject, name.getImpl(), &obj);

    STAFException::checkRC(
        rc, "STAFObjectMarshallingContextGetMapClassDefinition", osRC);

    STAFObjectPtr objPtr = STAFObjectPtr(new STAFObject(obj),
                                         STAFObjectPtr::INIT);

    if (objPtr->type() == kSTAFNoneObject)
    {
        objPtr = STAFObject::createMap();

        objPtr->put("keys", STAFObject::createList());
        objPtr->put("name", name);

    }

    return STAFMapClassDefinitionPtr(
        new STAFMapClassDefinition(objPtr), STAFMapClassDefinitionPtr::INIT);
}


STAF_INLINE bool STAFObject::hasMapClassDefinition(const STAFString &name)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    unsigned int hasMCD = 0;

    rc = STAFObjectMarshallingContextHasMapClassDefinition(
             fObject, name.getImpl(), &hasMCD);

    STAFException::checkRC(
        rc, "STAFObjectMarshallingContextHasMapClassDefinition", osRC);

    return (hasMCD == 0) ? false : true;
}


STAF_INLINE STAFObjectIteratorPtr STAFObject::mapClassDefinitionIterator()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObjectIterator_t iter = 0;

    rc = STAFObjectConstructMapClassDefinitionIterator(&iter, fObject);

    STAFException::checkRC(
        rc, "STAFObjectConstructMapClassDefinitionIterator", osRC);

    return STAFObjectIteratorPtr(new STAFObjectIterator(iter),
                                 STAFObjectIteratorPtr::INIT);
}



STAF_INLINE void STAFObject::setRootObject(const STAFObject &obj)
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;

    rc = STAFObjectMarshallingContextSetRootObject(fObject, obj.fObject);

    STAFException::checkRC(
        rc, "STAFObjectMarshallingContextSetRootObject", osRC);
}


STAF_INLINE void STAFObject::setRootObject(const STAFObjectPtr &objPtr)
{
    setRootObject(*objPtr);
}


STAF_INLINE STAFObjectPtr STAFObject::getRootObject()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObject_t obj = 0;

    rc = STAFObjectMarshallingContextGetRootObject(fObject, &obj);

    STAFException::checkRC(
        rc, "STAFObjectMarshallingContextGetRootObject", osRC);

    return STAFObjectPtr(new STAFObject(obj), STAFObjectPtr::INIT);
}


STAF_INLINE STAFObject::~STAFObject()
{
    STAFObjectDestruct(&fObject);
}


STAF_INLINE STAFObjectIterator::STAFObjectIterator(STAFObjectIterator_t iter)
    : fIter(iter)
{
    /* Do nothing */
}


STAF_INLINE bool STAFObjectIterator::hasNext()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    unsigned int hasNext = 0;

    rc = STAFObjectIteratorHasNext(fIter, &hasNext);

    STAFException::checkRC(rc, "STAFObjectIteratorHasNext", osRC);

    return (hasNext == 0) ? false : true;
}


STAF_INLINE STAFObjectPtr STAFObjectIterator::next()
{
    unsigned int osRC = 0;
    STAFRC_t rc = kSTAFOk;
    STAFObject_t retObj = 0;

    rc = STAFObjectIteratorGetNext(fIter, &retObj);

    STAFException::checkRC(rc, "STAFObjectIteratorGetNext", osRC);

    return STAFObjectPtr(new STAFObject(retObj), STAFObjectPtr::INIT);
}


STAF_INLINE STAFObjectIterator::~STAFObjectIterator()
{
    STAFObjectIteratorDestruct(&fIter);
}


STAF_INLINE STAFMapClassDefinitionPtr STAFMapClassDefinition::create(
    const STAFString &name)
{
    STAFObjectPtr mapClassDefObj = STAFObject::createMap();

    mapClassDefObj->put("keys", STAFObject::createList());
    mapClassDefObj->put("name", name);

    return STAFMapClassDefinitionPtr(new STAFMapClassDefinition(mapClassDefObj),
                                     STAFMapClassDefinitionPtr::INIT);
}


STAF_INLINE STAFMapClassDefinitionPtr STAFMapClassDefinition::createReference(
        STAFMapClassDefinitionPtr source)
{
    return STAFMapClassDefinitionPtr(
        new STAFMapClassDefinition(
            STAFObject::createReference(source->fMapClassDefObj)),
        STAFMapClassDefinitionPtr::INIT);
}


STAF_INLINE STAFObjectPtr STAFMapClassDefinition::createInstance()
{
    static STAFString mapClassKey("staf-map-class-name");
    STAFObjectPtr mapClassInstance = STAFObject::createMap();

    mapClassInstance->put(mapClassKey, fMapClassDefObj->get("name"));

    return mapClassInstance;
}


STAF_INLINE STAFMapClassDefinitionPtr STAFMapClassDefinition::reference()
{
    return STAFMapClassDefinitionPtr(
        new STAFMapClassDefinition(
            STAFObject::createReference(fMapClassDefObj)),
        STAFMapClassDefinitionPtr::INIT);
}


STAF_INLINE void STAFMapClassDefinition::addKey(const STAFString &keyName)
{
    STAFObjectPtr theKey = STAFObject::createMap();

    theKey->put("key", keyName);

    fMapClassDefObj->get("keys")->append(theKey);}



STAF_INLINE void STAFMapClassDefinition::addKey(const STAFString &keyName,
                                                const STAFString &displayName)
{
    STAFObjectPtr theKey = STAFObject::createMap();

    theKey->put("key", keyName);
    theKey->put("display-name", displayName);

    fMapClassDefObj->get("keys")->append(theKey);
}


STAF_INLINE void STAFMapClassDefinition::setKeyProperty(
    const STAFString &keyName,
    const STAFString &propName,
    const STAFString &propValue)
{
    for (STAFObjectIteratorPtr iter = fMapClassDefObj->get("keys")->iterate();
         iter->hasNext();)
    {
        STAFObjectPtr thisKey = iter->next();

        if (thisKey->get("key")->asString() == keyName)
            thisKey->put(propName, propValue);
    }
}


STAF_INLINE STAFObjectIteratorPtr STAFMapClassDefinition::keyIterator()
{
    return fMapClassDefObj->get("keys")->iterate();
}


STAF_INLINE STAFString STAFMapClassDefinition::name() const
{
    return fMapClassDefObj->get("name")->asString();
}


STAF_INLINE STAFObjectPtr STAFMapClassDefinition::getMapClassDefinitionObject()
{
    return fMapClassDefObj;
}


STAF_INLINE STAFMapClassDefinition::STAFMapClassDefinition(
    STAFObjectPtr mapClassDefObj) : fMapClassDefObj(mapClassDefObj)
{
    /* Do nothing */
}

#endif
