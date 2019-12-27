/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004, 2005                                  */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/
#include "DeviceData.h"
#include "ResourceManager.h"
#include "LtkTest.h"
#include "JobData.h"
#include "PoolData.h"
#include "GuiData.h"

//for respool + job scheduel
#include "STAF_fstream.h"
#include "STAFTimestamp.h"
#include "STAFFileSystem.h"
//#include "STAFDataTypes.h"
ofstream out("LogOut.txt", std::ios_base::trunc);


// DEVICE Service Data
struct DeviceServiceData {
	unsigned int fDebugMode;                        // Debug Mode flag
	STAFString fShortName;                          // Short service name
	STAFString fName;                               // Registered service name
	STAFHandlePtr fHandlePtr;                       // Device service's STAF handle
	STAFString fLocalMachineName;                   // Local machine name
	STAFString fPoolDir;                            // Pool Directory which will write down the registeration information
	STAFCommandParserPtr fRegisterParser;           // DEVICE REGISTER command parser
	STAFCommandParserPtr fUnregisterParser; // DEVICE UNREGISTER command parser
        STAFCommandParserPtr fUnlockParser;// DEVICE UNLOCK command parser
        STAFCommandParserPtr fLockParser;// DEVICE UNLOCK command parser
        //STAFCommandParserPtr fRemoteControl;// DEVICE UNLOCK command parser
	STAFCommandParserPtr fJobStatusParser;          // job Status command parser
	STAFCommandParserPtr fQueryParser;              // RESPOOL QUERY command parser
	STAFCommandParserPtr fRequestParser;            // RESPOOL REQUEST command parser
	STAFCommandParserPtr fCancelParser;             // RESPOOL CACNEL command parser
	STAFCommandParserPtr fListParser;               // DEVICE LIST command parser
	STAFCommandParserPtr fRunParser;                // DEVICE RUN command parser
	STAFCommandParserPtr fHelpParser;               // DEVICE HELP command parser
	STAFCommandParserPtr fVersionParser;            // DEVICE VERSION command parser
	STAFCommandParserPtr fParmsParser;              // DEVIC PARMS command parser
	STAFCommandParserPtr fGuiRegisterParser;        // GUI SERVER  REGISTER command parser
	STAFCommandParserPtr fGuiUnregisterParser; // GUI SERVER  UNREGISTER command parser

	// Map Class Definitions for marshalled results
	STAFMapClassDefinitionPtr fListDeviceMapClass;
	STAFMapClassDefinitionPtr fListJobDataClass;
	STAFMapClassDefinitionPtr fQueryDeviceMapClass;
	STAFMapClassDefinitionPtr fProcessEndMapClass;

	STAFMutexSemPtr fDeviceMapSem;          // Semaphore to control access to the DeviceMap
	DeviceMap fDeviceMap;                   // Map of all Devices

	STAFRWSemPtr fPoolMapRWSem;             //read/write semaphore to control access to the PoolMap
	PoolMap fPoolMap;                       // Map of all resource pools

	STAFMutexSemPtr fJobListSem;            // Semaphore to control
	JobList fJobList;                       //list all the jobs

	STAFMutexSemPtr fGuiSetSem;             //semaphore to control
	//std::set<STAFString> fGuiSet;   //list all the guis
	GuiSet fGuiSet;                         //list all the guis
	// Map Class Definitions for marshalled results
	STAFMapClassDefinitionPtr fPoolClass;
	STAFMapClassDefinitionPtr fPoolInfoClass;
	STAFMapClassDefinitionPtr fSettingsClass;
	STAFMapClassDefinitionPtr fRequestClass;
	STAFMapClassDefinitionPtr fReadyClass;
	STAFMapClassDefinitionPtr fResourceClass;
	STAFMapClassDefinitionPtr fResourceOwnerClass;
};

// Static Variables
//////////////////////////////////////////////////////////
///////////////////////////for respool + job queue begins/////////
static const int sMinimumPriority = 1;
static const int sMaximumPriority = 99;
static const int sDefaultPriority = 50;
//process sync emu lbzhu
enum ProcessSync { kWait, kAsync };
static const unsigned int sCurrFileFormat = 1;

static STAFString sHelpMsg;
static STAFString sLineSep;
static const STAFString sVersionInfo("3.4.0");
static const STAFString sLocal("local");
static const STAFString sHelp("HELP");
static const STAFString sVar("VAR");
static const STAFString sResStrResolve("RESOLVE REQUEST ");
static const STAFString sString(" STRING ");
static const STAFString sLeftCurlyBrace(kUTF8_LCURLY);
static const STAFString sGarbageCollect("GARBAGECOLLECT");
static const STAFString sEntry("ENTRY");
static const STAFString sPriority("PRIORITY");
static const STAFString sTimeout("TIMEOUT");
static const STAFString sNo("No");
static const STAFString sYes("Yes");
static const STAFString sPoolName("cloudtest");
static const STAFString sPool("POOL");
//job related
static const STAFString sTaskName("TASKNAME");
static const STAFString sGeneral("General");
static const STAFString sImageConfig("ImageConfig");
static const STAFString sLTKConfig("LTKConfig");
static const STAFString sLogConfig("LogConfig");
static const STAFString sExtraAct("ExtraAct");
static const STAFString sConfigSep("=");
static const STAFString sCaseList("CaseList");
static STAFEventSem gEndSem;
static STAFEventSem gPDataSem;
static int fEventManagerID = 0;

// Prototypes TBD
//1. pending process about handleUnregister,handleRegister does NOT implement.
//2. write back all the registeration info into board_id.rpl does NOT implement before exit
//3. the board_id detect thread about board_id status instead of online, it we do NOT specify WORKDIR, the command will be started from whatever directory STAFProc is currently in
//    the STAFProc can be launched anywhere if the PATH of STAF is set into the PATH variable
//4. divide a task into two parts, request resource and run specified test case. it can request multi entries and run case then.
//5. add the xml parser about generate job.xml
static STAFResultPtr handleRegister(STAFServiceRequestLevel30 *,
				    DeviceServiceData *);
static STAFResultPtr handleUnregister(STAFServiceRequestLevel30 *,
				      DeviceServiceData *);
static STAFResultPtr handleUnlock(STAFServiceRequestLevel30 *,
                                  DeviceServiceData *);
static STAFResultPtr handleLock(STAFServiceRequestLevel30 *,
                                  DeviceServiceData *);
static STAFResultPtr handleJobUpdate(STAFServiceRequestLevel30 *,
				     DeviceServiceData *);
static STAFResultPtr handleQuery(STAFServiceRequestLevel30 *,
				 DeviceServiceData *);
static STAFResultPtr handleRequest(STAFServiceRequestLevel30 *,
				   DeviceServiceData *);
static STAFResultPtr handleCancel(STAFServiceRequestLevel30 *,
				  DeviceServiceData *);
static STAFResultPtr handleList(STAFServiceRequestLevel30 *,
				DeviceServiceData *);
static STAFResultPtr handleGuiRegister(STAFServiceRequestLevel30 *,
				       DeviceServiceData *);
static STAFResultPtr handleGuiUnregister(STAFServiceRequestLevel30 *,
					 DeviceServiceData *);
static STAFResultPtr handleRun(STAFServiceRequestLevel30 *,
			       DeviceServiceData *);
static STAFResultPtr handleHelp(STAFServiceRequestLevel30 *,
				DeviceServiceData *);
static STAFResultPtr handleVersion(STAFServiceRequestLevel30 *,
				   DeviceServiceData *);

static STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
				DeviceServiceData *pData,
				const STAFString &theString);

static STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo,
			       DeviceServiceData *pData,
			       STAFCommandParseResultPtr &parsedResult,
			       const STAFString &fOption,
			       unsigned int optionIndex = 1);

STAFResultPtr resolveOpLocal(DeviceServiceData *pData,
			     STAFCommandParseResultPtr &parsedResult,
			     const STAFString &fOption,
			     unsigned int optionIndex = 1);

static STAFResultPtr convertOptionStringToUInt( const STAFString &theString,
						const STAFString &optionName,
						unsigned int &number,
						const unsigned int minValue = 0,
						const unsigned int maxValue = UINT_MAX);

static STAFResultPtr resourceRelease( DeviceServiceData *pData, const STAFString &releaseEntry);
static void registerHelpData(DeviceServiceData *pData,
			     unsigned int errorNumber,
			     const STAFString &shortInfo,
			     const STAFString &longInfo);

static void unregisterHelpData(DeviceServiceData *pData,
			       unsigned int errorNumber);

static void registerShutDown(DeviceServiceData *pData);
static STAFResultPtr  runCaseWithTaskName(DeviceServiceData *pData, const STAFString &ipaddr, const STAFString &taskName, const std::vector<STAFString> &caseList, const JobMap &jobMap, const STAFString &board_id);

///////////////////////////////////////////////////////////////
//schedule dynamically, pending status etc begins//
static unsigned int readPoolFile(const STAFString &fileName, PoolData &poolData);
static unsigned int writePoolFile(const STAFString &fileName, PoolData &poolData);
static void readUIntFromFile(istream &input, unsigned int &data,
			     unsigned int length = 4);
static void writeUIntToFile(ostream &output, unsigned int data,
			    unsigned int length = 4);

static void readStringFromFile(istream &input, STAFString &inString);

static void writeStringToFile(ostream &output, STAFString &outString);
////////////////////////////////////////////////////////////////////////
///respool + job queue etc ends/////////////////////////////////////////

//some common function
void log(DeviceServiceData *pData, STAFString level, STAFString message)
{
	pData->fHandlePtr->submit("local", "LOG", "LOG MACHINE LOGNAME " + pData->fShortName +
				  " LEVEL " + level + " MESSAGE " + pData->fHandlePtr->wrapData(message));
}

//op---------0: board, 1: job
enum NotifyType { kNothing, kJob, kBoard, kDelay, kMaxValue };
void NotifyGui(DeviceServiceData *pData, int op, STAFString param, STAFString ipaddr = "local") //the params does NOT support space
{
	STAFString tempCmd("start command ./udp_server parms 127.0.0.1 6788 ");
	STAFString num(op);

	if (op >= kMaxValue) {
		DEBUGOUT("error op");
		return;
	}
	tempCmd += num;
	tempCmd += " ";
	tempCmd += param;
	DEBUGOUTFILE("tempCmd=" << tempCmd << " ipaddr=" << ipaddr);
//pData->fHandlePtr->submit("local","Process",tempCmd);
	pData->fHandlePtr->submit(ipaddr, "Process", tempCmd);
}

void handleProcessEndMsg(DeviceServiceData *pData, STAFObjectPtr queueMessageMap)
{
cout<<"handleProcessEndMsg"<<endl;
	STAFObjectPtr messageMap = queueMessageMap->get("message");
	STAFString handle = messageMap->get("handle")->asString();
	STAFString rc = messageMap->get("rc")->asString();
	STAFString endpoint = queueMessageMap->get("machine")->asString();

	// Free the process handle

//    STAFResult freeHandleResult = fHandle.submit2(
//        endpoint, "PROCESS", "FREE HANDLE " + handle);

	// Log a Pass or Fail message in the service log
	STAFString status = "";

	if (rc.isEqualTo("0"))
		status = "Pass";
	else
		status = "Fail";

	// Resolve the STAF/Config/Machine variable on the endpoint to get
	// the machine name which is used in the key for the fProcessHandleMap
	// and in logged messages
	DEBUGOUTFILE("status=" << status << " handle=" << handle << " rc=" << rc << " endpoint=" << endpoint);

	STAFString machineName = endpoint;

	DEBUGOUTFILE("before submit");
	STAFResultPtr varResult = pData->fHandlePtr->submit(
		endpoint, "VAR", "RESOLVE STRING {STAF/Config/Machine}");

	if (varResult->rc == kSTAFOk) //indicate pass
		machineName = varResult->result;

	DEBUGOUTFILE("after submit");

	// Create a formatted result for the process (like you get if the
	// WAIT option had been used on the PROCESS START request)
	STAFObjectPtr mc = STAFObject::createMarshallingContext();
	mc->setMapClassDefinition(pData->fListDeviceMapClass->reference());

	//STAFMarshallingContext mc = new STAFMarshallingContext();
	//mc.setMapClassDefinition(pData->fProcessEndMapClass->reference());

	// Not calling STAFMapClass createInstance method to avoid
	// getting an unchecked cast warning
	//Map processEndMap = fProcessEndMapClass.createInstance();
	STAFObjectPtr processEndMap = STAFObject::createMap();
	//Map<String, Object> processEndMap = new TreeMap<String, Object>();
	processEndMap->put("staf-map-class-name", "LTKTEST");

	processEndMap->put("rc", rc);

	STAFString key = messageMap->get("key")->asString();

	if (key.length() != 0)
		processEndMap->put("key", key);

	STAFObjectPtr fileList = messageMap->get("fileList");
	STAFString result;
//	DEBUGOUT("debug");
	if (fileList->type() == kSTAFListObject) {
//	DEBUGOUT("debug");
		// Get stdout/stderr information to return in the result
		for (STAFObjectIteratorPtr iter = fileList->iterate();
		     iter->hasNext();) {
			STAFObjectPtr fileMap = iter->next();

//	DEBUGOUT("debug");
			if (fileMap->type() == kSTAFMapObject) {
//	DEBUGOUT("debug");
				if (fileMap->get("rc")->asString() == "0")
//	DEBUGOUT("debug");
					result += fileMap->get("data")->asString().strip();
				else
					result += "Error getting stdout/stderr, rc=" +
						  fileMap->get("rc")->asString();
		     //	DEBUGOUT("debug,result="<<result);
			}
		}
	}

	processEndMap->put("fileList", result);


/*    List<Map<String, Object>> returnFileList =
	new ArrayList<Map<String, Object>>();
    List fileList = (List)messageMap.get("fileList");
    Iterator fileIter = fileList.iterator();

    while (fileIter.hasNext())
    {
	Map fileMap = (Map)fileIter.next();

	// Not calling STAFMapClass createInstance method to avoid
	// getting an unchecked cast warning
	//Map returnFileMap = fProcessReturnFileMapClass.createInstance();
	Map<String, Object> returnFileMap = new TreeMap<String, Object>();
	returnFileMap.put("staf-map-class-name",
			  fProcessReturnFileMapClass.name());
	returnFileMap.put("rc", (String)fileMap.get("rc"));
	returnFileMap.put("data", (String)fileMap.get("data"));
	returnFileList.add(returnFileMap);
    }

    processEndMap.put("fileList", returnFileList);*/

	mc->setRootObject(processEndMap);

	STAFString message = "[ID=";
	message += key;
	message += "] [";
	message += machineName;
	message += ":";
//			   message += reqNum;
	message += "] ";
	message += " output= ";
	message += result;
	message += " Process completed.\n";
//			   message += mc->marshall();
	DEBUGOUTFILE("status=" << status << " message=" << message);
	STAFString board_id;
	STAFString ipaddr;
	log(pData, status, message);
	{
		STAFMutexSemLock lock(*pData->fJobListSem);
		int i = 0;
		for (i = 0; i < pData->fJobList.size(); i++) {
			//if(pData->fJobList.at(i)->ID==key)
			if (pData->fJobList.at(i)->JobName == key) {
				DEBUGOUTFILE("PASS find the key=" << key);
				pData->fJobList.at(i)->statusUpdateTime = STAFTimestamp::now().asString();
				pData->fJobList.at(i)->status = status;

				pData->fJobList.at(i)->statusAll += status;
				pData->fJobList.at(i)->statusAll += ";";
				pData->fJobList.at(i)->output = result;
//				pData->fJobList.at(i)->output+="\nTC_LCD_01:pass\nTC_LCD_02:pass";
				board_id = pData->fJobList.at(i)->board_id;
				ipaddr = pData->fJobList.at(i)->ipaddr;
				break;
			}
		}
		if (i == pData->fJobList.size())
			DEBUGOUTFILE("can't find the key=" << key);
		else
			NotifyGui(pData, kJob, key, ipaddr);     //job update
	}
	//post action
	DEBUGOUTFILE("resource release begin");
	STAFResultPtr resultcmd = resourceRelease(pData, board_id);
	DEBUGOUTFILE("resource release end,ret=" << resultcmd->rc);

	DEBUGOUT("log process end");
}

//void run()
unsigned int run(void *data)
{
	DeviceServiceData *pData;

	pData = static_cast<DeviceServiceData *>(data);
	if (pData == NULL) {
		DEBUGOUT("input is null");
		return 1;
	}

	STAFResultPtr getResultPtr ;
	STAFObjectPtr queueMessageMap ;
	unsigned int continueRunning = 1;
	int numErrors = 0;

	// Maximum consecutive errors submitting a local QUEUE GET WAIT
	// request before we decide to exit the infinite loop
	int maxErrors = 5;

	while (continueRunning) {
		//it will return while the serive is entering term pharse
		/*if(1==gEndSem.query() ){
			DEBUGOUT("the gEndSem event is posted");
			break;}
		   else
			DEBUGOUT("the gEndSem event is NOT posted");*/
		// Need a try/catch block in case an error occurs getting
		// a message off the queue so we can continue processing
		// additional messages

		//try
		{

			DEBUGOUT("get wait begins");
			/*STAFString queueCmd("GET HANDLE ");
						       queueCmd+=pData->fHandlePtr->getHandle();
						       queueCmd+=" WAIT ";*/
			STAFString queueCmd("GET WAIT");
			DEBUGOUT("queueCmd=" << queueCmd);
			getResultPtr = pData->fHandlePtr->submit(
				"local", "QUEUE", queueCmd);
			//"local", "QUEUE", "GET WAIT 5s");
			DEBUGOUT("get wait ends,rc=" << getResultPtr->rc);

			if (getResultPtr->rc == kSTAFOk)
				numErrors = 0;
			else if (getResultPtr->rc == kSTAFTimeout ) { //the wait timeout, do NOT check the result queue message, since there is no message arrived
				numErrors = 0;
				continue;
			}else if (getResultPtr->rc == kSTAFHandleDoesNotExist ) {
				// If the handle doesn't exist, all submit requests
				// using this handle will fail so exit this thread
				continueRunning = 0; // Exit MonitorThread
				break;
			}else{
				numErrors++;

				STAFString msg;
				msg += "STAF local QUEUE GET WAIT failed with RC: ";
				msg += getResultPtr->rc;
				msg += ", Result: ";
				msg += getResultPtr->result;
				DEBUGOUT("msg=" << msg);
				if (numErrors < maxErrors)
					continue;
				else{
					msg = "Exiting MonitorThread after the QUEUE GET request failed ";
					msg += maxErrors;
					msg += " consecutive times";

					log(pData, "error", msg);

					continueRunning = false; // Exit MonitorThread
					break;
				}
			}
		} //end of try
		/*       catch (Throwable t)
		       {
			   numErrors++;

			   String msg = "Exception getting a message off the queue." +
			       " Exception:\n" + t.toString() + "\nResult from " +
			       "STAF local QUEUE GET WAIT request:\n" +
			       getResult.result;

			   t.printStackTrace();

			   fService.log("error", msg);

			   if (numErrors < maxErrors)
			   {
			       continue;
			   }
			   else
			   {
			       msg = "Exiting MonitorThread after the QUEUE GET " +
				   "request failed " + maxErrors +
				   " consecutive times";

			       fService.log("error", msg);

			       continueRunning = false;
			       break;
			   }
		       }*/

		// Need a try/catch block so can catch any errors that
		// occur when processing the queued message

//        try
		{
			queueMessageMap = getResultPtr->resultObj;

			STAFString queueType = queueMessageMap->get("type")->asString();
			DEBUGOUT("queuetype=" << queueType);
			if (queueType.length() == 0 )
				continue;
			else if (queueType.isEqualTo("STAF/RequestComplete", kSTAFStringCaseInsensitive))
				DEBUGOUTFILE("STAF/RequestComplete");
			else if (queueType.isEqualTo("STAF/PROCESS/END", kSTAFStringCaseInsensitive)) {
				DEBUGOUTFILE("STAF/PROCESS/END");
				handleProcessEndMsg(pData, queueMessageMap);
			}else if (queueType.isEqualTo("STAF/Shutdown", kSTAFStringCaseInsensitive)) {
				DEBUGOUTFILE("STAF/Shutdown");
				break;
			}
		}
		//STAFThreadManager::sleepCurrentThread(100);//sleep 0.1 second
	} //while
//    try
	{
		DEBUGOUTFILE("out of run thread");
		//fService.fHandle.unRegister();
	}
	return 0;
}

