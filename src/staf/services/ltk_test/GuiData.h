#ifndef GUI__DATA__H
#define GUI__DATA__H
#include <set>
#include "STAF.h"
#include "STAFMutexSem.h"
#include "STAFCommandParser.h"
#include "STAFServiceInterface.h"
#include "STAFEventSem.h"
#include "STAFUtil.h"

struct GuiData {
	GuiData()
	{       /* Do Nothing */
	}

	GuiData(const STAFString &physicalInterfaceID, const STAFString &Machine=STAFString()) :
		physicalInterfaceID(physicalInterfaceID), Machine(Machine)
	{       /* Do Nothing */
	}

	friend bool operator >(const GuiData &first, const GuiData &second)
	{
		return first.physicalInterfaceID > second.physicalInterfaceID;
	}

	friend bool operator <(const GuiData &first, const GuiData &second)
	{
		return first.physicalInterfaceID < second.physicalInterfaceID;
	}
	STAFString Machine;                                             //machine name
	STAFString physicalInterfaceID;                                 //ipaddr

};
typedef std::set<GuiData> GuiSet;
#endif
