/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001, 2004                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFString.h"
#include "STAF_iostream.h"
#include "STAFUtil.h"
#include <stdlib.h>
#include <list>
#include <vector>

// Debugging flag

bool gDebug = false;

// Print Mode

const unsigned int kPMAuto = 0;
const unsigned int kPMVerbose = 1;
const unsigned int kPMRaw = 2;

bool gPrintModeLocked = false;
unsigned int gPrintMode = kPMAuto;
unsigned int gUnmarshallingFlags = kSTAFUnmarshallingDefaults;
bool gPrintQuotes = false;

// Indentation

unsigned int gIndentDelta = 2;
STAFString gSpaces("                                        "    // 40 Spaces
                   "                                        ");  // times 2
STAFString gListIndent("[");
STAFString gListOutdent("]");
STAFString gMapIndent("{");
STAFString gMapOutdent("}");

// Separators

STAFString gEntrySeparator("");
STAFString gMapKeySeparator(": ");

// Table data

STAFString gHyphens("----------------------------------------"    // 40 Hyphens
                    "----------------------------------------");  // times 2
STAFString gColumnSeparator(" ");
STAFString gCRLF = STAFString(kUTF8_CR) + STAFString(kUTF8_LF);
STAFString gCR = STAFString(kUTF8_CR);
STAFString gLF = STAFString(kUTF8_LF);
STAFString gTab = STAFString(kUTF8_TAB);
STAFString gSpace(" ");
bool gPrintTables = true;
bool gStrictTables = true;
unsigned int gMaxTableWidth = 79;
unsigned int gMaxLinesPerTableRecord = 20;

struct TableColumn
{
    STAFString key;
    STAFString header;
    STAFString longHeader;
    STAFString shortHeader;
    unsigned int longHeaderLength;
    unsigned int shortHeaderLength;
    unsigned int maxValueLength;
    unsigned int minimumWidth;
    unsigned int maximumWidth;
    unsigned int width;

    unsigned int currTestWidth;
    unsigned int maxTestWidth;
    unsigned int bestTestWidth;

    // sizeCounts[x] = # records with size 'x'
    std::vector<unsigned int> sizeCounts;

    // cwLineCounts[x][y] = # records that fit on 'y' lines with 'x' width
    std::vector<std::vector<unsigned int> > cwLineCounts;

    // cwLineCountSums[x][y] = # records that fit on <= 'y' lines with 'x' width
    std::vector<std::vector<unsigned int> > cwLineCountSums;

    // cwLineSums[x] = # lines used to display all column data using 'x' width
    std::vector<unsigned int> cwLineSums;
};

typedef std::vector<TableColumn> TableColumnList;

inline unsigned int cwLineCountsIndex(unsigned int columnWidth,
                                      unsigned int lineNumber)
{
    return (columnWidth * (gMaxLinesPerTableRecord + 1)) + lineNumber;
}

inline unsigned int cwLinesNeeded(TableColumnList::iterator &clIter,
                                  unsigned int widthToCheck,
                                  unsigned int numItems)
{
    for (unsigned int line = 0; line < gMaxLinesPerTableRecord; ++line)
    {
        if (clIter->cwLineCountSums[widthToCheck - 1][line] == numItems)
            return line + 1;
    }

    return gMaxLinesPerTableRecord + 1;
}