/**
 * This helper class encapsulates the data for a STAF service request
 * that has been triggered*/
unsigned int boarddetect(void *data)
{
	DeviceServiceData *pData;

	pData = static_cast<DeviceServiceData *>(data);
	if (pData == NULL) {
		DEBUGOUT("input is null");
		return 0;
	}


	DeviceMap tempDeviceMap_init_offline_count;
		{	
			STAFMutexSemLock lock(*pData->fDeviceMapSem);
			tempDeviceMap_init_offline_count=pData->fDeviceMap;
		}
        DeviceMap::iterator iter_init_offline_count;
        for (iter_init_offline_count = tempDeviceMap_init_offline_count.begin(); iter_init_offline_count != tempDeviceMap_init_offline_count.end(); ++iter_init_offline_count)
	{
		iter_init_offline_count->second->off_line_count = 0;
		iter_init_offline_count->second->task_lock = 0;
		iter_init_offline_count->second->under_task = 0;
	}
int detect_loop=0;
int detect_time_loop=0;
	while (1) {
		//DEBUGOUT(" fName="<<pData->fName<<" local name="<<pData->fLocalMachineName);
STAFTimestamp dec_time= STAFTimestamp::now();
int dec_month=dec_time.getMonth()*2592000;
int dec_day=dec_time.getMonth()*86400;
int dec_hour=dec_time.getHour()*3600;
int dec_min=dec_time.getMinute()*60;
int dec_sec=dec_time.getSecond();
int detect_time_start = dec_month+dec_day+dec_hour+dec_min+dec_sec;

detect_loop++;
		{
			DeviceMap tempDeviceMap;
			{
				STAFMutexSemLock lock(*pData->fDeviceMapSem);
				tempDeviceMap = pData->fDeviceMap;
			}
			//DEBUGOUTFILE("Thread " << STAFThreadManager::getCurrentThreadID()<<"releasing fDeviceMapSem lock");
			DeviceMap preDeviceMap = tempDeviceMap;
			DeviceMap::iterator iter;
/*
    unsigned int getYear() const;       // 4-digit year
    unsigned int getMonth() const;      // 1-12
    unsigned int getDay() const;        // 1-31
    unsigned int getHour() const;       // 0-23
    unsigned int getMinute() const;     // 0-59
    unsigned int getSecond() const;     // 0-59
*/

cout<<"===== Detect Board For Loop "<<detect_loop<<";board_num:"<<tempDeviceMap.size()<<";Time:"<<dec_time.asString()<<" ======"<<endl;
			for (iter = tempDeviceMap.begin(); iter != tempDeviceMap.end(); ++iter) {
				//STAFObjectPtr resultMap = pData->fListDeviceMapClass->createInstance();
				//	DEBUGOUTFILE("board_id="<<iter->second->board_id<<" board_type ="<<iter->second->board_type<<" physicalInterfaceID="<<iter->second->physicalInterfaceID);
STAFString board_status_before=iter->second->board;
				STAFResultPtr resultPtr =  pData->fHandlePtr->submit(
					iter->second->physicalInterfaceID, "PING", "PING" );
				if (resultPtr->rc != kSTAFOk) {
					//                      DEBUGOUTFILE("ping fails");
					//			fail=1;
					iter->second->online = "offline";
					//if(iter->second->status == "start"){
					//	iter->second->status = "Unknown";}
					iter->second->board = "OFF";
					//continue;
					//break;
				}else{
					//                      DEBUGOUTFILE("ping pass");
					STAFString boardCmd(" start command ./rcc_utilc PARMS query "); //the board detect binary is hard-coded rcc_utilc
					boardCmd += iter->second->board_id;
					boardCmd += " onlineStatus WAIT 5s STDERRTOSTDOUT RETURNSTDOUT";
					iter->second->online = "online";
					//check the board_id status
					STAFResultPtr resultPtr =  pData->fHandlePtr->submit(
						iter->second->physicalInterfaceID, "PROCESS", boardCmd );
					if (resultPtr->rc != kSTAFOk) {
						if (resultPtr->rc == kSTAFTimeout) //timeout, 5s
							iter->second->board = "query timeout";
							//					DEBUGOUTFILE("query tiemout");
						else
							iter->second->board = "query fail";
							//					DEBUGOUTFILE("query fail");
					}else{ //query pass, check the board status, the result map is STAF/Service/Process/CompletionInfo and STAF/Service/Process/ReturnFileInfo,
						//	the fileList is LIST type. get the map result then get the list result of data.
						STAFString sMapClassKey("staf-map-class-name");
						if (kSTAFMapObject == resultPtr->resultObj->type()) {
							STAFObjectPtr temp = resultPtr->resultObj->get("fileList");
							if (kSTAFListObject == temp->type()) { //just show the first list value into the
								STAFObjectIteratorPtr iterPtr = temp->iterate();
								STAFObjectPtr thisObj = iterPtr->next();
								iter->second->board = thisObj->get("data")->asString().replace("\n", " ");
								//					DEBUGOUTFILE("query pass"<<thisObj->get("data")->asString().replace("\n"," "));//it is list object
							}else{
								//					DEBUGOUTFILE("query pass, the filelist type is NOT List");//it is list object
							}
						}else{
							//					DEBUGOUTFILE("query pass, the resultObj type is NOT MAP");//it is list object
						}
					}
				}

STAFString board_status_after=iter->second->board;
if ( board_status_after != board_status_before )
{
	std::set<GuiData>::iterator it_gui;
	{
		STAFMutexSemLock lock(*pData->fGuiSetSem);
		for (it_gui = pData->fGuiSet.begin(); it_gui != pData->fGuiSet.end(); it_gui++)
			NotifyGui(pData, 2, "refresh_list", it_gui->physicalInterfaceID);
	}
}
if ( iter->second->board.find("Online",0) == -1 )
{
	//iter->second->off_line_count++;
	iter->second->off_line_count+=detect_time_loop;
}else
{
	iter->second->off_line_count=0;
}
cout << "check:" << iter->second->board_id << "|offline: " << iter->second->off_line_count << endl ;
//if ( iter->second->off_line_count > 500 )
if ( iter->second->off_line_count > 43200 )
{
	cout << "remove_ResourceList:" << iter->second->board_id << endl ;

	bool del_board=false;
	std::vector<STAFString> removeVec;
	DeviceServiceData *pData1 = reinterpret_cast<DeviceServiceData *>(data);
	//STAFRWSemWLock wLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;};
	PoolMap::iterator poolIterator;
	poolIterator = pData1->fPoolMap.find(sPoolName.toUpperCase());
	PoolDataPtr poolPtr = (*poolIterator).second;
	ResourceList::iterator resIterator;
	for (resIterator = poolPtr->resourceList.begin(); resIterator != poolPtr->resourceList.end(); resIterator++) 
	{
		//cout << "loop:" << (*resIterator).pDevice->board_id << endl ;
		if ((*resIterator).pDevice->board_id == iter->second->board_id)
		{
			poolPtr->resourceList.erase(resIterator);
			if (poolPtr->resourceList.size() == 0)
				fEventManagerID = 0;
			del_board=true;
			break;
		}
	}
	if ( del_board )
	{
		cout << "remove_fDeviceMap:" << iter->second->board_id << endl ;
		removeVec.push_back(iter->second->ID);
		for(int i=0;i<removeVec.size();i++)
		{
pData->fDeviceMap.erase(removeVec.at(i));
//tempDeviceMap.erase(removeVec.at(i));
}
		STAFFSPath poolFilePath;
		poolFilePath.setRoot(pData->fPoolDir);
		poolFilePath.setName("board_id");
		poolFilePath.setExtension("rpl");
		poolFilePath.getEntry()->remove();
		STAFString fileName = poolFilePath.asString();
		if (writePoolFile(fileName, (*poolPtr)) != kReadorWriteOk)
		{
				//return STAFResultPtr(new STAFResult(kSTAFFileWriteError, fileName),
                             //STAFResultPtr::INIT);
		}
	}
}







			} //for





			if (tempDeviceMap != preDeviceMap) {
				DEBUGOUTFILE("needs update");
				cout<<"#### update pData->fDeviceMap ###";
				STAFMutexSemLock lock(*pData->fDeviceMapSem);
				pData->fDeviceMap = tempDeviceMap;
				DEBUGOUTFILE("update done");
			}

		} //part 1 to do ping actions
		{
			std::vector<STAFString> releaseBoardVec;

			{
				// Make sure that the resource pool is in pData->poolMap
				{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}
				PoolMap::iterator poolIterator;
				PoolDataPtr poolPtr;
				poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());

				if (poolIterator == pData->fPoolMap.end()) {
					DEBUGOUTFILE("can't get the pool map");
					continue;
				}

				poolPtr = (*poolIterator).second;
				// Lock the poolData semaphore for the duration of this block
				STAFMutexSemLock lock(*poolPtr->accessSem);
				if (poolPtr->readyList.size() > 0) {
					ReadyList::iterator iter;
					RequestDataPtr reqPtr;

					for (iter = poolPtr->readyList.begin();
					     iter != poolPtr->readyList.end(); ++iter) {
						reqPtr = *iter;
						// Lock the poolData semaphore for the duration of this block
						//		STAFMutexSemLock lock(*poolPtr->accessSem);
						DEBUGOUTFILE("launch test case by delay deployment");
						STAFString delayComp(reqPtr->taskName);
						delayComp += ";";
						delayComp += reqPtr->board_id;
						NotifyGui(pData, kDelay, delayComp, reqPtr->orgEndpoint); //notify GUI about the board assignment
						STAFResultPtr resultcmd = runCaseWithTaskName(pData, reqPtr->orgEndpoint, reqPtr->taskName, reqPtr->CaseList, reqPtr->jobMap, reqPtr->board_id);
						if (resultcmd->rc != kSTAFOk) {
							DEBUGOUTFILE("push back the board_id into releaseBoardVec");
							releaseBoardVec.push_back(reqPtr->board_id);

						}
					} //end of for
					poolPtr->readyList.clear(); //remove all the elements from the ready list after all req job has been dispatched
				}
			} //lock life scope
			if (releaseBoardVec.size() > 0) {
				for (int i = 0; i < releaseBoardVec.size(); i++) {
					DEBUGOUTFILE("resource release begin becase run case fails, board_id=" << releaseBoardVec.at(i));
					STAFResultPtr resultcmd = resourceRelease(pData, releaseBoardVec.at(i));
					DEBUGOUTFILE("resource release end,ret=" << resultcmd->rc);
				}
			}else{
				//	DEBUGOUTFILE("do NOT need resource release since job finishes normally");
			}
		} //part 2 to do job dispath
		 //DEBUGOUT("Thread " << STAFThreadManager::getCurrentThreadID()<<"acquired fDeviceMapSem lock and sleeping for 5 seconds");
		STAFThreadManager::sleepCurrentThread(5000);
dec_time= STAFTimestamp::now();
dec_month=dec_time.getMonth()*2592000;
dec_day=dec_time.getMonth()*86400;
dec_hour=dec_time.getHour()*3600;
dec_min=dec_time.getMinute()*60;
dec_sec=dec_time.getSecond();
int detect_time_end = dec_month+dec_day+dec_hour+dec_min+dec_sec;
detect_time_loop=detect_time_end-detect_time_start;
		//DEBUGOUT("Thread " << STAFThreadManager::getCurrentThreadID()<<"releasing fDeviceMapSem lock");
	} //end of while 1

	DEBUGOUTFILE("the boarddetect  is exit");
	return 0;
}

// Begin implementation

STAFRC_t STAFServiceGetLevelBounds(unsigned int levelID,
				   unsigned int *minimum,
				   unsigned int *maximum)
{
	switch (levelID) {
	case kServiceInfo:
	{
		*minimum = 30;
		*maximum = 30;
		break;
	}
	case kServiceInit:
	{
		*minimum = 30;
		*maximum = 30;
		break;
	}
	case kServiceAcceptRequest:
	{
		*minimum = 30;
		*maximum = 30;
		break;
	}
	case kServiceTerm:
	case kServiceDestruct:
	{
		*minimum = 0;
		*maximum = 0;
		break;
	}
	default:
	{
		return kSTAFInvalidAPILevel;
	}
	}

	return kSTAFOk;
}


STAFRC_t STAFServiceConstruct(STAFServiceHandle_t *pServiceHandle,
			      void *pServiceInfo, unsigned int infoLevel,
			      STAFString_t *pErrorBuffer)
{
	STAFRC_t rc = kSTAFUnknownError;

	try
	{
		if (infoLevel != 30) return kSTAFInvalidAPILevel;

		STAFServiceInfoLevel30 *pInfo =
			reinterpret_cast<STAFServiceInfoLevel30 *>(pServiceInfo);

		DeviceServiceData data;
		data.fDebugMode = 0;
		data.fShortName = pInfo->name;
		data.fName = "STAF/Service/";
		data.fName += pInfo->name;

		for (unsigned int i = 0; i < pInfo->numOptions; ++i) {
			if (STAFString(pInfo->pOptionName[i]).upperCase() == "DEBUG")
				data.fDebugMode = 1;
			else{
				STAFString optionError(pInfo->pOptionName[i]);
				*pErrorBuffer = optionError.adoptImpl();
				return kSTAFServiceConfigurationError;
			}
		}

		// Set service handle

		*pServiceHandle = new DeviceServiceData(data);

		return kSTAFOk;
	}
	catch (STAFException &e)
	{
		STAFString result;

		result += STAFString("In DeviceService.cpp: STAFServiceConstruct")
			  + kUTF8_SCOLON;

		result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
		result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
		result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
		result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

		*pErrorBuffer = result.adoptImpl();
	}
	catch (...)
	{
		STAFString error("DeviceService.cpp: STAFServiceConstruct: "
				 "Caught unknown exception");
		*pErrorBuffer = error.adoptImpl();
	}

	return kSTAFUnknownError;
}


