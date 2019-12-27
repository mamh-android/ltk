/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_RefPtr
#define STAF_RefPtr

#include "STAFThread.h"

#ifdef __cplusplus

// STAFBuffer<T> - This class provides a simple delete wrapper

template <class TheType>
class STAFBuffer
{
public:

    enum Init { INIT = 0 };
    enum Type { SCALAR = 0, ARRAY = 1 };

    STAFBuffer(TheType *, Init init, Type type = ARRAY);

    TheType *operator->() const { return  fPtr; }
    TheType &operator*()  const { return *fPtr; }
    operator TheType *()  const { return  fPtr; }

    ~STAFBuffer();

private:

    // Don't allow assignment or copy construction
    STAFBuffer(const STAFBuffer &);
    STAFBuffer &operator=(const STAFBuffer &from);
    STAFBuffer &operator=(const TheType *);

    TheType *fPtr;
    Type fType;
};

template <class TheType>
inline STAFBuffer<TheType>::STAFBuffer(TheType *ptr, Init init, Type type)
    : fPtr(ptr), fType(type)
{
    /* Do Nothing */
}

template <class TheType>
inline STAFBuffer<TheType>::~STAFBuffer()
{
    if (fType == SCALAR) delete fPtr;
    else delete [] fPtr;
}


// STAFRefPtr<T> - This class provides thread safe reference counted pointers

template <class TheType>
class STAFRefPtr
{
public:

    enum Init { INIT = 0 };
    enum Type { SCALAR = 0, ARRAY = 1, CUSTOM = 2, CUSTOMARRAY = 3 };
    typedef void(*TypeFreeFunction)(TheType *);
    typedef void(*TypeArrayFreeFunction)(TheType *, unsigned int);

    STAFRefPtr();
    STAFRefPtr(TheType *, Init init, Type type = SCALAR, TypeFreeFunction = 0);
    STAFRefPtr(TheType *, Init init, unsigned int size,
               TypeArrayFreeFunction arrayFreeFunc);
    STAFRefPtr(const STAFRefPtr &from);

    STAFRefPtr &operator=(const STAFRefPtr &from);
    bool operator==(const STAFRefPtr &rhs);
    TheType *operator->() const { return  fPtr; }
    TheType &operator*()  const { return *fPtr; }
    operator TheType *()  const { return  fPtr; }

    STAFThreadSafeScalar_t count() { return fCount ? *fCount : -1; }

    ~STAFRefPtr();

private:

    // Don't allow assignment from raw pointer type.
    STAFRefPtr &operator=(const TheType *);

    TheType *fPtr;
    Type fType;
    union
    {
        TypeFreeFunction fFreeFunc;
        TypeArrayFreeFunction fArrayFreeFunc;
    };
    unsigned int fSize;
    STAFThreadSafeScalar_t *fCount;
};

template <class TheType>
inline STAFRefPtr<TheType>::STAFRefPtr() : fPtr(0), fType(SCALAR), fFreeFunc(0),
                                           fCount(0), fSize(0)
{ /* Do Nothing */ }

template <class TheType>
inline STAFRefPtr<TheType>::STAFRefPtr(TheType *ptr, Init init, Type type,
                                       TypeFreeFunction freeFunc)
    : fPtr(ptr), fType(type), fFreeFunc(freeFunc), fCount(0), fSize(0)
{
    fCount = new STAFThreadSafeScalar_t(1);
}

template <class TheType>
inline STAFRefPtr<TheType>::STAFRefPtr(TheType *ptr, Init init,
                                       unsigned int size,
                                       TypeArrayFreeFunction freeFunc)
    : fPtr(ptr), fType(CUSTOMARRAY), fArrayFreeFunc(freeFunc), fCount(0),
      fSize(size)
{
    fCount = new STAFThreadSafeScalar_t(1);
}

template <class TheType>
inline STAFRefPtr<TheType>::STAFRefPtr(const STAFRefPtr &from)
    : fPtr(from.fPtr), fType(from.fType), fSize(from.fSize), fCount(from.fCount)
{
    if (fType == CUSTOMARRAY)
        fArrayFreeFunc = from.fArrayFreeFunc;
    else
        fFreeFunc = from.fFreeFunc;

    if (fCount != 0)
        STAFThreadSafeIncrement(fCount);
}

template <class TheType>
inline STAFRefPtr<TheType> &STAFRefPtr<TheType>::operator=(const STAFRefPtr &from)
{
    if (fPtr == from.fPtr) return *this;

    if (fCount != 0)
    {
        if (STAFThreadSafeDecrement(fCount) == 0)
        {
            if (fType == SCALAR) delete fPtr;
            else if (fType == ARRAY) delete [] fPtr;
            else if (fType == CUSTOM) fFreeFunc(fPtr);
            else fArrayFreeFunc(fPtr, fSize);

            delete fCount;
        }
    }

    fPtr = from.fPtr;
    fType = from.fType;
    fFreeFunc = from.fFreeFunc;
    fSize = from.fSize;
    fCount = from.fCount;

    if (fCount != 0)
        STAFThreadSafeIncrement(fCount);

    return *this;
}

template <class TheType>
inline bool STAFRefPtr<TheType>::operator==(const STAFRefPtr &rhs)
{
    return (fPtr == rhs.fPtr);
}

template <class TheType>
inline STAFRefPtr<TheType>::~STAFRefPtr()
{
    if (fCount != 0)
    {
        if (STAFThreadSafeDecrement(fCount) == 0)
        {
            if (fType == SCALAR) delete fPtr;
            else if (fType == ARRAY) delete [] fPtr;
            else if (fType == CUSTOM) fFreeFunc(fPtr);
            else fArrayFreeFunc(fPtr, fSize);

            delete fCount;
        }
    }
}

// End #ifdef __cplusplus

#endif

#endif