bool incColumnTestWidths(TableColumnList &columnList,
                         unsigned int remainingTableWidth)
{
    bool noMoreArrangements = true;
    int index = 0;

    // First, start from the next to last index and go back towards the
    // beginning of the list looking for loop indices that have maxed out.
    //
    // 'index' will be set to the index of the innermost column which didn't
    // max out.

    for (index = columnList.size() - 2; index >= 0; --index)
    {
        TableColumn &currCol = columnList[index];

        // Set the column to the next increment that decreases line counts
        //
        // XXX: This algorithm could use a little extra help.  For example,
        //      it might want to take header lengths into accounts, so that
        //      you might get a full header length, even though the data
        //      won't show up any better.  This probably needs to be hooked
        //      into the same data/algorithm that "eval" uses.

        unsigned int currMax = STAF_MIN(currCol.maxTestWidth,
                                        currCol.maximumWidth);
        unsigned int currLineSums =
                     currCol.cwLineSums[currCol.currTestWidth - 1];

        while ((currCol.currTestWidth <= currMax) &&
               (currCol.cwLineSums[currCol.currTestWidth - 1] == currLineSums))
        {
            ++currCol.currTestWidth;
        }

        // If this column has maxed out, reset its curr counter to the minimum.
        // It's max counter is set farther down.
        //
        // If it hasn't maxed out, then terminate the loop.

        if (currCol.currTestWidth > currMax)
        {
            currCol.currTestWidth = currCol.minimumWidth;
        }
        else
        {
            noMoreArrangements = false;
            break;
        }
    }

    if (noMoreArrangements) return false;

    // Now, calculate how much width is taken up by the columns that didn't
    // max out

    unsigned int currWidth = 0;
    int i = 0;

    for (i = 0; i <= index; ++i)
        currWidth += columnList[i].currTestWidth - columnList[i].minimumWidth;

    // Next, set the maxed out columns new max values

    for (i = index + 1; i < (columnList.size() - 1); ++i)
    {
        columnList[i].maxTestWidth = columnList[i].minimumWidth +
                                     remainingTableWidth - currWidth;
    }

    // Finally update the last column
    //
    // Note: The last column is not "free".  I.e., it's value is completely
    //       dependent on the values of the other columns.
    //
    // XXX: Might want to look at being able to set which column is the
    //      dependent column, as currently, the last column can sometimes get
    //      "odd" values, including settings greater than its maximum necessary
    //      width.

    columnList[i].currTestWidth = columnList[i].minimumWidth + 
                                  remainingTableWidth - currWidth;

    return true;
}

unsigned int evalColumnWidths(TableColumnList &columnList)
{
    if (gDebug) cout << "Evaluating (curr/min/tmax/max): ";

    unsigned int bestEval = 0;
    unsigned int currEval = 0;

    for (unsigned int i = 0; i < columnList.size(); ++i)
    {
        if (gDebug)
        {
            cout << columnList[i].currTestWidth << "/"
                 << columnList[i].minimumWidth << "/"
                 << columnList[i].maxTestWidth << "/"
                 << columnList[i].maximumWidth << " ";
        }

        bestEval += columnList[i].cwLineSums[columnList[i].bestTestWidth - 1];
        currEval += columnList[i].cwLineSums[columnList[i].currTestWidth - 1];
    }

    if (gDebug) cout << endl;

    if (bestEval > currEval)
    {
        for (unsigned int j = 0; j < columnList.size(); ++j)
            columnList[j].bestTestWidth = columnList[j].currTestWidth;
    }

    return 1;
}

void dumpTableColumnList(TableColumnList &tableColumnList)
{
    for (TableColumnList::iterator iter = tableColumnList.begin();
         iter != tableColumnList.end();
         ++iter)
    {
        cout << "Key                : " << iter->key << endl;
        cout << "Header             : " << iter->header << endl;
        cout << "Long header        : " << iter->longHeader << endl;
        cout << "Short header       : " << iter->shortHeader << endl;
        cout << "Long Header Length : " << iter->longHeaderLength << endl;
        cout << "Short Header Length: " << iter->shortHeaderLength << endl;
        cout << "Max Value Length   : " << iter->maxValueLength << endl;
        cout << "Minimum width      : " << iter->minimumWidth << endl;
        cout << "Maximum width      : " << iter->maximumWidth << endl;
        cout << "Width              : " << iter->width << endl;

        unsigned int maxSizeCount = gMaxTableWidth * gMaxLinesPerTableRecord;

        for (unsigned int i = 0; i <= maxSizeCount; ++i)
        {
            if (iter->sizeCounts[i])
                cout << "Size " << i << ": " << iter->sizeCounts[i] << endl;
        }

        for (unsigned int j = 0; j < gMaxTableWidth; ++j)
        {
            cout << "Lines need for column width " << (j + 1) << ": "
                 << iter->cwLineSums[j] << endl;
        }

        for (unsigned int x = 0; x < gMaxTableWidth + 1; ++x)
        {
            cout << "Column width: " << (x + 1) << endl
                 << "Number of lines/records/total-records: " << endl;

            for (unsigned int y = 0; y < gMaxLinesPerTableRecord + 1; ++y)
            {
                cout << (y + 1) << "/" << iter->cwLineCounts[x][y]
                     << "/" << iter->cwLineCountSums[x][y] << " ";
            }

            cout << endl;
        }
    }

    cout << endl;
}


inline void pauseIt(STAFString message = "Enter a number: ")
{
    cout << message << endl;
    unsigned int a = 0;
    cin >> a;
}

// General constants

STAFString sMapClassKey("staf-map-class-name");
STAFString sKey("key");
STAFString sDisplayName("display-name");
STAFString sShortDisplayName("display-short-name");
STAFString sColon(kUTF8_COLON);
STAFString sSingleQuote(kUTF8_SQUOTE);
STAFString sDoubleQuote(kUTF8_DQUOTE);
STAFString sEscapedSingleQuote("\\'");

