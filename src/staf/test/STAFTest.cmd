/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2001                                              */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

/******************************************************************************/
/* REXX STAFTest Program for Software Testing Automation Framework (STAF)     */
/* -------------------------------------------------------------------------- */
/* Description: This program tests STAF.                                      */
/* Options: See HELP                                                          */
/*                                                                            */
/*  Note: This program requires RxPP (REXX pre-processor) to be used to       */
/*        generate the executable cmd file.                                   */
/* -------------------------------------------------------------------------- */
/* History:                                                                   */
/*  1.00  DHR  02/10/1998  Initial implementation                             */
/******************************************************************************/
options "EXMODE ETMODE"
signal on halt name STAFAbort
parse arg request

call InitParser

call AddOption "GENWLONLY", 1, "NO"
call AddOption "NOGENWL", 1, "NO"
call AddOption "INTERNALONLY", 1, "NO"
call AddOption "EXTERNALONLY", 1, "NO"
call AddOption "QUIET", 1, "NO"
call AddOption "HELP", 1, "NO"
call AddOption "PINGONLY", 1, "NO"
call AddOption "PROCESSONLY", 1, "NO"
call AddOption "MACHINE", 1, "YES"
call AddOption "WHERE", 1, "YES"
call AddOption "LOOPS", 1, "YES"
call AddOption "MAXERROR", 1, "YES"
call AddOptionGroup "GENWLONLY PINGONLY PROCESSONLY INTERNALONLY EXTERNALONLY", 0, 1
call AddOptionGroup "NOGENWL GENWLONLY", 0, 1
rc = ParseString(request, "errorBuffer")
if rc \= 0 then do
  say errorBuffer
  exit
end

if optionTimes("HELP") > 0 then do
   say "STAFTest [WHERE <where>] [MACHINE <machine>]"
   say "         [LOOPS <loops>] [MAXERROR <maxerror>]"
   say "         [GENWLONLY] [INTERNALONLY] [EXTERNALONLY] [PINGONLY] [PROCESSONLY]"
   say "         [NOGENWL] [QUIET] [HELP]"
   exit
end

if optionTimes("NOGENWL") > 0 then genwl = 0
else genwl = 1

if optionTimes("WHERE") > 0 then where = OptionValue("WHERE")
else where = "LOCAL"

if optionTimes("QUIET") > 0 then loud = 0
else loud = 1

maxerror = optionValue("MAXERROR")
if maxerror < 1 | datatype(maxerror,'W') = 0 then maxerror = 25

times = optionValue("LOOPS")
if times < 1 | datatype(times,'W') = 0 then times = 0
else times = times + 1

if \ loud then do
  say "STAFTest Initiated in QUIET Mode..."
end

parse source OSType . .
if OSType = "OS/2" then shellCommand = "cmd.exe"
else if OSType = "AIX" then shellCommand = "ksh"
else shellCommand = "command.com"

call STAFErrorText
initiated = time('l')
call time('r')
crlf = x2c("0D0A")
numeric digits 25
loop = 1
totok = 0
error = 0
errortxt = crlf
cleanup = 0
delimit='\'
dc = '*'                                 /* Means don't care what is returned */
logfile = STAFTest.log
testcase = "STAFTest"||time('s')||random(1,99)

call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs

call RxFuncAdd 'STAFLoadFuncs', 'RXSTAF', 'STAFLoadFuncs'
call STAFLoadFuncs

/* Load the STAF RxThread functions */
call RxFuncAdd "RxThreadLoadFuncs", "RxThread", "RxThreadLoadFuncs"
call RxThreadLoadFuncs

call STAFRegister "STAFTest"
if RESULT \= 0 then do
  say "Error registering with STAF, rc="RESULT
  exit
end

machine = optionValue("MACHINE")
if machine = '' then do
  call autodoit 0 where dc "VAR" "resolve system string {STAF/Config/Machine}"
  if STAFRC = 0 then machine = STAFResult
end

call autodoit 0 "LOCAL" dc "VAR" "resolve system string {STAF/Config/EffectiveMachine}"
if STAFRC = 0 then origmachine = STAFResult

internal = 0
external = 0

if optionTimes("PROCESSONLY") > 0 then do
  do until loop = times
    call autodoit 0 where dc "PROCESS" 'start command" shellCommand "title STAFTest-'||origmachine||"-"||loop 'parms "/c dir" wait'
    loop = loop + 1
  end
  call Endit
  call STAFUnRegister
  exit
end

if optionTimes("PINGONLY") > 0 then do
  do until loop = times
    call autodoit 0 where "PONG" "PING" "PING"
   loop = loop + 1
  end
  call Endit
  call STAFUnRegister
  exit
end

if optionTimes("GENWLONLY") > 0 then do
  call GENWLSetup
  do until loop = times
    call GENWLRun
    loop = loop + 1
  end
  call EndIt
  call STAFUnRegister
  exit
end

external = 1
if optionTimes("INTERNALONLY") > 0 then external = 0

internal = 1
if optionTimes("EXTERNALONLY") > 0 then internal = 0

if internal then do
  if genwl then call GENWLSetup
  call autodoit 0 where dc "VAR" "set system var For"||testcase||"={For"||testcase||"{Ever"||testcase||"}}"
  call autodoit 0 where dc "VAR" "set system var Ever"||testcase||"={For"||testcase||"}"
  call autodoit 0 where dc "VAR" "set var TraceFile={STAF/Config/STAFRoot}\bin\STAF.trc"
  call autodoit 10 where dc "PROCESS" "start command" shellCommand "workdir" time('s')
end

if external then do
  timestamp=date('w') date() time('l')
  msglen = length(timestamp) + 26
  level = substr('0', 1, 32, '0')
  level = overlay('1', level, random(18,32))
  call autodoit 0 where dc "LOG" "log global logname staftest level" level "message :"||msglen||":Log STAFTest Initiated on" timestamp
  call autodoit 0 where dc "LOG" "log machine logname staftest level" level "message :"||msglen||":Log STAFTest Initiated on" timestamp
  call autodoit 0 where dc "LOG" "log handle logname staftest level" level "message :"||msglen||":Log STAFTest Initiated on" timestamp
end

