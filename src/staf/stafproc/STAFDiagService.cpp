/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2004, 2005                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFProc.h"
#include "STAFProcUtil.h"
#include "STAFDiagService.h"
#include "STAFDiagManager.h"
#include "STAFCommandParser.h"
#include <set>
#include <algorithm>
#include <vector>

// RECORD TRIGGER <Trigger> SOURCE <Source>
// LIST   [TRIGGER <Trigger> | SOURCE <Source> | TRIGGERS | SOURCES | SETTINGS]
// RESET  FORCE
// ENABLE
// DISABLE
// HELP

typedef std::set<STAFString> STAFStringList;
typedef std::vector<STAFDiagManager::DiagData> DiagDataVector;

// Descending Count Sorting Function for DiagData

static bool descendingCountSort(STAFDiagManager::DiagData lhs,
                                STAFDiagManager::DiagData rhs)
{
    return (lhs.count > rhs.count);
}

// Ascending Source Sorting Function for DiagData

struct AscendingSourceSort
{
    bool operator()(STAFDiagManager::DiagData lhs,
                    STAFDiagManager::DiagData rhs)
    {
        unsigned int comp = 0;

        STAFStringCompareTo(lhs.source.toUpperCase().getImpl(),
                            rhs.source.toUpperCase().getImpl(),
                            &comp, 0);
        
        return (comp == 1);
    }
};

static const STAFString sNotApplicable = "<N/A>";
static STAFString sHelpMsg;


