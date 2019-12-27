/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_DataTypes
#define STAF_DataTypes

#include "STAFString.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Begin C language definitions */

typedef enum
{
    kSTAFNoneObject               = 0,
    kSTAFScalarStringObject       = 1,
    kSTAFListObject               = 2,
    kSTAFMapObject                = 3,
    kSTAFMarshallingContextObject = 4
} STAFObjectType_t;

typedef struct STAFObjectImpl *STAFObject_t;
typedef struct STAFObjectIteratorImpl *STAFObjectIterator_t;

typedef enum
{
    kSTAFMarshallingDefaults = 0x00000000
} STAFObjectMarshallingFlags_t;


typedef enum
{
    kSTAFUnmarshallingDefaults = 0x00000000,
    kSTAFIgnoreIndirectObjects = 0x00000001
} STAFObjectUnmarshallingFlags_t;


// Object constructors/destructors
//
// Note: When a STAFObject is destructed, it recursively deletes all nested
//       objects
//
// Note: Might need to require a Marshalling Context in order to be able
//       to construct objects.  It could be handy for validating metadata, etc.

STAFRC_t STAFObjectConstructCopy(STAFObject_t *copy, STAFObject_t source);
STAFRC_t STAFObjectConstructReference(STAFObject_t *ref, STAFObject_t source);
STAFRC_t STAFObjectConstructNone(STAFObject_t *pNone);
STAFRC_t STAFObjectConstructScalarString(STAFObject_t *pScalar,
                                         STAFStringConst_t string);
STAFRC_t STAFObjectConstructList(STAFObject_t *list);
STAFRC_t STAFObjectConstructMap(STAFObject_t *map);
STAFRC_t STAFObjectConstructMarshallingContext(STAFObject_t *context);
STAFRC_t STAFObjectDestruct(STAFObject_t *object);

// General functions

STAFRC_t STAFObjectIsStringMarshalledData(STAFStringConst_t string,
                                          unsigned int *isMarshalledData);

// Object functions

STAFRC_t STAFObjectGetType(STAFObject_t object, STAFObjectType_t *type);
STAFRC_t STAFObjectGetSize(STAFObject_t object, unsigned int *size);
STAFRC_t STAFObjectIsReference(STAFObject_t object, unsigned int *isRef);
STAFRC_t STAFObjectUnmarshallFromString(STAFObject_t *newContext,
                                        STAFStringConst_t string,
                                        STAFObject_t context,
                                        unsigned int flags);
STAFRC_t STAFObjectMarshallToString(STAFObject_t object, STAFObject_t context,
                                    STAFString_t *string, unsigned int flags);
STAFRC_t STAFObjectGetStringValue(STAFObject_t object, STAFString_t *string);
STAFRC_t STAFObjectGetFormattedStringValue(STAFObject_t object,
                                           STAFString_t *string,
                                           unsigned int flags);

// Scalar functions

STAFRC_t STAFObjectScalarGetStringValue(STAFObject_t object,
                                        STAFStringConst_t *string);
STAFRC_t STAFObjectScalarGetUIntValue(STAFObject_t object,
                                      unsigned int *uInt,
                                      unsigned int defaultValue);

// List functions

STAFRC_t STAFObjectListAppend(STAFObject_t list, STAFObject_t object);

// Iterator functions

STAFRC_t STAFObjectConstructListIterator(STAFObjectIterator_t *iter,
                                         STAFObject_t list);
STAFRC_t STAFObjectIteratorHasNext(STAFObjectIterator_t iter,
                                   unsigned int *hasNext);
STAFRC_t STAFObjectIteratorGetNext(STAFObjectIterator_t iter,
                                   STAFObject_t *object);
STAFRC_t STAFObjectIteratorDestruct(STAFObjectIterator_t *iter);

// Map functions

STAFRC_t STAFObjectMapGet(STAFObject_t map, STAFStringConst_t key,
                          STAFObject_t *object);
STAFRC_t STAFObjectMapPut(STAFObject_t map, STAFStringConst_t key,
                          STAFObject_t object);
STAFRC_t STAFObjectMapHasKey(STAFObject_t map, STAFStringConst_t key,
                             unsigned int *hasKey);
STAFRC_t STAFObjectConstructMapKeyIterator(STAFObjectIterator_t *pIter,
                                           STAFObject_t map);
STAFRC_t STAFObjectConstructMapValueIterator(STAFObjectIterator_t *pIter,
                                             STAFObject_t map);

// Marshalling Context functions

STAFRC_t STAFObjectMarshallingContextSetMapClassDefinition(
    STAFObject_t context,
    STAFStringConst_t name,
    STAFObject_t mapClassDefinition);

STAFRC_t STAFObjectMarshallingContextGetMapClassDefinition(
    STAFObject_t context,
    STAFStringConst_t name,
    STAFObject_t *mapClassDefinition);

STAFRC_t STAFObjectMarshallingContextHasMapClassDefinition(
    STAFObject_t context,
    STAFStringConst_t name,
    unsigned int *pHasMapClassDefinition);

STAFRC_t STAFObjectMarshallingContextSetRootObject(STAFObject_t context,
                                                   STAFObject_t object);
STAFRC_t STAFObjectMarshallingContextGetRootObject(STAFObject_t context,
                                                   STAFObject_t *object);
STAFRC_t STAFObjectMarshallingContextAdoptRootObject(STAFObject_t context,
                                                     STAFObject_t *object);
