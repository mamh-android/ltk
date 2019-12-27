/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#include "STAF_iostream.h"
#include "STAF_fstream.h"
#include <string.h>
#include "RexxVar.h"

const int MAX_VALUE_LENGTH = 255;

RexxVar::RexxVar(const char *name)
{
    // Initialize the name of this variable
    fname.strlength = strlen(name);
    fname.strptr = new char[fname.strlength + 1];
    strcpy(fname.strptr, name);

    // Initialize the value of this variable
    // Note that for the call to RexxVariablePool() to get the value
    // you must set the strlength of fvalue to the size of the allocated
    // buffer.  THIS IS CONTRARY TO WHAT IT SAYS IN THE HARDCOPY AND
    // ONLINE PUBS!!
    fvalue.strlength = MAX_VALUE_LENGTH;
    fvalue.strptr = new char[MAX_VALUE_LENGTH + 1];
    fvalue.strptr[0] = 0;

    SHVBLOCK command;

    command.shvnext = (SHVBLOCK *)0;        // Only one request block
    command.shvcode = RXSHV_SYFET;          // Fetch the value
    command.shvname = fname;
    command.shvvalue = fvalue;
    command.shvvaluelen = 0;                // This will be returned to us
    command.shvnamelen = fname.strlength;

    frc = RexxVariablePool(&command);

    if ((frc != RXSHV_OK) && (frc != RXSHV_NEWV)) {
        strcpy(fvalue.strptr, "UNKNOWN");
        fvalue.strlength = strlen("UNKNOWN");
    }
    else {
        // Note that the length of the returned value is not set within the
        // value RXSTRING but within the shvvaluelen data member.  THIS IS
        // CONTRARY TO WHAT IS SAYS IN THE HARDCOPY AND ONLINE PUBS!!
        fvalue.strlength = command.shvvaluelen;
        fvalue.strptr[fvalue.strlength] = 0;
    }

}

RexxVar::~RexxVar()
{
    delete [] fname.strptr;
    delete [] fvalue.strptr;
}

RexxVar &RexxVar::operator=(const char *value)
{
    delete [] fvalue.strptr;

    fvalue.strlength = strlen(value);
    fvalue.strptr = new char[fvalue.strlength + 1];
    strcpy(fvalue.strptr, value);

    SHVBLOCK command;

    command.shvnext = (SHVBLOCK *)0;        // Only one request block
    command.shvcode = RXSHV_SYSET;          // Set the value
    command.shvname = fname;
    command.shvvalue = fvalue;
    command.shvvaluelen = fvalue.strlength;
    command.shvnamelen = fname.strlength;

    frc = RexxVariablePool(&command);

    // What to do about failed set attempts?

    return *this;
}

RexxVar &RexxVar::operator=(RXSTRING &value)
{
    delete [] fvalue.strptr;

    fvalue.strlength = value.strlength;
    fvalue.strptr = new char[fvalue.strlength + 1];

    // When assigning from a RXSTRING we can't be sure that there won't be any
    // NUL characters, thus we must use memcpy() instead of strcpy().
    memcpy(fvalue.strptr, value.strptr, fvalue.strlength);
    fvalue.strptr[fvalue.strlength] = 0;

    SHVBLOCK command;

    command.shvnext = (SHVBLOCK *)0;        // Only one request block
    command.shvcode = RXSHV_SYSET;          // Set the value
    command.shvname = fname;
    command.shvvalue = fvalue;
    command.shvvaluelen = fvalue.strlength;
    command.shvnamelen = fname.strlength;

    frc = RexxVariablePool(&command);

    // What to do about failed set attempts?

    return *this;
}

const char *RexxVar::Value() const
{
    return fvalue.strptr;
}

unsigned long RexxVar::ValueLength() const
{
    return fvalue.strlength;
}

const char *RexxVar::Name() const
{
    return fname.strptr;
}

unsigned long RexxVar::NameLength() const
{
    return fname.strlength;
}

unsigned long RexxVar::Status() const
{
    return frc;
}

ostream &operator<<(ostream &os, const RexxVar &rhs)
{
    os << rhs.fname.strptr << '(' << rhs.fname.strlength << ") = '"
       << rhs.fvalue.strptr << "'(" << rhs.fvalue.strlength << "), status:"
       << rhs.frc;

    return os;
}