STAFString massageArgument(char *theArg)
{
    STAFString arg(theArg);

    if (getenv("STAF_OLDCLI") != 0) return arg;

    if (arg.find(kUTF8_SPACE) != STAFString::kNPos)
    {
        if ((arg.find(sDoubleQuote) != 0) ||
            (arg.findLastOf(sDoubleQuote, STAFString::kNPos,
                            STAFString::kChar) !=
             arg.length(STAFString::kChar)))
        {
            arg = sColon + STAFString(arg.length(STAFString::kChar)) +
                  sColon + arg;
        }
    }

    return arg;
}


STAFString quoteString(const STAFString &input)
{
    if (!gPrintQuotes) return input;

    if (input.find(sSingleQuote) == STAFString::kNPos)
        return STAFString(sSingleQuote) + input + STAFString(sSingleQuote);

    if (input.find(sDoubleQuote) == STAFString::kNPos)
        return STAFString(sDoubleQuote) + input + STAFString(sDoubleQuote);

    return STAFString(sSingleQuote) +
           input.replace(sSingleQuote, sEscapedSingleQuote) +
           STAFString(sSingleQuote);
}


void printVerbose(const STAFObjectPtr &objPtr, const STAFObjectPtr &context,
                  unsigned int indentLevel)
{
    switch (objPtr->type())
    {
        case kSTAFListObject:
        {
            cout << gListIndent;

            ++indentLevel;

            if (objPtr->size() > 0) cout << endl;

            // Print out each object

            for (STAFObjectIteratorPtr iterPtr = objPtr->iterate();
                 iterPtr->hasNext();)
            {
                STAFObjectPtr thisObj = iterPtr->next();

                if ((thisObj->type() == kSTAFListObject) ||
                    (thisObj->type() == kSTAFMapObject) ||
                    (thisObj->type() == kSTAFMarshallingContextObject))
                {
                    cout << gSpaces.subString(0, indentLevel * gIndentDelta);

                    printVerbose(thisObj, context, indentLevel);
                }
                else
                {
                    cout << gSpaces.subString(0, indentLevel * gIndentDelta);

                    if (thisObj->type() == kSTAFNoneObject)
                        cout << thisObj->asString();
                    else
                        cout << thisObj->asString();
                }

                if (iterPtr->hasNext()) cout << gEntrySeparator;

                cout << endl;
            }

            --indentLevel;

            if (objPtr->size() > 0)
                cout << gSpaces.subString(0, indentLevel * gIndentDelta);

            cout << gListOutdent;

            break;
        }
        case kSTAFMapObject:
        {
            cout << gMapIndent;

            ++indentLevel;

            if (objPtr->size() > 0) cout << endl;

            if (objPtr->hasKey(sMapClassKey) &&
                context && (context->type() == kSTAFMarshallingContextObject) &&
                context->hasMapClassDefinition(objPtr->get(sMapClassKey)->asString()))
            {
                // Map object has a map class definition in the context

                STAFMapClassDefinitionPtr mapClass =
                    context->getMapClassDefinition(
                        objPtr->get(sMapClassKey)->asString());

                // Determine maximum key length

                STAFObjectIteratorPtr iterPtr;
                unsigned int maxKeyLength = 0;

                for (iterPtr = mapClass->keyIterator(); iterPtr->hasNext();)
                {
                    STAFObjectPtr theKey = iterPtr->next();
                    STAFString theKeyString;

                    if (theKey->hasKey(sDisplayName))
                        theKeyString = theKey->get(sDisplayName)->asString();
                    else
                        theKeyString = theKey->get(sKey)->asString();

                    if (theKeyString.length(STAFString::kChar) > maxKeyLength)
                        maxKeyLength = theKeyString.length(STAFString::kChar);
                }

                // Now print each object in the map

                for (iterPtr = mapClass->keyIterator(); iterPtr->hasNext();)
                {
                    STAFObjectPtr theKey = iterPtr->next();
                    STAFString theKeyString;

                    if (theKey->hasKey(sDisplayName))
                        theKeyString = theKey->get(sDisplayName)->asString();
                    else
                        theKeyString = theKey->get(sKey)->asString();

                    cout << gSpaces.subString(0, indentLevel * gIndentDelta)
                         << theKeyString
                         << gSpaces.subString(0, maxKeyLength -
                                theKeyString.length(STAFString::kChar))
                         << gMapKeySeparator;

                    STAFObjectPtr thisObj =
                        objPtr->get(theKey->get(sKey)->asString());

                    if ((thisObj->type() == kSTAFListObject) ||
                        (thisObj->type() == kSTAFMapObject) ||
                        (thisObj->type() == kSTAFMarshallingContextObject))
                    {
                        printVerbose(thisObj, context, indentLevel);
                    }
                    else if (thisObj->type() == kSTAFNoneObject)
                    {
                        cout << thisObj->asString();
                    }
                    else
                    {
                        cout << thisObj->asString();
                    }

                    if (iterPtr->hasNext()) cout << gEntrySeparator;

                    cout << endl;
                }
            }
            else
            {
                // Determine maximum key length

                STAFObjectIteratorPtr iterPtr;
                unsigned int maxKeyLength = 0;

                for (iterPtr = objPtr->keyIterator(); iterPtr->hasNext();)
                {
                    STAFString theKeyString = iterPtr->next()->asString();

                    if (theKeyString.length(STAFString::kChar) > maxKeyLength)
                        maxKeyLength = theKeyString.length(STAFString::kChar);
                }

                // Now print each object in the map

                for (iterPtr = objPtr->keyIterator(); iterPtr->hasNext();)
                {
                    STAFString theKeyString = iterPtr->next()->asString();

                    cout << gSpaces.subString(0, indentLevel * gIndentDelta)
                         << theKeyString
                         << gSpaces.subString(0, maxKeyLength -
                                theKeyString.length(STAFString::kChar))
                         << gMapKeySeparator;

                    STAFObjectPtr thisObj = objPtr->get(theKeyString);

                    if ((thisObj->type() == kSTAFListObject) ||
                        (thisObj->type() == kSTAFMapObject) ||
                        (thisObj->type() == kSTAFMarshallingContextObject))
                    {
                        printVerbose(thisObj, context, indentLevel);
                    }
                    else if (thisObj->type() == kSTAFNoneObject)
                    {
                        cout << thisObj->asString();
                    }
                    else
                    {
                        cout << thisObj->asString();
                    }

                    if (iterPtr->hasNext()) cout << gEntrySeparator;

                    cout << endl;
                }
            }

            --indentLevel;

            if (objPtr->size() > 0)
                cout << gSpaces.subString(0, indentLevel * gIndentDelta);

            cout << gMapOutdent;

            break;
        }
        case kSTAFMarshallingContextObject:
        {
            printVerbose(objPtr->getRootObject(), objPtr, indentLevel);
            break;
        }
        default:
        {
            cout << gSpaces.subString(0, indentLevel * gIndentDelta)
                 << objPtr->asString();
            break;
        }
    }
}


