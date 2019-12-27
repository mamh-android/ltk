/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF.h"
#include "STAFVariablePool.h"
#include "STAFUtil.h"

STAFRC_t STAFVariablePool::set(const STAFString &name,
                               const STAFString &value)
{
    STAFString errorBuffer = "";
    return STAFVariablePool::set(name, value, errorBuffer, false);
}

STAFRC_t STAFVariablePool::set(const STAFString &name,
                               const STAFString &value,
                               STAFString &errorBuffer,
                               bool failIfExists)
{
    STAFString lowerCaseName(name.toLowerCase());

    STAFMutexSemLock varMapLock(fVarMapSem);

    if (fVarMap.find(lowerCaseName) == fVarMap.end())
    {
        // Variable does not exist so add the variable to the variable map

        Variable theVar(name, value);
        fVarMap[lowerCaseName] = theVar;
    }
    else
    {
        // Variable already exists

        if (!failIfExists)
        {
            // Update the variable's value in the variable value

            fVarMap[lowerCaseName].value = value;
        }
        else
        {
            // Return an error and set the result to the variable's value

            errorBuffer = fVarMap[lowerCaseName].value;
            return kSTAFAlreadyExists;
        }
    }

    return kSTAFOk;
}


STAFRC_t STAFVariablePool::get(const STAFString &name,
                                    STAFString &value) const
{
    STAFString lowerCaseName(name.toLowerCase());
    STAFVariablePool *self = (STAFVariablePool *)this;
    STAFMutexSemLock varMapLock(self->fVarMapSem);
    VariableMap::const_iterator varMapIter = fVarMap.find(lowerCaseName);

    if (varMapIter == fVarMap.end())
        return kSTAFVariableDoesNotExist;

    value = varMapIter->second.value;

    return kSTAFOk;
}


STAFRC_t STAFVariablePool::del(const STAFString &name)
{
    STAFRC_t rc = kSTAFOk;
    STAFString lowerCaseName(name.toLowerCase());  
    STAFMutexSemLock varMapLock(fVarMapSem);

    if (fVarMap.find(lowerCaseName) == fVarMap.end())
        rc = kSTAFVariableDoesNotExist;
    else
        fVarMap.erase(lowerCaseName);

    return rc;
}


static STAFString sLCurly(kUTF8_LCURLY);
static STAFString sRCurly(kUTF8_RCURLY);
static STAFString sCaret(kUTF8_CARET);
static STAFString sSpecial = sLCurly + sCaret;
static unsigned int maxVars = 100;  // Maximum variable resolution depth


STAFVariablePool::VariableMap STAFVariablePool::getVariableMapCopy()
{
    STAFMutexSemLock varMapLock(fVarMapSem);

    return fVarMap;
}


STAFRC_t STAFVariablePool::resolve(const STAFString &aString,
                                   const STAFVariablePool *poolList[],
                                   unsigned int numPools,
                                   STAFString &result,
                                   bool ignoreErrors)
{
    if (numPools == 0)
    {
        result = STAFString("No variable pools specified");
        return kSTAFInvalidParm;
    }

    if (poolList == 0)
    {
        result = "Invalid variable pool list";
        return kSTAFInvalidParm;
    }

    STAFRC_t rc = kSTAFOk;

    unsigned int index = 0;
    unsigned int closeIndex = 0;
    unsigned int count = 0;
  
    result = aString;

    // Find all possible variables

    for (unsigned int i = 0; (i < result.length()) && (count < maxVars);
         ++i, ++count)
    {
        // Find the first ^ or { character
        index = result.findFirstOf(sSpecial, i);
        
        if (index == STAFString::kNPos)
            break;

        i = index;
        
        if ((result.subString(index, 1) == sCaret) &&
            (index != result.length() - 1))
        {
            //   ^{ means output '{' to the result
            //   ^^ means output just one ^ to the result
            if ((result.sizeOfChar(index + 1) == 1) &&
                ((result.subString((index + 1), 1) == sLCurly) ||
                 (result.subString((index + 1), 1) == sCaret)))
            {
                result = result.subString(0, index) +
                         result.subString(index + 1);
            }
        }
        else if (result.subString(index, 1) == sLCurly)
        {
            // Find position of next closing brace.  
            closeIndex = result.find(sRCurly, index + 1);

            if (closeIndex == STAFString::kNPos)
            {
                if (ignoreErrors)
                {
                    // Assume that this is not a STAF variable
                    break;
                }

                // This is an error due to non-matching braces
                result = STAFString(
                    "Variable resolution failed for string: " + aString +
                    "\n\nYou are trying to resolve a variable reference that "
                    "has a non-matching left or right curly brace.");
                return kSTAFInvalidResolveString;
            }
            
            // Is there is another '{' between this '{' and the closing brace?
            // If so, find the last '{' one between these braces in order to
            // expand each variable reference within it before expanding
            // the entire variable reference.

            unsigned int openIndex = index;   // Save index value
            unsigned int nextOpenIndex = result.find(sLCurly, openIndex + 1);

            for (; ((nextOpenIndex != STAFString::kNPos) &&
                    (nextOpenIndex < closeIndex));)
            {
                openIndex = nextOpenIndex;
                nextOpenIndex = result.find(sLCurly, openIndex + 1);
            }

            // Decrement by 1 to offset increment at end of for loop.
            --i;
            
            // Resolve the variable to which these braces refer in place
            
            STAFString varName = result.subString(openIndex + 1,
                                                  closeIndex - openIndex - 1);
            STAFString varValue;

            rc = kSTAFVariableDoesNotExist;

            for (unsigned int poolIndex = 0;
                 ((poolIndex < numPools) && (rc == kSTAFVariableDoesNotExist));
                 ++poolIndex)
            {
                // XXX: Do we really want to allow null pools?
                //      This was probably just necessary while I hacked this
                //      together.
                if (poolList[poolIndex] != 0)
                    rc = poolList[poolIndex]->get(varName, varValue);
            }
            
            if (rc != kSTAFOk)
            {
                if (ignoreErrors)
                {
                    // Assume that this is not a STAF variable
                    i++; 
                }
                else
                {
                    result = STAFString(
                        "Variable resolution failed for string: " + aString +
                        "\n\nYou are trying to resolve a variable that does "
                        "not exist: " + varName);
                    return rc;
                }
            }
            else
            {
                result = result.subString(0, openIndex) + varValue +
                         result.subString(closeIndex + 1);
            }
        }
    }

    if (count >= maxVars)
    {
        // Error due to non-resolvable string
        //   e.g. a={b}, b={a}, then resolving {a} would return this error.
        result = STAFString(
            "Variable resolution failed for string: " + aString +
            "\n\nYou are trying to resolve a variable that cannot be "
            "resolved probably due to recursive variable definitions.");
        return kSTAFUnResolvableString;
    }

    return kSTAFOk;
}

