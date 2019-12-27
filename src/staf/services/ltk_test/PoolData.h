#ifndef POOL__DATA__H
#define POOL__DATA__H
#include "STAFString.h"
#include "ResourceManager.h"
class PoolData
{
public:
PoolData();

PoolData(const STAFString &aPoolName, const STAFString &aPoolDescription);

unsigned int fileFormat;                        // No use. Format of the pool file
						//  "0" - REXX version in STAF 2.2 and <
						//   1  - C++ version in STAF 2.3 and later
STAFString poolName;                            // Pool Name
STAFString poolDescription;                     // Pool Description
unsigned int numResources;                      // Total # of entries in ResourceList, used in Fist/Random mode
unsigned int usedResources;                     // # of entries used in ResourceList, used in First/Random Mode
ResourceList resourceList;                      // List of entries in a resource pool
RequestList requestList;                        // List of pending requests
ReadyList readyList;                            // List of ready requests
STAFMutexSemPtr accessSem;                      // Semaphore to control access to PoolData, currently it is used for pend, may add another sem for resourcelist
};
typedef STAFRefPtr<PoolData> PoolDataPtr;


// PoolMap -- KEY:   Pool name in upper case,
//            VALUE: Pointer to PoolData information

typedef std::map<STAFString, PoolDataPtr> PoolMap;

// Read/Write File Return Codes

enum ReadFileRC {
	kReadorWriteOk = 0,
	kReadEndOfFile = 1,
	kReadInvalidFormat = 2,
	kFileOpenError = 3
};


#endif