do until loop = times
  if loud then do
    say ""
    do 3
      say "==============================================================================="
    end
    say "Loop" loop "start. " totok "total test iterations run."
    say "A total of" error "errors encountered with" maxerror "maximum errors allowed."
    do 3
      say "==============================================================================="
    end
    say ""
  end
  if internal then do
    if genwl then call GENWLRun
    call autodoit 0 where dc "PROCESS" "start command" shellCommand "title STAFTest-"||origmachine"-"loop "workload STAFTest var STAFVar=AutoVar env STAFEnv=AutoEnv"
    if STAFRC > 0 then do
      say "Unable to start process"
      call STAFUnRegister
      exit
    end
    PROCHandle = STAFResult
    call autodoit 0 where dc "MISC" "version"
    call autodoit 0 where dc "MISC" "machine "||machine
    call autodoit 0 where dc "VAR" "resolve string {TraceFile}"
    tracefile=STAFResult
    call autodoit 0 where dc "MISC" "trace" "to FILE" tracefile
    call autodoit 0 where dc "MISC" "trace" "to STDOUT"
    call autodoit 0 where dc "MISC" "trace" "off All"
    call autodoit 0 where dc "MISC" "trace" "list"
    call autodoit 0 where dc "MISC" "trace" "on ServiceError"
    call autodoit 0 where dc "MISC" "trace" "list"
    call autodoit 7 where dc "PING" "STAF"
    call autodoit 0 where dc "MISC" "trace" "off ServiceError"
    call autodoit 0 where dc "MISC" "trace" "on All"
    call autodoit 0 where dc "MISC" "trace" "off All"
    call autodoit 7 where dc "MISC" "trace" "STAFStuff"
    call autodoit 0 where "PONG" "PING" "ping"
    call autodoit 0 where dc "VAR" 'set system var Start="'||initiated||'"'
    call autodoit 0 where dc "VAR" "set system var When={Start}"
    echoval = date('w')
    call autodoit 0 where echoval "ECHO" "echo "||echoval
    call autodoit 0 where dc "DELAY" "delay 1"
    call autodoit 0 where dc "SERVICE" "list"
    call autodoit 0 where dc "PROCESS" "query all"
    call autodoit 0 where dc "PROCESS" "query handle "||PROCHandle
    call autodoit 5 where dc "PROCESS" "query handle "||time('s')||time('s')
    call autodoit 12 where dc "PROCESS" "free handle "||PROCHandle
    call autodoit 0 where dc "TRUST" "set default level 5"
    trustmachine = "STAFTest"||time('s')||random(1,99)
    call autodoit 0 where dc "TRUST" "set machine "||trustmachine||" level "||random(1,5)
    call autodoit 23 where dc "TRUST" "delete machine "||time('s')
    call autodoit 24 where dc "TRUST" "set machine badlevel level 99"
    call autodoit 0 where dc "TRUST" "list"
    call autodoit 0 where dc "TRUST" "get machine" trustmachine
    call autodoit 0 where dc "TRUST" "delete machine" trustmachine
    call autodoit 0 where dc "HANDLE" "query all"
    call autodoit 0 where dc "HANDLE" "query name STAFTest"
    call autodoit 0 where dc "HANDLE" "query all INPROCESS"
    call autodoit 0 where dc "VAR" "list"
    call autodoit 13 where dc "VAR" "get system var Machine"
    call autodoit 0 where dc "VAR" "resolve system string {When}"
    call autodoit 16 "ausvmr.austin.ibm.com" dc "PING" "ping"
    call autodoit 0 where dc "FS" "copy file {STAF/Config/STAFRoot}\bin\staf.cfg tofile {STAF/Config/STAFRoot}\bin\STAFTest.XXX tomachine" machine
    call autodoit 17 where dc "FS" "copy file {STAF/Config/STAFRoot}\bin\staf.cfg tofile {STAF/Config/STAFRoot}\bin\STAFTest.YYY tomachine" machine "FAILIFNEW"
    call autodoit 17 where dc "FS" "copy file {STAF/Config/STAFRoot}\bin\staf.cfg tofile {STAF/Config/STAFRoot}\bin\STAFTest.XXX tomachine" machine "FAILIFEXISTS"
    call autodoit 17 where dc "FS" "get file {STAF/Config/STAFRoot}\bin\STAFTest.YYY"
    call autodoit 0 where dc "FS" "get file {STAF/Config/STAFRoot}\bin\STAFTest.XXX"
    call autodoit 2 where dc "UNKNOWNSERVICE" ""
    call autodoit 16 time('s')||STAFHandle||time('s') dc "VAR" "GET VAR BADHANDLE"
    call autodoit 7 where dc "PING" "INVALIDREQUEST"
    call autodoit 13 where dc "VAR" "resolve system string {UNKNOWNVAR"||time('s')||"}"
    call autodoit 14 where dc "VAR" "resolve system string {For"||testcase||"}"
    call autodoit 15 where dc "VAR" "resolve system string {BadVar"
    call autodoit 16 "UNKNOWNMACHINE"||time('s') dc "PING" "ping"
    queuepriority = random(1,5000)
    if translate(where) = "LOCAL" then QHandle = STAFHandle
    else QHandle = PROCHandle
    call autodoit 0 where dc "QUEUE" "queue handle" QHandle "message" testcase||"/DefaultTest"
    call autodoit 0 where dc "QUEUE" "queue handle" QHandle "priority" 1 "message" testcase||"/Priority-1-Test"
    call autodoit 0 where dc "QUEUE" "queue handle" QHandle "priority" queuepriority "message" testcase||"/QueueTest"
    call autodoit 0 where dc "QUEUE" "list handle" QHandle
    if translate(where) = "LOCAL" then
    do
      call autodoit 0 where dc "QUEUE" "peek handle" QHandle
      call autodoit 0 where dc "QUEUE" "peek handle" QHandle "contains Queue"
      call autodoit 0 where dc "QUEUE" "peek handle" QHandle "contains Queue contains Test"
      call autodoit 0 where dc "QUEUE" "peek handle" QHandle "priority 1"
      call autodoit 0 where dc "QUEUE" "get handle" QHandle "priority 1"
      call autodoit 0 where dc "QUEUE" "get handle" QHandle "contains Default"
      call autodoit 29 where dc "QUEUE" "get handle" time('s')||time('s')
      call autodoit 0 where dc "QUEUE" "get handle" QHandle
      call autodoit 29 where dc "QUEUE" "get handle" QHandle
      call autodoit 0 where dc "QUEUE" "queue handle" QHandle "priority" queuepriority "message" testcase||"/QueueTest"
      call autodoit 0 where 1 "QUEUE" "delete handle" QHandle
      call autodoit 29 where dc "QUEUE" "peek handle" QHandle
      call autodoit 0 where dc "QUEUE" "queue handle" QHandle "priority" 1 "message" "STAF/Queue"
      call autodoit 0 where dc "QUEUE" "queue handle" QHandle "priority" 2 "message" testcase||"/QueueTest"
      call autodoit 0 where dc "QUEUE" "queue handle" QHandle "priority" 2 "message" testcase||"/QueueTest"
      call autodoit 0 where dc "QUEUE" "list handle" QHandle
      call autodoit 0 where 2 "QUEUE" "delete handle" QHandle "contains Test"
      call autodoit 0 where 1 "QUEUE" "delete handle" QHandle "priority 1"
      call autodoit 29 where dc "QUEUE" "peek handle" QHandle
      call autodoit 5 where dc "QUEUE" "list handle" QHandle||time('s')||time('s')
    end
    call autodoit 0 where dc "SEM" "event" testcase "post"
    call autodoit 0 where dc "SEM" "event" testcase "query"
    call autodoit 0 where dc "SEM" "event" testcase "reset"
    call autodoit 0 where dc "SEM" "event" testcase "pulse"
    call autodoit 0 where dc "SEM" "list event"
    call autodoit 37 where dc "SEM" "event" testcase "wait 100"
    call autodoit 0 where dc "SEM" "mutex" testcase "request 100"
    call autodoit 0 where dc "SEM" "mutex" testcase "query"
    call autodoit 0 where dc "SEM" "list mutex"
    call autodoit 0 where dc "SEM" "mutex" testcase "release"
    call autodoit 34 where dc "SEM" "mutex" STAFHandle||time('s') "query"
    call autodoit 0 where dc "SEM" "event" testcase "delete"
    call autodoit 0 where dc "FS" "help"
    call autodoit 0 where dc "HANDLE" "help"
    call autodoit 0 where dc "MISC" "help"
    call autodoit 0 where dc "PROCESS" "help"
    call autodoit 0 where dc "QUEUE" "help"
    call autodoit 0 where dc "SERVICE" "help"
    call autodoit 0 where dc "TRUST" "help"
    call autodoit 0 where dc "VAR" "help"
    call autodoit 0 where dc "SEM" "help"
    call autodoit 12 where dc "process free handle" PROCHandle
    call autodoit 0 where dc "process stop handle" PROCHandle
    call SysSleep 2
    call autodoit 11 where dc "process stop handle" PROCHandle
    call autodoit 0 where dc "process free handle" PROCHandle
  end
  if external then do
    call autodoit 0 where dc "HELP" "version"
    call autodoit 0 where dc "HELP" "help"
    call autodoit 0 where dc "HELP" "error"
    call autodoit 0 where dc "HELP" "error" random(1,38)
    call autodoit 0 where dc "HELP" "error" random(4000,5000)
    call autodoit 0 where dc "MONITOR" "help"
    timestamp=date('w') date() time('l')
    msglen = length(timestamp) + 17
    call autodoit 0 where dc "MONITOR" "log message :"msglen":"||"Monitor STAFTest" timestamp
    call autodoit 0 where dc "MONITOR" "query machine" origmachine "handle" STAFHandle
    call autodoit 7 where dc "MONITOR" "ask machine" origmachine "handle" STAFHandle
    call autodoit 17 where dc "MONITOR" "query machine" origmachine "handle HAN"||time('s')
    call autodoit 0 where dc "MONITOR" "list machines"
    call autodoit 0 where dc "MONITOR" "list machine" origmachine
    call autodoit 0 where dc "MONITOR" "refresh var"
    call autodoit 0 where dc "LOG" "help"
    call autodoit 0 where dc "LOG" "list global"
    call autodoit 0 where dc "LOG" "query global logname staftest last" random(1,10)
    call autodoit 0 where dc "LOG" "query global logname staftest contains Monday contains Tuesday first 2 levelbitstring"
    call autodoit 0 where dc "LOG" "query machine" origmachine "logname staftest from 19970101@08:30:45 to today first 5"
    call autodoit 0 where dc "LOG" "query global logname staftest total"
    timestamp=date('w') date() time('l')
    msglen = length(timestamp) + 26
    level = substr('0', 1, 32, '0')
    level = overlay('1', level, random(1,8))
    call autodoit 0 where dc "LOG" "query machine" origmachine "handle" STAFHandle "logname staftest levelmask 11111111111111111111111111111111 STATS"
    call autodoit 0 where dc "LOG" "query machine" origmachine "handle" STAFHandle "logname staftest total"
    if STAFResult > 10 then do
      call autodoit 0 where dc "LOG" "purge machine" origmachine "handle" STAFHandle "logname staftest first 5 confirm"
      call autodoit 0 where dc "LOG" "delete machine" origmachine "handle" STAFHandle "logname staftest confirm"
    end
    call autodoit 0 where dc "LOG" "log handle logname staftest level" level "message :"||msglen||":Log STAFTest Continues at" timestamp
    call autodoit 0 where dc "LOG" "query machine" origmachine "handle" STAFHandle "logname staftest qmachine" origmachine "qhandle" STAFHandle
    call autodoit 7 where dc "LOG" "ask global logname staftest last" random(1,10)
    call autodoit 17 where dc "LOG" "query machine" origmachine "handle HAN"||time('s')" logname LOG"||time('s')
    call autodoit 20 where dc "LOG" "delete machine" time('s') "logname" time('s') "confirm"
    call autodoit 4001 where dc "LOG" "query machine" origmachine "logname staftest first" origmachine
    call autodoit 4002 where dc "LOG" "query global logname staftest from "||time('s')
    call autodoit 4003 where dc "LOG" "query global logname staftest from today@"||time('s')
    call autodoit 4004 where dc "LOG" "query global logname staftest levelmask BADLEVEL"
    call autodoit 4008 where dc "LOG" "purge global logname staftest CONFIRM"
    call autodoit 0 where dc "LOG" "list machines"
  end
  loop = loop + 1
  if error > 0 & loud then do
    say "###############################################################################"
    say "Error Count =" error
    say "###############################################################################"
    say "Errors (Timestamp|Handle|Where|Service|Request|STAFRC|STAFResult):" errortxt
    say "###############################################################################"
    say "Error Count =" error
    say "###############################################################################"
  end