STAFDiagService::STAFDiagService() : STAFService("DIAG")
{
    // Assign the help text string for the service

    sHelpMsg = STAFString("*** DIAG Service Help ***") +
        *gLineSeparatorPtr + *gLineSeparatorPtr +
        "RECORD TRIGGER <Trigger> SOURCE <Source>" +
        *gLineSeparatorPtr +
        "LIST   < [TRIGGER <Trigger> | SOURCE <Source> | TRIGGERS | SOURCES]" +
        *gLineSeparatorPtr +
        "         [SORTBYCOUNT | SORTBYTRIGGER | SORTBYSOURCE] > |" +
        *gLineSeparatorPtr +
        "       SETTINGS" +
        *gLineSeparatorPtr +
        "RESET  FORCE" +
        *gLineSeparatorPtr +
        "ENABLE" +
        *gLineSeparatorPtr +
        "DISABLE" +
        *gLineSeparatorPtr +
        "HELP";

    // Create the command request parsers

    // Record options
 
    fRecordParser.addOption("RECORD", 1, STAFCommandParser::kValueNotAllowed);
    fRecordParser.addOption("TRIGGER", 1, STAFCommandParser::kValueRequired);
    fRecordParser.addOption("SOURCE", 1, STAFCommandParser::kValueRequired);
 
    // Record groups
 
    fRecordParser.addOptionGroup("TRIGGER", 1, 1);
    fRecordParser.addOptionGroup("SOURCE", 1, 1);
    fRecordParser.addOptionNeed("TRIGGER", "SOURCE");
    fRecordParser.addOptionNeed("SOURCE", "TRIGGER");
 
    // List options

    fListParser.addOption("LIST", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("TRIGGER", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption("SOURCE", 1, STAFCommandParser::kValueRequired);
    fListParser.addOption("TRIGGERS", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SOURCES", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SETTINGS", 1, STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SORTBYCOUNT", 1,
                          STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SORTBYTRIGGER", 1,
                          STAFCommandParser::kValueNotAllowed);
    fListParser.addOption("SORTBYSOURCE", 1,
                          STAFCommandParser::kValueNotAllowed);
 
    // List groups
 
    fListParser.addOptionGroup(
        "TRIGGER SOURCE TRIGGERS SOURCES SETTINGS", 0, 1);
    fListParser.addOptionGroup(
        "SORTBYCOUNT SORTBYTRIGGER SORTBYSOURCE SETTINGS", 0, 1);
 
    // Reset options
 
    fResetParser.addOption("RESET",  1, STAFCommandParser::kValueNotAllowed);
    fResetParser.addOption("FORCE", 1, STAFCommandParser::kValueNotAllowed);
 
    // Reset groups
 
    fResetParser.addOptionGroup("FORCE", 1, 1);

    // Enable options
    
    fEnableParser.addOption("ENABLE", 1, STAFCommandParser::kValueNotAllowed);

    // Disable options

    fDisableParser.addOption("DISABLE", 1,
                             STAFCommandParser::kValueNotAllowed);

    // Construct map class for a settings info

    fSettingsClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/Settings");
    fSettingsClass->addKey("diagnostics", "Diagnostics");
    fSettingsClass->addKey("lastResetTimestamp", "Last Reset / First Enabled");
    fSettingsClass->addKey("lastDisabledTimestamp", "Last Disabled");

    // Construct map class for a list all info

    fAllDiagInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/AllDiagInfo");
    fAllDiagInfoClass->addKey("fromTimestamp", "From Date-Time");
    fAllDiagInfoClass->addKey("toTimestamp", "To Date-Time");
    fAllDiagInfoClass->addKey("elapsedTime", "Elapsed Time");
    fAllDiagInfoClass->addKey("numberOfTriggers", "Number of Triggers");
    fAllDiagInfoClass->addKey("numberOfSources", "Number of Sources");
    fAllDiagInfoClass->addKey("comboList", "Trigger/Source Combinations");

    // Construct map class for a diagnostics info for a trigger

    fTriggerInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/TriggerInfo");
    fTriggerInfoClass->addKey("fromTimestamp", "From Date-Time");
    fTriggerInfoClass->addKey("toTimestamp", "To Date-Time");
    fTriggerInfoClass->addKey("elapsedTime", "Elapsed Time");
    fTriggerInfoClass->addKey("trigger", "Trigger");
    fTriggerInfoClass->addKey("numberOfSources", "Number of Sources");
    fTriggerInfoClass->addKey("sourceList", "Sources");

    // Construct map class for a triggers info

    fTriggersInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/TriggersInfo");
    fTriggersInfoClass->addKey("fromTimestamp", "From Date-Time");
    fTriggersInfoClass->addKey("toTimestamp", "To Timestamp");
    fTriggersInfoClass->addKey("elapsedTime", "Elapsed Time");
    fTriggersInfoClass->addKey("numberOfTriggers", "Number of Triggers");
    fTriggersInfoClass->addKey("triggerList", "Triggers");

    // Construct map class for a source info

    fSourceInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/SourceInfo");
    fSourceInfoClass->addKey("fromTimestamp", "From Date-Time");
    fSourceInfoClass->addKey("toTimestamp", "To Date-Time");
    fSourceInfoClass->addKey("elapsedTime", "Elapsed Time");
    fSourceInfoClass->addKey("source", "Source");
    fSourceInfoClass->addKey("numberOfTriggers", "Number of Triggers");
    fSourceInfoClass->addKey("triggerList", "Triggers");

    // Construct map class for a sources info

    fSourcesInfoClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/SourcesInfo");
    fSourcesInfoClass->addKey("fromTimestamp", "From Date-Time");
    fSourcesInfoClass->addKey("toTimestamp", "To Date-Time");
    fSourcesInfoClass->addKey("elapsedTime", "Elapsed Time");
    fSourcesInfoClass->addKey("numberOfSources", "Number of Sources");
    fSourcesInfoClass->addKey("sourceList", "Sources");
    
    // Construct map class for unique trigger/source combination info

    fComboCountClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/ComboCount");
    fComboCountClass->addKey("trigger", "Trigger");
    fComboCountClass->addKey("source", "Source");
    fComboCountClass->addKey("count", "Count");

    // Construct map class for a count trigger info

    fTriggerCountClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/TriggerCount");
    fTriggerCountClass->addKey("trigger", "Trigger");
    fTriggerCountClass->addKey("count", "Count");

    // Construct map class for a count source info

    fSourceCountClass = STAFMapClassDefinition::create(
        "STAF/Service/Diag/SourceCount");
    fSourceCountClass->addKey("source", "Source");
    fSourceCountClass->addKey("count", "Count");
}


STAFDiagService::~STAFDiagService()
{
    /* Do Nothing */
}


STAFString STAFDiagService::info(unsigned int) const
{
    return name() + ": Internal";
}


STAFServiceResult STAFDiagService::acceptRequest(
    const STAFServiceRequest &requestInfo)
{
    STAFString action = requestInfo.fRequest.subWord(0, 1).lowerCase();
 
    if (action == "record")
        return handleRecord(requestInfo);
    else if (action == "list")
        return handleList(requestInfo);
    else if (action == "reset")
        return handleReset(requestInfo);
    else if (action == "enable")
        return handleEnable(requestInfo);
    else if (action == "disable")
        return handleDisable(requestInfo);
    else if (action == "help") 
        return handleHelp(requestInfo);
    else
    {
        STAFString errMsg = STAFString("'") +
            requestInfo.fRequest.subWord(0, 1) +
            "' is not a valid command request for the " + name() +
            " service" + *gLineSeparatorPtr + *gLineSeparatorPtr +
            sHelpMsg;

        return STAFServiceResult(kSTAFInvalidRequestString, errMsg);
    }
}


STAFServiceResult STAFDiagService::handleRecord(
    const STAFServiceRequest &requestInfo)
{
    // Verify that this request came from the local machine and that
    // the requesting machine/user has at least trust level 3

    IVALIDATE_LOCAL_TRUST(3, "RECORD");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fRecordParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    // Resolve any STAF variables specified in values for TRIGGER and SOURCE
 
    DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
    STAFString errorBuffer;
    STAFString trigger;
    STAFString source;

    STAFRC_t rc = RESOLVE_STRING_OPTION("TRIGGER", trigger);

    if (!rc) rc = RESOLVE_STRING_OPTION("SOURCE", source);

    if (rc) return STAFServiceResult(rc, errorBuffer);
    
    // Add to diagnostics map

    return STAFServiceResult(gDiagManagerPtr->record(trigger, source));
}


STAFServiceResult STAFDiagService::handleList(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 2

    IVALIDATE_TRUST(2, "LIST");
    
    STAFCommandParseResultPtr parsedResult = 
        fListParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }
 
    STAFObjectPtr mc = STAFObject::createMarshallingContext();

    if (parsedResult->optionTimes("SETTINGS") != 0)
    {
        // Create a marshalled map containing the settings for DIAG service

        mc->setMapClassDefinition(fSettingsClass->reference());

        STAFObjectPtr settingsMap = fSettingsClass->createInstance();

        settingsMap->put("diagnostics", gDiagManagerPtr->getEnabledAsString());
        settingsMap->put("lastDisabledTimestamp",
                         gDiagManagerPtr->getDisabledTimestamp().asString());

        if (gDiagManagerPtr->getResetTimestamp() != 0)
            settingsMap->put("lastResetTimestamp",
                             gDiagManagerPtr->getResetTimestamp().asString());
        
        mc->setRootObject(settingsMap);

        return STAFServiceResult(kSTAFOk, mc->marshall());
    }

    // Get a copy of the Diagnostics Map

    STAFDiagManager::DiagMap diagMap = gDiagManagerPtr->getDiagMapCopy();

    // Determine Start Time - If never been enabled/reset, assign "<N/A>".

    STAFString startTime;

    if (gDiagManagerPtr->getResetTimestamp() == 0)
        startTime = sNotApplicable;
    else
        startTime = gDiagManagerPtr->getResetTimestamp().asString();

    // Determine Stop Time - If enabled, use current time; otherwise, use
    // time last disabled.

    STAFString stopTime;

    if (gDiagManagerPtr->getEnabled())
        stopTime = STAFTimestamp::now().asString();
    else
        stopTime = gDiagManagerPtr->getDisabledTimestamp().asString();
    
    // Determine Elapsed Time.  If startTime == "<N/A>", assign "<N/A>".
    
    STAFString elapsedTime;

    if (startTime == sNotApplicable)
        elapsedTime = sNotApplicable;
    else if (STAFTimestamp(startTime) > STAFTimestamp(stopTime))
        elapsedTime = sNotApplicable;
    else
    {
        elapsedTime = STAFTimestamp::getElapsedTime(STAFTimestamp(stopTime) -
                                                    STAFTimestamp(startTime));
    }

    // Determine how to sort the results

    STAFDiagSortBy_t sortBy = kSTAFDiagSortByCount; // Default Sort Option

    if (parsedResult->optionTimes("SORTBYTRIGGER") != 0)
        sortBy = kSTAFDiagSortByTrigger;
    else if (parsedResult->optionTimes("SORTBYSOURCE") != 0)
        sortBy = kSTAFDiagSortBySource;

    // Determine the LIST option which dictates the results to generate.
    
    if (parsedResult->optionTimes("TRIGGER") != 0)
    {
        // List all the sources for the specified trigger

        // Resolve any STAF variables specified in values for TRIGGER

        DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
        STAFString errorBuffer;
        STAFString trigger;

        STAFRC_t rc = RESOLVE_STRING_OPTION("TRIGGER", trigger);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString triggerLowerCase = trigger.toLowerCase();
        unsigned int sourceCount  = 0;
        DiagDataVector diagDataVector;
        STAFDiagManager::DiagMap::iterator mIter;
       
        for (mIter = diagMap.begin(); mIter != diagMap.end(); mIter++)
        {
            if (triggerLowerCase == mIter->second.trigger.toLowerCase())
            {
                diagDataVector.push_back(mIter->second);
                sourceCount++;
            }
        }

        // Sort the diagDataVector (already sorted by trigger)

        switch (sortBy)
        {
            case kSTAFDiagSortByCount:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            case kSTAFDiagSortByTrigger:
            {
                // Sort by count since only one trigger

                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            case kSTAFDiagSortBySource:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          AscendingSourceSort());
                break;
            }
             
            default: break;
        }

        // Create a marshalled map containing diagnostics info for
        // the specified trigger

        mc->setMapClassDefinition(fTriggerInfoClass->reference());
        mc->setMapClassDefinition(fSourceCountClass->reference());

        STAFObjectPtr triggerInfoMap = fTriggerInfoClass->createInstance();

        if (startTime != sNotApplicable)
            triggerInfoMap->put("fromTimestamp", startTime);

        triggerInfoMap->put("toTimestamp", stopTime);

        if (elapsedTime != sNotApplicable)
            triggerInfoMap->put("elapsedTime", elapsedTime);

        triggerInfoMap->put("trigger", trigger);
        triggerInfoMap->put("numberOfSources", STAFString(sourceCount));

        // Iterate thru the sorted diagDataVector and generate the result

        DiagDataVector::iterator vIter;
        STAFObjectPtr outputList = STAFObject::createList();
        
        for (vIter = diagDataVector.begin(); vIter != diagDataVector.end();
             vIter++)
        {
            STAFObjectPtr sourceCountMap = fSourceCountClass->createInstance();
            sourceCountMap->put("count", STAFString(vIter->count));
            sourceCountMap->put("source", vIter->source);

            outputList->append(sourceCountMap);
        }

        triggerInfoMap->put("sourceList", outputList);

        mc->setRootObject(triggerInfoMap);
    }
    else if (parsedResult->optionTimes("SOURCE") != 0)
    {
        // List all the triggers for the specified source

        // Resolve any STAF variables specified in values for SOURCE

        DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, requestInfo);
        STAFString errorBuffer;
        STAFString source;

        STAFRC_t rc = RESOLVE_STRING_OPTION("SOURCE", source);

        if (rc) return STAFServiceResult(rc, errorBuffer);

        STAFString sourceLowerCase = source.toLowerCase();
        unsigned int triggerCount  = 0;
        DiagDataVector diagDataVector;
        STAFDiagManager::DiagMap::iterator mIter;
       
        for (mIter = diagMap.begin(); mIter != diagMap.end(); mIter++)
        {
            if (sourceLowerCase == mIter->second.source.toLowerCase())
            {
                diagDataVector.push_back(mIter->second);
                triggerCount++;
            }
        }

        // Sort the diagDataVector (already sorted by trigger)

        switch (sortBy)
        {
            case kSTAFDiagSortByCount:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            case kSTAFDiagSortBySource:
            {
                // Source by count since only only source

                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }
             
            default: break;
        }

        // Create a marshalled map containing diagnostics info for
        // the specified source

        mc->setMapClassDefinition(fSourceInfoClass->reference());
        mc->setMapClassDefinition(fTriggerCountClass->reference());

        STAFObjectPtr sourceInfoMap = fSourceInfoClass->createInstance();

        if (startTime != sNotApplicable)
            sourceInfoMap->put("fromTimestamp", startTime);

        sourceInfoMap->put("toTimestamp", stopTime);

        if (elapsedTime != sNotApplicable)
            sourceInfoMap->put("elapsedTime", elapsedTime);

        sourceInfoMap->put("source", source);
        sourceInfoMap->put("numberOfTriggers", STAFString(triggerCount));
       
        // Iterate thru the sorted diagDataVector and generate the result

        DiagDataVector::iterator vIter;
        STAFObjectPtr outputList = STAFObject::createList();
        
        for (vIter = diagDataVector.begin(); vIter != diagDataVector.end();
             vIter++)
        {
            STAFObjectPtr triggerCountMap =
                fTriggerCountClass->createInstance();
            triggerCountMap->put("count", STAFString(vIter->count));
            triggerCountMap->put("trigger", vIter->trigger);

            outputList->append(triggerCountMap);
        }

        sourceInfoMap->put("triggerList", outputList);

        mc->setRootObject(sourceInfoMap);
    }
    else if (parsedResult->optionTimes("TRIGGERS") != 0)
    {
        // List all the triggers

        unsigned int triggerCount = 0;
        STAFDiagManager::DiagMap triggerMap;
        STAFDiagManager::DiagMap::iterator mIter;
       
        // Create a map of unique triggers, accumulating counts by trigger

        for (mIter = diagMap.begin(); mIter != diagMap.end(); mIter++)
        {
            if (triggerMap.find(mIter->second.trigger.toLowerCase()) ==
                triggerMap.end())
            {
                triggerMap[mIter->second.trigger.toLowerCase()] =
                    mIter->second;
            }
            else
            {
                triggerMap[mIter->second.trigger.toLowerCase()].count +=
                    mIter->second.count;
            }
        }
        
        // Iterate thru the TriggerMap and create a vector of triggers

        DiagDataVector diagDataVector;
       
        for (mIter = triggerMap.begin(); mIter != triggerMap.end(); mIter++)
        {
            diagDataVector.push_back(mIter->second);
        }

        // Sort the diagDataVector (already sorted by trigger)

        switch (sortBy)
        {
            case kSTAFDiagSortByCount:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            case kSTAFDiagSortBySource:
            {
                // Sort by Count since source will not be in the result

                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }
             
            default: break;
        }

        // Create a marshalled map containing diagnostics info for
        // all the triggers
        
        mc->setMapClassDefinition(fTriggersInfoClass->reference());
        mc->setMapClassDefinition(fTriggerCountClass->reference());

        STAFObjectPtr triggersInfoMap = fTriggersInfoClass->createInstance();

        if (startTime != sNotApplicable)
            triggersInfoMap->put("fromTimestamp", startTime);

        triggersInfoMap->put("toTimestamp", stopTime);

        if (elapsedTime != sNotApplicable)
            triggersInfoMap->put("elapsedTime", elapsedTime);
        
        // Iterate thru the sorted diagDataVector and generate the result

        DiagDataVector::iterator vIter;
        STAFObjectPtr outputList = STAFObject::createList();
        
        for (vIter = diagDataVector.begin(); vIter != diagDataVector.end();
             vIter++)
        {
            STAFObjectPtr triggerCountMap =
                fTriggerCountClass->createInstance();
            triggerCountMap->put("count", STAFString(vIter->count));
            triggerCountMap->put("trigger", vIter->trigger);

            // Add remaining map entries here
            outputList->append(triggerCountMap);

            triggerCount++;
        }

        triggersInfoMap->put("triggerList", outputList);
        triggersInfoMap->put("numberOfTriggers", STAFString(triggerCount));

        // Set the root object for the marshalled context and assign the marshalled output     
        mc->setRootObject(triggersInfoMap);
    }
    else if (parsedResult->optionTimes("SOURCES") != 0)
    {
        // List all the sources

        unsigned int sourceCount = 0;
        STAFDiagManager::DiagMap sourceMap;
        STAFDiagManager::DiagMap::iterator mIter;
       
        // Create a map of unique sources, accumulating counts by source

        for (mIter = diagMap.begin(); mIter != diagMap.end(); mIter++)
        {
            if (sourceMap.find(mIter->second.source.toLowerCase()) ==
                sourceMap.end())
            {
                sourceMap[mIter->second.source.toLowerCase()] =
                    mIter->second;
            }
            else
            {
                sourceMap[mIter->second.source.toLowerCase()].count +=
                    mIter->second.count;
            }
        }
        
        // Iterate thru the SourceMap and create a vector of sources

        DiagDataVector diagDataVector;
       
        for (mIter = sourceMap.begin(); mIter != sourceMap.end(); mIter++)
        {
            diagDataVector.push_back(mIter->second);
        }

        // Sort the diagDataVector (already sorted by trigger)

        switch (sortBy)
        {
            case kSTAFDiagSortByCount:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            case kSTAFDiagSortBySource:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          AscendingSourceSort());
                break;
            }

            case kSTAFDiagSortByTrigger:
            {
                // Sort by Count since trigger will not be in the result

                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            default: break;
        }

        // Create a marshalled map containing diagnostics info for
        // all the sources
        
        mc->setMapClassDefinition(fSourcesInfoClass->reference());
        mc->setMapClassDefinition(fSourceCountClass->reference());

        STAFObjectPtr sourcesInfoMap = fSourcesInfoClass->createInstance();

        if (startTime != sNotApplicable)
            sourcesInfoMap->put("fromTimestamp", startTime);

        sourcesInfoMap->put("toTimestamp", stopTime);

        if (elapsedTime != sNotApplicable)
            sourcesInfoMap->put("elapsedTime", elapsedTime);

        // Iterate thru the sorted diagDataVector and generate the result

        DiagDataVector::iterator vIter;
        STAFObjectPtr outputList = STAFObject::createList();
        
        for (vIter = diagDataVector.begin(); vIter != diagDataVector.end();
             vIter++)
        {
            STAFObjectPtr sourceCountMap =
                fSourceCountClass->createInstance();
            sourceCountMap->put("count", STAFString(vIter->count));
            sourceCountMap->put("source", vIter->source);

            outputList->append(sourceCountMap);
            sourceCount++;
        }

        sourcesInfoMap->put("sourceList", outputList);
        sourcesInfoMap->put("numberOfSources", STAFString(sourceCount));

        mc->setRootObject(sourcesInfoMap);
    }
    else
    {
        // List all diagnostic data

        STAFStringList triggerList;
        STAFStringList sourceList;
        DiagDataVector diagDataVector;
        STAFDiagManager::DiagMap::iterator mIter;
       
        for (mIter = diagMap.begin(); mIter != diagMap.end(); mIter++)
        {
            diagDataVector.push_back(mIter->second);

            // Generate a list of all unique triggers

            if (triggerList.find(mIter->second.trigger.toLowerCase()) ==
                triggerList.end())
            {
                triggerList.insert(mIter->second.trigger.toLowerCase());
            }

            // Generate a list of all unique sources

            if (sourceList.find(mIter->second.source.toLowerCase()) ==
                sourceList.end())
            {
                sourceList.insert(mIter->second.source.toLowerCase());
            }
        }

        // Sort the diagDataVector (already sorted by trigger)

        switch (sortBy)
        {
            case kSTAFDiagSortByCount:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          descendingCountSort);
                break;
            }

            case kSTAFDiagSortBySource:
            {
                std::sort(diagDataVector.begin(), diagDataVector.end(),
                          AscendingSourceSort());
                break;
            }
             
            default: break;
        }

        // Create a marshalled map containing all diagnostics info

        mc->setMapClassDefinition(fAllDiagInfoClass->reference());
        mc->setMapClassDefinition(fComboCountClass->reference());

        STAFObjectPtr allInfoMap = fAllDiagInfoClass->createInstance();

        if (startTime != sNotApplicable)
            allInfoMap->put("fromTimestamp", startTime);

        allInfoMap->put("toTimestamp", stopTime);

        if (elapsedTime != sNotApplicable)
            allInfoMap->put("elapsedTime", elapsedTime);

        allInfoMap->put("numberOfTriggers", STAFString(triggerList.size()));
        allInfoMap->put("numberOfSources", STAFString(sourceList.size()));
        
        // Iterate thru the sorted diagDataVector and generate the result

        DiagDataVector::iterator vIter;
        STAFObjectPtr outputList = STAFObject::createList();
        
        for (vIter = diagDataVector.begin(); vIter != diagDataVector.end();
             vIter++)
        {
            STAFObjectPtr comboCountMap = fComboCountClass->createInstance();
            comboCountMap->put("count", STAFString(vIter->count));
            comboCountMap->put("trigger", vIter->trigger);
            comboCountMap->put("source", vIter->source);

            outputList->append(comboCountMap);
        }

        allInfoMap->put("comboList", outputList);

        mc->setRootObject(allInfoMap);
    }
    
    return STAFServiceResult(kSTAFOk, mc->marshall());
}


STAFServiceResult STAFDiagService::handleReset(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "RESET");
 
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fResetParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    return STAFServiceResult(gDiagManagerPtr->reset());
}


STAFServiceResult STAFDiagService::handleEnable(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "ENABLE");

    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fEnableParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    return STAFServiceResult(gDiagManagerPtr->enable());
}


STAFServiceResult STAFDiagService::handleDisable(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 4

    IVALIDATE_TRUST(4, "DISABLE");
    
    // Parse the request

    STAFCommandParseResultPtr parsedResult = 
        fDisableParser.parse(requestInfo.fRequest);

    if (parsedResult->rc != kSTAFOk)
    {
        return STAFServiceResult(kSTAFInvalidRequestString,
                                 parsedResult->errorBuffer, 0);
    }

    return STAFServiceResult(gDiagManagerPtr->disable());
}


STAFServiceResult STAFDiagService::handleHelp(
    const STAFServiceRequest &requestInfo)
{
    // Verify that the requesting machine/user has at least trust level 1

    IVALIDATE_TRUST(1, "HELP");

    return STAFServiceResult(kSTAFOk, sHelpMsg);
}
