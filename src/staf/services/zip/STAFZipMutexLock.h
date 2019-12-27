/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_ZIPMUTEXLOCK
#define STAF_ZIPMUTEXLOCK


#ifdef __cplusplus
extern "C"
{
#endif

struct ZipLock
{
    ZipLock() : lockSem(new STAFMutexSem, STAFMutexSemPtr::INIT),
                 numOwners(0)
    { /* Do Nothing */ }

    STAFMutexSemPtr lockSem;
    unsigned int numOwners;
};

typedef std::map<STAFString, ZipLock> LockMap;

#ifdef __cplusplus
}
#endif



class STAFZipMutexLock
{
private:

    // attributes
    LockMap sLockMap;

    STAFMutexSem sLockMapSem;

public:

    // methods
    STAFZipMutexLock()
    { /* Do Nothing */ }
        

    void request(STAFString);
    
    void release(STAFString);

    ~STAFZipMutexLock()
    { /* Do Nothing */ }

};


#endif
