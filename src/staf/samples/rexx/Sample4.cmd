/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*********************************************************************/
/* Sample4.cmd - Rexx sample program using STAF                      */
/*********************************************************************/
/* This sample writes 10 log messages to a global log file called    */
/* LOGTEST and then queries them.  The size of the string written is */
/* based on a random number generated.                               */
/*                                                                   */
/* Returns: 0, on success                                            */
/*         >0, if an error is encountered                            */
/*********************************************************************/
/* Date        Who           Comment                                 */
/* ----------  ------------  --------------------------------------- */
/* 03/02/1998  D. Randall    File Created                            */
/*********************************************************************/
SIGNAL ON HALT NAME STAFAbort

/* Load STAF functions */
call RxFuncAdd "STAFLoadFuncs", "RXSTAF", "STAFLoadFuncs"
call STAFLoadFuncs

/* Register Sample to STAF */
call STAFRegister "STAF_REXX_Sample4"
if RESULT \= 0 then
do
  say "Error registering to STAF:" RESULT
  RETURN RESULT
end

/* Query STAF for a variable */
STAFRC = STAFSubmit("local", "var", "global resolve {STAF/Config/Machine}")
if STAFRC = 0 then machine = STAFResult
else
do
  say "Unable to determine machine name!"
  call STAFUnRegister
  RETURN STAFRC
end

service = "LOG"
times = 10
count = 0
level = error
logtype = "log global logname "
logname=LogTest
string = "Software Testing Automation Framework (STAF) (C) Copyright IBM Corp.",
         "1998. All Rights Reserved"

say  "Settings:" logtype logname "level" level "(times="  times ")"
do count = 1 for times
  message = substr(string,1,1 + random(99))
  msglen = length(message)
  data = "LOG GLOBAL LOGNAME" logname "LEVEL" level "MESSAGE :"msglen":"message
  STAFRC = STAFSubmit("LOCAL", service, data)
  if STAFRC \= 0 then
    say "Log Error: " STAFRC STAFResult
  say count "of" times "Logged"
end

say "Querying last 10 log records..."
data = "QUERY GLOBAL LOGNAME" logname "LAST 10"
STAFRC = STAFSubmit("local", service, data)
say STAFResult

/* Unregister */
call STAFUnRegister
RETURN 0

/*********************************************************************/
/* STAFAbort - If user aborts, make sure STAF unregister occurs.     */
/*********************************************************************/
STAFAbort:
  call STAFUnRegister
EXIT 1