bool printSimple(const STAFObjectPtr &objPtr, const STAFObjectPtr &context)
{
    if (objPtr->type() == kSTAFMarshallingContextObject) return false;

    if ((objPtr->type() == kSTAFListObject) ||
        (objPtr->type() == kSTAFMapObject))
    {
        STAFObjectIteratorPtr iter =
            (objPtr->type() == kSTAFListObject) ?
                objPtr->iterate() :
                objPtr->valueIterator();

        while (iter->hasNext())
        {
            STAFObjectPtr thisObj = iter->next();

            switch (thisObj->type())
            {
                case kSTAFListObject:
                case kSTAFMapObject:
                case kSTAFMarshallingContextObject:
                    return false;
                default:
                    break;
            }
        }
    }

    if (objPtr->type() == kSTAFListObject)
    {
        for (STAFObjectIteratorPtr iterPtr = objPtr->iterate();
             iterPtr->hasNext();)
        {
            STAFObjectPtr thisObj = iterPtr->next();

            cout << thisObj->asString() << endl;
        }
    }
    else if (objPtr->type() == kSTAFMapObject)
    {
        if (objPtr->hasKey(sMapClassKey) &&
            context && (context->type() == kSTAFMarshallingContextObject) &&
            context->hasMapClassDefinition(objPtr->get(sMapClassKey)->asString()))
        {
            // Map object has a map class definition in the context

            STAFMapClassDefinitionPtr mapClass =
                context->getMapClassDefinition(
                    objPtr->get(sMapClassKey)->asString());

            // Determine maximum key length

            STAFObjectIteratorPtr iterPtr;
            unsigned int maxKeyLength = 0;

            for (iterPtr = mapClass->keyIterator(); iterPtr->hasNext();)
            {
                STAFObjectPtr theKey = iterPtr->next();
                STAFString theKeyString;

                if (theKey->hasKey(sDisplayName))
                    theKeyString = theKey->get(sDisplayName)->asString();
                else
                    theKeyString = theKey->get(sKey)->asString();

                if (theKeyString.length(STAFString::kChar) > maxKeyLength)
                    maxKeyLength = theKeyString.length(STAFString::kChar);
            }

            // Now print each object in the map

            for (iterPtr = mapClass->keyIterator(); iterPtr->hasNext();)
            {
                STAFObjectPtr theKey = iterPtr->next();
                STAFString theKeyString;

                if (theKey->hasKey(sDisplayName))
                    theKeyString = theKey->get(sDisplayName)->asString();
                else
                    theKeyString = theKey->get(sKey)->asString();

                cout << theKeyString
                     << gSpaces.subString(0, maxKeyLength -
                                          theKeyString.length(STAFString::kChar))
                     << gMapKeySeparator
                     << objPtr->get(theKey->get(sKey)->asString())->asString()
                     << endl;
            }
        }
        else
        {
            // Determine maximum key length

            STAFObjectIteratorPtr iterPtr;
            unsigned int maxKeyLength = 0;

            for (iterPtr = objPtr->keyIterator(); iterPtr->hasNext();)
            {
                STAFString theKeyString = iterPtr->next()->asString();

                if (theKeyString.length(STAFString::kChar) > maxKeyLength)
                    maxKeyLength = theKeyString.length(STAFString::kChar);
            }

            // Now print each object in the map

            for (iterPtr = objPtr->keyIterator(); iterPtr->hasNext();)
            {
                STAFString theKeyString = iterPtr->next()->asString();

                cout << theKeyString
                     << gSpaces.subString(0, maxKeyLength -
                                          theKeyString.length(STAFString::kChar))
                     << gMapKeySeparator
                     << objPtr->get(theKeyString)->asString()
                     << endl;
            }
        }
    }
    else
    {
        cout << objPtr->asString();
    }

    return true;
}