STAFRC_t STAFServiceInit(STAFServiceHandle_t serviceHandle,
			 void *pInitInfo, unsigned int initLevel,
			 STAFString_t *pErrorBuffer)
{
	STAFRC_t retCode = kSTAFUnknownError;

	try
	{
		if (initLevel != 30) return kSTAFInvalidAPILevel;

		DeviceServiceData *pData =
			reinterpret_cast<DeviceServiceData *>(serviceHandle);

		STAFServiceInitLevel30 *pInfo =
			reinterpret_cast<STAFServiceInitLevel30 *>(pInitInfo);

		retCode = STAFHandle::create(pData->fName, pData->fHandlePtr);

		DEBUGOUT("handle =" << pData->fHandlePtr->getHandle());

		if (retCode != kSTAFOk)
			return retCode;

		//ADD options

		pData->fRegisterParser = STAFCommandParserPtr(new STAFCommandParser,
							      STAFCommandParserPtr::INIT);
		pData->fRegisterParser->addOption("REGISTER", 1, STAFCommandParser::kValueNotAllowed);
		pData->fRegisterParser->addOption("BOARD_ID", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("BOARD_TYPE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("CHIP_NAME", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("CHIP_STEPPING", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("BOARD_REGISTER_DATE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("BOARD_STATUS", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("USER_TEAM", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("CURRENT_USER", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("BOARD_ECO", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("LCD_RESOLUTION", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("LCD_SCREENSIZE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("DDR_TYPE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("DDR_SIZE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("EMMC_TYPE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("EMMC_SIZE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("RF_NAME", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("RF_TYPE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("SERIAL", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("MCU", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("USB", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("USERNAME", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("USERTEAM", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("DRO", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOption("CHIP_TYPE", 1, STAFCommandParser::kValueRequired);
		pData->fRegisterParser->addOptionNeed("REGISTER", "BOARD_ID");
		pData->fRegisterParser->addOptionNeed("REGISTER", "BOARD_TYPE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "CHIP_NAME");
		pData->fRegisterParser->addOptionNeed("REGISTER", "CHIP_STEPPING");
		pData->fRegisterParser->addOptionNeed("REGISTER", "BOARD_REGISTER_DATE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "BOARD_STATUS");
		pData->fRegisterParser->addOptionNeed("REGISTER", "USER_TEAM");
		pData->fRegisterParser->addOptionNeed("REGISTER", "CURRENT_USER");
		pData->fRegisterParser->addOptionNeed("REGISTER", "BOARD_ECO");
		pData->fRegisterParser->addOptionNeed("REGISTER", "LCD_RESOLUTION");
		pData->fRegisterParser->addOptionNeed("REGISTER", "LCD_SCREENSIZE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "DDR_TYPE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "DDR_SIZE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "EMMC_TYPE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "EMMC_SIZE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "RF_NAME");
		pData->fRegisterParser->addOptionNeed("REGISTER", "RF_TYPE");
		pData->fRegisterParser->addOptionNeed("REGISTER", "SERIAL");
		pData->fRegisterParser->addOptionNeed("REGISTER", "MCU");
		pData->fRegisterParser->addOptionNeed("REGISTER", "USB");

		//UNREGISTER options

		pData->fUnregisterParser = STAFCommandParserPtr(new STAFCommandParser,
								STAFCommandParserPtr::INIT);
		pData->fUnregisterParser->addOption("UNREGISTER", 1,
						    STAFCommandParser::kValueNotAllowed);
		pData->fUnregisterParser->addOption("BOARD_ID", 1,
						    STAFCommandParser::kValueRequired);
		pData->fUnregisterParser->addOption("FORCE", 1,
						    STAFCommandParser::kValueNotAllowed);
		pData->fUnregisterParser->addOption("MATCH", 1,
						    STAFCommandParser::kValueNotAllowed);
		pData->fUnregisterParser->addOptionNeed("UNREGISTER", "BOARD_ID");

		//UNlock options

		pData->fUnlockParser = STAFCommandParserPtr(new STAFCommandParser,
		                                         STAFCommandParserPtr::INIT);
		pData->fUnlockParser->addOption("UNLOCK", 1,
		                                STAFCommandParser::kValueNotAllowed);
		pData->fUnlockParser->addOption("BOARD_ID", 1,
		                                STAFCommandParser::kValueRequired);
		pData->fUnlockParser->addOption("FORCE", 1,
		                                STAFCommandParser::kValueNotAllowed);
		pData->fUnlockParser->addOption("MATCH", 1,
		                                STAFCommandParser::kValueNotAllowed);
		pData->fUnlockParser->addOptionNeed("UNLOCK", "BOARD_ID");

		//lock options

		pData->fLockParser = STAFCommandParserPtr(new STAFCommandParser,
		                                         STAFCommandParserPtr::INIT);
		pData->fLockParser->addOption("LOCK", 1,
		                                STAFCommandParser::kValueNotAllowed);
		pData->fLockParser->addOption("BOARD_ID", 1,
		                                STAFCommandParser::kValueRequired);
		pData->fLockParser->addOption("FORCE", 1,
		                                STAFCommandParser::kValueNotAllowed);
		pData->fLockParser->addOption("MATCH", 1,
		                                STAFCommandParser::kValueNotAllowed);
		pData->fLockParser->addOptionNeed("LOCK", "BOARD_ID");


		//JOBSTATUS options
		pData->fJobStatusParser = STAFCommandParserPtr(new STAFCommandParser,
							       STAFCommandParserPtr::INIT);
		pData->fJobStatusParser->addOption("JOBUPDATE", 1,
						   STAFCommandParser::kValueNotAllowed);
		pData->fJobStatusParser->addOption("JOBNAME", 1,
						   STAFCommandParser::kValueRequired);
		pData->fJobStatusParser->addOption("JOBSTATUS", 1,
						   STAFCommandParser::kValueRequired);
		pData->fJobStatusParser->addOptionNeed("JOBUPDATE", "JOBNAME");
		pData->fJobStatusParser->addOptionNeed("JOBUPDATE", "JOBSTATUS");

		//QUERY options
		pData->fQueryParser = STAFCommandParserPtr(new STAFCommandParser,
							   STAFCommandParserPtr::INIT);
		pData->fQueryParser->addOption("QUERY", 1,
					       STAFCommandParser::kValueNotAllowed);
		pData->fQueryParser->addOption("PENDING", 1,
					       STAFCommandParser::kValueNotAllowed);
		pData->fQueryParser->addOption("POOL", 1,
					       STAFCommandParser::kValueRequired);
		//pData->fQueryParser->addOptionNeed("QUERY", "POOL");

		//REQUEST options
		pData->fRequestParser = STAFCommandParserPtr(new STAFCommandParser,
							     STAFCommandParserPtr::INIT);
		pData->fRequestParser->addOption("REQUEST", 1,
						 STAFCommandParser::kValueNotAllowed);
		pData->fRequestParser->addOption("POOL", 1,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption("TASKNAME", 1,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption("ASYNC", 1,
						 STAFCommandParser::kValueNotAllowed); //lbzhu add ASYNC
		pData->fRequestParser->addOption("TIMEOUT", 1,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption("FIRST", 1,
						 STAFCommandParser::kValueNotAllowed);
		pData->fRequestParser->addOption("RANDOM", 1,
						 STAFCommandParser::kValueNotAllowed);
		pData->fRequestParser->addOption("ENTRY", 1,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption("RELEASE", 1,
						 STAFCommandParser::kValueNotAllowed);
		pData->fRequestParser->addOption("PRIORITY", 1,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption("GARBAGECOLLECT", 1,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption("NOTRUN", 1,
						 STAFCommandParser::kValueNotAllowed); //just for debug usage() which will not run test case
		pData->fRequestParser->addOption("CASENAME", 0,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption(sGeneral, 0,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption(sImageConfig, 0,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption(sLTKConfig, 0,
						 STAFCommandParser::kValueRequired);
                pData->fRequestParser->addOption(sLogConfig, 0,
                                                 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOption(sExtraAct, 0,
						 STAFCommandParser::kValueRequired);
		pData->fRequestParser->addOptionGroup("FIRST RANDOM ENTRY", 0, 1);
		pData->fRequestParser->addOptionGroup("TIMEOUT ASYNC", 0, 1);
		//pData->fRequestParser->addOptionNeed("REQUEST", "POOL");
		pData->fRequestParser->addOptionNeed("REQUEST", "TASKNAME");
		pData->fRequestParser->addOptionNeed("RELEASE", "ENTRY");
		pData->fRequestParser->addOptionNeed("REQUEST", "CASENAME");

		//LIST options
		pData->fListParser = STAFCommandParserPtr(new STAFCommandParser,
							  STAFCommandParserPtr::INIT);
		pData->fListParser->addOption("LIST", 1,
					      STAFCommandParser::kValueNotAllowed);
		pData->fListParser->addOption("BOARD_ID", 1,
					      STAFCommandParser::kValueRequired);
		pData->fListParser->addOption("JOBS", 1,
					      STAFCommandParser::kValueNotAllowed);
		pData->fListParser->addOption("TASKNAME", 1,
					      STAFCommandParser::kValueRequired);
		pData->fListParser->addOption("GUIS", 1,
					      STAFCommandParser::kValueNotAllowed);
		pData->fListParser->addOptionNeed("TASKNAME JOBS BOARD_ID GUIS ", "LIST");


		//Cancel  options----cancel the pending request
		pData->fCancelParser = STAFCommandParserPtr(new STAFCommandParser,
							    STAFCommandParserPtr::INIT);
		pData->fCancelParser->addOption("CANCEL", 1,
						STAFCommandParser::kValueNotAllowed);
		pData->fCancelParser->addOption("ENTRY", 1,
						STAFCommandParser::kValueRequired);
		pData->fCancelParser->addOption("BOARD_ID", 1,
					      STAFCommandParser::kValueRequired);
		pData->fCancelParser->addOption("TASK_NAME", 1,
					      STAFCommandParser::kValueRequired);
		pData->fCancelParser->addOption("ENTRY_IP", 1,
						STAFCommandParser::kValueRequired);
		pData->fCancelParser->addOption("FIRST", 1,
						STAFCommandParser::kValueNotAllowed);
		pData->fCancelParser->addOption("LAST", 1,
						STAFCommandParser::kValueNotAllowed);
		pData->fCancelParser->addOptionNeed("ENTRY FIRST LAST BOARD_ID TASK_NAME ENTRY_IP", "CANCEL");
		//pData->fCancelParser->addOptionNeed("CANCEL", "ENTRY");
		//pData->fCancelParser->addOptionNeed("CANCEL", "BOARD_ID");
		//pData->fCancelParser->addOptionGroup("FIRST RANDOM ", 0, 1);


		//RunCase options,casename is unlimited
		pData->fRunParser = STAFCommandParserPtr(new STAFCommandParser,
							 STAFCommandParserPtr::INIT);
		pData->fRunParser->addOption("RUN", 1,
					     STAFCommandParser::kValueNotAllowed);
		pData->fRunParser->addOption("CASENAME", 0,
					     STAFCommandParser::kValueRequired);
		pData->fRunParser->addOption("TASKNAME", 1,
					     STAFCommandParser::kValueRequired);
		pData->fRunParser->addOptionNeed("RUN", "CASENAME");
		pData->fRunParser->addOptionNeed("RUN", "TASKNAME");

		//HELP options
		pData->fHelpParser = STAFCommandParserPtr(new STAFCommandParser,
							  STAFCommandParserPtr::INIT);
		pData->fHelpParser->addOption("HELP", 1,
					      STAFCommandParser::kValueNotAllowed);

		//GUIREGISTER options
		pData->fGuiRegisterParser = STAFCommandParserPtr(new STAFCommandParser,
								 STAFCommandParserPtr::INIT);
		pData->fGuiRegisterParser->addOption("GUIREGISTER", 1,
						     STAFCommandParser::kValueNotAllowed);

		//HELP options
		pData->fGuiUnregisterParser = STAFCommandParserPtr(new STAFCommandParser,
								   STAFCommandParserPtr::INIT);
		pData->fGuiUnregisterParser->addOption("GUIUNREGISTER", 1,
						       STAFCommandParser::kValueNotAllowed);

		//VERSION options

		pData->fVersionParser = STAFCommandParserPtr(new STAFCommandParser,
							     STAFCommandParserPtr::INIT);
		pData->fVersionParser->addOption("VERSION", 1,
						 STAFCommandParser::kValueNotAllowed);

		// Construct map class for the result from a LIST request.
		pData->fListDeviceMapClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/ListDevice");

		pData->fListDeviceMapClass->addKey("board_id",    "(EEPROM)board_id");
//        pData->fListDeviceMapClass->addKey("type",    "Type");
		pData->fListDeviceMapClass->addKey("board_type",   "(EEPROM)board_type");
		pData->fListDeviceMapClass->addKey("chip_name",   "(EEPROM)chip_name");
		pData->fListDeviceMapClass->addKey("chip_stepping",   "(EEPROM)chip_stepping");
		pData->fListDeviceMapClass->addKey("board_register_date",   "(EEPROM)board_register_date");
		pData->fListDeviceMapClass->addKey("board_status",   "(EEPROM)board_status");
		pData->fListDeviceMapClass->addKey("user_team",   "(EEPROM)user_team");
		pData->fListDeviceMapClass->addKey("current_user",   "(EEPROM)current_user");
		pData->fListDeviceMapClass->addKey("board_eco",   "(EEPROM)board_eco");
		pData->fListDeviceMapClass->addKey("lcd_resolution",   "(EEPROM)lcd_resolution");
		pData->fListDeviceMapClass->addKey("lcd_screensize",   "(EEPROM)lcd_screensize");
		pData->fListDeviceMapClass->addKey("ddr_type",   "(EEPROM)ddr_type");
		pData->fListDeviceMapClass->addKey("ddr_size",   "(EEPROM)ddr_size");
		pData->fListDeviceMapClass->addKey("emmc_type",   "(EEPROM)emmc_type");
		pData->fListDeviceMapClass->addKey("emmc_size",   "(EEPROM)emmc_size");
		pData->fListDeviceMapClass->addKey("rf_name",   "(EEPROM)rf_name");
		pData->fListDeviceMapClass->addKey("rf_type",   "(EEPROM)rf_type");
		pData->fListDeviceMapClass->addKey("dro",   "(EXTRA_INFO)DRO");
		pData->fListDeviceMapClass->addKey("chip_type",   "(EXTRA_INFO)CHIP_TYPE");
		pData->fListDeviceMapClass->addKey("serial",   "(CONNECTED)serial");
		pData->fListDeviceMapClass->addKey("mcu",   "(CONNECTED)mcu");
		pData->fListDeviceMapClass->addKey("usb",   "(CONNECTED)usb");
		pData->fListDeviceMapClass->addKey("ID",   "(AUTO_INCREMENT)ID");
		pData->fListDeviceMapClass->addKey("machine",   "(AUTO_DETECT)machine");
		pData->fListDeviceMapClass->addKey("physicalInterfaceID",   "(AUTO_DETECT)physicalInterfaceID");
		//pData->fListDeviceMapClass->addKey("status",   "(AUTO_DETECT)status");
		pData->fListDeviceMapClass->addKey("online",   "(AUTO_DETECT)StafConnection");
		pData->fListDeviceMapClass->addKey("board",   "(AUTO_DETECT)BoardConnection");
		pData->fListDeviceMapClass->addKey("shareStartTime",   "(AUTO_DETECT)shareStartTime");
		pData->fListDeviceMapClass->addKey("username",   "(AUTO_DETECT)username");
		pData->fListDeviceMapClass->addKey("userteam",   "(AUTO_DETECT)userteam");
		pData->fListDeviceMapClass->addKey("task_lock",   "(AUTO_DETECT)task_lock");
		pData->fListDeviceMapClass->addKey("under_task",   "(AUTO_DETECT)job_running");
		//construct for fListJobDataClass, list all job status
		pData->fListJobDataClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/ListJob");

		pData->fListJobDataClass->addKey("JobName",   "(AUTO_GENERATE)JobName");
		pData->fListJobDataClass->addKey("ID",   "(AUTO_GENERATE)ID");
		pData->fListJobDataClass->addKey("board_id",   "(AUTO_GENERATE)board_id");
		pData->fListJobDataClass->addKey("ipaddr",   "(AUTO_GENERATE)ipaddr");
		pData->fListJobDataClass->addKey("taskName",   "(AUTO_GENERATE)taskname");
		pData->fListJobDataClass->addKey("caseName",   "(AUTO_GENERATE)caseName");
		pData->fListJobDataClass->addKey("startTime",   "(AUTO_GENERATE)startTime");
		pData->fListJobDataClass->addKey("statusUpdateTime",   "(AUTO_GENERATE)statusUpdateTime");
		pData->fListJobDataClass->addKey("status",   "(AUTO_DETECT)status");
		pData->fListJobDataClass->addKey("statusAll",   "(AUTO_DETECT)statusAll");
		pData->fListJobDataClass->addKey("output",   "(AUTO_DETECT)output");

		// Construct map class for the result from a QUERY request.

		pData->fQueryDeviceMapClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/QueryDevice");

		pData->fQueryDeviceMapClass->addKey("board_type",   "board_type");
		// Construct the map class for detailed pool information output

		pData->fPoolInfoClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/PoolInfo");

		pData->fPoolInfoClass->addKey("description",  "Description");
		pData->fPoolInfoClass->addKey("requestList",  "Pending Requests");
		pData->fPoolInfoClass->addKey("readyList",  "Ready Requests");
		pData->fPoolInfoClass->addKey("resourceList", "Resources");

		// Construct map class for a pending request
		pData->fRequestClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/Request");
		pData->fRequestClass->addKey("priority", "Priority");
		pData->fRequestClass->addKey("requestFromIpaddr", "requestFromIpaddr");
		pData->fRequestClass->addKey("requestedTimestamp",
					     "Date-Time Requested");
		pData->fRequestClass->addKey("requestedEntry", "Requested Entry");
		pData->fRequestClass->addKey("taskName",    "taskName");

		// Construct map class for a ready request
		pData->fReadyClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/Ready");
		pData->fReadyClass->addKey("priority", "Priority");
		pData->fReadyClass->addKey("requestFromIpaddr", "requestFromIpaddr");
		pData->fReadyClass->addKey("requestedTimestamp",
					   "Date-Time Requested");
		pData->fReadyClass->addKey("requestedEntry", "Requested Entry");
		pData->fReadyClass->addKey("taskName",    "taskName");

		// Construct map class for a resource owner
		pData->fResourceOwnerClass = STAFMapClassDefinition::create(
			"STAF/Service/ResPool/ResourceOwner");

		pData->fResourceOwnerClass->addKey("taskName",    "taskName");
		// Construct map class for a resource

		pData->fResourceClass = STAFMapClassDefinition::create(
			"STAF/Service/ResPool/Resource");

		pData->fResourceClass->addKey("entry", "Entry");
		pData->fResourceClass->addKey("owner", "Owner");
		pData->fResourceClass->addKey("preTasks", "PreTasks");

		//Construct map class for the result from process end.
		pData->fProcessEndMapClass = STAFMapClassDefinition::create(
			"STAF/Service/LTKTEST/ProcessEnd");

		pData->fProcessEndMapClass->addKey("rc", "Return Code");
		pData->fProcessEndMapClass->addKey("key", "Key");
		pData->fProcessEndMapClass->addKey("fileList", "Files");



		// Resolve the line separator variable for the local machine

		STAFResultPtr result = pData->fHandlePtr->submit(
			"local", "VAR", "RESOLVE STRING {STAF/Config/Sep/Line}");

		if (result->rc != kSTAFOk) {
			*pErrorBuffer = result->result.adoptImpl();
			return result->rc;
		}else sLineSep = result->result;

		// Resolve the machine name variable for the local machine

		result = pData->fHandlePtr->submit(
			"local", "VAR", "RESOLVE STRING {STAF/Config/Machine}");

		if (result->rc != kSTAFOk) {
			*pErrorBuffer = result->result.adoptImpl();
			return result->rc;
		}else pData->fLocalMachineName = result->result;

		// Create mutex semaphores for the printer and modem data maps
		pData->fDeviceMapSem = STAFMutexSemPtr(
			new STAFMutexSem, STAFMutexSemPtr::INIT);
		pData->fJobListSem = STAFMutexSemPtr(
			new STAFMutexSem, STAFMutexSemPtr::INIT);
		pData->fGuiSetSem = STAFMutexSemPtr(
			new STAFMutexSem, STAFMutexSemPtr::INIT);
		// Assign the help text string for the service

		sHelpMsg = STAFString("*** ") + pData->fShortName + " Service Help ***" +
			   sLineSep + sLineSep +
//            "RUN TASKNAME <taskname>	CASENAME <caseid> [CASENAME <caseid>] ... " +
			   sLineSep +
//            "ADD     < PRINTER <PrinterName> | MODEM <ModemName> > MODEL <Model> SN <Serial#>" +
			   "REGISTER    BOARD_ID <board_id>  BOARD_TYPE <board_type>  CHIP_NAME <chip_name> CHIP_STEPPING <chip_stepping> BOARD_REGISTER_DATE <board_register_date> BOARD_STATUS <board_status> USER_TEAM <user_team> CURRENT_USER <current_user> BOARD_ECO <board_eco> LCD_RESOLUTION <lcd_resolution> LCD_SCREENSIZE <lcd_screensize> DDR_TYPE <ddr_type> DDR_SIZE <ddr_size> EMMC_TYPE <emmc_type> EMMC_SIZE <emmc_size> RF_NAME <rf_name> RF_TYPE <rf_type> SERIAL <serial> MCU <mcu> USB <usb> USERNAME <username> USERTEAM <userteam> DRO <dro> CHIP_TYPE <chip_type>" +
			   sLineSep +
			   "UNREGISTER  BOARD_ID <board_id>   [FORCE]" +
			   sLineSep +
			   "JOBUPDATE  JOBNAME <jobname> JOBSTATUS <status> " +
			   sLineSep +
			   "QUERY   [PENDING]" +
			   //"QUERY   POOL <PoolName>" +
//           "DELETE  < PRINTER <printerName> | MODEM <ModemName> > CONFIRM" +
			   sLineSep +
			   //"REQUEST POOL <PoolName> TASKNAME <taskname> [FIRST | RANDOM | ENTRY <Value> ] [PRIORITY <Number>] [ASYNC | TIMEOUT <Number>[s|m|h|d|w]] " + //lbzhu [GARBAGECOLLECT <Yes | No>]" +
			   "REQUEST  TASKNAME <taskname> [FIRST | RANDOM | ENTRY <key>=<value> ] CASENAME <caseid> [CASENAME <caseid>] ...  [PRIORITY <Number>] [ASYNC | TIMEOUT <Number>[s|m|h|d|w]] [General <key>=<value> ] ... [ImageConfig <key>=<value> ] ... [ LTKConfig <key>=<value> ] ... [ ExtraAct <key>=<value> ] .... " +
			   //lbzhu [GARBAGECOLLECT <Yes | No>]" +
			   sLineSep +
			   "LIST    [BOARD_ID <board_id> | JOBS | TASKNAME <taskname> | GUIS ]" +
			   sLineSep +
			   "CANCEL ENTRY <Value>  BOARD_ID <Value>  TASK_NAME <Value> ENTRY_IP <Value> [FIRST | LAST ]" +
			   sLineSep +
			   "GUIREGISTER <machineName> " + //{STAF/Config/Machine}
			   sLineSep +
			   "GUIUNREGISTER" +
//            sLineSep +
//            "QUERY   PRINTER <PrinterName> | MODEM <ModemName>" +
			   sLineSep +
			   "VERSION" +
			   sLineSep +
			   "HELP";

		// Register Help Data
		registerHelpData(pData, KSTAFLtkTestNotEntryOwner, STAFString("Entry does NOT exist"), STAFString("the selected entry has NOT been added into resource"));
		registerHelpData(pData, kSTAFLtkTestHasPendingRequests, STAFString("there is still pending request"),
				 STAFString("there is still pending request, release or unregister it using force in case"));
		registerHelpData(pData, kSTAFLtkTestPutPendingRequests, STAFString("it will be put into pending request list"),
				 STAFString("the entry is occupied, so this request will be put into pending request list"));
		registerHelpData(pData, kSTAFLtkTestNoEntriesAvailable, STAFString("no entry in the resource is avaiable"),
				 STAFString("the first/random selection can be met since there is no entry in the resource queue "));
		registerHelpData(pData, kSTAFLtkTestNotTaskMatch, STAFString("task does NOT match anything in the resource pool request"),
				 STAFString("the specified task has NOT been added by the REQUEST function"));
		registerHelpData(pData, kSTAFLtkTestRunNotKeyAvailable, STAFString("unexpected erorr the ID in device pool does NOT match the resource pool"),
				 STAFString("unexpected erorr the ID in device pool does NOT match the resource pool, may caused by ASYNC issue"));
		registerHelpData(pData, kSTAFLtkTestNotPoolMatch, STAFString("there is no such pool map"),
				 STAFString("pool map name is hard-coded, sPoolName,may changed later"));

		//register shutdown notification
		registerShutDown(pData);

		//below code is for poolmap
		pData->fPoolMapRWSem = STAFRWSemPtr(new STAFRWSem, STAFRWSemPtr::INIT);
		//create a dir
		STAFFSPath poolFilePath;
		poolFilePath.setRoot(pInfo->writeLocation);
		poolFilePath.addDir("service");
		poolFilePath.addDir(pData->fShortName.toLowerCase());
		pData->fPoolDir = poolFilePath.asString();
		//poolFilePath.setRoot(pData->fPoolDir);
		DEBUGOUT("the fpoolDir=" << pData->fPoolDir);
		//poolFilePath.setRoot("/usr/local/staf/data/STAF/service/LTKTEST");
		//poolFilePath.setName(poolName);
		poolFilePath.setName("board_id");
		//poolFilePath.setExtension(sPoolExt);
		poolFilePath.setExtension("rpl");

		// Write the pool data
		STAFString fileName = poolFilePath.asString();
		DEBUGOUT("the fileName=" << fileName);
		// Read the pool file and store its data in PoolData
		//PoolData poolData;
		PoolData poolData(sPoolName, "record all the registered board_id" );
		DEBUGOUT("before read pool file");
		unsigned int status = readPoolFile(fileName, poolData);
		DEBUGOUT("status=" << status);
		if (status == kReadEndOfFile) {
			STAFString error(
				"STAFResPoolService.cpp: STAFServiceInit: "
				"Invalid file contents in resource pool " + fileName);
			cout << error << endl;
			*pErrorBuffer = error.adoptImpl();
			//   return kSTAFFileReadError; do NOT return, since it is first initialized
		}else if (status == kReadInvalidFormat) {
			STAFString error =
				"STAFResPoolService.cpp: STAFServiceInit: "
				"Invalid file format (" + poolData.fileFormat;
			error += ") in resource pool file " + fileName;
			cout << error << endl;
			*pErrorBuffer = error.adoptImpl();
			return kSTAFFileReadError;
		}else if (status == kFileOpenError) {
			STAFString error(
				"STAFResPoolService.cpp: STAFServiceInit: "
				"Error opening resource pool file " + fileName);
			cout << error << endl;
			*pErrorBuffer = error.adoptImpl();
			// return kSTAFFileReadError; do NOT return, since it is not existed
		}

		writePoolFile(fileName, poolData);
		// Add the pool data to the Pool Map
		DEBUGOUT("init, size=" << pData->fPoolMap.size());
		pData->fPoolMap.insert(PoolMap::value_type(
					       poolData.poolName.toUpperCase(),
					       PoolDataPtr(new PoolData(poolData), PoolDataPtr::INIT)));
		DEBUGOUT("init, size=" << pData->fPoolMap.size());

		// add the registered board_id into pdata deviceMap, and fEventManagerID will be added later
		int highestId = 0;
		for (int i = 0; i < poolData.resourceList.size(); i++) {
			//pData->fDeviceMap.insert(DeviceMap::value_type(poolData.resourceList.at(i).ID,DeviceDataPtr(new DeviceData(poolData.resourceList.at(i).entry,poolData.resourceList.at(i).board_type,poolData.resourceList.at(i).ipaddr,poolData.resourceList.at(i).ID),DeviceDataPtr::INIT)));
			pData->fDeviceMap.insert(DeviceMap::value_type(poolData.resourceList.at(i).pDevice->ID, poolData.resourceList.at(i).pDevice));
			//add code to registe the board in case it has NEVER been registered.
			poolData.resourceList.at(i).pDevice->shareStartTime = STAFTimestamp::now().asString().replace(":", "");

			int index = poolData.resourceList.at(i).pDevice->ID.asUInt();
			if (highestId < index)
				highestId = index;
		} //end of for
		fEventManagerID = highestId;

		static STAFThreadManager tm(2);
		DEBUGOUT("the result of dispath is " << tm.dispatch(run, pData));
		DEBUGOUT("the result of dispath is" << tm.dispatch(boarddetect, pData));

		/* if (writePoolFile(fileName, poolData) != kReadorWriteOk)
		   {
			return kSTAFFileWriteError;
		   }*/

	} //end of try

	catch (STAFException &e)
	{
		STAFString result;

		result += STAFString("In DeviceService.cpp: STAFServiceInit")
			  + kUTF8_SCOLON;

		result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
		result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
		result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
		result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

		*pErrorBuffer = result.adoptImpl();
	}
	catch (...)
	{
		STAFString error("DeviceService.cpp: STAFServiceInit: "
				 "Caught unknown exception");
		*pErrorBuffer = error.adoptImpl();
	}

	return retCode;
}


STAFRC_t STAFServiceAcceptRequest(STAFServiceHandle_t serviceHandle,
				  void *pRequestInfo, unsigned int reqLevel,
				  STAFString_t *pResultBuffer)
{
	if (reqLevel != 30) return kSTAFInvalidAPILevel;

	STAFRC_t retCode = kSTAFUnknownError;

	try
	{
		STAFResultPtr result(new STAFResult(),
				     STAFResultPtr::INIT);

		STAFServiceRequestLevel30 *pInfo =
			reinterpret_cast<STAFServiceRequestLevel30 *>(pRequestInfo);

		DeviceServiceData *pData =
			reinterpret_cast<DeviceServiceData *>(serviceHandle);

		// Determine the command request (the first word in the request)

		STAFString request(pInfo->request);
		STAFString action = request.subWord(0, 1).toLowerCase();

		// Call functions for the request

		if (action == "register")
			result = handleRegister(pInfo, pData);
		else if (action == "unregister")
			result = handleUnregister(pInfo, pData);
		else if (action == "unlock")
		    result = handleUnlock(pInfo, pData);
		else if (action == "lock")
		    result = handleLock(pInfo, pData);
		else if (action == "jobupdate")
			result = handleJobUpdate(pInfo, pData);
		else if (action == "query")
			result = handleQuery(pInfo, pData);
		else if (action == "request")
			result = handleRequest(pInfo, pData);
		else if (action == "cancel")
			result = handleCancel(pInfo, pData);
		else if (action == "list")
			result = handleList(pInfo, pData);
		else if (action == "guiregister")
			result = handleGuiRegister(pInfo, pData);
		else if (action == "guiunregister")
			result = handleGuiUnregister(pInfo, pData);
		else if (action == "run")
			result = handleRun(pInfo, pData);
		else if (action == "help")
			result = handleHelp(pInfo, pData);
		else if (action == "version")
			result = handleVersion(pInfo, pData);
		else{
			STAFString errMsg = STAFString("'") + request.subWord(0, 1) +
					    "' is not a valid command request for the " +
					    pData->fShortName + " service" + sLineSep + sLineSep +
					    sHelpMsg;

			result = STAFResultPtr(new STAFResult(
						       kSTAFInvalidRequestString, errMsg), STAFResultPtr::INIT);

		}

		*pResultBuffer = result->result.adoptImpl();
		retCode = result->rc;
	}
	catch (STAFException &e)
	{
		retCode = e.getErrorCode();

		STAFString result;

		result += STAFString("In DeviceService.cpp: STAFServiceAcceptRequest")
			  + kUTF8_SCOLON;

		result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
		result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
		result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
		result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

		*pResultBuffer = result.adoptImpl();
	}
	catch (...)
	{
		STAFString error("DeviceService.cpp: STAFServiceAcceptRequest: "
				 "Caught unknown exception");
		*pResultBuffer = error.adoptImpl();
	}

	return retCode;
}


STAFRC_t STAFServiceTerm(STAFServiceHandle_t serviceHandle,
			 void *pTermInfo, unsigned int termLevel,
			 STAFString_t *pErrorBuffer)
{
	if (termLevel != 0) return kSTAFInvalidAPILevel;

	STAFRC_t retCode = kSTAFUnknownError;

	try
	{
		retCode = kSTAFOk;

		DeviceServiceData *pData =
			reinterpret_cast<DeviceServiceData *>(serviceHandle);

		// Un-register Help Data

		unregisterHelpData(pData, KSTAFLtkTestNotEntryOwner);
		unregisterHelpData(pData, kSTAFLtkTestHasPendingRequests);
		unregisterHelpData(pData, kSTAFLtkTestPutPendingRequests);
		unregisterHelpData(pData, kSTAFLtkTestNoEntriesAvailable);
		unregisterHelpData(pData, kSTAFLtkTestNotTaskMatch);
		unregisterHelpData(pData, kSTAFLtkTestRunNotKeyAvailable);
		unregisterHelpData(pData, kSTAFLtkTestNotPoolMatch);

		//post end semphore for all the threads
		//gEndSem.post();
		DEBUGOUT("gEndSem posted");
		DEBUGOUT("exit STAFServiceTerm");
	}
	catch (STAFException &e)
	{
		STAFString result;

		result += STAFString("In DeviceService.cpp: STAFServiceTerm")
			  + kUTF8_SCOLON;

		result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
		result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
		result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
		result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

		*pErrorBuffer = result.adoptImpl();
	}
	catch (...)
	{
		STAFString error("DeviceService.cpp: STAFServiceTerm: "
				 "Caught unknown exception");
		*pErrorBuffer = error.adoptImpl();
	}

	return retCode;
}


STAFRC_t STAFServiceDestruct(STAFServiceHandle_t *serviceHandle,
			     void *pDestructInfo, unsigned int destructLevel,
			     STAFString_t *pErrorBuffer)
{
	if (destructLevel != 0) return kSTAFInvalidAPILevel;

	STAFRC_t retCode = kSTAFUnknownError;

	try
	{
		DeviceServiceData *pData =
			reinterpret_cast<DeviceServiceData *>(*serviceHandle);
		DEBUGOUT("before delete pdata in destruction ");
		delete pData;
		DEBUGOUT("after delete pdata in destruction ");
		*serviceHandle = 0;

		retCode = kSTAFOk;
	}
	catch (STAFException &e)
	{
		STAFString result;

		result += STAFString("In DeviceService.cpp: STAFServiceDestruct")
			  + kUTF8_SCOLON;

		result += STAFString("Name: ") + e.getName() + kUTF8_SCOLON;
		result += STAFString("Location: ") + e.getLocation() + kUTF8_SCOLON;
		result += STAFString("Text: ") + e.getText() + kUTF8_SCOLON;
		result += STAFString("Error code: ") + e.getErrorCode() + kUTF8_SCOLON;

		*pErrorBuffer = result.adoptImpl();
	}
	catch (...)
	{
		STAFString error("DevicePoolService.cpp: STAFServiceDestruct: "
				 "Caught unknown exception");
		*pErrorBuffer = error.adoptImpl();
	}

	return retCode;
}


// Handles device register entry requests

STAFResultPtr handleRegister(STAFServiceRequestLevel30 *pInfo,
			     DeviceServiceData *pData)
{
	// Verify the requester has at least trust level 3

	VALIDATE_TRUST(3, pData->fShortName, "REGISTER", pData->fLocalMachineName);

	// Parse the request

	STAFCommandParseResultPtr parsedResult =
		pData->fRegisterParser->parse(pInfo->request);

	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);

	// Resolve any STAF variables in the board_id option's value
	STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "board_id");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString board_id = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "board_type");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString board_type = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "chip_name");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString chip_name = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "chip_stepping");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString chip_stepping = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "board_register_date");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString board_register_date = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "board_status");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString board_status = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "user_team");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString user_team = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "current_user");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString current_user = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "board_eco");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString board_eco = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "lcd_resolution");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString lcd_resolution = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "lcd_screensize");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString lcd_screensize = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "ddr_type");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString ddr_type = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "ddr_size");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString ddr_size = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "emmc_type");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString emmc_type = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "emmc_size");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString emmc_size = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "rf_name");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString rf_name = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "rf_type");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString rf_type = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "serial");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString serial = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "mcu");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString mcu = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "usb");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString usb = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "username");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString username = resultPtr->result;
	
	resultPtr = resolveOp(pInfo, pData, parsedResult, "userteam");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString userteam = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "DRO");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString dro = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "CHIP_TYPE");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString chip_type = resultPtr->result;

	bool task_lock_temp=0;

	//auto get the registered machine name
	//STAFString machine = pInfo->machine;
	STAFString machine = pInfo->machineNickname;
	STAFString physicalInterfaceID = pInfo->physicalInterfaceID;
	STAFString shareStartTime = STAFTimestamp::now().asString().replace(":", "");
	// Add the device to the board_id map or the modem map and
	// write an informational message to the service log
	STAFString index ; //add ID column
	if (board_id != "") {
		//////////////////////////////////////////////////////////////////////////
		// design flow about the poolMap management
		// step 1 : get the poolMap from fPoolMap
		//// step 2a: check if the board_id name has been registered, return AlreadyExists if yes;
		// step 2a: check if the board_id name has been registered, update it if yes;
		//		else 2b: insert the board_id into fDeviceMap if it is not existed
		// step 3 : get the head info about pool Ptr, clear all the resourceList, since it will push_back all the registered board_id into resourceList,
		//	       the method may update
		// step 4 : Delete the old pool file and write the new pool file

		//STAFMutexSemLock lock(*pData->fDeviceMapSem);//lock should be used before register
		STAFRWSemWLock wLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}; //pool map write lock
		// step 1
		PoolMap::iterator poolIterator;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());
		DEBUGOUTFILE("spoolname=" << sPoolName << " size=" << pData->fPoolMap.size());
		if (poolIterator == pData->fPoolMap.end())
			return STAFResultPtr(new STAFResult(kSTAFLtkTestNotPoolMatch, sPoolName),
					     STAFResultPtr::INIT);
		DEBUGOUTFILE("resourceList.size=" << (*poolIterator).second->resourceList.size());

		// step 2a
		PoolDataPtr poolPtr = (*poolIterator).second;
		STAFMutexSemLock lock(*poolPtr->accessSem);
		bool alreadyExist = false;
		//STAFString index;
		for (unsigned int j = 0; j < poolPtr->resourceList.size(); j++) {
			//	DEBUGOUT("j="<<j<<" the entry="<<poolPtr->resourceList[j].entry);
			//if (poolPtr->resourceList[j].entry == board_id)
			DEBUGOUTFILE("j=" << j << " the entry=" << (poolPtr->resourceList[j].pDevice)->board_id);
			if ((poolPtr->resourceList[j].pDevice)->board_id == board_id) {
				alreadyExist = true;
				index = (poolPtr->resourceList[j].pDevice)->ID;
				DEBUGOUTFILE("alreadyExist=true" << alreadyExist << " index=" << index);
//                 return STAFResultPtr(new STAFResult(kSTAFAlreadyExists,board_id),
//                                    STAFResultPtr::INIT);
			}
		}

		//the poolptr won't affect the newPool any more since it uses variable
		PoolData newPool = *(poolPtr);
		newPool.resourceList.clear();

		////////////////////////////////////////////////////////////////////
		// step 2b, insert the board_id into fDeviceMap if it is not existed
		//	use the fEventManagerID to be the KEY	so that the job can be tracked
		///////////////////////////////////////////////////////////////////
		//STAFString index;a
		//ResourceData newResource;
		if (false == alreadyExist) { //will will create a new resource
			fEventManagerID++;
			index = fEventManagerID;
			DeviceDataPtr tempPDevice(new DeviceData(board_id, board_type, machine, physicalInterfaceID, index, chip_name, chip_stepping, board_register_date,
								 board_status, user_team, current_user, board_eco, lcd_resolution, lcd_screensize,
								 ddr_type, ddr_size, emmc_type, emmc_size, rf_name, rf_type, serial, mcu, usb, shareStartTime, username, userteam, dro,  chip_type),
						  DeviceDataPtr::INIT);
			pData->fDeviceMap.insert(DeviceMap::value_type(index, tempPDevice));
			pData->fDeviceMap[index]->under_task=0;
			pData->fDeviceMap[index]->task_lock=0;
			pData->fDeviceMap[index]->off_line_count=0;
			ResourceData newResource(tempPDevice);
			//////////////////////////////////////////////////////////////
			//add new board, the pending request may be met
			///////////////////////////////////////////////////////////
			if (poolPtr->requestList.size() > 0) {
				//STAFMutexSemLock lock(*poolPtr->accessSem);
				RequestList::iterator iter;
				RequestDataPtr reqPtr;
				bool match = false;

				for (iter = poolPtr->requestList.begin(); iter != poolPtr->requestList.end(); ++iter) {
					reqPtr = *iter;
					if ((reqPtr->requestType != kEntry) ||
					    ((reqPtr->requestType == kEntry) &&
					     RequestData::existInPool(tempPDevice, reqPtr->requestedEntry)
					    )) {

						// Assign the resource to the request
						reqPtr->retCode = kSTAFOk;
						reqPtr->resultBuffer = tempPDevice->board_id;
						reqPtr->board_id = tempPDevice->board_id;
						// Update the resource entry's ownership information

						newResource.owned = 1;
						newResource.requestedTime = reqPtr->requestedTime;
						newResource.taskName = reqPtr->taskName;
						newResource.acquiredTime = STAFTimestamp::now().asString();
						newResource.garbageCollect = reqPtr->garbageCollect;
						newResource.preTasks.insert(reqPtr->taskName);
						poolPtr->usedResources++;
						(*poolIterator).second->resourceList.push_back(newResource);
						match = true;

						if (true == reqPtr->AsyncOrNot) { //Async
							DEBUGOUTFILE("put to the ready listcase with taskname=" << reqPtr->taskName << "board_id=" << reqPtr->board_id << "size of case=" << reqPtr->CaseList.size() << "jobmap size=" << reqPtr->jobMap.size());
							(*poolIterator).second->readyList.push_back(reqPtr);
						}else
							// Wakeup the requester
							reqPtr->wakeup->post();
						 // Remove the satisfied request from the pending
						// request list and break out of the loop
						poolPtr->requestList.erase(iter);

						break; //won't go to next loop
					}

				} // end of for loop
				if (false == match) {
					(*poolIterator).second->resourceList.push_back(newResource);
					DEBUGOUTFILE("append " << board_id << " to resourcelist since it does NOT meet any entry in the request list");
				}
			}else //end of go thourgh the request list
				(*poolIterator).second->resourceList.push_back(newResource);
		}else{
			pData->fDeviceMap[index]->board_type = board_type;
			pData->fDeviceMap[index]->machine = machine;
			pData->fDeviceMap[index]->physicalInterfaceID = physicalInterfaceID;
			pData->fDeviceMap[index]->ID = index;
			pData->fDeviceMap[index]->chip_name = chip_name;
			pData->fDeviceMap[index]->chip_stepping = chip_stepping;
			pData->fDeviceMap[index]->board_register_date = board_register_date;
			pData->fDeviceMap[index]->board_status = user_team;
			pData->fDeviceMap[index]->current_user = current_user;
			pData->fDeviceMap[index]->board_eco = board_eco;
			pData->fDeviceMap[index]->lcd_resolution = lcd_resolution;
			pData->fDeviceMap[index]->lcd_screensize = lcd_screensize;
			pData->fDeviceMap[index]->ddr_type = ddr_type;
			pData->fDeviceMap[index]->ddr_size = ddr_size;
			pData->fDeviceMap[index]->emmc_type = emmc_type;
			pData->fDeviceMap[index]->emmc_size = emmc_size;
			pData->fDeviceMap[index]->rf_name = rf_name;
			pData->fDeviceMap[index]->rf_type = rf_type;
			pData->fDeviceMap[index]->serial = serial;
			pData->fDeviceMap[index]->mcu = mcu;
			pData->fDeviceMap[index]->usb = usb;
			pData->fDeviceMap[index]->username = username;
			pData->fDeviceMap[index]->userteam = userteam;
			pData->fDeviceMap[index]->dro = dro;
			pData->fDeviceMap[index]->chip_type = chip_type;
			//pData->fDeviceMap[index]->task_lock=0;
			pData->fDeviceMap[index]->off_line_count=0;
			DEBUGOUTFILE("exit update for board_id" << board_id );
		}
		STAFString logMsg = "[ID=" + index + "]" + " REGISTER board_id request.  board_id=" + board_id +
				    " board_type=" + board_type + " machine=" + machine + " chip_name=" + chip_name;
		log(pData, "info", logMsg);
		//(*poolIterator).second->resourceList.push_back(ResourceData(board_id,board_type,ipaddr,index));
		//also add it into the resource list so that it is avaiable. lbzhu check if we need add it into below statement

		// step 3
		//PoolData newPool ; add all resource into the new pool then write then back into file
		DeviceMap::iterator iter;
		for (iter = pData->fDeviceMap.begin(); iter != pData->fDeviceMap.end(); ++iter) {
			//ResourceData resourceData(iter->second->board_id,iter->second->board_type,iter->second->ipaddr,iter->second->ID);
			ResourceData resourceData(iter->second);
			newPool.resourceList.push_back(resourceData);
			newPool.numResources++;
		}

		// step 4
		STAFFSPath poolFilePath;
		poolFilePath.setRoot(pData->fPoolDir);
		poolFilePath.setName("board_id");
		poolFilePath.setExtension("rpl");
		DEBUGOUTFILE("fpooldir=" << pData->fPoolDir << " spoolName=" << sPoolName);
		poolFilePath.getEntry()->remove();
		STAFString fileName = poolFilePath.asString();
		if (writePoolFile(fileName, newPool) != kReadorWriteOk)
			return STAFResultPtr(new STAFResult(kSTAFFileWriteError, fileName), STAFResultPtr::INIT);

		if (false == alreadyExist) {
			//register the share info into database, if it is the new entry
			//./ShareBoardRecord -Share <board_id> -StartTime <TimeStamp> -UserName <username> -Ipaddr <ipaddr>
			STAFString ShareCommand(" start command ./ShareBoardRecord PARMS -Share ");
			ShareCommand += board_id;
			ShareCommand += " -StartTime ";
			ShareCommand += shareStartTime;
			ShareCommand+=" -UserName ";
			ShareCommand+=username;
			ShareCommand+=" -Ipaddr ";
			ShareCommand+=physicalInterfaceID;
			ShareCommand+=" -Machine ";
			ShareCommand+=machine;
			ShareCommand+=" -UserTeam ";
			if (userteam=="")
			{userteam="Unknow";}
			ShareCommand+=userteam;
			ShareCommand += " wait 5s  RETURNSTDOUT STDERRTOSTDOUT";

			STAFResultPtr resultcmd =  pData->fHandlePtr->submit("local", "PROCESS", ShareCommand );
			cout << ShareCommand ;
			if (resultcmd->rc == 0) //means pass
				DEBUGOUTFILE("PASS to share board info register " << board_id << " startime=" << shareStartTime);
			else
				DEBUGOUTFILE("fail to share board  " << board_id << " sharecommand=" << ShareCommand);
		} //if new board added
	}
	// notify all the registered GUI SERVER
	std::set<GuiData>::iterator it;
	{
		STAFMutexSemLock lock(*pData->fGuiSetSem);
		for (it = pData->fGuiSet.begin(); it != pData->fGuiSet.end(); it++)
			NotifyGui(pData, kBoard, board_id, it->physicalInterfaceID);
	}

	// Return an Ok result
	//return STAFResultPtr(new STAFResult(kSTAFOk,fEventManagerID), STAFResultPtr::INIT);
	//return the index if the board has registered, and only update it
	return STAFResultPtr(new STAFResult(kSTAFOk, index), STAFResultPtr::INIT);
//    return STAFResultPtr(new STAFResult(kSTAFOk), STAFResultPtr::INIT);
}


// Handles device deletion requests

STAFResultPtr handleUnregister(STAFServiceRequestLevel30 *pInfo,
			       DeviceServiceData *pData)
{
	// Verify the requester has at least trust level 4

	VALIDATE_TRUST(4, pData->fShortName, "UNREGISTER", pData->fLocalMachineName);

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fUnregisterParser->parse(pInfo->request);
	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString, parsedResult->errorBuffer), STAFResultPtr::INIT);

	// Resolve any STAF variables in the print option's value
	STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "board_id");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString board_id = resultPtr->result;
	DEBUGOUTFILE("board_id=" << board_id);

	/*tbd check if there are any pending reqeuests and if FORCE is not specified
	 */

	/*
	   check if delete the board with regexpression
	 */
	bool match = false;
	if (parsedResult->optionTimes("MATCH"))
		match = true;
	 // Find the device in the printer or modem map and remove it and
	// write an informational message to the service log

	if (board_id != "") {
		//check if the board_id is existed or not
		STAFMutexSemLock lock(*pData->fDeviceMapSem);
		DeviceMap::iterator iter;
		bool found = false;

		STAFRWSemWLock wLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;};
		PoolMap::iterator poolIterator;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());
		DEBUGOUTFILE("spoolname=" << sPoolName << " size=" << pData->fPoolMap.size());
		PoolDataPtr poolPtr = (*poolIterator).second;
		ResourceList::iterator resIterator;

		std::vector<STAFString> removeVec;
		for (iter = pData->fDeviceMap.begin(); iter != pData->fDeviceMap.end(); ++iter) {
			/*if(iter->second->board_id.matchesWildcards(board_id,kSTAFStringCaseInsensitive))
			   {
				DEBUGOUT("match the wildcard , it is"<<iter->second->board_id);
			   }*/
			if ((match && iter->second->board_id.matchesWildcards(board_id, kSTAFStringCaseInsensitive)) || (board_id == iter->second->board_id)) {
				DEBUGOUTFILE("find the matched board_id" << iter->second->board_id);
				//pData->fDeviceMap.erase(iter->second->ID);
				removeVec.push_back(iter->second->ID);
				found = true;
				DEBUGOUTFILE("try deleteing  the record in poolmap");
				/*			STAFRWSemWLock wLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;};
							PoolMap::iterator poolIterator;
							poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());
								DEBUGOUT("spoolname="<<sPoolName<<" size="<<pData->fPoolMap.size());

								PoolDataPtr poolPtr = (*poolIterator).second;*/
				//unshare the board and add info into database
				//./ShareBoardRecord -Unhare <board_id> -StartTime <TimeStamp> -EndTime <endtime>
				STAFString UnshareCommand(" start command ./ShareBoardRecord PARMS -Unshare ");
				UnshareCommand += iter->second->board_id;
				UnshareCommand += " -StartTime ";
				UnshareCommand += iter->second->shareStartTime;
				UnshareCommand += " -EndTime ";
				UnshareCommand += STAFTimestamp::now().asString().replace(":", "");
				UnshareCommand += " wait 5s  RETURNSTDOUT STDERRTOSTDOUT";

				STAFResultPtr resultcmd =  pData->fHandlePtr->submit("local", "PROCESS", UnshareCommand );
				if (resultcmd->rc == 0)        //means pass
					DEBUGOUTFILE("PASS to unshare board info register " << iter->second->board_id << " starttime=" << iter->second->shareStartTime);
				else
					DEBUGOUTFILE("fail to unshare board  " << iter->second->board_id);

				//step 2 delete the resource list
				for (resIterator = poolPtr->resourceList.begin(); resIterator != poolPtr->resourceList.end(); resIterator++) {
					if ((*resIterator).pDevice->board_id == iter->second->board_id) {
						//if((*resIterator).pDevice->board_id==board_id)
						//if((match &&(*resIterator).pDevice->board_id.matchesWildcards(board_id,kSTAFStringCaseInsensitive)) || ( (*resIterator).pDevice->board_id==board_id))
						DEBUGOUTFILE(" the entry=" << board_id);
						poolPtr->resourceList.erase(resIterator);
						if (poolPtr->resourceList.size() == 0)
							fEventManagerID = 0;
						break;
					}
				}

			}        //end of if
			DEBUGOUTFILE("checking " << iter->second->board_id);

			/*for (unsigned int j = 0; j < poolPtr->resourceList.size(); j++){

				if (poolPtr->resourceList[j].device.board_id == board_id){
					DEBUGOUT("j="<<j<<" the entry="<<poolPtr->resourceList[j].device.board_id);
					poolPtr->resourceList.erase(
				}
			   }*/
		} //end of for

		//check if the board_id is existed or not
		//if(iter == pData->fDeviceMap.end())//means it does NOT exist
		if (false == found) //means it does NOT exist

			return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, "board_id does not exist in pool"), STAFResultPtr::INIT);
		else{
			//step 1: remove all the board in the fdeviceMap;
			for (int i = 0; i < removeVec.size(); i++)
				pData->fDeviceMap.erase(removeVec.at(i));

			//may move it to the deconstructor function. write it back into the board file
			STAFFSPath poolFilePath;
			poolFilePath.setRoot(pData->fPoolDir);
			poolFilePath.setName("board_id");
			poolFilePath.setExtension("rpl");
			DEBUGOUTFILE("fpooldir=" << pData->fPoolDir << " spoolName=" << sPoolName);
			poolFilePath.getEntry()->remove();
			STAFString fileName = poolFilePath.asString();
			if (writePoolFile(fileName, (*poolPtr)) != kReadorWriteOk)
				return STAFResultPtr(new STAFResult(kSTAFFileWriteError, fileName),
						     STAFResultPtr::INIT);
		}
		STAFString logMsg = "UNREGISTER PRINTER request.  Name=" + board_id;
		log(pData, "info", logMsg);
		// notify all the registered GUI SERVER
		std::set<GuiData>::iterator it;
		{
			STAFMutexSemLock lock(*pData->fGuiSetSem);
			for (it = pData->fGuiSet.begin(); it != pData->fGuiSet.end(); it++)
				NotifyGui(pData, kBoard, board_id, it->physicalInterfaceID);
		}
	}

	// Return an Ok result
	return STAFResultPtr(new STAFResult(kSTAFOk, board_id + " is unregistered successfully"), STAFResultPtr::INIT);
}