end

call Endit
call Cleanup
exit

autodoit:
  parse arg Aerrorok Awhere Aresult Aservice Arequest
  Aerror = 0
  if loud then do
    say "..............................................................................."
    say "Service="Aservice " Where="Awhere " ExpectedSTAFRC="Aerrorok " TotalErrors="error
    say "LoopNum="loop " Handle="STAFHandle " Started="initiated " Elapsed="time('e')
    say "Request="Arequest
    if Aresult \= '*' then say "ExpectedRequest="||Aresult
  end
  STAFRC = STAFSubmit(Awhere, Aservice, Arequest)
  if STAFRC = Aerrorok then do
    if Aresult \= '*' & Aresult \= STAFResult then Aerror = 1
    else do
      totok = totok + 1
      if STAFRC > 0 & loud then say "STAFRC="||STAFRC
      if STAFResult \= 0 & STAFResult > '' & loud then say STAFResult
      else nop
    end
  end
  else Aerror = 1
  if Aerror = 1 then do
    if loud then do
      say "###############################################################################"
      say "STAFRC="||STAFRC "STAFResult="||STAFResult
      say "###############################################################################"
    end
    errortxt = errortxt||time('l')||"|"||STAFHandle||"|"||Awhere||"|"||Aservice||"|"||Arequest||"|"||STAFRC||"|"||STAFResult||crlf
    error = error + 1
    if cleanup = 0 & error >= maxerror then call STAFAbort
  end
