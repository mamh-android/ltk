/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Exception
#define STAF_Exception

#ifdef __cplusplus

#include <cstring>
#include <cstdio>
#include "STAF_iostream.h"
#include "STAFError.h"

/* The THROW_STAF_EXCEPTION macro is used to throw a STAFException or */
/* one of its derivatives.  It also adds the location information to  */
/* the exception.                                                     */

#define THROW_STAF_EXCEPTION(e)\
{ e.setLocation(__FILE__, __LINE__); throw e; }

class STAFException
{
public:

    STAFException(const char *text, unsigned int errorCode = 0,
                  const char *name = "STAFException")
        : fErrorCode(errorCode)
    {
        // Using strncpy to make sure don't exceed the size of fName
        strncpy(fName, name, sizeof(fName) - 1);
        fName[sizeof(fName) - 1] = 0;

        // Using strncpy to make sure don't exceed the size of fText
        strncpy(fText, text, sizeof(fText) - 1);
        fText[sizeof(fText) - 1] = 0;

        fLocation[0] = 0;
    }

    void setLocation(const char *filename, unsigned int lineNumber)
    {
        // Make sure don't exceed the size of fLocation.
        // Note: The 16 is based on the lineNumber length being at most 13,
        //       plus 2 for the size of "(" and ")" plus 1.
        sprintf(fLocation, "%.*s(%u)",
                static_cast<int>(sizeof(fLocation) - 16),
                filename, lineNumber);
    }

    void trace(const char *caughtWhere = 0);
    void trace(unsigned int tracePoint, const char *caughtWhere = 0);

    static void checkRC(STAFRC_t rc, const char *errorMessage, 
                        unsigned int osRC = 0);

    inline void write(const char *caughtWhere = 0, ostream &os = cout)
    {
        if (caughtWhere) os << "In " << caughtWhere << ":" << endl;

        os << "Caught STAFException" << endl
           << "Name      : " << getName() << endl
           << "Location  : " << getLocation() << endl
           << "Text      : " << getText() << endl
           << "Error code: " << getErrorCode() << endl; 
    }
    
    const char *getName() { return fName; }
    const char *getText() { return fText; }
    const char *getLocation() { return fLocation; }
    unsigned int getErrorCode() { return fErrorCode; }

    ~STAFException() { /* Do Nothing */ }

protected:

    static void checkStandardExceptions(char *buffer, STAFRC_t rc,
                                        const char *errorMessage,
                                        unsigned int osRC);
private:

    char fName[128];
    /* Note: If change size of fText, must change size of buffer */
    char fText[512];
    char fLocation[384];
    unsigned int fErrorCode;
};


/* The STAF_EXCEPTION_DEFINITION macro is used to create new types of */
/* STAF Exceptions.                                                 */

#define STAF_EXCEPTION_DEFINITION(className, baseName)\
class className : public baseName\
{\
public:\
    className(const char *text, unsigned int errorCode = 0,\
              const char *name = #className)\
        : baseName(text, errorCode, name)\
    { /* Do Nothing */ }\
\
    static void checkRC(STAFRC_t rc, const char *errorMessage, unsigned int osRC = 0)\
    {\
        if (rc != kSTAFOk)\
        {\
            /* Note the buffer size must be same as the fText size */\
            char buffer[512] = { 0 };\
            checkStandardExceptions(buffer, rc, errorMessage, osRC);\
            sprintf(buffer, "%.*s",\
                    static_cast<int>(sizeof(buffer) - 1), errorMessage);\
            THROW_STAF_EXCEPTION(className(buffer, rc));\
        }\
    }\
\
    ~className() { /* Do Nothing */ }\
};

STAF_EXCEPTION_DEFINITION(STAFOutOfBoundsException,   STAFException);
STAF_EXCEPTION_DEFINITION(STAFInvalidObjectException, STAFException);
STAF_EXCEPTION_DEFINITION(STAFInvalidParmException,   STAFException);
STAF_EXCEPTION_DEFINITION(STAFBaseOSErrorException,   STAFException);

inline void STAFException::checkRC(STAFRC_t rc, const char *errorMessage,
                                   unsigned int osRC)
{
    if (rc != kSTAFOk)
    {
        // Note the buffer size must be same as the fText size
        char buffer[512] = { 0 };
        checkStandardExceptions(buffer, rc, errorMessage, osRC);
        // Makes sure no buffer overflow occurs if errorMessage
        // exceeds size of buffer
        sprintf(buffer, "%.*s", static_cast<int>(sizeof(buffer) - 1),
                errorMessage);
        THROW_STAF_EXCEPTION(STAFException(buffer, rc));
    }
}

inline void STAFException::checkStandardExceptions(char *buffer, STAFRC_t rc,
                                                   const char *errorMessage,
                                                   unsigned int osRC)
{
    switch (rc)
    {
        case kSTAFBaseOSError:
            // Makes sure no buffer overflow occurs 
            // Note: The 16 is based on the osRC length being at most 13,
            //       plus 2 for the size of ": " plus 1.
            sprintf(buffer, "%.*s: %u", static_cast<int>(sizeof(buffer) - 16),
                    errorMessage, osRC);
            THROW_STAF_EXCEPTION(STAFBaseOSErrorException(buffer, rc));

        case kSTAFInvalidObject:
            // Makes sure no buffer overflow occurs if errorMessage 
            // exceeds size of buffer
            sprintf(buffer, "%.*s", static_cast<int>(sizeof(buffer) - 1),
                    errorMessage);
            THROW_STAF_EXCEPTION(STAFInvalidObjectException(buffer, rc));

    case kSTAFInvalidParm:
            // Makes sure no buffer overflow occurs if errorMessage 
            // exceeds size of buffer
            sprintf(buffer, "%.*s", static_cast<int>(sizeof(buffer) - 1),
                    errorMessage);
            THROW_STAF_EXCEPTION(STAFInvalidParmException(buffer, rc));

        default:
            break;
    }
}
 
#endif

#endif