bool printTable(const STAFObjectPtr &objPtr, const STAFObjectPtr &context)
{
    if (!gPrintTables) return false;

    if (objPtr->type() != kSTAFListObject) return false;

    STAFObjectIteratorPtr iter = objPtr->iterate();

    if (!iter->hasNext()) return false;

    STAFObjectPtr firstObj = iter->next();

    if (firstObj->type() != kSTAFMapObject) return false;

    if (!firstObj->hasKey(sMapClassKey)) return false;

    STAFString mapClassName = firstObj->get(sMapClassKey)->asString();

    if (!context) return false;
    if (context->type() != kSTAFMarshallingContextObject) return false;
    if (!context->hasMapClassDefinition(mapClassName)) return false;

    STAFMapClassDefinitionPtr mapClassObj =
        context->getMapClassDefinition(mapClassName);

    // Initialize table info from map class

    unsigned int maxSizeCount = gMaxTableWidth * gMaxLinesPerTableRecord;
    TableColumnList columnList;

    for (STAFObjectIteratorPtr mcIter = mapClassObj->keyIterator();
         mcIter->hasNext();)
    {
        TableColumn thisColumn;

        thisColumn.sizeCounts = std::vector<unsigned int>(maxSizeCount + 1, 0);
        thisColumn.cwLineCounts = std::vector<std::vector<unsigned int> >(gMaxTableWidth + 1, std::vector<unsigned int>(gMaxLinesPerTableRecord + 1, 0));
        thisColumn.cwLineCountSums = std::vector<std::vector<unsigned int> >(gMaxTableWidth + 1, std::vector<unsigned int>(gMaxLinesPerTableRecord + 1, 0));
        thisColumn.cwLineSums = std::vector<unsigned int>(gMaxTableWidth, 0);

        STAFObjectPtr thisKey = mcIter->next();

        thisColumn.key = thisKey->get(sKey)->asString();

        if (thisKey->hasKey(sDisplayName))
            thisColumn.longHeader = thisKey->get(sDisplayName)->asString();
        else
            thisColumn.longHeader = thisKey->get(sKey)->asString();

        if (thisKey->hasKey(sShortDisplayName))
            thisColumn.shortHeader = thisKey->get(sShortDisplayName)->asString();
        else
            thisColumn.shortHeader = thisColumn.longHeader;

        thisColumn.header = thisColumn.longHeader;

        thisColumn.longHeaderLength =
            thisColumn.longHeader.length(STAFString::kChar);
        thisColumn.shortHeaderLength =
            thisColumn.shortHeader.length(STAFString::kChar);

        thisColumn.width = 0;
        thisColumn.maxValueLength = 0;

        columnList.push_back(thisColumn);
    }

    // Process first object

    TableColumnList::iterator clIter;
    
    for (clIter = columnList.begin(); clIter != columnList.end(); ++clIter)
    {
        STAFObjectPtr thisValueObj = firstObj->get(clIter->key);

        if ((thisValueObj->type() != kSTAFScalarStringObject) &&
            (thisValueObj->type() != kSTAFNoneObject))
        {
            if (gStrictTables) return false;
        }

        unsigned int firstItemColumnWidth =
            thisValueObj->asString().length(STAFString::kChar);

        ++clIter->sizeCounts[STAF_MIN(firstItemColumnWidth, maxSizeCount)];

        for (unsigned int columnIndex = 0, columnWidth = 1;
             columnIndex < gMaxTableWidth;
             ++columnIndex, ++columnWidth)
        {
            // At least one line is needed even if the length of an entry
            // in the table is 0 (e.g. due to an empty string, etc.)
            unsigned int linesNeeded = STAF_MAX(
                1, firstItemColumnWidth / columnWidth);

            if (firstItemColumnWidth % columnWidth != 0)
                ++linesNeeded;

            linesNeeded = STAF_MIN(linesNeeded, gMaxLinesPerTableRecord);

            ++clIter->cwLineCounts[columnIndex][linesNeeded - 1];
            clIter->cwLineSums[columnIndex] += linesNeeded;

            for (unsigned int lineIndex = linesNeeded - 1;
                 lineIndex < gMaxLinesPerTableRecord + 1;
                 ++lineIndex)
            {
                ++clIter->cwLineCountSums[columnIndex][lineIndex];
            }
        }

        clIter->maxValueLength = STAF_MAX(clIter->maxValueLength,
                                          firstItemColumnWidth);
    }
    
    // Loop through and process the rest of the objects
    
    while (iter->hasNext())
    {
        STAFObjectPtr thisObj = iter->next();

        // Make sure we are still in a valid table

        if (thisObj->type() != kSTAFMapObject) return false;

        if (thisObj->get(sMapClassKey)->asString() != mapClassName)
            return false;

        // Update table info with this object

        for (clIter = columnList.begin(); clIter != columnList.end(); ++clIter)
        {
            STAFObjectPtr thisValueObj = thisObj->get(clIter->key);

            if ((thisValueObj->type() != kSTAFScalarStringObject) &&
                (thisValueObj->type() != kSTAFNoneObject))
            {
                if (gStrictTables) return false;
            }

            unsigned int thisItemColumnWidth =
                thisObj->get(clIter->key)->asString().length(STAFString::kChar);

            ++clIter->sizeCounts[STAF_MIN(maxSizeCount, thisItemColumnWidth)];

            for (unsigned int columnIndex = 0, columnWidth = 1;
                 columnIndex < gMaxTableWidth;
                 ++columnIndex, ++columnWidth)
            {
                // At least one line is needed even if the length of an entry
                // in the table is 0 (e.g. due to an empty string, etc.)
                unsigned int linesNeeded = STAF_MAX(
                    1, thisItemColumnWidth / columnWidth);
                
                if (thisItemColumnWidth % columnWidth != 0)
                    ++linesNeeded;

                linesNeeded = STAF_MIN(linesNeeded, gMaxLinesPerTableRecord);
                
                ++clIter->cwLineCounts[columnIndex][linesNeeded - 1];
                clIter->cwLineSums[columnIndex] += linesNeeded;

                for (unsigned int lineIndex = linesNeeded - 1;
                     lineIndex < gMaxLinesPerTableRecord + 1;
                     ++lineIndex)
                {
                    ++clIter->cwLineCountSums[columnIndex][lineIndex];
                }
            }

            clIter->maxValueLength = STAF_MAX(clIter->maxValueLength,
                                              thisItemColumnWidth);
        }
    }
    
    // Now determine appropriate column widths

    unsigned int minimumTableWidth = (columnList.size() - 1) *
                                     gColumnSeparator.length(STAFString::kChar);
    unsigned int maximumTableWidth = minimumTableWidth;

    for (clIter = columnList.begin(); clIter != columnList.end(); ++clIter)
    {
        minimumTableWidth += clIter->shortHeaderLength;
        clIter->minimumWidth = clIter->shortHeaderLength;

        if (clIter->longHeaderLength > clIter->maxValueLength)
        {
            maximumTableWidth += clIter->longHeaderLength;
            clIter->maximumWidth = clIter->longHeaderLength;
        }
        else
        {
            maximumTableWidth += clIter->maxValueLength;
            clIter->maximumWidth = clIter->maxValueLength;
        }

        clIter->width = clIter->maximumWidth;
    }

    // Check to make sure the table will fit at its minimum size
    
    if (minimumTableWidth > gMaxTableWidth) return false;

    // Adjust column sizings if necessary

    if (maximumTableWidth > gMaxTableWidth)
    {
        unsigned int remainingTableWidth = gMaxTableWidth - minimumTableWidth;

        if (gDebug)
            cout << "remainingWidth: " << remainingTableWidth << endl;

        // Initialize test width data in columns

        unsigned int index = 0;

        for (index = 0; index < columnList.size(); ++index)
        {
            columnList[index].currTestWidth = columnList[index].minimumWidth;
            columnList[index].bestTestWidth = columnList[index].minimumWidth;
            columnList[index].maxTestWidth =
                columnList[index].minimumWidth + remainingTableWidth;
        }

        // Update last column to start testing with the maximum width

        columnList[columnList.size() - 1].currTestWidth =
            columnList[columnList.size() - 1].maxTestWidth;

        // Now evaluate the different permutations

        do
        {
            evalColumnWidths(columnList);
        } while (incColumnTestWidths(columnList, remainingTableWidth));

        // Now, set width to bestTestWidth and update headers if necessary

        for (clIter = columnList.begin(); clIter != columnList.end(); ++clIter)
        {
            clIter->width = clIter->bestTestWidth;

            if (clIter->width < clIter->longHeaderLength)
                clIter->header = clIter->shortHeader;
        }
    }

    if (gDebug) dumpTableColumnList(columnList);

    // Now, print out the table header

    for (clIter = columnList.begin(); clIter != columnList.end();)
    {
        cout << clIter->header;

        unsigned int spacesNeeded =
            clIter->width - clIter->header.length(STAFString::kChar);

        while (spacesNeeded > gSpaces.length())
        {
            cout << gSpaces;
            spacesNeeded -= gSpaces.length();
        }

        cout << gSpaces.subString(0, spacesNeeded);

        if (++clIter != columnList.end())
            cout << gColumnSeparator;
    }

    cout << endl;
    
    for (clIter = columnList.begin(); clIter != columnList.end();)
    {
        unsigned int hyphensNeeded = clIter->width;

        while (hyphensNeeded > gHyphens.length())
        {
            cout << gHyphens;
            hyphensNeeded -= gHyphens.length();
        }

        cout << gHyphens.subString(0, hyphensNeeded);

        if (++clIter != columnList.end())
            cout << gColumnSeparator;
    }
    
    cout << endl;

    // Now, loop through and print out each object
    
    for (iter = objPtr->iterate (); iter->hasNext();)
    {
        STAFObjectPtr thisObj = iter->next();
        bool done = false;

        for (unsigned int lineNum = 0;
             !done && lineNum < gMaxLinesPerTableRecord;
             ++lineNum)
        {
            done = true;

            for (clIter = columnList.begin(); clIter != columnList.end();)
            {
                STAFString thisColumnFullString =
                    thisObj->get(clIter->key)->asString();

                if (thisColumnFullString.length(STAFString::kChar) >
                    (lineNum + 1) * clIter->width)
                {
                    done = false;
                }

                STAFString thisColumnString = thisColumnFullString.subString(
                    clIter->width * lineNum, clIter->width, STAFString::kChar);

                if ((lineNum == gMaxLinesPerTableRecord - 1) && !done)
                {
                    cout << "(More...)";
                }
                else
                {
                    cout << thisColumnString.replace(gCR, gSpace)
                            .replace(gLF, gSpace).replace(gTab, gSpace);
                }

                unsigned int spacesNeeded =
                    clIter->width - thisColumnString.length(STAFString::kChar);

                while (spacesNeeded > gSpaces.length())
                {
                    cout << gSpaces;
                    spacesNeeded -= gSpaces.length();
                }

                cout << gSpaces.subString(0, spacesNeeded);

                if (++clIter != columnList.end())
                    cout << gColumnSeparator;
            }

            cout << endl;
        }
    }
    
    return true;
}

