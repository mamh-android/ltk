/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_VariablePool
#define STAF_VariablePool

#include <map>
#include "STAF.h"
#include "STAFString.h"
#include "STAFRefPtr.h"
#include "STAFMutexSem.h"
#include "STAFUtil.h"

#define DEFINE_VAR_POOL_LIST(varPoolList, varPoolListSize, serviceRequest)\
const STAFVariablePool *varPoolList[] =\
{\
    serviceRequest.fRequestVarPool,\
    serviceRequest.fSourceSharedVarPool,\
    serviceRequest.fLocalSharedVarPool,\
    serviceRequest.fLocalSystemVarPool\
};\
unsigned int varPoolListSize = \
sizeof(varPoolList) / sizeof(const STAFVariablePool *);


class STAFVariablePool
{
public:

    struct Variable
    {
        Variable(const STAFString &aName = STAFString(),
                 const STAFString &aValue = STAFString())
            : name(aName), lowerCaseName(name.toLowerCase()),
              value(aValue)
        { /* Do Nothing */ }

        STAFString name;
        STAFString lowerCaseName;
        STAFString value;
    };

    typedef std::map<STAFString, Variable> VariableMap;

    STAFVariablePool() { /* Do Nothing */ }
    STAFVariablePool(const VariableMap &varMap) : fVarMap(varMap)
    { /* Do Nothing */ }

    STAFRC_t set(const STAFString &name, const STAFString &value);
    STAFRC_t set(const STAFString &name, const STAFString &value,
                 STAFString &errorBuffer, bool failIfExists = false);
    STAFRC_t get(const STAFString &name, STAFString &value) const;
    STAFRC_t del(const STAFString &name);

    
    VariableMap getVariableMapCopy();

    static STAFRC_t resolve(const STAFString &aString,
                            const STAFVariablePool *poolList[],
                            unsigned int numPools,
                            STAFString &result,
                            bool ignoreVarResolveErrors = false);

private:

    // Don't allow copy construction or assignment
    STAFVariablePool(const STAFVariablePool &);
    STAFVariablePool &operator=(const STAFVariablePool &);

    VariableMap  fVarMap;
    STAFMutexSem fVarMapSem;
};

typedef STAFRefPtr<STAFVariablePool> STAFVariablePoolPtr;

#endif
