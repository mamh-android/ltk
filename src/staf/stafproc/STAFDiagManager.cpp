/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFDiagManager.h"
#include "STAFTimestamp.h"


STAFRC_t STAFDiagManager::record(const STAFString &trigger,
                                 const STAFString &source)
{
    if (fEnabled)
    {
        STAFString key = trigger.toLowerCase() + source.toLowerCase();

        STAFMutexSemLock diagMapLock(fDiagMapSem);

        if (fDiagMap.find(key) == fDiagMap.end())
            fDiagMap[key] = DiagData(trigger, source);
        else
            fDiagMap[key].count++;

        return kSTAFOk;
    }
    else
    {
        return kSTAFDiagnosticsNotEnabled;
    }
}


STAFRC_t STAFDiagManager::record(const STAFString &trigger,
                                 const STAFString &handleName,
                                 const STAFString &machine,
                                 STAFHandle_t handle)
{
    STAFString source = handleName + ";" + machine + ";" + handle;

    return record(trigger, source);
}

STAFRC_t STAFDiagManager::reset()
{
    STAFMutexSemLock diagMapLock(fDiagMapSem);

    // Clear diagnostics map
    fDiagMap.clear();
    
    fResetTimestamp = STAFTimestamp::now();

    // Set disabled timestamp to current time as well
    fDisabledTimestamp = fResetTimestamp;

    return kSTAFOk;
}


STAFRC_t STAFDiagManager::enable()
{
    if (!fEnabled)
    {
        fEnabled = 1;

        if (fResetTimestamp == 0)
        {
            // Get first enabled timestamp
            fResetTimestamp = STAFTimestamp::now();
        }
    }
    
    return kSTAFOk;
}


STAFRC_t STAFDiagManager::disable()
{
    if (fEnabled)
    {
        fDisabledTimestamp = STAFTimestamp::now();

        fEnabled = 0;
    }

    return kSTAFOk;
}


unsigned int STAFDiagManager::getEnabled()
{
    return fEnabled;
}


STAFString STAFDiagManager::getEnabledAsString()
{
    if (fEnabled)
        return "Enabled";
    else
        return "Disabled";
}


STAFTimestamp STAFDiagManager::getDisabledTimestamp()
{
    return fDisabledTimestamp;
}


STAFTimestamp STAFDiagManager::getResetTimestamp()
{
    return fResetTimestamp;
}


STAFDiagManager::DiagMap STAFDiagManager::getDiagMapCopy()
{
    STAFMutexSemLock diagMapLock(fDiagMapSem);

    return fDiagMap;
}
