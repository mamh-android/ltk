/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*********************************************************************/
/* Sample3.cmd - Rexx sample program using STAF                      */
/*********************************************************************/
/* This sample writes a monitor message and then queries it, this    */
/* is done 10 times.  A size of the string written is based on a     */
/* random number generated.                                          */
/*                                                                   */
/* Returns: 0, on success                                            */
/*         >0, if an error is encountered                            */
/*********************************************************************/
/* Date        Who           Comment                                 */
/* ----------  ------------  --------------------------------------- */
/* 03/01/1998  D. Randall    File Created                            */
/*********************************************************************/
SIGNAL ON HALT NAME STAFAbort

/* Load STAF functions */
call RxFuncAdd "STAFLoadFuncs", "RXSTAF", "STAFLoadFuncs"
call STAFLoadFuncs

/* Register Sample to STAF */
call STAFRegister "STAF_REXX_Sample3"
if RESULT \= 0 then
do
  say "Error registering to STAF:" RESULT
  RETURN RESULT
end

/* Query STAF for a variable */
STAFRC = STAFSubmit("local", "var", "resolve {STAF/Config/MachineNickname}")
if STAFRC = 0 then machine = STAFResult
else
do
  say "Unable to determine machine nickname!"
  call STAFUnRegister
  RETURN STAFRC
end

service = "monitor"
times = 10
wait = 0
string = "Software Testing Automation Framework (STAF) (C) Copyright IBM Corp.",
         "1998 All Rights Reserved"

do times
  message = substr(string,1,1 + random(99))
  msglen = length(message)
  data = "LOG MESSAGE :"msglen":"message
  STAFRC = STAFSubmit("local", service, data)
  if STAFRC \= 0 then
    say "Monitor Log Error: " STAFRC STAFResult

  data = "query machine" machine "handle" STAFHandle
  STAFRC = STAFSubmit("local", service, data)
  if STAFRC = 0 then
    say STAFResult
  else
    say "Monitor Query Error: " STAFRC STAFResult
end

/* Unregister */
call STAFUnRegister
RETURN 0

/*********************************************************************/
/* STAFAbort - If user aborts, make sure STAF unregister occurs.     */
/*********************************************************************/
STAFAbort:
  call STAFUnRegister
  EXIT 1

