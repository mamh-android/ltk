/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*********************************************************************/
/* Sample1.cmd - Rexx sample program using STAF                      */
/*********************************************************************/
/* This sample loads the STAF Functions, registers to STAF, queries  */
/* a global variable from STAF, inititates a synchronous process     */
/* which is a chkdsk of the boot drive, then unregisters.            */
/*                                                                   */
/* Returns: 0, on success                                            */
/*         >0, if an error is encountered                            */
/*********************************************************************/
/* Date        Who           Comment                                 */
/* ----------  ------------  --------------------------------------- */
/* 02/01/1998  D. Randall    File Created                            */
/*********************************************************************/
SIGNAL ON HALT NAME STAFAbort

/* Load STAF functions */
call RxFuncAdd "STAFLoadFuncs", "RXSTAF", "STAFLoadFuncs"
call STAFLoadFuncs

/* Register to STAF */
call STAFRegister "STAF_REXX_Sample1"
if RESULT \= 0 then
do
  say "Error registering to STAF:" RESULT
  RETURN RESULT
end

/* Query STAF for a variable */
STAFRC = STAFSubmit("local", "var", "resolve {STAF/Config/BootDrive}")
if STAFRC = 0 then
  bootdrive = STAFResult
else
do
  say "Unable to determine boot drive!"
  call STAFUnRegister
  RETURN STAFRC
end

/* Build the process start request with a work load name of STAFSample */
request = "START WAIT COMMAND chkdsk.com WORKLOAD STAFSample"

/* Pass the boot drive parameter to chkdsk */
parms = "PARMS" bootdrive

/* Query STAF for a variable */
STAFRC = STAFSubmit("local", "var", "resolve {STAF/Config/Sep/File}")
if STAFRC = 0 then
  filesep = STAFResult
else
do
  say "Unable to determine file seperator!"
  call STAFUnRegister
  RETURN STAFRC
end

/* Set the working directory */
workdir = "WORKDIR" bootdrive||filesep

/* Submit the request to STAF */
say "Attempting to CHKDSK bootdrive" bootdrive
STAFRC = STAFSubmit("local", "process", request parms workdir)
say "Submit Return Code="STAFRC ", Result="STAFResult

/* Unregister */
call STAFUnRegister
RETURN 0

/*********************************************************************/
/* STAFAbort - If user aborts, make sure STAF unregister occurs.     */
/*********************************************************************/
STAFAbort:
  call STAFUnRegister
  EXIT 1