return

STAFAbort:
  call EndIt
  say "Are you sure you want to exit STAFTest (Y/N)?"
  parse upper pull response
  if response \= "Y" then return
  if error > 0 then do
    say "Do you want to append the errors to" logfile "(Y/N)?"
    parse upper pull response
    if response = "Y" then do
      status = FileStatus(logfile, 10, "write")
      if status \= errorNotReady then do
        rc = charout(logfile, "********************************************************")
        rc = charout(logfile, errortxt)
      end
      else say "Unable to write to" logfile
    end
  end
  call Cleanup
exit

GENWLSetup:
  call autodoit 0 where dc "VAR" "resolve system string {STAF/Config/BootDrive}"
  GW_bootdrive = STAFResult
  GW_workload = "STAFTest Handle" STAFHandle "Time" time('l')
  if where = "LOCAL" then GW_machine = machine
  else GW_machine = where
  GW_data = "WORKLOAD STAFTest"||STAFHandle||crlf ,
            'VAR "STAFTest=Workload-Var"'||crlf ,
            "MACHINE" GW_machine||crlf ,
            "    PROCESS"||crlf ,
            "        COMMAND" shellCommand||crlf ,
            '        PARMS "/c dir"'||crlf ,
            "        WORKDIR " GW_bootdrive||delimit||crlf ,
            '        ENV "STAFTest=Environment"'||crlf ,
            '        VAR "STAFTest=Local-Var"'||crlf ,
            "    END"||crlf
  if optionTimes("GENWLONLY") > 0 then GW_data = GW_data||"    PROCESS COMMAND pmseek.exe END"||crlf
  GW_data = GW_data||'   VAR "STAFTest=Machine-Var"'||crlf||"END"||crlf

  do until rc = 1
    gwlfile = "GW"||time('s')||random(1,9)||".gwl"
    rc = FileStatus(gwlfile, 10, "write")
  end

  rc = charout(gwlfile, GW_data, 1)
  if rc = 0 then call stream gwlfile, "c", "close"
return

GENWLRun:
  call autogenwl 0 gwlfile "start"
  call autogenwl 0 gwlfile "query"
  call autogenwl 0 gwlfile "stop"
  call autogenwl 0 gwlfile "query"
  call autogenwl 0 gwlfile "free"
  call autogenwl 0 gwlfile "query"
  call autogenwl 1 "FILE"||time('s') "query"
return

autogenwl:
  parse arg AG_errorok AG_file AG_opt
  if loud then do
    say "..............................................................................."
    say "Service=GENWL  Where="where " ExpectedRC="AG_errorok " TotalErrors="error
    say "LoopNum="loop " Handle="STAFHandle " Started="initiated " Elapsed="time('e')
    say "Request="AG_opt
  end
  call genwl AG_file "-"||AG_opt "-usehandle" STAFHandle
  if RESULT = AG_errorok then totok = totok + 1
  else do
    if loud then do
      say "###############################################################################"
      say "RC="||RC "Result="||Result
      say "###############################################################################"
    end
    errortxt = errortxt||time('l')||"|"||STAFHandle||"|"||where||"|GENWL|"||AG_opt||"|"||RC||"|"||Result||crlf
    error = error + 1
    if cleanup = 0 & error >= maxerror then call STAFAbort
  end
return

CleanUp:
  cleanup = 1
  say "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  say "Clean Up Time"
  say "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  if external then call autodoit 0 where dc "LOG" "delete machine" origmachine "handle" STAFHandle "logname staftest confirm"
  if internal then do
    call autodoit 0 where dc "VAR" "delete system var For"||testcase "delete Ever"||testcase||""
    if genwl then '@del' gwlfile '>nul 2>nul'
  end
  call STAFUnRegister
  if error > 0 & loud then do
    say "###############################################################################"
    say "Error Count =" error
    say "###############################################################################"
    say "Errors (Timestamp|Handle|Where|Service|Request|STAFRC|STAFResult):" errortxt
    say "###############################################################################"
    say "Error Count =" error
    say "###############################################################################"
  end
return

EndIt:
  say "*******************************************************************************"
  say loop-1 "loops run. " totok "total test iterations run."
  say "A total of" error "errors encountered."
  say "*******************************************************************************"
return

/******************************************************************************/
/* From here on are imported functions, DO NOT CHANGE ANYTHING BELOW HERE     */
/******************************************************************************/
/******************************************************************************/
/* STAFWrapData - Wraps a REXX String using colon delimited STAF format       */
/* Arguments: REXX string                                                     */
/* Returns  : STAF formatted string                                           */
/******************************************************************************/
STAFWrapData: PROCEDURE
  parse arg data
  RETURN ":"LENGTH(data)":"data
/* End of STAFWrapData */
/******************************************************************************/
/* MakeSTAFResult - Creates REXX STAFResult string                            */
/* Arguments: The return code                                                 */
/*            The result string                                               */
/* Returns  : STAFResult string                                               */
/******************************************************************************/
MakeSTAFResult: PROCEDURE
  parse arg theRC, resultString
  RETURN REVERSE(D2C(theRC, 4)) || resultString