STAFResultPtr handleUnlock(STAFServiceRequestLevel30 *pInfo,DeviceServiceData *pData)
{
    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "UNLOCK", pData->fLocalMachineName);
    
    // Parse the request
    STAFCommandParseResultPtr parsedResult = pData->fUnlockParser->parse(pInfo->request);
    if (parsedResult->rc != kSTAFOk){
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
   
    // Resolve any STAF variables in the print option's value
    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "board_id");
    if (resultPtr->rc != kSTAFOk) return resultPtr;
    STAFString board_id = resultPtr->result;
	DEBUGOUTFILE("board_id="<<board_id);
    
	bool match=false;
	if(parsedResult->optionTimes("MATCH")){
		match=true;
	}

    if (board_id != "")
    {
        STAFMutexSemLock lock(*pData->fDeviceMapSem);
		DeviceMap::iterator iter;
		bool found=false;

		STAFRWSemWLock wLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;};
		PoolMap::iterator poolIterator;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());
		DEBUGOUTFILE("spoolname="<<sPoolName<<" size="<<pData->fPoolMap.size());
		PoolDataPtr poolPtr = (*poolIterator).second;
		ResourceList::iterator resIterator;

		std::vector<STAFString> removeVec;
        for (iter = pData->fDeviceMap.begin(); iter != pData->fDeviceMap.end(); ++iter)
        {
		if((match &&iter->second->board_id.matchesWildcards(board_id,kSTAFStringCaseInsensitive)) || (board_id == iter->second->board_id))
		{
			found=true;
			iter->second->task_lock=false;
			//iter->second->under_task=false;
		}
	}
    }
    return STAFResultPtr(new STAFResult(kSTAFOk,board_id+" is unlock successfully"), STAFResultPtr::INIT);
}


