/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

// RexxVar.h
// By Charles Rankin

#ifndef CVR_RexxVar
#define CVR_RexxVar

#include "STAF_iostream.h"
#include "STAF_rexx.h"

class RexxVar {
public:

    RexxVar(const char *name);
    ~RexxVar();

    RexxVar &operator=(const char *value);
    RexxVar &operator=(RXSTRING &value);

    const char *Name() const;
    unsigned long NameLength() const;

    const char *Value() const;
    unsigned long ValueLength() const;

    unsigned long Status() const;

    friend ostream &operator<<(ostream &, const RexxVar &);

private:

    RXSTRING fname;
    RXSTRING fvalue;
    ULONG frc;

};

#endif
