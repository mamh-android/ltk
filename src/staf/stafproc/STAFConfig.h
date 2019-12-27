/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_Config
#define STAF_Config

#include <deque>
#include "STAFString.h"

extern unsigned int readConfigFile(const STAFString &filename);

extern std::deque<STAFString> getSetVarLines();

#endif
