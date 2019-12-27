/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/*********************************************************************/
/* Sample5.cmd - Rexx sample program using STAF                      */
/*********************************************************************/
/* This sample uses the STAF Ping command to ping a STAF client and  */
/* and measure the throughput.                                       */
/*                                                                   */
/* Accepts: [Machine] [LoopCount] [DisplayModulus]                   */
/*                                                                   */
/* Returns: 0, on success                                            */
/*         >0, if an error is encountered                            */
/*********************************************************************/
/* Date        Who           Comment                                 */
/* ----------  ------------  --------------------------------------- */
/* 03/02/1998  C. Rankin     File Created                            */
/*********************************************************************/
SIGNAL ON HALT NAME STAFAbort

parse arg Machine LoopCount DisplayModulus

if (Machine = "?") | (Machine = "/?") | (Machine = "-?") then
do
    say
    say "Usage: PingTest [Machine] [LoopCount] [DisplayModulus]"
    RETURN 1
end

if Machine = "" then Machine = "LOCAL"
if LoopCount = "" then LoopCount = 999999999
if DisplayModulus = "" then DisplayModulus = 1000

call RxFuncAdd "STAFLoadFuncs", "RXSTAF", "STAFLoadFuncs"
call STAFLoadFuncs

call STAFRegister "STAF_PING_Test", "STAFHandle"
if RESULT \= 0 then
do
    say "Error registering with STAF, RC:" RESULT
    RETURN 1
end

say TIME() "- Started"

startDate = DATE('B')
startTime = TIME('S')
errors = 0

do i=1 to LoopCount
    call STAFSubmit STAFHandle, Machine, "PING", "PING"

    if RESULT \= 0 then
    do
        errors = errors + 1
        say TIME() "- Loop #"i", Error #"errors", RC:" RESULT
    end
    else if STAFResult \= "PONG" then
    do
        errors = errors + 1
        say TIME() "- Loop #"i", Error #"errors", STAFResult =" STAFResult
    end

    if i // DisplayModulus = 0 then
        say TIME() "- Ended Loop #"i", Errors =" errors
end

call AtEnd

RETURN 0


/*********************************************************************/
/* AtEnd - Unregister from STAF and calculate final PING throughput. */
/*********************************************************************/
AtEnd:

    call STAFUnRegister STAFHandle
    say TIME() "- Ended"

    endTime = TIME('S')
    endDate = DATE('B')

    endSecs = ((endDate - 720000) * 86400) + endTime
    startSecs = ((startDate - 720000) * 86400) + startTime

    say "Average:" FORMAT((i / (endSecs - startSecs)), 3, 2) "pings/sec"

    RETURN 0


/*********************************************************************/
/* STAFAbort - If user aborts, make sure STAF unregister occurs.     */
/*********************************************************************/
STAFAbort:

    call AtEnd
    EXIT 1
