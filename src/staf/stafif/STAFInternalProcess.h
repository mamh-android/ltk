/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

#ifndef STAF_InternalProcess
#define STAF_InternalProcess

#ifdef __cplusplus

#include "STAFString.h"

// Do not reference; not exported
extern char *gSTAFProcessShellSubstitutionChars;

// Defines a structure to receive process shell command substitution data
struct STAFProcessShellSubstitutionData
{
    STAFString command;
    STAFString title;
    STAFString workload;
    STAFString stdinfile;
    STAFString stdoutfile;
    STAFString stderrfile;
    STAFString username;
    STAFString password;
};

STAFRC_t STAFProcessValidateShellSubstitutionChars(const STAFString &shellCmd);

STAFRC_t STAFProcessReplaceShellSubstitutionChars(const STAFString &shellCmd, 
             const STAFProcessShellSubstitutionData &data, STAFString &output);

#endif

#endif
