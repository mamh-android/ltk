#ifndef  DEVICE__DATA__H
#define DEVICE__DATA__H
#include <deque>
#include <map>
#include <vector>
#include <list>
#include <set>
#include "STAF.h"
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFEventSem.h"
#include "STAFRWSem.h"
#include "STAFThreadManager.h"
#include "STAFUtil.h"
//for respool + job scheduel
#include "STAF_fstream.h"
#include "STAFTimestamp.h"
#include "STAFFileSystem.h"
//#include "STAFDataTypes.h"
// Device Data - contains data for a device

struct DeviceData {
	DeviceData()
	{       /* Do Nothing */
		 off_line_count=0;
		 task_lock=false;
	}

	DeviceData(const STAFString &board_id, const STAFString &board_type, const STAFString &machine, const STAFString &physicalInterfaceID, const STAFString &ID,
		   const STAFString &chip_name=STAFString(),
		   const STAFString &chip_stepping=STAFString(), const STAFString &board_register_date=STAFString(), const STAFString &board_status=STAFString(),
		   const STAFString &user_team=STAFString(), const STAFString & current_user=STAFString(), const STAFString &board_eco=STAFString(),
		   const STAFString &lcd_resolution=STAFString(), const STAFString &lcd_screensize=STAFString(), const STAFString &ddr_type=STAFString(),
		   const STAFString &ddr_size=STAFString(), const STAFString &emmc_type=STAFString(), const STAFString &emmc_size=STAFString(),
		   const STAFString &rf_name=STAFString(), const STAFString &rf_type=STAFString(), const STAFString &serial=STAFString(), const STAFString &mcu=STAFString(),
		   const STAFString &usb=STAFString(), const STAFString &shareStartTime=STAFString(), const STAFString &username=STAFString(), const STAFString &userteam=STAFString(), const STAFString &dro=STAFString(), const STAFString &chip_type=STAFString()

		   ) :
		board_id(board_id), board_type(board_type), machine(machine), physicalInterfaceID(physicalInterfaceID), ID(ID), chip_name(chip_name), chip_stepping(chip_stepping),
		board_register_date(board_register_date), board_status(board_status), user_team(user_team), current_user(current_user), board_eco(board_eco),
		lcd_resolution(lcd_resolution), lcd_screensize(lcd_screensize), ddr_type(ddr_type), ddr_size(ddr_size), emmc_type(emmc_type), emmc_size(emmc_size),
		rf_name(rf_name), rf_type(rf_type), serial(serial), mcu(mcu), usb(usb), shareStartTime(shareStartTime), username(username), userteam(userteam), dro(dro), chip_type(chip_type),
		online(""), board("")               //default value
	{       /* Do Nothing */
	}

	STAFString board_id;            // Device board_id name
	STAFString board_type;          // Device board_type
	STAFString chip_name;
	STAFString chip_stepping;
	STAFString board_register_date;     //5
	STAFString board_status;
	STAFString user_team;
	STAFString current_user;
	STAFString board_eco;
	STAFString lcd_resolution;          //10
	STAFString lcd_screensize;
	STAFString ddr_type;
	STAFString ddr_size;
	STAFString emmc_type;
	STAFString emmc_size;              //15
	STAFString rf_name;
	STAFString rf_type;
	STAFString serial;
	STAFString mcu;
	STAFString usb;                                 //20 end of eeprom
	STAFString ID;                                  //auto generated identification of deviceData
	STAFString machine;                             //registered machine name
	STAFString physicalInterfaceID;                 //registered machine physical ipaddr
//	STAFString   status;               //pass,fail,running
	STAFString online;                              //online or offline
	STAFString board;                               //on or off
	STAFString shareStartTime;                      //indicate when start shareing this board
	STAFString username;                            //indicate the user name which is written by user shares the board
        STAFString userteam;                            //indicate the user team which is written by user shares the board
        STAFString dro;                            //indicate the user team which is written by user shares the board
        STAFString chip_type;                            //indicate the user team which is written by user shares the board
	int off_line_count;
	bool task_lock;
	bool under_task;
};

typedef std::deque<DeviceData> DeviceList; //no use

typedef STAFRefPtr<DeviceData> DeviceDataPtr;

// DeviceMap -- KEY:   Device name,
//              VALUE: Pointer to DeviceData information

typedef std::map<STAFString, DeviceDataPtr> DeviceMap;

typedef STAFRefPtr<DeviceData> DeviceServiceDataPtr; //no use

#endif
