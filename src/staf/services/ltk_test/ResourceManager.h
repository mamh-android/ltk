#ifndef RESOURCE__MANAGER__H
#define RESOURCE__MANAGER__H
#include "DeviceData.h"
// Valid Request Types
enum RequestType {
	kRandom = 0,
	kFirst = 1,
	kEntry = 2
};

// Resource Data - contains data for a entry in a resource pool

class ResourceData
{
public:
ResourceData() : owned(0), garbageCollect(true)
{               /* Do Nothing */
}

//   ResourceData(const STAFString &aEntry, const STAFString &aPlatform,const STAFString &aIpAddr, const STAFString &aID) :
//       entry(device.), board_type(aPlatform),ipaddr(aIpAddr),ID(aID),owned(0), garbageCollect(true)
ResourceData(const DeviceDataPtr &input) :
	pDevice(input), owned(0), garbageCollect(true)
{               /* Do Nothing */
}

/*STAFString   entry;            // Entry value, currently is the board_id name
    STAFString   board_type;
    STAFString   ipaddr;
    STAFString   ID;*/
//	struct DeviceData device;
DeviceDataPtr pDevice;                  //use the ptr of device
unsigned int owned;                     // 0 means Available; 1 means Owned
STAFString taskName;                    // Originating request's taskname, showing which task occupy this device
STAFString requestedTime;               // Time made request
//std::vector<STAFString> preTasks;  //all the previous run in this resource
std::set<STAFString> preTasks;          //all the previous run in this resource
STAFString acquiredTime;                // Time acquired the resource
bool garbageCollect;                    // no use. true means perform garbage collection
					// false means no garbage collection
};
//its about the job queue etc
typedef std::vector<ResourceData> ResourceList;
typedef std::map<STAFString, std::map<STAFString, STAFString> > JobMap;

// Request Data - contains data for a pending request for a resource
class RequestData
{
public:
RequestData();

RequestData(const STAFString &aUUID, const STAFString &ataskName,
	    const std::vector<STAFString> &aCaseList, const JobMap &aJobMap,
	    const STAFString &aHandleName, const STAFHandle_t aHandle,
	    const STAFString aUser, const STAFString aEndpoint,
	    const bool aGarbageCollect, const bool aAsyncOrNot,
	    const RequestType aRequestType,
	    const STAFString &aRequestedEntry,
	    const int aPriority);

static const STAFString sConfigSep;
static const STAFString sAndSep;
static const STAFString sOrSep;
static bool existInPool(const DeviceDataPtr &pDevice, STAFString requestEntry);
STAFString orgUUID;                     // no use Originating request's STAF UUID
//STAFString      orgMachine;    // Originating request's machine name
STAFString taskName;                    // Originating request's Task Name
std::vector<STAFString> CaseList;        //the caselist should be assigned with taskname
JobMap jobMap;                          //the configuration of job, each task has the same on jobconfiguration
STAFString orgName;                     // no use Originating request's handle name
STAFHandle_t orgHandle;                 // no use Originating request's STAF handle
STAFString orgUser;                     // no use Originating request's user
STAFString orgEndpoint;                 // Originating request's endpoint
STAFString requestedTime;               // Originating request's date/time
STAFEventSemPtr wakeup;                 // Semaphore to wake up a pending request, is to be used for the wait method
STAFRC_t retCode;                       // Return code indicating if request was
					//   successfully satisfied
STAFString resultBuffer;                // Entry obtained if retCode = 0
STAFRefPtr<bool> garbageCollectedPtr;    //no use
bool garbageCollect;               // no use true means perform garbage collection
				   // false means no garbage collection
bool AsyncOrNot;                        //Async or timeout. 1: Async; 0:Timeout
RequestType requestType;                // Request type
STAFString requestedEntry;              // Entry, if requestType == kEntry
STAFString board_id;                    // the specified board_id while request mets
int priority;                           // Priority for the request, default is 50
};

typedef STAFRefPtr<RequestData> RequestDataPtr;
// RequestList  --  Ordered list of pending requests for a pool
typedef std::list<RequestDataPtr> RequestList;

typedef std::list<RequestDataPtr> ReadyList; //list all the ready list for a pool, which will be

#endif
