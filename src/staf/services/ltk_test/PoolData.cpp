#include "PoolData.h"
#include "STAFMutexSem.h"

static const unsigned int sCurrFileFormat = 1;
PoolData::PoolData() : usedResources(0),
	accessSem(new STAFMutexSem(), STAFMutexSemPtr::INIT)
{       /* Do nothing */
}

PoolData::PoolData(const STAFString &aPoolName, const STAFString &aPoolDescription)
	: poolName(aPoolName), poolDescription(aPoolDescription),
	numResources(0), usedResources(0),
	accessSem(new STAFMutexSem(), STAFMutexSemPtr::INIT)
{
	fileFormat = sCurrFileFormat;
}