STAFResultPtr handleLock(STAFServiceRequestLevel30 *pInfo,DeviceServiceData *pData)
{
    // Verify the requester has at least trust level 4

    VALIDATE_TRUST(4, pData->fShortName, "LOCK", pData->fLocalMachineName);
    
    // Parse the request
    STAFCommandParseResultPtr parsedResult = pData->fLockParser->parse(pInfo->request);
    if (parsedResult->rc != kSTAFOk){
        return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,parsedResult->errorBuffer), STAFResultPtr::INIT);
    }
   
    // Resolve any STAF variables in the print option's value
    STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "board_id");
    if (resultPtr->rc != kSTAFOk) return resultPtr;
    STAFString board_id = resultPtr->result;
	DEBUGOUTFILE("board_id="<<board_id);
    
	bool match=false;
	if(parsedResult->optionTimes("MATCH")){
		match=true;
	}

    if (board_id != "")
    {
        STAFMutexSemLock lock(*pData->fDeviceMapSem);
		DeviceMap::iterator iter;
		bool found=false;

		STAFRWSemWLock wLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;};
		PoolMap::iterator poolIterator;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());
		DEBUGOUTFILE("spoolname="<<sPoolName<<" size="<<pData->fPoolMap.size());
		PoolDataPtr poolPtr = (*poolIterator).second;
		ResourceList::iterator resIterator;

		std::vector<STAFString> removeVec;
        for (iter = pData->fDeviceMap.begin(); iter != pData->fDeviceMap.end(); ++iter)
        {
		if((match &&iter->second->board_id.matchesWildcards(board_id,kSTAFStringCaseInsensitive)) || (board_id == iter->second->board_id))
		{
			found=true;
			iter->second->task_lock=1;
		}
	}
    }
    return STAFResultPtr(new STAFResult(kSTAFOk,board_id+" lock successfully"), STAFResultPtr::INIT);
}



//handle Cancel pending request
STAFResultPtr handleCancel(STAFServiceRequestLevel30 *pInfo,
			   DeviceServiceData *pData)
{
	STAFString result;
	STAFRC_t rc = kSTAFOk;
	STAFString cancel_task_name;
	// Verify the requester has at least trust level 5
	cout<<"cancel start"<<endl;
	VALIDATE_TRUST(5, pData->fShortName, "CANCEL", pData->fLocalMachineName);

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fCancelParser->parse(pInfo->request);
	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);

	bool cancel_pending_task=false;
	bool cancel_board_task=false;
	bool cancel_task=false;
	bool cancel_pending_task_ip=false;

	if ( (parsedResult->optionTimes("BOARD_ID")))
		cancel_board_task = true;
	if ( (parsedResult->optionTimes("ENTRY")))
		cancel_pending_task = true;
	if ( (parsedResult->optionTimes("TASK_NAME")))
		cancel_task = true;
	if ( (parsedResult->optionTimes("ENTRY_IP")))
		cancel_pending_task_ip = true;

	if ( cancel_pending_task )
	{
		bool cancel_entry_found=false;
		STAFResultPtr resultPtr;
		STAFString CancelEntry = STAFString("");
		resultPtr = resolveOp(pInfo, pData, parsedResult, sEntry);
		if (resultPtr->rc != 0) return resultPtr;
		CancelEntry = resultPtr->result;
		cout<<"cancel task"<<CancelEntry<<endl;
		// Get a read lock on the Pool Map for the duration of this block
		{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}

		// Make sure that the resource pool is in pData->poolMap
		PoolMap::iterator poolIterator;
		PoolDataPtr poolPtr;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());

		if (poolIterator == pData->fPoolMap.end())
			return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, sPoolName),
					     STAFResultPtr::INIT);

		poolPtr = (*poolIterator).second;
		// Lock the poolData semaphore for the duration of this block, for the reqlist
		STAFMutexSemLock lock(*poolPtr->accessSem);

		// Iterate through the pending requests.  Find the first pending
		// request that will be canceled
		//   b) requestType == kEntry and the entry just released
		//      matches the requestedEntry.

		if (poolPtr->requestList.size() > 0) {
			RequestList::iterator iter;
			RequestDataPtr reqPtr;
			
			for (iter = poolPtr->requestList.begin();
			     iter != poolPtr->requestList.end(); ++iter) {
				reqPtr = *iter;

				if (reqPtr->requestedEntry == CancelEntry) {
					poolPtr->requestList.erase(iter);
					cancel_entry_found=true;
					break;
				}
			}
		}else{return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, CancelEntry), STAFResultPtr::INIT);}

		if (cancel_entry_found)
		{return STAFResultPtr(new STAFResult(kSTAFOk, "Cancel successful"), STAFResultPtr::INIT);}
		else{return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, "cancel entry not find"), STAFResultPtr::INIT);}
	}




	if ( cancel_pending_task_ip )
	{
		bool cancel_entry_found=false;
		STAFResultPtr resultPtr;
		STAFString CancelEntry = STAFString("");
		STAFString physicalInterfaceID = pInfo->physicalInterfaceID;
		resultPtr = resolveOp(pInfo, pData, parsedResult, "ENTRY_IP");
		if (resultPtr->rc != 0) return resultPtr;
		CancelEntry = resultPtr->result;
		if (cancel_task)
		{
			resultPtr = resolveOp(pInfo, pData, parsedResult, "TASK_NAME");
			if (resultPtr->rc != 0) return resultPtr;
			cancel_task_name = resultPtr->result;
		}else
		{cancel_task_name = "NO_TASK_NAME_FILTER";}
	
		cout<<"cancel task:"<<physicalInterfaceID<<":"<<CancelEntry<<":"<<cancel_task_name<<endl;
		// Get a read lock on the Pool Map for the duration of this block
		{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}

		// Make sure that the resource pool is in pData->poolMap
		PoolMap::iterator poolIterator;
		PoolDataPtr poolPtr;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());

		if (poolIterator == pData->fPoolMap.end())
			return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, sPoolName),
					     STAFResultPtr::INIT);

		poolPtr = (*poolIterator).second;
		STAFMutexSemLock lock(*poolPtr->accessSem);

		if (poolPtr->requestList.size() > 0) 
		{
			RequestList::iterator iter;
			RequestDataPtr reqPtr;
			
			for (iter = poolPtr->requestList.begin();
			     iter != poolPtr->requestList.end(); ++iter) 
			{
				reqPtr = *iter;

				if (cancel_task)
				{
					if (reqPtr->requestedEntry == CancelEntry && reqPtr->orgEndpoint == physicalInterfaceID && reqPtr->taskName == cancel_task_name ) 
					{
						poolPtr->requestList.erase(iter);
						cancel_entry_found=true;
						break;
					}
				}else
				{
					if (reqPtr->requestedEntry == CancelEntry && reqPtr->orgEndpoint == physicalInterfaceID) 
					{
						poolPtr->requestList.erase(iter);
						cancel_entry_found=true;
						break;
					}
				}
			}
		}else{return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, CancelEntry), STAFResultPtr::INIT);}

		if (cancel_entry_found)
		{return STAFResultPtr(new STAFResult(kSTAFOk, "Cancel successful"), STAFResultPtr::INIT);}
		else{return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, "cancel entry not find"), STAFResultPtr::INIT);}
	}








	if ( cancel_board_task )
	{
		STAFResultPtr resultPtr;
		STAFString Cancel_board_id = STAFString("");
		resultPtr = resolveOp(pInfo, pData, parsedResult, "BOARD_ID");
		if (resultPtr->rc != 0) {return resultPtr;}
		Cancel_board_id = resultPtr->result;
		cout<<"cancel board:"<<Cancel_board_id<<endl;

		DeviceMap tempDeviceMap;
		{
			STAFMutexSemLock lock(*pData->fDeviceMapSem);
			tempDeviceMap = pData->fDeviceMap;
		}
		DeviceMap::iterator iter;
		for (iter = tempDeviceMap.begin(); iter != tempDeviceMap.end(); ++iter) 
		{
			if (Cancel_board_id == iter->second->board_id)
			{
				STAFString boardCmd(" start command ./rcc_utilc PARMS ltkex "); //the board detect binary is hard-coded rcc_utilc
				boardCmd += iter->second->board_id;
				boardCmd += " cancelJob";
				boardCmd += " WAIT 30s STDERRTOSTDOUT RETURNSTDOUT";
				STAFResultPtr resultPtr =  pData->fHandlePtr->submit( iter->second->physicalInterfaceID, "PROCESS", boardCmd );
				if (resultPtr->rc != kSTAFOk) {return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, "board is found , but cancel execute fail!"), STAFResultPtr::INIT);}
				else 	
				{
					cout<<"cancel board: "<<Cancel_board_id<<" Success"<<endl;
					return STAFResultPtr(new STAFResult(kSTAFOk, "Cancel board successful"),STAFResultPtr::INIT);
				}
			}
		}
		cout<<"Can't find board to cancel:"<<Cancel_board_id<<endl;
		return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, Cancel_board_id), STAFResultPtr::INIT);
	}


/*   //blocking for pending task can only remove one + can't detect task is finished
	if ( cancel_all_task )
	{
		bool cancel_result=cancel_result=true;
		STAFResultPtr resultPtr;
		STAFString TaskName = STAFString("");
		resultPtr = resolveOp(pInfo, pData, parsedResult, "TASK_NAME");
		if (resultPtr->rc != 0) return resultPtr;
		TaskName = resultPtr->result;
		cout<<"cancel task: "<<TaskName<<endl;
		STAFString cancel_error_mesg = STAFString("Fail Cancel board:\n");
//cancel job for pending 
			{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}

		PoolMap::iterator poolIterator;
		PoolDataPtr poolPtr;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());

		if (poolIterator == pData->fPoolMap.end())
			return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, sPoolName),
					     STAFResultPtr::INIT);

		poolPtr = (*poolIterator).second;
		STAFMutexSemLock lock(*poolPtr->accessSem);

			if (poolPtr->requestList.size() > 0) {
			RequestList::iterator iter;
			RequestDataPtr reqPtr;
			int pend_task_num=1;
			for (iter = poolPtr->requestList.begin();
			     iter != poolPtr->requestList.end(); ++iter) {
				reqPtr = *iter;

				if (reqPtr->taskName == TaskName) {
					poolPtr->requestList.erase(iter);
					break;
				}
			}
		}
//end cancel job for pending 

//cancel job for running
cout<<"cancel job for running"<<endl;
		STAFObjectPtr resultList = STAFObject::createList();
		STAFMutexSemLock lock_job_running(*pData->fJobListSem);
		JobList::iterator iter;
		for (iter = pData->fJobList.begin();
		     (iter != pData->fJobList.end())  ; ++iter) {
			if (TaskName == (*iter)->taskName) {

				STAFString boardCmd(" start command ./rcc_utilc PARMS ltkex "); //the board detect binary is hard-coded rcc_utilc
				boardCmd += (*iter)->board_id;
				boardCmd += " onlineStatus WAIT 2s STDERRTOSTDOUT RETURNSTDOUT";
				boardCmd += "cancelJob";
				STAFResultPtr resultPtr =  pData->fHandlePtr->submit( (*iter)->ipaddr, "PROCESS", boardCmd );
				if (resultPtr->rc != kSTAFOk) 
				{
					cout<<"cancel "<<TaskName<<":"<<(*iter)->board_id<<" Fail"<<endl;
					cancel_error_mesg +=TaskName+":"+(*iter)->board_id+"\n";
					cancel_result=false;
				}
				else 	
				{
					cout<<"cancel "<<TaskName<<":"<<(*iter)->board_id<<" Success"<<endl;
				}
			}
		}
//end cancel job for running 
		if (cancel_result)
		{ return STAFResultPtr(new STAFResult(kSTAFOk, "Cancel task successful"),STAFResultPtr::INIT); }else
		{ return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, cancel_error_mesg), STAFResultPtr::INIT);}

	}
*/
	return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, "cancel fail"), STAFResultPtr::INIT);
} //end of handle cancel