STAFRC_t STAFObjectMarshallingContextGetPrimaryObject(STAFObject_t context,
                                                      STAFObject_t *object);
STAFRC_t STAFObjectConstructMapClassDefinitionIterator(
    STAFObjectIterator_t *pIter, STAFObject_t context);

/* End C language definitions */

#ifdef __cplusplus
}

/* Begin C++ language definitions */

#include "STAFRefPtr.h"

class STAFObject;
typedef STAFRefPtr<STAFObject> STAFObjectPtr;


class STAFObjectIterator
{
public:

    bool hasNext();
    STAFObjectPtr next();

    ~STAFObjectIterator();

private:

    // Don't allow copy construction or assignment
    STAFObjectIterator(const STAFObjectIterator &);
    STAFObjectIterator &operator=(const STAFObjectIterator &);

    STAFObjectIterator(STAFObjectIterator_t iter);

    friend class STAFObject;

    STAFObjectIterator_t fIter;
};

typedef STAFRefPtr<STAFObjectIterator> STAFObjectIteratorPtr;


class STAFMapClassDefinition;
typedef STAFRefPtr<STAFMapClassDefinition> STAFMapClassDefinitionPtr;

class STAFMapClassDefinition
{
public:

    static STAFMapClassDefinitionPtr create(const STAFString &name);
    static STAFMapClassDefinitionPtr createReference(
        STAFMapClassDefinitionPtr source);

    STAFObjectPtr createInstance();

    STAFMapClassDefinitionPtr reference();

    void addKey(const STAFString &keyName);
    void addKey(const STAFString &keyName, const STAFString &displayName);

    void setKeyProperty(const STAFString &keyName, const STAFString &propName,
                        const STAFString &propValue);

    STAFObjectIteratorPtr keyIterator();

    STAFString name() const;

    STAFObjectPtr getMapClassDefinitionObject();

private:

    STAFMapClassDefinition(STAFObjectPtr mapClassDefObj);

    friend class STAFObject;

    STAFObjectPtr fMapClassDefObj;
};



class STAFObject
{
public:

    // Creation methods

    static STAFObjectPtr create(STAFObject_t source);
    static STAFObjectPtr createReference(const STAFObject &source);
    static STAFObjectPtr createReference(const STAFObjectPtr &source);
    static STAFObjectPtr createReference(STAFObject_t source);
    static STAFObjectPtr createNone();
    static STAFObjectPtr createScalar(const STAFString &aString);
    static STAFObjectPtr createList();
    static STAFObjectPtr createMap();
    static STAFObjectPtr createMarshallingContext();

    // General methods

    static bool isMarshalledData(const STAFString &aString);

    // General object methods

    STAFObjectType_t type();
    unsigned int size();

    bool isRef();
    STAFObjectPtr reference();

    STAFString asString();
    STAFString asFormattedString();

    STAFString marshall(unsigned int flags = kSTAFMarshallingDefaults);
    void marshall(STAFString &output,
                  unsigned int flags = kSTAFMarshallingDefaults);

    // Note: This method always returns a Marshalling Context
    static STAFObjectPtr unmarshall(const STAFString &input,
                                    unsigned int flags =
                                    kSTAFUnmarshallingDefaults);

    STAFObject_t getImpl();

    // List methods

    void append(const STAFObjectPtr &objPtr);
    void append(const STAFString &aString);

    STAFObjectIteratorPtr iterate();

    // Map methods

    bool hasKey(const STAFString &key);

    STAFObjectPtr get(const STAFString &key);

    void put(const STAFString &key, const STAFObjectPtr &objPtr);
    void put(const STAFString &key, const STAFString &aString);

    STAFObjectIteratorPtr keyIterator();
    STAFObjectIteratorPtr valueIterator();

    // Marshalling Context methods

    void setMapClassDefinition(const STAFMapClassDefinitionPtr &defPtr);

    STAFMapClassDefinitionPtr getMapClassDefinition(const STAFString &name);

    bool hasMapClassDefinition(const STAFString &name);

    STAFObjectIteratorPtr mapClassDefinitionIterator();

    void setRootObject(const STAFObjectPtr &objPtr);
    STAFObjectPtr getRootObject();

    // Destructor

    ~STAFObject();

private:

    // Don't allow copy construction or assignment
    STAFObject(const STAFObject &);
    STAFObject &operator=(const STAFObject &);

    friend class STAFObjectIterator;

    enum ObjectType
    {
        STAF_NONE_OBJECT                = 0,
        STAF_LIST_OBJECT                = 1,
        STAF_MAP_OBJECT                 = 2,
        STAF_MARSHALLING_CONTEXT_OBJECT = 3
    };

    enum ObjectRef
    {
        STAF_REF_OBJECT = 0
    };

    STAFObject(const STAFString &aString);
    STAFObject(ObjectType objType);
    STAFObject(ObjectRef objRefMarker, STAFObject_t);
    STAFObject(STAFObject_t obj);

    void append(const STAFObject &obj);
    void put(const STAFString &key, const STAFObject &obj);
    void setMapClassDefinition(const STAFMapClassDefinition &def);
    void setRootObject(const STAFObject &obj);

    STAFObject_t fObject;
};

// Now include inline definitions

#ifndef STAF_NATIVE_COMPILER
#include "STAFDataTypesInlImpl.cpp"
#endif

// End C++ language definitions

// End #ifdef __cplusplus
#endif

#endif
