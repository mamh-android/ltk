/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*********************************************************************/
/* Sample2.cmd - Rexx sample program using STAF                      */
/*********************************************************************/
/* This sample loads REXX and STAF Functions, registers to STAF,     */
/* inititates an asynchronous PMSEEK process, queries, stops,        */
/* queries again then frees the process, then unregisters.           */
/*                                                                   */
/* Note that this example explictly uses STAFHandle in all calls     */
/* and additional error checking is needed.                          */
/*                                                                   */
/* Returns: 0,  on success                                           */
/*         >0, if an error is encountered                            */
/*********************************************************************/
/* Date        Who           Comment                                 */
/* ----------  ------------  --------------------------------------- */
/* 02/02/1998  D. Randall    File Created                            */
/*********************************************************************/
SIGNAL ON HALT NAME STAFAbort

/* Load system functions */
call RxFuncAdd "SysLoadFuncs", "REXXUTIL", "SysLoadFuncs"
call SysLoadFuncs

/* Load STAF functions */
call RxFuncAdd "STAFLoadFuncs", "RXSTAF", "STAFLoadFuncs"
call STAFLoadFuncs

/* Register Sample to STAF */
call STAFRegister "STAF_REXX_Sample2", "STAFHandle"
if RESULT \= 0 then
do
  say "Error registering to STAF:" RESULT
  RETURN RESULT
end

/* Set a global variable for start timestamp */
request = " GLOBAL SET Start="||DATE('s')||'-'||TIME()
STAFRC = STAFSubmit(STAFHandle, "local", "VAR", request)

/* Build the process start request with a work load name of STAFSample2 */
request = "START COMMAND pmseek.exe WORKLOAD STAFSample2"

/* Submit the request to STAF */
STAFRC = STAFSubmit(STAFHandle, "local", "PROCESS", request)
if STAFRC = 0 then
do
  PROCHandle = STAFResult
  STAFRC = STAFSubmit(STAFHandle, "local", "VAR", "LIST")
  say STAFResult
  say "Press <Enter> to continue"
  pull response

  request = "query handle" PROCHandle
  STAFRC = STAFSubmit(STAFHandle, "local", "PROCESS", request)
  say STAFResult
  say "Press <Enter> to stop PMSEEK"
  pull response

  STAFRC = STAFSubmit(STAFHandle, "local", "PROCESS", "STOP HANDLE" PROCHandle)
  call SysSleep 1

  STAFRC = STAFSubmit(STAFHandle, "local", "PROCESS", request)
  say STAFResult

  STAFRC = STAFSubmit(STAFHandle, "local", "PROCESS", "FREE HANDLE" PROCHandle)

  STAFRC = STAFSubmit(STAFHandle, "local", "var", "resolve {Start}")
  if (STAFRC = 0) & (STAFResult \= '') then Start = STAFResult

  say "Start =" Start
  say "End   =" DATE('s')||'-'||TIME()
end

/* Unregister */
call STAFUnRegister STAFHandle
RETURN 0

/*********************************************************************/
/* STAFAbort - If user aborts, make sure STAF unregister occurs.     */
/*********************************************************************/
STAFAbort:
  call STAFUnRegister STAFHandle
  EXIT 1