/* End of MakeSTAFResult */
/******************************************************************************/
/* STAFErrorText - STAF defined error codes and text.                         */
/* Arguments: none                                                            */
/* Returns  : 0                                                               */
/******************************************************************************/
STAFErrorText: PROCEDURE EXPOSE STAFError.

  STAFError.!Ok = 0
  STAFError.!Ok.!Text = "No Error"
  STAFError.!InvalidAPI = 1
  STAFError.!InvalidAPI.!Text = "Invalid API"
  STAFError.!UnknownService = 2
  STAFError.!UnknownService.!Text = "Unknown Service"
  STAFError.!InvalidHandle = 3
  STAFError.!InvalidHandle.!Text = "Invalid Handle"
  STAFError.!HandleAlreadyExists = 4
  STAFError.!HandleAlreadyExists.!Text = "Handle Already Exists"
  STAFError.!HandleDoesNotExist = 5
  STAFError.!HandleDoesNotExist.!Text = "Handle Does Not Exist"
  STAFError.!UnknownError = 6
  STAFError.!UnknownError.!Text = "Unknown Error"
  STAFError.!InvalidRequestString = 7
  STAFError.!InvalidRequestString.!Text = "Invalid Request String"
  STAFError.!InvalidServiceResult = 8
  STAFError.!InvalidServiceResult.!Text = "Invalid Service Result"
  STAFError.!REXXError = 9
  STAFError.!REXXError.!Text = "Rexx Error"
  STAFError.!BaseOSError = 10
  STAFError.!BaseOSError.!Text = "Base OS Error"
  STAFError.!ProcessAlreadyComplete = 11
  STAFError.!ProcessAlreadyComplete.!Text = "Process Already Complete"
  STAFError.!ProcessNotComplete = 12
  STAFError.!ProcessNotComplete.!Text = "Process Not Complete"
  STAFError.!VariableDoesNotExist = 13
  STAFError.!VariableDoesNotExist.!Text = "Variable Does Not Exist"
  STAFError.!UnResolvableString = 14
  STAFError.!UnResolvableString.!Text = "UnResolvable String"
  STAFError.!InvalidResolveString = 15
  STAFError.!InvalidResolveString.!Text = "Invalid Resolve String"
  STAFError.!NoPathToMachine = 16
  STAFError.!NoPathToMachine.!Text = "No Path To Machine"
  STAFError.!FileOpenError = 17
  STAFError.!FileOpenError.!Text = "File Open Error"
  STAFError.!FileReadError = 18
  STAFError.!FileReadError.!Text = "File Read Error"
  STAFError.!FileWriteError = 19
  STAFError.!FileWriteError.!Text = "File Write Error"
  STAFError.!FileDeleteError = 20
  STAFError.!FileDeleteError.!Text = "File Delete Error"
  STAFError.!STAFNotRunning = 21
  STAFError.!STAFNotRunning.!Text = "STAF Not Running"
  STAFError.!CommunicationError = 22
  STAFError.!CommunicationError.!Text = "Communication Error"
  STAFError.!TrusteeDoesNotExist = 23
  STAFError.!TrusteeDoesNotExist.!Text = "Trustee Does Not Exist"
  STAFError.!InvalidTrustLevel = 24
  STAFError.!InvalidTrustLevel.!Text = "Invalid Trust Level"
  STAFError.!AccessDenied = 25
  STAFError.!AccessDenied.!Text = "Access Denied"
  STAFError.!STAFRegistrationError = 26
  STAFError.!STAFRegistrationError.!Text = "STAF Registration Error"
  STAFError.!ServiceConfigurationError = 27
  STAFError.!ServiceConfigurationError.!Text = "Service Configuration Error"
  STAFError.!QueueFull = 28
  STAFError.!QueueFull.!Text = "Queue Full"
  STAFError.!NoQueueElement = 29
  STAFError.!NoQueueElement.!Text = "No Queue Element"
  STAFError.!NotifieeDoesNotExist = 30
  STAFError.!NotifieeDoesNotExist.!Text = "Notifiee Does Not Exist"
  STAFError.!InvalidAPILevel = 31
  STAFError.!InvalidAPILevel.!Text = "Invalid API Level"
  STAFError.!ServiceNotUnregisterable = 32
  STAFError.!ServiceNotUnregisterable.!Text = "Service Not Unregisterable"
  STAFError.!ServiceNotAvailable = 33
  STAFError.!ServiceNotAvailable.!Text = "Service Not Available"
  STAFError.!SemaphoreDoesNotExist = 34
  STAFError.!SemaphoreDoesNotExist.!Text = "Semaphore Does Not Exist"
  STAFError.!NotSemaphoreOwner = 35
  STAFError.!NotSemaphoreOwner.!Text = "Not Semaphore Owner"
  STAFError.!SemaphoreHasPendingRequests = 36
  STAFError.!SemaphoreHasPendingRequests.!Text = "Semaphore Has Pending Requests"
  STAFError.!Timeout = 37
  STAFError.!Timeout.!Text = "Timeout"
  STAFError.!JavaError = 38
  STAFError.!JavaError.!Text = "Java Error"
  STAFError.!ConverterError = 39
  STAFError.!ConverterError.!Text = "Converter Error"
  STAFError.!TotalCodes = 39
  STAFError.!UserStartCodes = 4000

  RETURN 0

/* End of STAFErrorText */
InitParser:

    parse arg IP_ParserName

    call SetCurrentParser IP_ParserName

    OpString_Option.OpString_CurrentParser.0 = 0
    OpString_OptionGroup.OpString_CurrentParser.0 = 0
    OpString_OptionNeed.OpString_CurrentParser.0 = 0
    OpString_MaxArguments.OpString_CurrentParser = 0

    RETURN 0

/* End of InitOptions */

SetCurrentParser:

    parse arg OpString_CurrentParser

    if OpString_CurrentParser = "" then
        OpString_CurrentParser = "OPSTRING_DEFAULT"
    else OpString_CurrentParser = "OPSTRING_"OpString_CurrentParser

    OpString_CurrentParser = TRANSLATE(OpString_CurrentParser)

    RETURN 0

/* End of SetCurrentParser */


SetMaxArguments:

    parse arg SMA_NumArgs

    OpString_MaxArguments.OpString_CurrentParser = SMA_NumArgs

    RETURN 0