// Handles device list requests

STAFResultPtr handleList(STAFServiceRequestLevel30 *pInfo,
			 DeviceServiceData *pData)
{
	STAFString result;
	STAFRC_t rc = kSTAFOk;

	// Verify the requester has at least trust level 2

	VALIDATE_TRUST(2, pData->fShortName, "LIST", pData->fLocalMachineName);

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fListParser->parse(pInfo->request);
	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);

	/*output the request info*/
	/*STAFString machine=pInfo->machine;
	   STAFString machineNickname=pInfo->machineNickname;
	   STAFString endpoint=pInfo->endpoint;
	   STAFString physicalInterfaceID=pInfo->physicalInterfaceID;
	   DEBUGOUT("machine="<<machine);
	   DEBUGOUT("machineNickname="<<machineNickname);
	   DEBUGOUT("endpoint="<<endpoint);
	   DEBUGOUT("physicalInterfaceID="<<physicalInterfaceID);*/

	// Check if specified boards or modems
	bool boards = false;
	bool jobs = false;
	bool tasks = false;
	bool board_id = false;
	bool guis = false;

	if (!(parsedResult->optionTimes("BOARD_ID")) &&
	    !(parsedResult->optionTimes("JOBS")) &&
	    !(parsedResult->optionTimes("GUIS")) &&
	    !(parsedResult->optionTimes("TASKNAME"))
	    )
		boards = true;

	if (parsedResult->optionTimes("BOARD_ID")) {
		boards = true;
		board_id = true;
	}

	if (parsedResult->optionTimes("JOBS"))
		jobs = true;
	if (parsedResult->optionTimes("TASKNAME"))
		tasks = true;
	if (parsedResult->optionTimes("GUIS"))
		guis = true;
	// Create a marshalling context and set any map classes (if any).
	STAFObjectPtr mc = STAFObject::createMarshallingContext();

	// Add board_id entries to the result list
	if (boards ) {
		STAFString boardName;
		if (board_id) {
			STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "BOARD_ID");
			if (resultPtr->rc != kSTAFOk) return resultPtr;
			boardName = resultPtr->result;
		}
		mc->setMapClassDefinition(pData->fListDeviceMapClass->reference());

		// Create an empty result list to contain the result
		STAFObjectPtr resultList = STAFObject::createList();
		STAFMutexSemLock lock(*pData->fDeviceMapSem);
		DeviceMap::iterator iter;

		for (iter = pData->fDeviceMap.begin();
		     iter != pData->fDeviceMap.end(); ++iter) {

			if (!board_id ||  (boardName == iter->second->board_id)) { //list all
				STAFObjectPtr resultMap = pData->fListDeviceMapClass->createInstance();
				resultMap->put("ID",    iter->second->ID);
				resultMap->put("board_id",    iter->second->board_id);
//            resultMap->put("type",    "board_id");
				resultMap->put("board_type",   iter->second->board_type);
				resultMap->put("chip_name",   iter->second->chip_name);
				resultMap->put("chip_stepping",   iter->second->chip_stepping);
				resultMap->put("board_register_date",   iter->second->board_register_date);
				resultMap->put("board_status",   iter->second->board_status);
				resultMap->put("user_team",   iter->second->user_team);
				resultMap->put("current_user",   iter->second->current_user);
				resultMap->put("board_eco",   iter->second->board_eco);
				resultMap->put("lcd_resolution",   iter->second->lcd_resolution);
				resultMap->put("lcd_screensize",   iter->second->lcd_screensize);
				resultMap->put("ddr_type",   iter->second->ddr_type);
				resultMap->put("ddr_size",   iter->second->ddr_size);
				resultMap->put("emmc_type",   iter->second->emmc_type);
				resultMap->put("emmc_size",   iter->second->emmc_size);
				resultMap->put("rf_name",   iter->second->rf_name);
				resultMap->put("rf_type",   iter->second->rf_type);
				resultMap->put("serial",   iter->second->serial);
				resultMap->put("mcu",   iter->second->mcu);
				resultMap->put("usb",   iter->second->usb);
				resultMap->put("machine",   iter->second->machine);
				resultMap->put("physicalInterfaceID",   iter->second->physicalInterfaceID);
				// resultMap->put("status",   iter->second->status);
				resultMap->put("online",   iter->second->online);
				resultMap->put("board",   iter->second->board);
				resultMap->put("shareStartTime",   iter->second->shareStartTime);
				resultMap->put("username",   iter->second->username);
				resultMap->put("userteam",   iter->second->userteam);
				resultMap->put("dro",   iter->second->dro);
				resultMap->put("chip_type",   iter->second->chip_type);
				resultMap->put("task_lock",   iter->second->task_lock);
				resultMap->put("under_task",   iter->second->under_task);
				resultList->append(resultMap);
			}
		}
		mc->setRootObject(resultList);
	}else if (jobs) { //list all the job status
		mc->setMapClassDefinition(pData->fListJobDataClass->reference());
		// Create an empty result list to contain the result
		STAFObjectPtr resultList = STAFObject::createList();
		STAFMutexSemLock lock(*pData->fJobListSem);
		JobList::iterator iter;

		for (iter = pData->fJobList.begin();
		     iter != pData->fJobList.end(); ++iter) {
			STAFObjectPtr resultMap = pData->fListJobDataClass->createInstance();
			resultMap->put("JobName",    (*iter)->JobName);
			resultMap->put("ID",    (*iter)->ID);
			resultMap->put("board_id",    (*iter)->board_id);
			resultMap->put("ipaddr",    (*iter)->ipaddr);
			resultMap->put("taskName",    (*iter)->taskName);
			resultMap->put("caseName",    (*iter)->caseName);
			resultMap->put("startTime",    (*iter)->startTime);
			resultMap->put("statusUpdateTime",    (*iter)->statusUpdateTime);
			resultMap->put("status",    (*iter)->status);
			resultMap->put("statusAll",    (*iter)->statusAll);
			resultMap->put("output",    (*iter)->output);
			resultList->append(resultMap);
		}
		mc->setRootObject(resultList);
	}else if (tasks) { //list the interested task status
		STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "TASKNAME");
		if (resultPtr->rc != kSTAFOk) return resultPtr;
		STAFString taskName_list_string = resultPtr->result;
		mc->setMapClassDefinition(pData->fListJobDataClass->reference());
		// Create an empty result list to contain the result
		STAFObjectPtr resultList = STAFObject::createList();
		STAFMutexSemLock lock(*pData->fJobListSem);
		JobList::iterator iter;


	std::list<STAFString> taskName_list;
	int prepos = 0;
	unsigned int found = taskName_list_string.findFirstOf("&&", prepos);
	while (found != -1) 
	{
		STAFString taskName= taskName_list_string.subString(prepos, found - prepos);
		prepos = found + 2;
		found = taskName_list_string.findFirstOf("&&", prepos);
		taskName_list.push_back(taskName);
	}
	STAFString taskName_last=taskName_list_string.subString(prepos, taskName_list_string.length());
	taskName_list.push_back(taskName_last);
/*
		for ( std::list<STAFString>::iterator it = taskName_list.begin(); it != taskName_list.end(); it++ )
		 {
		  	STAFString taskName;taskName=(*it);
			cout<<"check:"<<taskName<<":"<<endl;
		 }
*/



		for (iter = pData->fJobList.begin();
		     (iter != pData->fJobList.end())  ; ++iter) {

		STAFString taskName;
		for ( std::list<STAFString>::iterator it = taskName_list.begin(); it != taskName_list.end(); it++ )
		 {
		  	taskName=(*it);
			cout<<"search:"<<taskName<<"::in jobs"<<endl;
			if (taskName == (*iter)->taskName) {
				STAFObjectPtr resultMap = pData->fListJobDataClass->createInstance();
				resultMap->put("JobName",    (*iter)->JobName);
				resultMap->put("ID",    (*iter)->ID);
				resultMap->put("board_id",    (*iter)->board_id);
				resultMap->put("ipaddr",    (*iter)->ipaddr);
				resultMap->put("taskName",    (*iter)->taskName);
				resultMap->put("caseName",    (*iter)->caseName);
				resultMap->put("startTime",    (*iter)->startTime);
				resultMap->put("statusUpdateTime",    (*iter)->statusUpdateTime);
				resultMap->put("status",    (*iter)->status);
				resultMap->put("statusAll",    (*iter)->statusAll);
				resultMap->put("output",    (*iter)->output);
				resultList->append(resultMap);
			}
		}
		}
		mc->setRootObject(resultList);
	}else if (guis) {

		STAFString GuiSetOut;
		std::set<GuiData>::iterator it;
		{
			STAFMutexSemLock lock(*pData->fGuiSetSem);
			for (it = pData->fGuiSet.begin(); it != pData->fGuiSet.end(); it++) {
				GuiSetOut += it->physicalInterfaceID;
				GuiSetOut += ":";
				GuiSetOut += it->Machine;
				GuiSetOut += "\n";
			}
		}
		return STAFResultPtr(new STAFResult(kSTAFOk, GuiSetOut),
				     STAFResultPtr::INIT);
	}


	return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()),
			     STAFResultPtr::INIT);
} //end of handle list


//handle run request
static STAFResultPtr  runCaseWithTaskName(DeviceServiceData *pData, const STAFString &ipaddr, const STAFString &taskName, const std::vector<STAFString> &caseList, const JobMap &jobMap, const STAFString &board_id)
{

	// Resolve the machine name variable for the local machine
	/*   STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "CASENAME");
	   STAFString caseid = resultPtr->result;*/
	//lauch the rcc_util to run test case
	STAFString PreRunCaseCommand;

	PreRunCaseCommand += "start command ./rcc_utilc parms ";
	PreRunCaseCommand += " ltkex ";
	STAFString RunCaseCommand;
	RunCaseCommand += " STDERRTOSTDOUT RETURNSTDOUT NOTIFY ONEND HANDLE ";
	RunCaseCommand += pData->fHandlePtr->getHandle();


	STAFResultPtr resultcmd ;
	{
		//{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}
		PoolMap::iterator poolIterator;
		poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());
		DEBUGOUTFILE("spoolname=" << sPoolName << " size=" << pData->fPoolMap.size());
		if (poolIterator == pData->fPoolMap.end())
			return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, sPoolName),
					     STAFResultPtr::INIT);
		DEBUGOUTFILE("resourceList.size=" << (*poolIterator).second->resourceList.size());
		STAFString JobStartTime = STAFTimestamp::now().asString().replace(":", "");
		STAFString JobXmlName;
		JobXmlName += JobStartTime;
		JobXmlName += ".xml ";

		if ( (*poolIterator).second->resourceList.size()) {
			//step 1: generate the task xml
			//xml generation the job xml, named <task>_<caseid>.xml;
			//STAFString jobGenCmd(" start command ./job_xml_generate PARMS -t job.xml CaseList testcase id ");//the board detect binary is hard-coded rcc_utilc

			STAFString jobGenCmd(" start command ./job_xml_generate PARMS -t job.xml ");
			/*	jobGenCmd +=" General taskname ";
				jobGenCmd +=taskName;
				jobGenCmd +=" /General ";*/
			std::vector<STAFString> config;
			config.push_back(sGeneral);
			config.push_back(sImageConfig);
			config.push_back(sLTKConfig);
                        config.push_back(sLogConfig);
			config.push_back(sExtraAct);
			for (int c = 0; c < config.size(); c++) {
				STAFString temp = config.at(c);
				if (jobMap.count(temp)) {
					jobGenCmd += " ";
					jobGenCmd += temp;
					jobGenCmd += " ";
					std::map<STAFString, STAFString> tempInnerMap = jobMap.at(temp);
					for (std::map<STAFString, STAFString>::iterator it = tempInnerMap.begin(); it != tempInnerMap.end(); it++) {
						jobGenCmd += " ";
						jobGenCmd += it->first;
						jobGenCmd += " ";
						jobGenCmd += it->second;
					}
					jobGenCmd += " /";
					jobGenCmd += temp;
				}
			}

			jobGenCmd += " CaseList ";
			for (int i = 0; i < caseList.size(); i++) {
				jobGenCmd += " testcase id ";
//						jobGenCmd += caseid;

STAFString test_case_id = caseList.at(i);
if ( test_case_id.find("-loop-",0) != -1 )
{
int case_id_size=test_case_id.findFirstOf("-loop-",0);
int loop_str_begin=case_id_size+6;
int case_loop_size=test_case_id.length()-case_id_size;
STAFString test_case = test_case_id.subString(0,case_id_size);
STAFString test_loop = test_case_id.subString(loop_str_begin,case_loop_size);
jobGenCmd += test_case;
jobGenCmd += " loop ";
jobGenCmd += test_loop;
cout << test_case << "for " << test_loop << " loops"<<endl;
}
else
{
				jobGenCmd += test_case_id;
				}


				jobGenCmd += " /testcase ";
			}
			jobGenCmd += " /CaseList -f ";
			jobGenCmd += JobXmlName;
			jobGenCmd += " wait 5s";
			DEBUGOUTFILE("jobGenCmd=" << jobGenCmd);
			resultcmd =  pData->fHandlePtr->submit("local", "PROCESS", jobGenCmd );
			if (resultcmd->rc == kSTAFOk)
				DEBUGOUTFILE("PASS to generated job xml with task " << taskName);
			else{
				if (resultcmd->rc == kSTAFTimeout) {
					DEBUGOUTFILE("Timeout failed to generated job xml with task " << taskName);
					return STAFResultPtr(new STAFResult(resultcmd->rc,  resultcmd->result), STAFResultPtr::INIT);
				}else{
					DEBUGOUTFILE("ERROR failed to generated job xml with task " << taskName);
					return STAFResultPtr(new STAFResult(resultcmd->rc,  resultcmd->result), STAFResultPtr::INIT);
				}
			} //end of if
		} //end of step 1



		//step 2: copy the , like: "staf local fs copy file 20140424-135751.xml.xml TOFILE /usr/local/staf/data/STAF/user/pxa1928-20140424-135751.xml FAILIFEXISTS tomachine 10.38.37.20"
		STAFString CopyFileCommand;
		CopyFileCommand += " copy file ";
		CopyFileCommand += JobXmlName;
		CopyFileCommand += " FAILIFEXISTS ";
		CopyFileCommand += "	TOFILE ";
		CopyFileCommand += " {STAF/DataDir}";
		CopyFileCommand += "{STAF/Config/Sep/File}";
		CopyFileCommand += "user";
		CopyFileCommand += "{STAF/Config/Sep/File}";
		//CopyFileCommand += " tomachine ";
		PoolDataPtr poolPtr = (*poolIterator).second;
		bool taskMatch = false;
		for (unsigned int j = 0; j < poolPtr->resourceList.size(); j++) {
			if ( (1 == poolPtr->resourceList[j].owned) && (poolPtr->resourceList[j].taskName == taskName) && (poolPtr->resourceList[j].pDevice->board_id == board_id)) {
				taskMatch = true;
				STAFString logMsg;
				logMsg += "[ID=";
				logMsg += poolPtr->resourceList[j].pDevice->ID;
				logMsg += "] ";
				logMsg += PreRunCaseCommand;
				log(pData, "start",      logMsg);
				//		poolPtr->resourceList[j].device.status="start";
				{ //update the fDeviceMap part status.
				 /*STAFMutexSemLock lock(*pData->fDeviceMapSem);
				    DeviceMap::iterator iter = pData->fDeviceMap.find(poolPtr->resourceList[j].pDevice->ID);
				    if (iter == pData->fDeviceMap.end()){
					 DEBUGOUT("can't find the key="<<poolPtr->resourceList[j].pDevice->ID);
					 return STAFResultPtr(new STAFResult(kSTAFLtkTestRunNotKeyAvailable,poolPtr->resourceList[j].pDevice->ID),STAFResultPtr::INIT);
				    }else{
					 iter->second->status="start";
				    }*/
				}
				JobData job;
				//JobData job;
				job.ID = poolPtr->resourceList[j].pDevice->ID;
				job.board_id = poolPtr->resourceList[j].pDevice->board_id;
				job.ipaddr = ipaddr;
				job.taskName = taskName;
				job.JobName = job.board_id + "-" + JobStartTime;
				job.startTime = STAFTimestamp::now().asString();
				for (int i = 0; i < caseList.size(); i++) {
					//job.caseName=caseid;
					job.caseName += caseList.at(i);
					job.caseName += " ";
				}
				//copy the xml to each lanuched board side
				STAFString EachCopyFileCommand = CopyFileCommand;
				EachCopyFileCommand += job.JobName;
				EachCopyFileCommand += ".xml ";
				EachCopyFileCommand += " TOMACHINE ";
				EachCopyFileCommand += poolPtr->resourceList[j].pDevice->physicalInterfaceID;
				resultcmd = pData->fHandlePtr->submit("local", "fs", EachCopyFileCommand);
				DEBUGOUTFILE("EachCopyFileCommand=" << EachCopyFileCommand << " rc=" << resultcmd->rc);
				if (resultcmd->rc != kSTAFOk)
					return STAFResultPtr(new STAFResult(resultcmd->rc,  resultcmd->result), STAFResultPtr::INIT);
				STAFString EachCommand = PreRunCaseCommand;
				EachCommand += poolPtr->resourceList[j].pDevice->board_id;
				EachCommand += " runJob ";
				EachCommand += " {STAF/DataDir}";
				EachCommand += "{STAF/Config/Sep/File}";
				EachCommand += "user/";
				//EachCommand += JobXmlName;
				EachCommand += job.JobName;
				EachCommand += ".xml";
				EachCommand += " ";
				EachCommand += RunCaseCommand;
				EachCommand += " KEY ";
				EachCommand += job.JobName;
				//EachCommand += poolPtr->resourceList[j].pDevice->ID;
				resultcmd = pData->fHandlePtr->submit(poolPtr->resourceList[j].pDevice->physicalInterfaceID, "PROCESS", EachCommand);
				DEBUGOUTFILE("EachCommand=" << EachCommand << " rc=" << resultcmd->rc);
				//check if the job has been lanuched successfully
				if (resultcmd->rc != kSTAFOk)
					job.status = "Fail to lanuch";
				else
					{ 
						job.status = "start";
						poolPtr->resourceList[j].pDevice->under_task=true;
					}
				{ //update the fJobList part, add a new record into it or may lbzhu update it if existed
					STAFMutexSemLock lock(*pData->fJobListSem);
					pData->fJobList.push_back(JobDataPtr(new JobData(job), JobDataPtr::INIT));
				}
				return STAFResultPtr(new STAFResult(resultcmd->rc,  resultcmd->result), STAFResultPtr::INIT);        //find the job and return
				break; //
			} //end of if

			DEBUGOUTFILE("owned=" << poolPtr->resourceList[j].owned << " j=" << j << " taskName=" << poolPtr->resourceList[j].taskName);
		} //end of for

		if (false == taskMatch)
			return STAFResultPtr(new STAFResult(kSTAFLtkTestNotTaskMatch, taskName), STAFResultPtr::INIT);DEBUGOUTFILE("RunCaseCommand=" << RunCaseCommand);
		DEBUGOUT("end");
	}
	return STAFResultPtr(new STAFResult(kSTAFOk,  resultcmd->result), STAFResultPtr::INIT);
}
STAFResultPtr handleRun(STAFServiceRequestLevel30 *pInfo,
			DeviceServiceData *pData)
{
	STAFString result;
	STAFRC_t rc = kSTAFOk;

	// Verify the requester has at least trust level 2
	VALIDATE_TRUST(2, pData->fShortName, "RUN", pData->fLocalMachineName);

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fRunParser->parse(pInfo->request);
	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);

	// Check if definte multi cases
	unsigned int numEntriesToAdd = parsedResult->optionTimes("CASENAME");
	unsigned int i;
	std::vector<STAFString> CaseList;

	for (i = 1; i <= numEntriesToAdd; i++) {
		STAFString thisid = parsedResult->optionValue("CASENAME", i);

		CaseList.push_back(thisid);
	}
	//resolve the TASKNAME variable
	STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sTaskName);
	if (resultPtr->rc != 0) return resultPtr;

	STAFString taskName = resultPtr->result;
//	STAFResultPtr resultcmd = runCaseWithTaskName(pData,taskName,CaseList);
	STAFResultPtr resultcmd ;

	return STAFResultPtr(new STAFResult(resultcmd->rc,  resultcmd->result), STAFResultPtr::INIT);
}