void printResult(const STAFResultPtr &result)
{
    // Update display settings

    if ((getenv("STAF_PRINT_MODE") != 0) && !gPrintModeLocked)
    {
        STAFString pMode = STAFString(getenv("STAF_PRINT_MODE")).lowerCase();

        if      (pMode == "verbose") gPrintMode = kPMVerbose;
        else if (pMode == "auto")    gPrintMode = kPMAuto;
        else if (pMode == "raw")     gPrintMode = kPMRaw;
    }

    if (getenv("STAF_PRINT_NO_TABLES") != 0)
        gPrintTables = false;

    if (getenv("STAF_NO_STRICT_TABLES") != 0)
        gStrictTables = false;

    if (getenv("STAF_IGNORE_INDIRECT_OBJECTS") != 0)
        gUnmarshallingFlags = kSTAFIgnoreIndirectObjects;

    if (getenv("STAF_INDENT_DELTA") != 0)
    {
        STAFString indentDeltaString = getenv("STAF_INDENT_DELTA");

        try { gIndentDelta = indentDeltaString.asUInt(); }
        catch (...) { /* Do nothing */ }
    }

    if (getenv("STAF_TABLE_WIDTH") != 0)
    {
        STAFString tableWidth = getenv("STAF_TABLE_WIDTH");

        try { gMaxTableWidth = tableWidth.asUInt(); }
        catch (...) { /* Do nothing */ }
    }

    if (getenv("STAF_TABLE_LINES_PER_RECORD") != 0)
    {
        STAFString linesPerRecord = getenv("STAF_TABLE_LINES_PER_RECORD");

        try { gMaxLinesPerTableRecord = linesPerRecord.asUInt(); }
        catch (...) { /* Do nothing */ }
    }

    // Now output the data

    // If requested, just dump out the literal string returned

    if (gPrintMode == kPMRaw)
    {
        cout << result->result;
        return;
    }

    // Otherwise, print the root object of the marshalling context

    // Note that the result has already been auto-unmarshalled by the
    // STAFResult class using the default unmarshalling flags

    STAFObjectPtr resultContext = result->resultContext;
    STAFObjectPtr resultObj = result->resultObj;
    
    if (gUnmarshallingFlags != kSTAFUnmarshallingDefaults)
    {
        // Need to unmarshall the result again with the specified
        // unmarshalling flags to get the marshalling context

        resultContext = STAFObject::unmarshall(result->result,
                                               gUnmarshallingFlags);
        resultObj = resultContext->getRootObject();
    }

    if (gPrintMode == kPMAuto)
    {
        if (!printSimple(resultObj, resultContext) &&
            !printTable(resultObj, resultContext))
        {
            printVerbose(resultObj, resultContext, 0);
        }
    }
    else
    {
        printVerbose(resultObj, resultContext, 0);
    }
}