/* End of SetMaxArguments */
AddOption:

    parse arg AO_Name, AO_Times, AO_Required

    OpString_Option.OpString_CurrentParser.0 =,
        OpString_Option.OpString_CurrentParser.0 + 1

    AO_Index = OpString_Option.OpString_CurrentParser.0
    OpString_Option.OpString_CurrentParser.AO_Index.!Name = AO_Name
    OpString_Option.OpString_CurrentParser.AO_Index.!Times = AO_Times
    OpString_Option.OpString_CurrentParser.AO_Index.!ValueRequired =,
        TRANSLATE(AO_Required)

    RETURN 0

/* End of AddOption */

AddOptionGroup:

    parse arg AOG_Group, AOG_Minimum, AOG_Maximum

    OpString_OptionGroup.OpString_CurrentParser.0 =,
        OpString_OptionGroup.OpString_CurrentParser.0 + 1

    AOG_Index = OpString_OptionGroup.OpString_CurrentParser.0
    OpString_OptionGroup.OpString_CurrentParser.AOG_Index.!Group = AOG_Group
    OpString_OptionGroup.OpString_CurrentParser.AOG_Index.!Minimum =,
        AOG_Minimum
    OpString_OptionGroup.OpString_CurrentParser.AOG_Index.!Maximum =,
        AOG_Maximum

    RETURN 0

/* End of AddOptionGroup */

AddOptionNeed:

    parse arg AON_Needer, AON_Needee

    OpString_OptionNeed.OpString_CurrentParser.0 =,
        OpString_OptionNeed.OpString_CurrentParser.0 + 1

    AON_Index = OpString_OptionNeed.OpString_CurrentParser.0
    OpString_OptionNeed.OpString_CurrentParser.AON_Index.!Needer = AON_Needer
    OpString_OptionNeed.OpString_CurrentParser.AON_Index.!Needee = AON_Needee

    RETURN 0

/* End of AddOptionGroup */

NumInstances:

    RETURN OpString_Instance.OpString_CurrentParser.0

/* End of NumInstances */

InstanceName:

    parse arg IN_Num

    RETURN OpString_Instance.OpString_CurrentParser.IN_Num.!Name

/* End of InstanceName */

InstanceValue:

    parse arg IN_Num

    RETURN OpString_Instance.OpString_CurrentParser.IN_Num.!Value

/* End of InstanceValue */

NumArguments:

    RETURN OpString_Arg.OpString_CurrentParser.0

/* End of NumArguments */

Argument:

    parse arg ARG_Num

    RETURN OpString_Arg.OpString_CurrentParser.ARG_Num

/* End of Argument */

/***********************************************************************/
/* OptionTimes - Determins how many times a given option was specified */
/*                                                                     */
/*   Accepts: The name of the option                                   */
/*   Returns: The number of times the option was specified.            */
/***********************************************************************/
OptionTimes:

    parse arg OT_Name

    OT_Times = 0

    do OT_i = 1 to OpString_Instance.OpString_CurrentParser.0
        if TRANSLATE(OpString_Instance.OpString_CurrentParser.OT_i.!Name) =,
           TRANSLATE(OT_Name) then
        do
            OT_Times = OT_Times + 1
        end
    end

    RETURN OT_Times

/************************************************************************/
/* OptionValue - Determines the value of the specified occurance of the */
/*               specified option                                       */
/*                                                                      */
/*   Accepts: The name of the option to determine the value of          */
/*            Which occurance of the option to look for                 */
/*   Returns: The value of the specified occurance of the given option. */
/*            Returns '' if the specified occurance does not exist (and */
/*              thus was not given on the command line)                 */
/************************************************************************/
OptionValue:

    parse arg OV_Name, OV_Occurance

    if OV_Occurance = '' then OV_Occurance = 1

    OV_Times = 0

    do OV_i = 1 to OpString_Instance.OpString_CurrentParser.0,
    UNTIL (OV_Times = OV_Occurance)
        if TRANSLATE(OpString_Instance.OpString_CurrentParser.OV_i.!Name) =,
           TRANSLATE(OV_Name) then
        do
            OV_Times = OV_Times + 1
        end
    end

    if OV_Times \= OV_Occurance then RETURN ''

    RETURN OpString_Instance.OpString_CurrentParser.OV_i.!Value

/* --- Internals ---                                                   */
/* OpString_Option.OpString_CurrentParser.0                            */
/* OpString_Option.OpString_CurrentParser.i.!Name                      */
/* OpString_Option.OpString_CurrentParser.i.!Times  - 0=unlimited      */
/* OpString_Option.OpString_CurrentParser.i.!ValueRequired  - Yes, No, */
/*                                                            Allowed  */
/* OpString_OptionGroup.OpString_CurrentParser.0                       */
/* OpString_OptionGroup.OpString_CurrentParser.i.!Group                */
/* OpString_OptionGroup.OpString_CurrentParser.i.!Minimum              */
/* OpString_OptionGroup.OpString_CurrentParser.i.!Maximum              */
/*                                                                     */
/* OpString_OptionNeed.OpString_CurrentParser.0                        */
/* OpString_OptionNeed.OpString_CurrentParser.i.!Needer                */
/* OpString_OptionNeed.OpString_CurrentParser.i.!Needee                */
/*                                                                     */
/* OpString_MaxArguments.OpString_CurrentParser                        */
/*                                                                     */
/* PS_Word.0                                                           */
/* PS_Word.i.!Data                                                     */
/* PS_Word.i.!Type - Option, Value                                     */
/*                                                                     */
/* OpString_Instance.OpString_CurrentParser.0                          */
/* OpString_Instance.OpString_CurrentParser.i.!Name                    */
/* OpString_Instance.OpString_CurrentParser.i.!Value                   */
/*                                                                     */
/* OpString_Arg.OpString_CurrentParser.0                               */
/* OpString_Arg.OpString_CurrentParser.i                               */