// Handles job  update requests
STAFResultPtr handleJobUpdate(STAFServiceRequestLevel30 *pInfo,
			      DeviceServiceData *pData)
{
	STAFString result;
	STAFRC_t rc = kSTAFOk;

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fJobStatusParser->parse(
		pInfo->request);

	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);


	// Resolve any STAF variables in the board_id option's value
	STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, "JOBNAME");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString jobname = resultPtr->result;

	resultPtr = resolveOp(pInfo, pData, parsedResult, "JOBSTATUS");
	if (resultPtr->rc != kSTAFOk) return resultPtr;
	STAFString jobstatus = resultPtr->result;
	STAFString ipaddr;

	STAFMutexSemLock lock(*pData->fJobListSem);
	int i = 0;
	for (i = 0; i < pData->fJobList.size(); i++) {
		//if(pData->fJobList.at(i)->ID==key)
		if (pData->fJobList.at(i)->JobName == jobname) {
			DEBUGOUTFILE("PASS find the jobname=" << jobname << " status=" << jobstatus);
			pData->fJobList.at(i)->statusUpdateTime = STAFTimestamp::now().asString();
			pData->fJobList.at(i)->status = jobstatus;
			pData->fJobList.at(i)->statusAll += jobstatus;
			pData->fJobList.at(i)->statusAll += ";";
			ipaddr = pData->fJobList.at(i)->ipaddr;
			break;
		}
	}

	if (i == pData->fJobList.size())
		DEBUGOUTFILE("can't find the jobname=" << jobname);
	else
		NotifyGui(pData, kJob, jobname, ipaddr); //means job update
	// Return the marshalled result of the query
	return STAFResultPtr(new STAFResult(kSTAFOk, "update pass"),
			     STAFResultPtr::INIT);
}


// Handles device query requests

STAFResultPtr handleQuery(STAFServiceRequestLevel30 *pInfo,
			  DeviceServiceData *pData)
{
	STAFString result;
	STAFRC_t rc = kSTAFOk;

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fQueryParser->parse(
		pInfo->request);

	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);

	// Set the poolName variable (resolve the pool name)
	/*STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);
	   if (resultPtr->rc != 0) return resultPtr;
	   STAFString poolName = resultPtr->result;
	    disable the poolname choice*/
	STAFString poolName = sPoolName;
	bool pending = false;
	if (parsedResult->optionTimes("PENDING"))
		pending = true;

	// Create a marshalled map of general pool information
	STAFObjectPtr mc = STAFObject::createMarshallingContext();
	mc->setMapClassDefinition(pData->fPoolInfoClass->reference());
	mc->setMapClassDefinition(pData->fRequestClass->reference());
	mc->setMapClassDefinition(pData->fReadyClass->reference());
	mc->setMapClassDefinition(pData->fResourceClass->reference());
	mc->setMapClassDefinition(pData->fResourceOwnerClass->reference());

	STAFObjectPtr poolInfoMap = pData->fPoolInfoClass->createInstance();
cout<<"set query lock"<<endl;
	// Get a read lock on the Pool Map for the duration of this block
	{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}
//{
//	{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}
//}
//STAFThreadManager::sleepCurrentThread(10000);
cout<<"set query lock end"<<endl;
	// Make sure the resource pool is in the Pool Map and get a pointer to it
	PoolMap::iterator poolIterator;
	PoolDataPtr poolPtr;

	poolIterator = pData->fPoolMap.find(poolName.toUpperCase());
	if (poolIterator == pData->fPoolMap.end())
		return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
				     STAFResultPtr::INIT);

	// Set a pointer to the resource pool being queried
	poolPtr = (*poolIterator).second;
	poolInfoMap->put("description", poolPtr->poolDescription);

	// Create an empty list object for marshalled list of pending requests
	STAFObjectPtr requestList = STAFObject::createList();
	// Append an entry to requestList for each pending request

	if (poolPtr->requestList.size() > 0) {
		RequestList::iterator iList;

		for (iList = poolPtr->requestList.begin();
		     iList != poolPtr->requestList.end(); ++iList) {
			// Add an entry in a marshalling context as a map to requestList
			STAFObjectPtr requestMap = pData->fRequestClass->createInstance();
			requestMap->put("taskName", (*iList)->taskName);
			requestMap->put("requestFromIpaddr", (*iList)->orgEndpoint);
			requestMap->put("requestedTimestamp", (*iList)->requestedTime);
			requestMap->put("priority", (*iList)->priority);

			if ((*iList)->requestType == kEntry)
				requestMap->put("requestedEntry", (*iList)->requestedEntry);
			else
				requestMap->put("requestedEntry", STAFObject::createNone());

			requestList->append(requestMap);
		}
	}
	poolInfoMap->put("requestList", requestList);

	// Create an empty list object for marshalled list of pending requests
	STAFObjectPtr readyList = STAFObject::createList();
	// Append an entry to readyList for each pending request

	if (poolPtr->readyList.size() > 0) {
		ReadyList::iterator iList;

		for (iList = poolPtr->readyList.begin();
		     iList != poolPtr->readyList.end(); ++iList) {
			// Add an entry in a marshalling context as a map to readyList
			STAFObjectPtr readyMap = pData->fReadyClass->createInstance();
			readyMap->put("taskName", (*iList)->taskName);
			readyMap->put("requestFromIpaddr", (*iList)->orgEndpoint);
			readyMap->put("requestedTimestamp", (*iList)->requestedTime);
			readyMap->put("priority", (*iList)->priority);

			if ((*iList)->requestType == kEntry)
				readyMap->put("requestedEntry", (*iList)->requestedEntry);
			else
				readyMap->put("requestedEntry", STAFObject::createNone());

			readyList->append(readyMap);
		}
	}
	poolInfoMap->put("readyList", readyList);

	// Create an empty list object for the marshalled list of the pool's
	// resource entries
	STAFObjectPtr resourceList = STAFObject::createList();

	// Append an entry to resourceList for each resource
	for (unsigned int i = 0; i < poolPtr->resourceList.size(); i++) {
		// Create a map of information about each resource
		STAFObjectPtr resourceMap = pData->fResourceClass->createInstance();
		resourceMap->put("entry", poolPtr->resourceList[i].pDevice->board_id);
		if (poolPtr->resourceList[i].preTasks.size()) { //check the size of previous tasks occupied
			std::set<STAFString>::iterator it;
			STAFString preTasksJoin;
			for (it = poolPtr->resourceList[i].preTasks.begin(); it != poolPtr->resourceList[i].preTasks.end(); it++) {
				preTasksJoin += *it;
				preTasksJoin += ";";
			}
			resourceMap->put("preTasks", preTasksJoin);
		}else
			resourceMap->put("preTasks", STAFObject::createNone());

		if (!poolPtr->resourceList[i].owned) //check if the resource is free or not
			resourceMap->put("owner", STAFObject::createNone());
		else{
			// Create a map of information about the resource owner
			STAFObjectPtr resourceOwnerMap =
				pData->fResourceOwnerClass->createInstance();

			resourceOwnerMap->put("taskName",
					      poolPtr->resourceList[i].taskName);
			resourceMap->put("owner", resourceOwnerMap);
		}

		// Add the entry in a marshalling context as a map to resourceList
		resourceList->append(resourceMap);
	}

	if (false == pending)
		poolInfoMap->put("resourceList", resourceList);

	// Set the marshalling context's root object
	mc->setRootObject(poolInfoMap);

	// Return the marshalled result of the query
	return STAFResultPtr(new STAFResult(kSTAFOk, mc->marshall()),
			     STAFResultPtr::INIT);
}

// Handles resource pool request requests

STAFResultPtr handleRequest(STAFServiceRequestLevel30 *pInfo,
			    DeviceServiceData *pData)
{
	STAFString result;
	STAFRC_t rc = kSTAFOk;

	// Verify the requester has at least trust level 5
	VALIDATE_TRUST(5, pData->fShortName, "REQUEST", pData->fLocalMachineName);

	// Parse the request
	STAFCommandParseResultPtr parsedResult = pData->fRequestParser->parse(
		pInfo->request);
	if (parsedResult->rc != kSTAFOk)
		return STAFResultPtr(new STAFResult(kSTAFInvalidRequestString,
						    parsedResult->errorBuffer), STAFResultPtr::INIT);

	STAFResultPtr resultPtr;
	// Set the poolName variable (resolve the pool name)
	/*STAFResultPtr resultPtr = resolveOp(pInfo, pData, parsedResult, sPool);
	   if (resultPtr->rc != 0) return resultPtr;
	   STAFString poolName = resultPtr->result;
	    //disable the pool name choice */
	STAFString poolName = sPoolName;

	// Set the poolName variable (resolve the TASKNAME name)
	resultPtr = resolveOp(pInfo, pData, parsedResult, sTaskName);
	if (resultPtr->rc != 0) return resultPtr;
	STAFString taskName = resultPtr->result;

	// Set the request type to random, first, or specified entry
	RequestType requestType = kRandom; // Random is the default request type
	STAFString requestedEntry = STAFString("");

	if (parsedResult->optionTimes(sEntry) > 0) {
		requestType = kEntry;
		resultPtr = resolveOp(pInfo, pData, parsedResult, sEntry);
		if (resultPtr->rc != 0) return resultPtr;
		requestedEntry = resultPtr->result;
	}

	unsigned int priority = sDefaultPriority;

	if (parsedResult->optionTimes(sPriority) > 0) {
		resultPtr = resolveOp(pInfo, pData, parsedResult, sPriority);
		if (resultPtr->rc != kSTAFOk) return resultPtr;

		// Convert resolved option string to an unsigned integer in range
		// 0 to 99
		resultPtr = convertOptionStringToUInt(
			resultPtr->result, sPriority, priority,
			sMinimumPriority, sMaximumPriority);
		if (resultPtr->rc != kSTAFOk) return resultPtr;
	}

	ProcessSync startMode = kAsync;
	unsigned int timeout = STAF_EVENT_SEM_INDEFINITE_WAIT;

	if (parsedResult->optionTimes(sTimeout) > 0) {
		// Set the timeout variable (resolve the TIMEOUT option value and check if
		// it is a valid duration string and if so convert it to its numeric value
		// in milliseconds

		resultPtr = resolveOp(pInfo, pData, parsedResult, sTimeout);
		if (resultPtr->rc != 0) return resultPtr;
		STAFString_t errorBuffer = 0;
		STAFRC_t rc = STAFUtilConvertDurationString(
			resultPtr->result.getImpl(), &timeout, &errorBuffer);

		if (rc != kSTAFOk) {
			return STAFResultPtr(
				       new STAFResult(
					       rc, STAFString("Invalid value for the TIMEOUT option: ") +
					       resultPtr->result + " \n\n" +
					       STAFString(errorBuffer, STAFString::kShallow)),
				       STAFResultPtr::INIT);
		}
		startMode = kWait; //lbzhu
	}

	// Check if definte multi cases, put the casename into CaseList
	unsigned int numEntriesToAdd = parsedResult->optionTimes("CASENAME");
	unsigned int i;
	std::vector<STAFString> CaseList;
	for (i = 1; i <= numEntriesToAdd; i++) {
		STAFString thisid = parsedResult->optionValue("CASENAME", i);

		CaseList.push_back(thisid);
	}

	// Check the job configuration,sGeneral,sImageConfig,sLTKConfig
	JobMap jobMap;
	std::vector<STAFString> config;
	config.push_back(sGeneral);
	config.push_back(sImageConfig);
	config.push_back(sLTKConfig);
        config.push_back(sLogConfig);
	config.push_back(sExtraAct);
	if (parsedResult->optionTimes(sGeneral) == 0) { //special case for taskname
		std::map<STAFString, STAFString> tempInnerMap;
		tempInnerMap.insert(std::pair<STAFString, STAFString>("taskname", taskName));
		jobMap.insert(JobMap::value_type(sGeneral, tempInnerMap));
		DEBUGOUTFILE("add taskname since no configuration for " << sGeneral);
	}

	for (int c = 0; c < config.size(); c++) { //still need go through other configurations
		numEntriesToAdd = parsedResult->optionTimes(config.at(c));
		if (numEntriesToAdd >= 1) {
			std::map<STAFString, STAFString> tempInnerMap;
			if (config.at(c) == sGeneral)
				tempInnerMap.insert(std::pair<STAFString, STAFString>("taskname", taskName));
			for (i = 1; i <= numEntriesToAdd; i++) {
				STAFString thisid = parsedResult->optionValue(config.at(c), i);
				int pos = thisid.findFirstOf(sConfigSep);
				STAFString key = thisid.subString(0, pos);
				STAFString value = thisid.subString(pos + 1, thisid.length() - pos - 1);
				DEBUGOUTFILE("thisid=" << thisid << " pos=" << pos << " key=" << key << " value=" << value);
				tempInnerMap.insert(std::pair<STAFString, STAFString>(key, value));
			}
			jobMap.insert(JobMap::value_type(config.at(c), tempInnerMap));
		}
	}

	// check if we need run casename
	bool NOTRUN = false;
	if (parsedResult->optionTimes("NOTRUN") > 0)
		NOTRUN = true;

	// Start processing the request
	RequestDataPtr requestDataPtr;
	PoolMap::iterator poolIterator;
	PoolDataPtr poolPtr;
	STAFResultPtr resultRunCase;
	STAFString board_id;
	STAFString physicalInterfaceID = pInfo->physicalInterfaceID;

	// Get a read lock on the Pool Map for the duration of this block
	{
		{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}

		// Make sure the specified resource pool is in the PoolMap
		poolIterator = pData->fPoolMap.find(poolName.toUpperCase());
		if (poolIterator == pData->fPoolMap.end())
			return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, poolName),
					     STAFResultPtr::INIT);

		poolPtr = (*poolIterator).second;
		unsigned int i;
		cout << __func__ << "|" << __LINE__ << endl;
		// Lock the poolData semaphore for the duration of this block
		{
			STAFMutexSemLock lock(*poolPtr->accessSem);
			STAFString ID;

			/*remove judgement about if the entry existed in the pool
				       if (requestType == kEntry)
			   {
			    // Verify that the specified entry exists in the pool

			    bool found = false;

			    for (i = 0; i < poolPtr->resourceList.size(); i++)
			    {
			   //       if (poolPtr->resourceList[i].pDevice->board_id == requestedEntry)
						    if(RequestData::existInPool(poolPtr->resourceList[i].pDevice,requestedEntry))
						    {
				    found = true;
							    ID=poolPtr->resourceList[i].pDevice->ID;
						    }
			    }

			    if (!found)
			    {
				// Specified entry does not exist

				return STAFResultPtr(
				    new STAFResult(
					kSTAFDoesNotExist, STAFString("Entry ") +
					requestedEntry + " does not exist"),
				    STAFResultPtr::INIT);
			    }
			   }
			   else if (poolPtr->resourceList.size() == 0)
			   {
			    return STAFResultPtr(new STAFResult(
				kSTAFLtkTestNoEntriesAvailable, poolName),
				STAFResultPtr::INIT);
			   }*/

			/*lbzhu if (garbageCollect)
			   {
			    // Register for notification when the handle ends

			    STAFResultPtr resultPtr = submitSTAFNotifyRegisterRequest(
				pData, pInfo->handle, pInfo->endpoint,
				pInfo->stafInstanceUUID);

			    if (resultPtr->rc != 0) return resultPtr;
			   }*/

			// Check if there are any available resources

			bool isResourceAvailable = false;
			bool owner = false;
			unsigned int resid;

			if (requestType == kEntry) {
				// A specific entry was requested
				bool entryExists = false;

				// Check if the specified entry exists and is available
				for (i = 0; i < poolPtr->resourceList.size(); i++) {
					//if (poolPtr->resourceList[i].pDevice->board_id == requestedEntry)
					if ( (poolPtr->resourceList[i].preTasks.count(taskName) == 0) && (RequestData::existInPool(poolPtr->resourceList[i].pDevice, requestedEntry))) {
						entryExists = true;
						if (!poolPtr->resourceList[i].owned) //lbzhu needs change

							isResourceAvailable = true;
						else //it will keep going through the whole resourcelist to check if it can be ownered

							continue;

						break;
					}
				}

				if (!entryExists) {
//may update add the input entry sensible or not
//						return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, STAFString("Entry ") +requestedEntry + " does not exist"),STAFResultPtr::INIT);
					DEBUGOUTFILE("Entry " << requestedEntry << " does not exist, but we support this feature by adding it into the pending list for delay deployment feature");
				}
			}else if ((poolPtr->numResources - poolPtr->usedResources) > 0) {
				// There are available resources and either the first
				// available entry or a random entry was requested
				unsigned int rIndex = rand() % (poolPtr->numResources -
								poolPtr->usedResources);

				// Find the first or Nth available resource in the list
				for (i = 0; i < poolPtr->resourceList.size(); i++) {
					if ((!poolPtr->resourceList[i].owned) &&
					    ((requestType == kFirst) || !rIndex--)) {
						isResourceAvailable = true;
						break;
					}
				}
			}

			if (isResourceAvailable) {
				// Mark the resource as OWNED and return the entry info

				poolPtr->usedResources++;
				poolPtr->resourceList[i].owned = 1;
				// poolPtr->resourceList[i].orgMachine = pInfo->machine;
				poolPtr->resourceList[i].taskName = taskName;
				STAFString currentTime = STAFTimestamp::now().asString();
				poolPtr->resourceList[i].requestedTime = currentTime;
				poolPtr->resourceList[i].acquiredTime = currentTime;
				poolPtr->resourceList[i].garbageCollect = false;
				poolPtr->resourceList[i].preTasks.insert(taskName);
				//poolPtr->resourceList[i].garbageCollect = garbageCollect;

				// Return the entry assigned to the request, lbzhu , push back an job entry in the handlerun() function
				//pData->fJobList.push_back(JobDataPtr(new JobData(taskName,poolPtr->resourceList[i].pDevice->ID,poolPtr->resourceList[i].pDevice->board_id,kUnknown),
				//						JobDataPtr::INIT));
				/*
				   lanuch the test case
				 */
				if (false == NOTRUN) {
					DEBUGOUTFILE("launch test case since resource require is met at the first, does NOT need delay");
					STAFString delayComp(taskName);
					delayComp += ";";
					delayComp += poolPtr->resourceList[i].pDevice->board_id;
					NotifyGui(pData, kDelay, delayComp, physicalInterfaceID); //notify GUI about the board assignment
					resultRunCase = runCaseWithTaskName(pData, physicalInterfaceID, taskName, CaseList, jobMap, poolPtr->resourceList[i].pDevice->board_id);
					if (resultRunCase->rc == kSTAFOk)
					{
						return STAFResultPtr(new STAFResult(resultRunCase->rc,  poolPtr->resourceList[i].pDevice->board_id + " result is " + resultRunCase->result), STAFResultPtr::INIT);
					}
					else{
						board_id = poolPtr->resourceList[i].pDevice->board_id;
						DEBUGOUTFILE("resource release since runcase fails");
						goto out;
					}
				}else{
					return STAFResultPtr(new STAFResult(kSTAFOk,
									    poolPtr->resourceList[i].pDevice->board_id),
							     STAFResultPtr::INIT);
				}
			}
			bool AsyncOrNot = (startMode == kWait) ? false : true;
			RequestData requestData(pInfo->stafInstanceUUID, taskName, CaseList, jobMap,
						pInfo->handleName, pInfo->handle,
						pInfo->user, pInfo->physicalInterfaceID,
						false, AsyncOrNot,
						//garbageCollect,
						requestType, requestedEntry, priority);

			requestDataPtr = RequestDataPtr(new RequestData(requestData),
							RequestDataPtr::INIT);

			RequestList::iterator iter;
			/*//add duplication request support, for delay deployment support
			   // ignore request duplication
			   //return kSTAFAlreadyExists means the same request has been put into the pending list
			   for (iter = poolPtr->requestList.begin();
			   iter != poolPtr->requestList.end() ;
			 ++iter)
			   {
					if((taskName == (*iter)->taskName )
									&& (requestType == (*iter)->requestType)
									&& (requestedEntry == (*iter)->requestedEntry))
					{
					STAFString retString("requester with taskname ");
						retString += taskName;
						retString += ", reqeuestedEntry ";
						retString += requestedEntry;
						retString += " has been put into pending request, ingore this time";

			   return STAFResultPtr(new STAFResult(kSTAFAlreadyExists,retString),
				    STAFResultPtr::INIT);
					}

			   }*/

			bool addedToList = false;
			// Insert the request into the pending request list in
			// ascending order by priority/requestedTime
			for (iter = poolPtr->requestList.begin();
			     iter != poolPtr->requestList.end() && !addedToList;
			     ++iter) {
				if (priority < (*iter)->priority) {
					poolPtr->requestList.insert(iter, requestDataPtr);
					addedToList = true;
				}
			} // End for loop for iterating thru pending request list

			if (!addedToList) {
				// Add to the end of the pending request list

				poolPtr->requestList.push_back(requestDataPtr);
			}


		}       // End block for locking the PoolData access semaphore

	}               // End block for putting a read lock on the PoolMap

	// Wait for the specified time for a resource to become available

	if (startMode == kWait)
		requestDataPtr->wakeup->wait(timeout);
	else{ //lbzhu add ASYNC support, the notify mechnaism needs update too.
		DEBUGOUTFILE("startMode=" << startMode);

		requestDataPtr->retCode = kSTAFLtkTestPutPendingRequests; //means it will be put into pending list
		///put it into pending job queue, may just insert it into resource pending queue. while it's ready, lanuch it
		//add casename into the requestdata
		//pData->fJobList.push_back(JobDataPtr(new JobData(taskName,ID,requestedEntry,kPending),JobDataPtr::INIT));
		return STAFResultPtr(new STAFResult(requestDataPtr->retCode,
						    requestDataPtr->resultBuffer), STAFResultPtr::INIT);
	}

	// If request's return code is not 0 (e.g. timed out or if a specific
	// entry was requested but it no longer exists in the resource list,
	// or if the request was cancelled), remove the request from the list.
	// Save the cost of getting a semaphore lock by first checking if the
	// request failed.

	if (requestDataPtr->retCode != kSTAFOk) {
		// Lock the poolData semaphore for the duration of this block
		STAFMutexSemLock lock(*poolPtr->accessSem);
		if (requestDataPtr->retCode != kSTAFOk) {
			poolPtr->requestList.remove(requestDataPtr);

			/*lbzhu if (garbageCollect)
			   {
			    // Delete the notification from the handle notification list

			    submitSTAFNotifyUnregisterRequest(
				pData, pInfo->handle, pInfo->endpoint,
				pInfo->stafInstanceUUID);
			   }*/
		}
	}else{ //lanuch the case

		if (false == NOTRUN) {
			DEBUGOUTFILE("currently does NOT support wait method");
			board_id = requestDataPtr->board_id;
//		resultRunCase= runCaseWithTaskName(pData,physicalInterfaceID,taskName,CaseList,jobMap,requestDataPtr->board_id);
//		if(resultRunCase->rc==kSTAFOk){
//			return STAFResultPtr(new STAFResult(resultRunCase->rc,  resultRunCase->result), STAFResultPtr::INIT);
//		}else{
//			board_id=requestDataPtr->board_id;
//			DEBUGOUTFILE("resource release since runcase fails");
//			goto out;
//		}
		}else{
			DEBUGOUTFILE("resource got and return ");
			return STAFResultPtr(new STAFResult(kSTAFOk,
							    requestDataPtr->board_id),
					     STAFResultPtr::INIT);
		}
	}

 out:
	DEBUGOUTFILE("resource release begins");
	STAFResultPtr resultcmd = resourceRelease(pData, board_id);
	DEBUGOUTFILE("resource release ends,ret=" << resultcmd->rc);

	// Return the return code and result assigned to the request

	return STAFResultPtr(new STAFResult(kSTAFOk,
					    "out resource release"), STAFResultPtr::INIT);
}

