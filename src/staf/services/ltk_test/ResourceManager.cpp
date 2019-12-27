#include "ResourceManager.h"
#include "STAFTimestamp.h"
#include "LtkTest.h"
static const int sDefaultPriority = 50;

RequestData::RequestData() : wakeup(new STAFEventSem(), STAFEventSemPtr::INIT), retCode(kSTAFTimeout)
{
	requestedTime = STAFTimestamp::now().asString();
	wakeup->reset();
	*garbageCollectedPtr = false;
	garbageCollect = true;
	priority = sDefaultPriority;

}

RequestData::RequestData(const STAFString &aUUID, const STAFString &ataskName,
			 const std::vector<STAFString> &aCaseList, const JobMap &aJobMap,
			 const STAFString &aHandleName, const STAFHandle_t aHandle,
			 const STAFString aUser, const STAFString aEndpoint,
			 const bool aGarbageCollect, const bool aAsyncOrNot,
			 const RequestType aRequestType,
			 const STAFString &aRequestedEntry,
			 const int aPriority)
	: orgUUID(aUUID), taskName(ataskName), CaseList(aCaseList), jobMap(aJobMap),
	orgName(aHandleName), orgHandle(aHandle), orgUser(aUser), orgEndpoint(aEndpoint),
	wakeup(new STAFEventSem(), STAFEventSemPtr::INIT),
	retCode(kSTAFTimeout),
	garbageCollectedPtr(new bool, STAFRefPtr<bool>::INIT),
	garbageCollect(aGarbageCollect), AsyncOrNot(aAsyncOrNot),
	requestType(aRequestType), requestedEntry(aRequestedEntry),
	priority(aPriority)
{
	requestedTime = STAFTimestamp::now().asString();
	wakeup->reset();
	*garbageCollectedPtr = false;
}
const STAFString RequestData::sConfigSep("=");
const STAFString RequestData::sAndSep("&&");
const STAFString RequestData::sOrSep("||");
/*const STAFString RequestData::sAndSep("&&");
   bool RequestData::existInPool(const DeviceDataPtr &pDevice, STAFString requestEntry)
   {
	int pos=requestEntry.findFirstOf(sConfigSep);
	STAFString key=requestEntry.subString(0,pos);
	STAFString value=requestEntry.subString(pos+1,requestEntry.length()-pos-1);
	cout<<"requestEntry="<<requestEntry<<" pos="<<pos<<" key="<<key<<" value="<<value<<"id="<<pDevice->board_id<<endl;
	if(key=="board_id")
	{
		if(pDevice->board_id==value){
				cout<<"got it, board_id"<<pDevice->board_id;
				return true;
		}else
			return false;
	}else if(key=="board_type"){
		if(pDevice->board_type==value){
				cout<<"got it, board_id"<<pDevice->board_id;
				return true;
		}else
			return false;
	}
	return true;
   }*/
/*
   key function to check if the currently pDevice meet the requirement shown in requestEntry
   currently it supports the AND combination.i.e.
   <key>=<value>&&<key>=<value>&&<key>=<value>

   besides, only if the board machine's staf connection is on AND board status is online or NULL, it will return true
 */
bool RequestData::existInPool(const DeviceDataPtr &pDevice, STAFString requestEntry)
{
	std::map<STAFString, STAFString> requestMap;
	int pos = 0;
	int prepos = 0;
	int stride = sAndSep.length();
	STAFString key;
	STAFString value;

	unsigned int found = requestEntry.findFirstOf(sAndSep, prepos);
	while (found != -1) {
		STAFString tempEntry = requestEntry.subString(prepos, found - prepos);
		int subpos = tempEntry.findFirstOf(sConfigSep);
		if (subpos != -1) {
			key = tempEntry.subString(0, subpos);
			value = tempEntry.subString(subpos + 1, tempEntry.length() - subpos - 1);
			requestMap.insert(std::pair<STAFString, STAFString>(key, value));
		}
		prepos = found + stride;;
		found = requestEntry.findFirstOf(sAndSep, prepos);

	}
	STAFString tempEntry = requestEntry.subString(prepos);
	int subpos = tempEntry.findFirstOf(sConfigSep);
	if (subpos != -1) {
		key = tempEntry.subString(0, subpos);
		value = tempEntry.subString(subpos + 1, tempEntry.length() - subpos - 1);
		requestMap.insert(std::pair<STAFString, STAFString>(key, value));
	}

	bool ret = true;
	bool lock_task=false;
	bool online_match = true;
	bool ignore_offline = false;
	for (std::map<STAFString, STAFString>::iterator it = requestMap.begin(); ((it != requestMap.end()) && (ret == true)); it++) {
		key = it->first;
		value = it->second;
		bool match = true;
		//bool online_match = true;
		//bool ignore_offline = false;
		if (key == "board_id")
			match = (pDevice->board_id == value);else if (key == "board_type")
			match = (pDevice->board_type == value);else if (key == "chip_name")
			match = (pDevice->chip_name == value);else if (key == "chip_stepping")
			match = (pDevice->chip_stepping == value);else if (key == "lcd_resolution")
			match = (pDevice->lcd_resolution == value);else if (key == "lcd_screensize")
			match = (pDevice->lcd_screensize == value);else if (key == "ddr_type")
			match = (pDevice->ddr_type == value);else if (key == "ddr_size")
			match = (pDevice->ddr_size == value);else if (key == "emmc_type")
			match = (pDevice->emmc_type == value);else if (key == "emmc_size")
			match = (pDevice->emmc_size == value);else if (key == "dro")
			match = (pDevice->dro == value);else if (key == "chip_type")
			match = (pDevice->chip_type == value);else if (key == "rf_name")
			match = (pDevice->rf_name == value);else if (key == "rf_type")
			match = (pDevice->rf_type == value);else if (key == "ignore_offline")
			{ignore_offline=true;}else if(key=="lock_task")
			{lock_task=true;}else
			DEBUGOUTFILE(" the key is an unknown one, just ignore this field");
		if( pDevice->task_lock )
		{
			match=false; 
			cout<<"task lock"<<endl;
		}

		//cout<<key<<pDevice->board_id<<pDevice->board.length()<<"|match:"<<match<<endl;
		if (pDevice->online == "offline" || ((pDevice->board.find("Online") == -1) && pDevice->board.length())) { //means staf offline, or board is offline

			online_match = false;


			DEBUGOUTFILE("false because of the staf connection=" << pDevice->online << ",board connection=" << pDevice->board << ",");
		}
		//cout<<" key="<<it->first<<" value="<<it->second<<" ret="<<ret<<" match="<<match<<" board_id="<<pDevice->board_id<<" board_type="<<pDevice->board_type<<endl;
		ret = ret && match ;
		cout<<" key="<<it->first<<" value="<<it->second<<"ret:"<<ret<<"match:"<<match<<"online-match"<<online_match<<"key:"<<key<<"ignore:"<<ignore_offline<<endl;
	}
	if ( ignore_offline )
	{online_match = true;}
	ret = ret && online_match;
	if (true == ret)
	{
		DEBUGOUTFILE("got it, board_id" << pDevice->board_id);
		bool board_running=pDevice->under_task;
		cout<<"BOARD:"<<pDevice->board_id<<"|in_job:"<<board_running<<"|lock:"<<lock_task<<endl;
		if ( lock_task && (!board_running) )
		{ 
			pDevice->task_lock=true; 
			cout<<"set task lock true"<<endl;
		}
		//cout<<pDevice->board_id<<":match:"<<ret<<endl;
	}

	return ret;
}