ParseString:

    parse arg PS_ParseString, PS_ErrorBuffer

    PS_WhiteSpace = '0D0A20'x   /* WhiteSpace = CR LF Space */
    PS_Word.0 = 0
    PS_CurrType = "Value"
    PS_CurrData = ""
    PS_InQuotes = 0
    PS_InEscape = 0
    PS_IsLiteral = 0
    PS_InLengthField = 0
    PS_InDataField = 0
    PS_DataLength = 0
    OpString_Arg.OpString_CurrentParser.0 = 0
    OpString_Instance.OpString_CurrentParser.0 = 0

    do PS_i = 1 to LENGTH(PS_ParseString)

       PS_Char = SUBSTR(PS_ParseString, PS_i, 1)

       if (PS_Char = ':') & (PS_InQuotes = 0) & (PS_InEscape = 0) &,
          (PS_InDataField = 0) & (PS_CurrData = "") then
       do
           PS_InLengthField = 1
       end
       else if PS_InLengthField then
       do
            if PS_Char = ':' then
            do
                PS_InLengthField = 0
                PS_InDataField = 1

                if PS_CurrData = "" then
                do
                    call VALUE PS_ErrorBuffer, "Invalid length delimited",
                                               "data specifier"
                    RETURN 1
                end

                PS_DataLength = PS_CurrData
                PS_CurrType = "Value"
                PS_CurrData = ""
            end
            else if VERIFY(PS_Char, "0123456789") = 0 then
            do
                PS_CurrData = PS_CurrData || PS_Char
            end
            else
            do
                call VALUE PS_ErrorBuffer, "Invalid length delimited",
                                           "data specifier"
                RETURN 1
            end

        end  /* In length field */
        else if PS_InDataField then
        do
            PS_CurrData = PS_CurrData || PS_Char
            PS_DataLength = PS_DataLength - 1

            if PS_DataLength = 0 then
            do
                PS_Word.0 = PS_Word.0 + 1
                PS_WordIndex = PS_Word.0
                PS_Word.PS_WordIndex.!Type = PS_CurrType
                PS_Word.PS_WordIndex.!Data = PS_CurrData

                PS_CurrType = "Value"
                PS_CurrData = ""

                PS_InDataField = 0
            end
        end
        else if VERIFY(PS_Char, PS_WhiteSpace) = 0 then
        do
            PS_InEscape = 0

            if PS_InQuotes then PS_CurrData = PS_CurrData || PS_Char
            else if PS_CurrData \== "" then
            do
                if PS_IsLiteral then
                    PS_CurrType = "Value"
                else if ValueIsOption(PS_CurrData) \= 0 then
                    PS_CurrType = "Option"

                PS_Word.0 = PS_Word.0 + 1
                PS_WordIndex = PS_Word.0
                PS_Word.PS_WordIndex.!Type = PS_CurrType
                PS_Word.PS_WordIndex.!Data = PS_CurrData

                PS_CurrType = "Value"
                PS_CurrData = ""

                PS_IsLiteral = 0
            end
        end  /* if whitespace */
        else if PS_Char = '\' then
        do
            if PS_InQuotes & \PS_InEscape then PS_InEscape = 1
            else
            do
                PS_CurrData = PS_CurrData || PS_Char
                PS_InEscape = 0
            end
        end
        else if PS_Char = '"' then
        do
            if PS_InEscape then PS_CurrData = PS_CurrData || PS_Char
            else if PS_InQuotes & PS_CurrData \== "" then
            do
                if PS_IsLiteral then
                    PS_CurrType = "Value"
                else if ValueIsOption(PS_CurrData) \= 0 then
                    PS_CurrType = "Option"

                PS_Word.0 = PS_Word.0 + 1
                PS_WordIndex = PS_Word.0
                PS_Word.PS_WordIndex.!Type = PS_CurrType
                PS_Word.PS_WordIndex.!Data = PS_CurrData

                PS_CurrType = "Value"
                PS_CurrData = ""

                PS_InQuotes = 0
                PS_IsLiteral = 0
            end
            else
            do
                PS_InQuotes = 1
                PS_IsLiteral = 1
            end

            PS_InEscape = 0

        end  /* end if quote character */
        else
        do
            PS_InEscape = 0
            PS_CurrData = PS_CurrData || PS_Char
        end

    end  /* looping through parse string */

    if PS_InLengthField | PS_InDataField then
    do
        call VALUE PS_ErrorBuffer, "Invalid length delimited data specifier"
        RETURN 1
    end
    else if PS_CurrData \== "" then
    do
        if PS_IsLiteral then
            PS_CurrType = "Value"
        else if ValueIsOption(PS_CurrData) \= 0 then
            PS_CurrType = "Option"

        PS_Word.0 = PS_Word.0 + 1
        PS_WordIndex = PS_Word.0
        PS_Word.PS_WordIndex.!Type = PS_CurrType
        PS_Word.PS_WordIndex.!Data = PS_CurrData
    end

    /* Now walk the word list looking for options, etc. */

    PS_ValueRequirement = "NO"
    PS_CurrOption = ""
    PS_CurrValue = ""

    do PS_i = 1 to PS_Word.0

        PS_CurrWord = PS_Word.PS_i.!Data

        if PS_Word.PS_i.!Type = "Option" then
        do
            do PS_OptionIndex = 1 to OpString_Option.OpString_CurrentParser.0,
               UNTIL TRANSLATE(OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Name) =,
                     TRANSLATE(PS_CurrWord)
            end

            if PS_ValueRequirement = "YES" then
            do
                call VALUE PS_ErrorBuffer,  "Option," PS_CurrOption",",
                           "requires a value"
                RETURN 1
            end
            else if PS_ValueRequirement = "ALLOWED" then
            do
                OpString_Instance.OpString_CurrentParser.0 = OpString_Instance.OpString_CurrentParser.0 + 1
                OpString_InstanceIndex = OpString_Instance.OpString_CurrentParser.0
                OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Name = PS_CurrOption
                OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Value = PS_CurrValue
            end

            /* Check once here for whether this new option instance will */
            /* exceed the limit for this option                          */

            if (OptionTimes(OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Name) =,
                OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Times) &,
               (OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Times \= 0) then
            do
                call VALUE PS_ErrorBuffer, "You may have no more than",
                     OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Times,
                     "instance(s) of option",
                     OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Name
                RETURN 1
            end

            PS_CurrOption = OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Name
            PS_CurrValue = ""
            PS_ValueRequirement =,
                TRANSLATE(OpString_Option.OpString_CurrentParser.PS_OptionIndex.!ValueRequired)

            if PS_ValueRequirement = "NO" then
            do
                OpString_Instance.OpString_CurrentParser.0 = OpString_Instance.OpString_CurrentParser.0 + 1
                OpString_InstanceIndex = OpString_Instance.OpString_CurrentParser.0
                OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Name = PS_CurrOption
                OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Value = PS_CurrValue

                PS_CurrOption = ""
                PS_CurrValue = ""
            end
        end
        else if PS_ValueRequirement = "NO" then
        do
            OpString_Arg.OpString_CurrentParser.0 = OpString_Arg.OpString_CurrentParser.0 + 1
            OpString_ArgIndex = OpString_Arg.OpString_CurrentParser.0
            OpString_Arg.OpString_CurrentParser.OpString_ArgIndex = PS_CurrWord
        end
        else
        do
            PS_CurrValue = PS_CurrWord

            OpString_Instance.OpString_CurrentParser.0 = OpString_Instance.OpString_CurrentParser.0 + 1
            OpString_InstanceIndex = OpString_Instance.OpString_CurrentParser.0
            OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Name = PS_CurrOption
            OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Value = PS_CurrValue

            PS_CurrOption = ""
            PS_CurrValue = ""

            PS_ValueRequirement = "NO"
        end

    end  /* end for each word */

    /* If the last word was an option, we need to check for its value */
    /* requirements here                                              */

    if PS_ValueRequirement = "YES" then
    do
        call VALUE PS_ErrorBuffer, "Option,",
             OpString_Option.OpString_CurrentParser.PS_OptionIndex.!Name",",
             "requires a value"
        RETURN 1
    end
    else if PS_ValueRequirement = "ALLOWED" then
    do
        OpString_Instance.OpString_CurrentParser.0 = OpString_Instance.OpString_CurrentParser.0 + 1
        OpString_InstanceIndex = OpString_Instance.OpString_CurrentParser.0
        OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Name = PS_CurrOption
        OpString_Instance.OpString_CurrentParser.OpString_InstanceIndex.!Value = PS_CurrValue
    end


    /* Check the restriction on number of arguments */

    if OpString_Arg.OpString_CurrentParser.0 >,
       OpString_MaxArguments.OpString_CurrentParser then
    do
        PS_InvalidArgIndex = OpString_MaxArguments.OpString_CurrentParser + 1

        call VALUE PS_ErrorBuffer, "You may have no more than",
                   OpString_MaxArguments.OpString_CurrentParser "argument(s).",
                   "  You specified" OpString_Arg.OpString_CurrentParser.0,
                   "argument(s).  The first excess argument is, ",
                   OpString_Arg.OpString_CurrentParser.PS_InvalidArgIndex"."
        RETURN 1
    end


    /* Now check all the group requirements */

    do PS_i = 1 to OpString_OptionGroup.OpString_CurrentParser.0

        PS_GroupCount = 0;
        PS_GroupWordCount =,
            WORDS(OpString_OptionGroup.OpString_CurrentParser.PS_i.!Group)

        do PS_j = 1 to PS_GroupWordCount
            if OptionTimes(WORD(OpString_OptionGroup.OpString_CurrentParser.PS_i.!Group, PS_j)) \= 0 then
                PS_GroupCount = PS_GroupCount + 1
        end

        if (PS_GroupCount <,
            OpString_OptionGroup.OpString_CurrentParser.PS_i.!Minimum) |,
           (PS_GroupCount >,
            OpString_OptionGroup.OpString_CurrentParser.PS_i.!Maximum) then
        do
            call VALUE PS_ErrorBuffer, "You must have at least",
                 OpString_OptionGroup.OpString_CurrentParser.PS_I.!Minimum",",
                 "but no more than",
                 OpString_OptionGroup.OpString_CurrentParser.PS_I.!Maximum,
                 "of the option(s),",
                 OpString_OptionGroup.OpString_CurrentParser.PS_I.!Group
            RETURN 1
        end

    end  /* do for each group */


    /* Now check the need requirements */

    do PS_i = 1 to OpString_OptionNeed.OpString_CurrentParser.0

        PS_FoundNeeder = 0
        PS_FoundNeedee = 0

        do PS_j = 1 to WORDS(OpString_OptionNeed.OpString_CurrentParser.PS_I.!Needer),
        while PS_FoundNeeder = 0
            if (OptionTimes(WORD(OpString_OptionNeed.OpString_CurrentParser.PS_i.!Needer, PS_j)) \= 0) then
                PS_FoundNeeder = 1
        end

        do PS_j = 1 to WORDS(OpString_OptionNeed.OpString_CurrentParser.PS_I.!Needee),
        while PS_FoundNeedee = 0
            if (OptionTimes(WORD(OpString_OptionNeed.OpString_CurrentParser.PS_i.!Needee, PS_j)) \= 0) then
                PS_FoundNeedee = 1
        end

        if (PS_FoundNeeder & \PS_FoundNeedee) then
        do
            call VALUE PS_ErrorBuffer, "When specifying one of the options",
                 OpString_OptionNeed.OpString_CurrentParser.PS_i.!Needer",",
                 "you must also specify one of the options",
                 OpString_OptionNeed.OpString_CurrentParser.PS_i.!Needee
            RETURN 1
        end

    end  /* do for each need */

    RETURN 0