STAFResultPtr handleGuiRegister(STAFServiceRequestLevel30 *pInfo,
				DeviceServiceData *pData)
{
	// Verify the requester has at least trust level 3
	VALIDATE_TRUST(3, pData->fShortName, "GUIREGISTER", pData->fLocalMachineName);
	STAFString physicalInterfaceID = pInfo->physicalInterfaceID;
	STAFString machineName = pInfo->machineNickname;
	GuiData registerTemp(physicalInterfaceID, machineName);
	{
		STAFMutexSemLock lock(*pData->fGuiSetSem);
		if (pData->fGuiSet.end() == pData->fGuiSet.find(registerTemp)) {
			DEBUGOUT("register done " << physicalInterfaceID);
			pData->fGuiSet.insert(registerTemp);
		}else
			DEBUGOUT("already registered");
	}

	// Return help text for the service
	return STAFResultPtr(new STAFResult(kSTAFOk, physicalInterfaceID),
			     STAFResultPtr::INIT);
}


STAFResultPtr handleGuiUnregister(STAFServiceRequestLevel30 *pInfo,
				  DeviceServiceData *pData)
{
	// Verify the requester has at least trust level 1

	VALIDATE_TRUST(3, pData->fShortName, "GUIUNREGISTER", pData->fLocalMachineName);
	STAFString physicalInterfaceID = pInfo->physicalInterfaceID;
	GuiData registerTemp(physicalInterfaceID);
	{
		STAFMutexSemLock lock(*pData->fGuiSetSem);
		if (pData->fGuiSet.end() == pData->fGuiSet.find(registerTemp))
			DEBUGOUT("never registered, ingnore GUI unregister");
		else{
			DEBUGOUT("GUI unregistered for " << physicalInterfaceID);
			pData->fGuiSet.erase(registerTemp);
		}
	}

	return STAFResultPtr(new STAFResult(kSTAFOk, physicalInterfaceID),
			     STAFResultPtr::INIT);
}

STAFResultPtr handleHelp(STAFServiceRequestLevel30 *pInfo,
			 DeviceServiceData *pData)
{
	// Verify the requester has at least trust level 1

	VALIDATE_TRUST(1, pData->fShortName, "HELP", pData->fLocalMachineName);

	// Return help text for the service

	return STAFResultPtr(new STAFResult(kSTAFOk, sHelpMsg),
			     STAFResultPtr::INIT);
}


STAFResultPtr handleVersion(STAFServiceRequestLevel30 *pInfo,
			    DeviceServiceData *pData)
{
	// Verify the requester has at least trust level 1

	VALIDATE_TRUST(1, pData->fShortName, "VERSION", pData->fLocalMachineName);

	// Return the version of the service

	return STAFResultPtr(new STAFResult(kSTAFOk, sVersionInfo),
			     STAFResultPtr::INIT);
}


STAFResultPtr resolveOp(STAFServiceRequestLevel30 *pInfo,
			DeviceServiceData *pData,
			STAFCommandParseResultPtr &parsedResult,
			const STAFString &fOption, unsigned int optionIndex)
{
	STAFString optionValue = parsedResult->optionValue(fOption, optionIndex);

	if (optionValue.find(sLeftCurlyBrace) == STAFString::kNPos)
		return STAFResultPtr(new STAFResult(kSTAFOk, optionValue),
				     STAFResultPtr::INIT);

	return resolveStr(pInfo, pData, optionValue);
}


STAFResultPtr resolveStr(STAFServiceRequestLevel30 *pInfo,
			 DeviceServiceData *pData,
			 const STAFString &theString)
{
	return pData->fHandlePtr->submit(sLocal, sVar, sResStrResolve +
					 STAFString(pInfo->requestNumber) +
					 sString +
					 pData->fHandlePtr->wrapData(theString));
}


void registerHelpData(DeviceServiceData *pData, unsigned int errorNumber,
		      const STAFString &shortInfo, const STAFString &longInfo)
{
	static STAFString regString("REGISTER SERVICE %C ERROR %d INFO %C "
				    "DESCRIPTION %C");

	pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
					  regString.getImpl(), pData->fShortName.getImpl(), errorNumber,
					  shortInfo.getImpl(), longInfo.getImpl()));
}


void unregisterHelpData(DeviceServiceData *pData, unsigned int errorNumber)
{
	static STAFString regString("UNREGISTER SERVICE %C ERROR %d");

	pData->fHandlePtr->submit(sLocal, sHelp, STAFHandle::formatString(
					  regString.getImpl(), pData->fShortName.getImpl(), errorNumber));
}

void registerShutDown(DeviceServiceData *pData)
{
	static STAFString regString("NOTIFY REGISTER ");

	pData->fHandlePtr->submit(sLocal, "SHUTDOWN", regString);
}

// Handles resource pool release entry requests
STAFResultPtr resourceRelease( DeviceServiceData *pData, const STAFString &releaseEntry)
{
	cout<<"resourceRelease"<<endl;
	STAFString result;
	STAFRC_t rc = kSTAFOk;
	// Get a read lock on the Pool Map for the duration of this block
	{cout << __func__ <<"|" << __LINE__<< endl;} STAFRWSemRLock  rLock(*pData->fPoolMapRWSem);{cout << __func__ <<"|" << __LINE__<< endl;}

	// Make sure that the resource pool is in pData->poolMap
	PoolMap::iterator poolIterator;
	PoolDataPtr poolPtr;

	poolIterator = pData->fPoolMap.find(sPoolName.toUpperCase());

	if (poolIterator == pData->fPoolMap.end())
		return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, sPoolName),
				     STAFResultPtr::INIT);

	poolPtr = (*poolIterator).second;

	// Lock the poolData semaphore for the duration of this block
	STAFMutexSemLock lock(*poolPtr->accessSem);

	// Find the entry in the resource pool
	bool entryExists = false;
	unsigned int resid;

	for (unsigned int i = 0; i < poolPtr->resourceList.size(); i++) {
		if (releaseEntry == poolPtr->resourceList[i].pDevice->board_id) {
			entryExists = true;
			resid = i;
cout<<"set board free:"<<poolPtr->resourceList[resid].pDevice->board_id<<poolPtr->resourceList[resid].pDevice->under_task<<endl;
					poolPtr->resourceList[resid].pDevice->under_task=false;
			break;
		}
	}

	// If the entry is not in the resource pool, return an error

	if (!entryExists)
		return STAFResultPtr(new STAFResult(kSTAFDoesNotExist, releaseEntry),
				     STAFResultPtr::INIT);

	// Check if the entry requested to be released is owned

	if (poolPtr->resourceList[resid].owned) {
		// Mark the resource as available
		poolPtr->usedResources--;
		poolPtr->resourceList[resid].owned = 0;

		// Iterate through the pending requests.  Find the first pending
		// request that can be satisfied.  A pending request can be
		// satisfied if either:
		//   a) requestType != kEntry (e.g. kFirst or kRandom)
		//   or
		//   b) requestType == kEntry and the entry just released
		//      matches the requestedEntry.
		// If a pending request can be satisfied, tell this requester
		// the resource he can have.

		if (poolPtr->requestList.size() > 0) {
			RequestList::iterator iter;
			RequestDataPtr reqPtr;

			for (iter = poolPtr->requestList.begin();
			     iter != poolPtr->requestList.end(); ++iter) {
				reqPtr = *iter;

				if ((reqPtr->requestType != kEntry) ||
				    ((reqPtr->requestType == kEntry) &&
				     (poolPtr->resourceList[resid].preTasks.count(reqPtr->taskName) == 0) &&
				     RequestData::existInPool(poolPtr->resourceList[resid].pDevice, reqPtr->requestedEntry)
				    )
				    ) {
					// Assign the resource to the request

					reqPtr->retCode = kSTAFOk;
					reqPtr->resultBuffer =
						poolPtr->resourceList[resid].pDevice->board_id;
					reqPtr->board_id = poolPtr->resourceList[resid].pDevice->board_id;
					// Update the resource entry's ownership information

					poolPtr->resourceList[resid].owned = 1;
					poolPtr->usedResources++;
					poolPtr->resourceList[resid].requestedTime = reqPtr->requestedTime;
					poolPtr->resourceList[resid].taskName = reqPtr->taskName;
					poolPtr->resourceList[resid].acquiredTime = STAFTimestamp::now().asString();
					poolPtr->resourceList[resid].garbageCollect = reqPtr->garbageCollect;
					poolPtr->resourceList[resid].preTasks.insert(reqPtr->taskName);
					cout<<"set board free:"<<poolPtr->resourceList[resid].pDevice->board_id<<poolPtr->resourceList[resid].pDevice->under_task<<endl;
					poolPtr->resourceList[resid].pDevice->under_task=false;
					if (true == reqPtr->AsyncOrNot)      //Async
						poolPtr->readyList.push_back(reqPtr);
						/*STAFResultPtr resultcmd = runCaseWithTaskName(pData,reqPtr->orgEndpoint, reqPtr->taskName,reqPtr->CaseList,reqPtr->jobMap,reqPtr->board_id);
						   if(resultcmd->rc!=kSTAFOk){
						   DEBUGOUTFILE("resource release begin");
						   STAFResultPtr resultcmd=resourceRelease(pData,reqPtr->board_id);
						   DEBUGOUTFILE("resource release end,ret="<<resultcmd->rc);
						   }*/
					else
						// Wakeup the requester
						reqPtr->wakeup->post();

					// Remove the satisfied request from the pending
					// request list and break out of the loop
					poolPtr->requestList.erase(iter);
					break;
				}
			}
		}else
			return STAFResultPtr(new STAFResult(KSTAFLtkTestNotEntryOwner, releaseEntry), STAFResultPtr::INIT);
	} // end if entry was owned

	// Return an Ok result
	return STAFResultPtr(new STAFResult(kSTAFOk, result), STAFResultPtr::INIT);
}

// Write the resource pool file.  Note that pool files are always written
// in the current format.
//previous version, it writes the poolName and PoolDescription into the file
//then write all the entries into that files

unsigned int writePoolFile(const STAFString &fileName, PoolData &poolData)
{
	// Open the pool file
	fstream poolfile(fileName.toCurrentCodePage()->buffer(),
			 ios::out | STAF_ios_binary);

	if (!poolfile) return kFileOpenError;

	// Write to the pool file
	writeUIntToFile(poolfile, sCurrFileFormat);
	writeStringToFile(poolfile, poolData.poolName);
	writeStringToFile(poolfile, poolData.poolDescription);
	unsigned int numResources = poolData.resourceList.size();
	writeUIntToFile(poolfile, numResources);
	DEBUGOUT("numResources=" << numResources);

	for (unsigned int i = 0; i < numResources; i++) {
		//the sequence is important
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->board_id);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->board_type);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->machine);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->physicalInterfaceID);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->ID);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->chip_name);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->chip_stepping);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->board_register_date);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->board_status);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->user_team);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->current_user);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->board_eco);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->lcd_resolution);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->lcd_screensize);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->ddr_type);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->ddr_size);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->emmc_type);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->emmc_size);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->rf_name);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->rf_type);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->serial);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->mcu);
		writeStringToFile(poolfile, poolData.resourceList[i].pDevice->usb);
		//       writeStringToFile(poolfile, poolData.resourceList[i].pDevice->username);
	}

	return kReadorWriteOk;
}

void writeUIntToFile(ostream &output, unsigned int data, unsigned int length)
{
	union {
		char bytes[4];
		unsigned int uint;
	};
	uint = STAFUtilSwapUInt(STAFUtilConvertNativeUIntToLE(data));

	output.write(&bytes[4 - length], length);
}


void readStringFromFile(istream &input, STAFString &inString)
{
	// First read in the UTF-8 data
	unsigned int stringLength = 0;

	readUIntFromFile(input, stringLength);

	char *inputData = new char[stringLength];
	input.read(inputData, stringLength);

	try
	{
		inString = STAFString(inputData, stringLength, STAFString::kUTF8);
	}
	catch (...)
	{
		delete [] inputData;
		throw;
	}

	delete [] inputData;
}

//write the string length and string into file
void writeStringToFile(ostream &output, STAFString &outString)
{
	unsigned int stringLength = outString.length();

	writeUIntToFile(output, stringLength);
	output.write(outString.buffer(), stringLength);
}

void readUIntFromFile(istream &input, unsigned int &data, unsigned int length)
{
	union {
		char bytes[4];
		unsigned int uint;
	};

	uint = 0;

	input.read(&bytes[4 - length], length);

	data = STAFUtilConvertLEUIntToNative(STAFUtilSwapUInt(uint));
}

// Read a resource pool file and store its contents in PoolData.
// Currently, there are two formats of resource pool files supported:
// - 1 is the current binary format used by the C++ Respool Service in STAF 2.3
//   and later
// - "0" is the old text format used by the REXX Respool Service in STAF 2.2
//   and earlier

unsigned int readPoolFile(const STAFString &fileName, PoolData &poolData)
{
	unsigned int rc = kReadorWriteOk;

	// Open the pool file (in binary mode)
	fstream poolfile(fileName.toCurrentCodePage()->buffer(),
			 ios::in | STAF_ios_binary);

	DEBUGOUT("before check pool file open");
	if (!poolfile) return kFileOpenError;

	// Check the format of the file to determine how to read the file
	DEBUGOUT("after check pool file open");
	readUIntFromFile(poolfile, poolData.fileFormat);
	DEBUGOUT("fileFormat=" << poolData.fileFormat);
	if (poolfile.eof()) return kReadEndOfFile;
	DEBUGOUT("after check pool file end");

	if (poolData.fileFormat == sCurrFileFormat) {
		// This is the current binary format used by the C++ Respool Service
		readStringFromFile(poolfile, poolData.poolName);
		readStringFromFile(poolfile, poolData.poolDescription);
		readUIntFromFile(poolfile, poolData.numResources);
		DEBUGOUT("poolData.poolName=" << poolData.poolName);
		DEBUGOUT("poolData.poolDescription=" << poolData.poolDescription);
		DEBUGOUT("poolData.numResources=" << poolData.numResources);

		STAFString entry, board_type, machine, physicalInterfaceID, ID, chip_name, chip_stepping,
			   board_register_date, board_status, user_team, current_user,
			   board_eco, lcd_resolution, lcd_screensize, ddr_type, ddr_size,
			   emmc_type, emmc_size, rf_name, rf_type, serial, mcu, usb, username, userteam, dro, chip_type;



		for (unsigned int i = 0; i < poolData.numResources; i++) {
			readStringFromFile(poolfile, entry);
			readStringFromFile(poolfile, board_type);
			readStringFromFile(poolfile, machine);
			readStringFromFile(poolfile, physicalInterfaceID);
			readStringFromFile(poolfile, ID);
			readStringFromFile(poolfile, chip_name);
			readStringFromFile(poolfile, chip_stepping);
			readStringFromFile(poolfile, board_register_date);
			readStringFromFile(poolfile, board_status);
			readStringFromFile(poolfile, user_team);
			readStringFromFile(poolfile, current_user);
			readStringFromFile(poolfile, board_eco);
			readStringFromFile(poolfile, lcd_resolution);
			readStringFromFile(poolfile, lcd_screensize);
			readStringFromFile(poolfile, ddr_type);
			readStringFromFile(poolfile, ddr_size);
			readStringFromFile(poolfile, emmc_type);
			readStringFromFile(poolfile, emmc_size);
			readStringFromFile(poolfile, rf_name);
			readStringFromFile(poolfile, rf_type);
			readStringFromFile(poolfile, serial);
			readStringFromFile(poolfile, mcu);
			readStringFromFile(poolfile, usb);
			// readStringFromFile(poolfile, username);

			ResourceData resourceData(DeviceDataPtr( new DeviceData(entry, board_type, machine, physicalInterfaceID, ID, chip_name, chip_stepping, board_register_date,
										board_status, user_team, current_user, board_eco, lcd_resolution, lcd_screensize,
										ddr_type, ddr_size, emmc_type, emmc_size, rf_name, rf_type, serial, mcu, usb, "", username, userteam, dro, chip_type ), DeviceDataPtr::INIT));
			poolData.resourceList.push_back(resourceData);
		}
	}else
		DEBUGOUT("wrong format" << poolData.fileFormat);

	return rc;
}


// Write the resource pool file.  Note that pool files are always written
STAFResultPtr convertOptionStringToUInt(const STAFString &theString,
					const STAFString &optionName,
					unsigned int &number,
					const unsigned int minValue,
					const unsigned int maxValue)
{
	// Convert an option value to an unsigned integer

	STAFString_t errorBufferT = 0;

	STAFRC_t rc = STAFUtilConvertStringToUInt(
		theString.getImpl(), optionName.getImpl(), &number,
		&errorBufferT, minValue, maxValue);

	if (rc == kSTAFOk)
		return STAFResultPtr(new STAFResult(), STAFResultPtr::INIT);
	else{
		return STAFResultPtr(
			       new STAFResult(rc, STAFString(errorBufferT, STAFString::kShallow)),
			       STAFResultPtr::INIT);
	}
}