int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cout << "Usage: STAF [-verbose] <Endpoint | LOCAL> <Service> "
             << "<Request>" << endl;
        return 1;
    }

    unsigned int argsBase = 1;

    if (STAFString(argv[1]).lowerCase() == "-verbose")
    {
        gPrintMode = kPMVerbose;
        gPrintModeLocked = true;
        argsBase += 1;
    }

    unsigned int verboseOutput = (getenv("STAF_QUIET_MODE") == 0) ? 1 : 0;
    unsigned int staticHandle = 0;

    if (getenv("STAF_STATIC_HANDLE") != 0)
        staticHandle = strtoul(getenv("STAF_STATIC_HANDLE"), 0, 10);
    
    STAFHandlePtr handlePtr;
    unsigned int rc = 0;

    if (staticHandle != 0)
        rc = STAFHandle::create(staticHandle, handlePtr);
    else
        rc = STAFHandle::create("STAF/Client", handlePtr);

    if (rc != 0)
    {
        cout << "Error registering with STAF, RC: " << rc << endl;
        return rc;
    }

    STAFString request;

    for (int i = argsBase + 2; i < (argc - 1); i++)
    {
        request += massageArgument(argv[i]) + STAFString(" ");
    }

    request += massageArgument(argv[argc - 1]);
    STAFResultPtr result = handlePtr->submit(argv[argsBase], argv[argsBase + 1],
                                             request, kSTAFReqSync);

    if (getenv("STAF_REPLACE_NULLS") != 0)
    {
        STAFString replaceNullStr = getenv("STAF_REPLACE_NULLS");
        result->result = result->result.replace(kUTF8_NULL, replaceNullStr);
        result->result = result->result.replace(kUTF8_NULL2, replaceNullStr);
    }

    if (result->rc != 0)
    {
        if (verboseOutput)
        {
            cout << "Error submitting request, RC: " << result->rc << endl;

            if (result->result.length() != 0)
            {
                cout << "Additional info" << endl << "---------------" << endl;
                printResult(result);
                cout << endl;
            }
        }
        else printResult(result);

        return result->rc;
    }

    if (verboseOutput)
    {
        cout << "Response" << endl << "--------" << endl;
        printResult(result);
        cout << endl;
    }
    else printResult(result);

    return 0;
}