/* End of ParseString */

ValueIsOption:

    parse arg VIO_Value

    do VIO_i = 1 to OpString_Option.OpString_CurrentParser.0
        if TRANSLATE(OpString_Option.OpString_CurrentParser.VIO_i.!Name) =,
           TRANSLATE(VIO_Value) then
        do
            RETURN VIO_i
        end
    end

    RETURN 0

/* End of ValueIsOption */


FileStatus:
  parse arg FS_file, FS_maxwait, FS_openfor

  FS_status = ''
  FS_rc = 0

  FS_openfor = translate(FS_openfor)
  if FS_openfor = "READ" then FS_openfor = "open read"
  else
    if FS_openfor = "WRITE" then FS_openfor = "open write"
    else FS_openfor = "open"

  call SysFileTree FS_file, FS_stem, 'F'              /* Check if file exists */
  if FS_stem.0 = 0 then do                                  /* File not found */
    FS_rc = 1
    errorBuffer = STAFError.!FileOpenError.!text || lineSep || FS_file
  end
  else                                     /* File exists, see if it is READY */
  do
    do FS_maxwait                       /* Repeat maxwait times or file READY */
      FS_status = stream(FS_file, "c", FS_openfor)
      if FS_status = "READY:" then leave                        /* File READY */
      call SysSleep 1
    end
    if FS_status \= "READY:" then
    do
      FS_rc = 2
      errorBuffer = "File not READY: " || lineSep || FS_file
    end
  end

RETURN FS_rc
