<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">

<html>

<head>
  <title>Software Testing Automation Framework (STAF)</title>
</head>

<body>

<?php
require "top.php";
require "navigate.php";
?>

<!-- Insert text for page here -->

<tr>
<td>

<center><h1>STAX History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for STAX  
  
  Legend:
   - Fixes
   + Features           
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 1.5.10: 04/18/2006

  - Fixed problem calling a function like has_key() directly on a STAXGlobal
    instance without having to call it's get() method first (Bug #1408579)

-------------------------------------------------------------------------------

Version 1.5.9: 12/07/2005

  - Fixed problem where the Monitored green ball icon was sometimes not
    displaying correctly in the STAX Monitor (Bug #1262534)
  - Fixed NullPointerException when listing threads or querying threads for a
    job that doesn't have any threads running (Bug #1329497)
  - Fixed NullPointerException that can occur in an error situation when
    running the STAX Monitor (Bug #1330198)

-------------------------------------------------------------------------------

Version 1.5.8: 08/15/2005

  - Fixed problem in &lt;loop> where the until expression was being evaluated at
    the top of each loop instead of at the bottom of each loop (Bug #1196936)
  - Added STAX Extensions File DTD to the STAX zip/jar file (Bug #1216745)
  - Fixed StringIndexOutOfBoundsException that occurs in STAX Monitor window
    if STAF is shutdown without first stopping the STAX Monitor (Bug #1029520)
  - Fixed some typos in STAX User's Guide for &lt;process> example (Bug #1238388)
  = Changed to not use enum as a Java variable name so can compile using 
    Java 5.0 since enum is now a Java keyword (Bug #1241613)
  - Fixed problem in STAXUtil.xml's STAFProcessUsing function to handle
    redirecting stderr to stdout corrrectly (Bug #1247056)
  - Fixed problem where multi-line Job results were not displayed correctly in
    the STAX Monitor (Bug #1256760)
  - Fixed problem where the job result may not be None if an error occurs 
    evaluating the value for a &lt;return> element (Bug #1257026)

-------------------------------------------------------------------------------

Version 1.5.7: 05/05/2005

  - Fixed problem in STAXUtilExportsSTAFVars function handling variables 
    containing double quotes (Bug #1114317)
  - Use default STAX Monitor properties if the monprp.ser file is corrupted
    (Bug #1118227)
  = Changed to use Xerces2-J 2.6.2 as the XML parser (Feature #1045560)
  = Removed the zxJDBC code from our distribution of Jython (Bug #1118221)
  - Changed resolve variable method to check for occurence of "{" anywhere
    in a string instead of just in the first position (Bug #1151440)
  - Fixed so that STAXPythonEvaluationErrors for 'STAXPyEvalResult =' no longer
    occur (Bug #1158649)
  - Changed license from GPL to CPL for all source code (Bug #1149491)
  - Changed to handle STAXGlobal list objects in the iterate/paralleliterate
    elements without specifying the get() method (Bug #1156045)
  - Changed to use Monospaced font for text areas in the STAX Monitor where
    Python code is specified (Bug #1163364)
  - Fixed problem using STAX Monitor's Job Wizard where if you enter more than
    one line of text for an argument, lines 1-n are not saved (Bug #1164120)
  - Fixed problem resubmitting a job via STAX Monitor if select "Local machine"
    for the Script File Machine (Bug #1164868)
  - Fixed STAX Monitor Job Wizard problems with saved function arguments
    (Bug #1190449)
  - Document that the STAX Service requires Java 1.4 or later (Bug #1192762)
  - Fixed typo in the STAX User's Guide for a &lt;job> example (Bug #1195335)      
  
-------------------------------------------------------------------------------

Version 1.5.6: 01/26/2005

  - Fixed job hang problem that could occur if a Python evaluation error occurs
    evaluating a list such as iterate's "in" attribute (Bug #1075469)
  - Fixed race condition in call element (Bug #1073774)
  - Fixed NullPointerException if STAX VERSION request fails due to problem
    accessing the STAX service machine (Bug #1105669)
  - Fixed STAX Monitor Job Wizard errors in generating function argument values
    (Bug #1042460)
  + Provided two new Python variables, STAXJobStartFunctionName and
    STAXJobStartFunctionArgs (Feature #1110050)
  
-------------------------------------------------------------------------------

Version 1.5.5.1: 11/06/2004

  - Retry querying STAX logs without the ALL option to avoid an error if the 
    STAX service is on a STAF V2.6.5 or earlier machine (Feature #1040232)

-------------------------------------------------------------------------------

Version 1.5.5: 11/05/2004

  - Fixed typo to specify function-no-args, not function-no-arg, in STAX User's
    Guide (Bug #1003544)
  - Fixed typo for loop example in STAX User's Guide (Bug #1004127)
  - Fixed STAXThread::pyListEval() to handle single item better (Bug #1017855)
  - Fixed so that a command parsing error on an EXECUTE request returns RC 7,
    "Invalid Request String", instead of RC 1, "Invalid API" (Bug #1020590).
  - Fixed location substitution for 'local' for a stafcmd when it's details
    are displayed in the STAX Monitor (Bug #1033015)
  - Fixed problem iterating Java list objects (Bug #1043148)
  + Added condition stack to output when querying a STAX thread for help in
    debugging problems (Feature #1054755)  
  + Changed to specify the ALL option when querying a STAX log in order to get
    all the records that meet the query criteria in case the LOG service limits
    the default maximum records returned on a QUERY request (Feature #1040232)
  - Fixed problem re-selecting rows in the STAX Monitor Active Processes and
    Active STAFCmds tables (Bug #1061183)

-------------------------------------------------------------------------------

Version 1.5.4: 07/21/2004

  + Added un-register of the service handle during term() (Feature #966079)
  - Fixed NumberFormatException in STAXTestcase.addElapsedTime (Bug #974925)
  + Added a STAXJobScriptFiles variable (Feature #977071)
  - Fixed "TypeError: call of non-function" problem where a job would log this
    error and hang if a variable named "type" was assigned in a job since the
    STAX service uses the Python built-in function named "type" (Bug #981435)
  - Fixed memory leak running a sub-job (Bug #981548)  
  - Changed to use upper-case STAX service name when setting the STAXJobUserLog
    variable and in the STAX Monitor (Bug #982163)
  + Added new Python variables for the STAX service name/machine and the
    Event service name/machine (Feature #982109)
  + Made some usability enhancements when setting STAX Monitor properties:
    - Automatically update Event machine/service based on the specified STAX
      service's Event settings
      Note:  Can no longer edit Event machine/service property fields
    - Allow local to be specified for the STAX Service machine
    (Feature #987502)
  - Fixed problem where &lt;defaultcall> passed None to the function if no
    arguments were specified (Bug #991804)

-------------------------------------------------------------------------------

Version 1.5.3: 05/26/2004

  + Updated to use XML Parser for Java (Xerces) version 1.3.0 (Feature #931256)
  + Updated to record more info from exceptions when parsing (Feature #931415)
  - Fixed problem where job would hang if called a function passing too many
    map/list arguments and if an extra argument is a STAXGlobal (Bug #945541)
  - Clarified STAXGlobal variable scope in STAX User's Guide (Bug #950525)
  - Fixed memory leak in the STAX service (Bug #958312)
  + Added new arguments when starting the STAX Monitor to configure its
    properties (Feature #909254)
  - Fixed problem where STAX Monitor was using wrong SCRIPTFILEMACHINE for
    local if XML Job File machine was not the local machine (Bug #803485)
  - Fixed problem where "global" variables are not visible to functions
    defined in one STAXPythonInterpreter when the function is called from a
    cloned STAXPythonInterpreter (Bug #960415)

-------------------------------------------------------------------------------

Version 1.5.2.1: 04/07/2004

  - Types for function list and map arguments not retained (Bug #931201)

-------------------------------------------------------------------------------

Version 1.5.2: 04/01/2004

  - Synchronize the Process/STAFCmd number and Process key increment methods
    (Bug #913141)
  - Fix NullPointerException closing the STAX Monitor (Bug #914357)
  + Add import of STAFResult class as STAFRC to make it's constant definitions
    for all the STAF return codes available to STAX jobs (Feature #915820)
  - Don't return a STAXInvalidStartFunction error when using the TEST option
    (but not a starting FUNCTION option) for a xml job file that does not
    contain a &lt;defaultcall> element (Bug #922757)
  - Fixed STAX Monitor Job Wizard problem when the XML file path contains
    spaces (Bug #922989)
  - Fixed problem where RC 7 was not being returned when using the PROCESS KEY
    option for &lt;process> elements on pre-V2.6.0 STAF machines (Bug #923350)

-------------------------------------------------------------------------------

Version 1.5.1: 03/03/2004

  - Fixed problem displaying job logs from main STAX Monitor window if STAX
    service name is not STAX (Bug #891895).
  - Fixed StringIndexOutOfBoundsExceptions in STAX Monitor which could occur
    monitoring a job if the STAX service name is not STAX (Bug #891970)
  - Fixed errors deleting temporary files/directory for extension jar files
    when starting the STAX Monitor (Bug #895302)
  + Made the STAX Monitor STAX and Event hostname text entry fields larger
    (Feature #837327)
  - Fixed STAX Monitor extension "leakage" problem where extension information
    for other jobs was showing up in the last job monitored (Bug #900523)
  + Instrumented the STAX servicee's handling of EXECUTE WAIT RETURNRESULT,
    LIST, and QUERY, SET requests to record diagnostics data to help prepare
    for the migration to STAF V3.0.  The STAX service requires STAF V2.6.0 or
    later as a result of this change. (Feature #853620)  
  + Updated the STAX &lt;process> element to use the notify key when submitting
    Process Start commands (Feature #626917)

-------------------------------------------------------------------------------

Version 1.5.0: 01/26/2004

  - Fixed problem in the STAXUtilImportSTAFVars function in STAXUtil.xml where
    it would fail if only one STAF variable was specified (Bug #816775)
  - Fixed error where STAX Monitor would not display log entries for levels
    Debug2/3, Trace 2/3, and User 1/8 (Bug #816882)
  - Fixed problem where the STAXMonitor's Current Selection tab displayed an
    extraneous vertical scrollbar (Bug #807198)
  + Added pyMapEval function (Feature #821394)
  - Fixed problem where STAX Monitor's Testcase Info table was not displaying
    testcases with a strict mode and 0 passes/fails (Bug #821471)
  - Added CLEARLOGS option to STAX QUERY JOB output (Bug #821926)
  - Fixed RC 7 trying to re-submit STAX job to run via STAX Monitor
    (Bug #817339)
  - Fixed problem where the entire STAX Monitor Testcase Information was not
    being displayed (Bug #825284)
  - Display stack trace when STAX Monitor extensions throw an 
    InvocationTargetException (Bug #825649)
  + Added support for logging the elapsed time and/or the number of starts for
    testcases and provided this info in the STAX Monitor.  Also, added support
    for logging testcase starts and stops in the STAX Job log.
    Note that the default settings for the STAX service log options are:
      Clear logs         : Disabled
      Log TC Elapsed Time: Enabled
      Log TC Num Starts  : Enabled
      Log TC Start/Stop  : Disabled
    These default settings can be changed when registering the STAX service or
    via the new SET option, or overridden by job when submitting a STAX job 
    for execution. (Feature #795402)
  - Fixed problem where STAX logs could not be displayed in the STAX Monitor
    if the service name was not "STAX" (Bug #836579)
  + Made the STAX Monitor STAX and Event hostname text entry fields larger
    (Feature #837327)
  + Added versioning support for STAX extensions including:  
    - Listing/querying STAX extensions, their versions, and other information
    - Support for a new EXTENSIONXMLFILE parameter to specify STAX extensions
      via XML when configuring the STAX service, including the ability to
      specify parameters for STAX extensions.
    - Ability to specify the version and description for a STAX extension
      via the manifest for the extension jar file  
    - Ability to specify that a STAX extension requires a particular STAX
      service and/or STAX Monitor version. (Feature #818693)
  + Enhanced STAX extension samples.  For example, modified the delay
    extension to demonstrate how to process parameters passed to extensions.
    (Feature #846091)
  + Provide a STAX Extensions Developer's Guide and document how to register
    STAX service/monitor extensions in the STAX User's Guide (Feature #846103)
  - Fixed Extensions tab colors on the Monitor Properties panel (Bug #852724)
  - Added a new section to the STAX User's Guide, Appendix G: Jython and
    CPython Differences (Bug #853490)
  - Fixed resource leak by closing BufferedReaders (Bug #853596)
  + Enhanced support for STAX Monitor Extensions including:
    - Automatically obtains any monitor extensions registered with the STAX
      service machine
    - Verifies that the required version of the STAX Monitor specified for
      each monitor extension is installed
    - Provides the ability to display information about registered monitor
      extensions via a new Properties->Extensions tab or via a new -extensions
      parameter when starting the STAX Monitor.
    - Renamed the old Properties->Extensions tab to Extension Jars to allow
      you to override or add local monitor extensions (Feature #853596)
  + Changed the EXECUTE request so that it now compiles all Python code in a
    STAX Job before running the job and if using the TEST option, so that all
    Python code compile errors are reported immediately (Feature #874173) 
  + Allow STAX Monitor Job and Job User log tables to be selectable, so they
    can be copied to the clipboard (Feature #845931)
  + Improved performance of the STAX service by caching compiled Python code
    (Feature #872627)
  + Register error messages for the STAX service with the HELP service
    (Feature #605788)
  + Added a STAX Monitor Job Wizard which guides the selection of functions 
    and specification of function arguments (Feature #826094)
  + Improved STAX Monitor CPU performance (Feature #879299)
  + Added STAX Monitor options to configure the Elapsed Time and Process 
    Monitor intervals (Feature #854416)
  - Fixed ArrayIndexOutOfBoundsException which can occur when items are 
    removed from the Active Processes, STAFCmds, or SubJobs tables
    (Bug #880229)
  - Fixed NullPointerException in the STAX Monitor when a service request 
    option ends with a semi-colon (Bug #880876)
  - Fixed repaint problem with STAX Monitor Info extensions (Bug #883873)
  - Fixed problem where STAX Monitor extensions to the Active Job Elements tree
    would not have the tree selection background color when selected
    (Bug #885150)
    
-------------------------------------------------------------------------------

Version 1.4.1: 09/29/2003

  - Fixed problem where if a process returned a negative RC, STAX was setting
    it to 0 instead.  Now if a negative RC is returned, the RC will be set to a
    large number, e.g. 4294967295 instead of -1 (Bug #752936)
  - Added more information about using &amp;lt; instead of &lt; in STAX xml files to
    the STAX User's Guide (Bug #777205)  
  - Added more error checking to the STAX Monitor's STAX Job Parameters 
    dialog (Bug #784441)
  + Added an option to automatically monitor sub-jobs in the STAX Monitor
    and added a new attribute, monitor, to the &lt;job> element (Feature #784463)
  + Added a new STAX Utilities function called STAXUtilImportSTAFConfigVars to
    STAXUtil.xml which extracts system information into a map (Feature #785025)
  - Fixed a race condition with the &lt;process> element where a 'quick' process
    could hang the STAX job (Bug #790966)
  - Fixed problem where STAX Monitor extensions could not update the Active
    Job Elements tree when monitoring an existing job (Bug #807527)
  - Increased flexibility in how the STAX Monitor finds the JSTAF.jar file
    when started via 'java -jar STAXMon.jar' (Bug #808980)
  - Fixed a function list/map argument type conversion problem (Bug #814783)  
  - Allow STAX Monitor extensions to have both ID and Name (Bug #815346)
        
-------------------------------------------------------------------------------

Version 1.4.0: 06/04/2003

  - Fixed problem where the STAX Monitor was only displaying Log files from
    the local machine (Bug #715990)
  - Fixed problem where raising a STAXImportError signal for a parser exception
    on a Chinese system caused a Java exception (Bug #723409)
  - Fixed problem where the STAX Monitor was not unregistering handles for
    Monitor extensions (Bug #731079)
  - Added more debugging info when a STAXPythonEvaluationError signal is
    raised (Bug #731650)  
  + Added an example of aliasing a STAX extension (Feature #733606)
  - Fixed STAX User Guide to refer to the STAXFunctionArgValidate signal
    rather than the STAXFunctionValidateArg signal (Bug #735830)
  + Added support for a &lt;job> element so that sub-jobs can be executed within
    a STAX job (Feature #606780)
  - Made sure all variables set by the process element (e.g. STAXResult,
    STAFResult, STAXHandleProcess) are initialized (Bug #745586)
  - Fixed hang problem that could occur when a process with a process-action
    element was terminated via a condition like a timer popping (Bug #745534)
  - Fixed race condition causing erroneous STAXProcessStartTimeout (Bug #745749)
  - Fixed problem with STAX Monitor not resetting the SCRIPTFILEMACHINE 
    option (Bug #744460)
  + Added support to return the result of a job's starting function via
    a new RETURNRESULT option for a EXECUTE WAIT request (Feature #745813)

-------------------------------------------------------------------------------

Version 1.3.3: 03/18/2003

  - Made EXECUTE options TEST, HOLD, and WAIT mutually exclusive (Bug #652822)
  - Fixed bug where the STAX Monitor was using case-sensitive checks of the 
    STAX machine name for incoming events (Bug #656113) 
  + Added support to limit the number of displayed Messages displayed in the
    STAX Monitor (Feature #657077)
  + Added EXECUTE option CLEARLOGS which will delete the existing STAX Job 
    and Job User logs before the current STAX job is executed (Feature #656121)
  - Fixed memory leak in STAXMonitor's STAFCmd nodes (Bug #660208)
  - Fixed memory leak in STAXMonitor's job start timestamp table (Bug #660534)
  - Fixed NullPointerException in STAXMonitorFrame when specifying a pre-1.3.2
    saved job parms file (Bug #661311)
  + Added a PROCESSTIMEOUT option to ensure that process start requests do
    not hang (Feature #667512) 
  + Added STAX Monitor support for the CLEARLOGS option (Feature #682127)
  - Updated STAX User Guide for typo in DTD and added references to the
    STAXGlobal class and another example (Bug #702113)
 
-------------------------------------------------------------------------------

Version 1.3.2: 12/10/2002

  - Fixed NullPointerException in STAXMonitorUtil.getElapsedTime (Bug #623768)
  + Added support for extensions in the STAX Monitor (Feature #605637)
  + Use tabbed pane in the STAXMonitor Start New Job panel (Feature #629113)
  + Use tabbed pane in the STAXMonitor Properties panel (Feature #629226)
  + Added SCRIPTFILE support to the STAX Job Monitor (Feature #629119)
  + Added verification in the STAX Job Monitor that Extension Jar files
    exist (Feature #631222) 
  - Fixed Monitor extensions incorrectly displaying other Active Job Elements 
    node text (Bug #639846)
  - Fixed synchronization problem on STAX job end (Bug #639150)
  - Fixed empty call problem where a STAXPythonEvaluationException signal was
    incorrectly being raised (Bug #638535)
  - Fixed NumberFormatException in ProcessAction (Bug #631300)
  + Provided some common STAX Utility functions in libraries/STAXUtil.xml and
    added new elements function-prolog and function-epilog to replace the
    now deprecated function-description element (Feature #641425)
  + Added support to more easily restart STAX jobs via the STAX Job Monitor
    (Feature #639010)
  + Added support for WAIT option on EXECUTE request (Feature #643626)
  - Fixed problem specifying more than one notification type for STAX Monitor 
    extensions (Bug #649300)
  + Added support to be able to log to the STAX Job Log from a &lt;script) element
    or other Python code using a new STAXJobUserLog variable (Feature #651190)
  - Fixed ArrayIndexOutOfBoundsException when viewing a STAX Job User Log via
    the STAX Monitor (Bug #651213)  
  - Fixed problem where STAX Monitor was not unregistering with the Event
    service if File->Exit was selected (Bug #652223)
  - Fixed NullPointerException in the STAX Monitor when terminating the job 
    via the Active Job Elements tree (Bug #652362)

-------------------------------------------------------------------------------

Version 1.3.1: 10/08/2002

  + Added a shell attribute to the process command element to override the
    default shell by process (Feature #619831)
  - Fixed incorrect STAFResult after starting a process (Bug #621073)

-------------------------------------------------------------------------------

Version 1.3.0.1: 09/25/2002

  - Fixed STAXXMLParseException: Can't find bundle for base name
    org.apache.xerces.impl.msg.XMLMessages, locale en_US (Bug #614659)
    
-------------------------------------------------------------------------------

Version 1.3.0: 09/24/2002

  - Fixed incorrect default value for STAXJarFile argument in sample1.xml 
    (Bug #604616)
  - Fixed problem where you couldn't cast STAX Extension types because they
    had been loaded by different class loaders (Bug #604759)
  + Added submit methods to STAXJob for extensions (Feature #607073)
  + Added support for the latest version of the XML Parser for Java (V4.0.5)
    (Feature #607057)
  - Fixed message logged for "No listener found for msg" warning (Bug #608063)
  + Updated STAX Monitor to use tabbed-panes for the various panels which
    allows you to select what is displayed via the View menu (Feature #605631)
  + Pass in a HashMap to STAXJob.generateEvent (Feature #608083)
  - Fixed synchronization problem for STAF commands submitted via submitAsync
    (Bug #613413)

-------------------------------------------------------------------------------

Version 1.2.1: 08/26/2002

  - Changed the makefile for STAX due to changes in the STAF Java service jar
    class loader (STAF Bug #597392).  Requires STAF 2.4.1 or later.
  - Fixed problem where STAXSTAFCommandAction was not generating a Stop event
    if the user terminated the job (Bug #1261)
  - Fixed RC 4002 when starting jobs via the STAX Monitor on high-end machines
    (Bug #1268)

-------------------------------------------------------------------------------

NOTE:  Support numbers and 4-digit bug numbers shown in the history for STAX do
       not correspond to SourceForge Bug and Feature numbers as STAX was not
       made available on SourceForge until Version 1.2.0.
       
Version 1.2.0: 08/05/2002

  - Fixed unhandled STAXTerminateThreadCondition error logged when a
    STAXPythonEvaluationError signal is raised on a function return (Bug #1252)
  - Fixed STAX Monitor bug where you couldn't display logs with > 3 "|" chars 
    on a multi-line entry (Bug #1244)
  + Enhanced the STAX service to load directly from a jar file, without
    CLASSPATH updates or the prerequisite of installing Jython or any
    additional jar files and updated to terminate all running jobs when the
    service is terminated. Requires STAF 2.4.0 or later. (STAF Feature #561673)
  - Updated sample1.xml to specify the location of the STAX.jar file in case it
    is not in the CLASSPATH.
  - Fixed Unix problem where the STAX Monitor could not display the Job
    User Log (Bug #1233)
  + Created a new STAXMon.jar file that only contains STAX Monitor files and
    removed the STAX Monitor files from STAX.jar.  Now, can start the STAX
    Monitor via 'java -jar STAXMon.jar'.  See the STAX User Guide for more
    information (Support #100110).

-------------------------------------------------------------------------------

Version 1.1.2: 06/18/2002

  - Fixed race condition between STAXProcessAction and STAXProcessActionFactory
    (Bug #1218)
  - Fixed Static handle error on starting STAX Monitor with STAF 2.2.0 (Bug 
    #1217)
  - Fixed NullPointerException that can occur when handling a condition
    that ends a job that uses functions with local scope (Bug #1161).
  - When attempting to import another XML file, if a parsing error occurs, 
    inlcude the XML file name in the error message (Bug #1166).
  + Added support to the STAX Monitor to view the Job and User Logs 
    (Support #100103).
  + Added support for the exact same file name for a process's stdout and
    stderr elements (Support #100101).
  + Added support for specifying one or more files containing Python code
    via a SCRIPTFILE option to the EXECUTE request (Support #100104).
  + Added option to not display &lt;No Monitor info> message (Support #100105).
  
-------------------------------------------------------------------------------

Version 1.1.1: 04/10/2002

  - Fix for &lt;process> and &lt;call> not correctly setting STAXResult to None.
  - Fixed STAX Monitor Job Parms File incompatabilities between Java 1.3 and
    Java 1.4 (Support #100092).
  - Fixed Java 1.4.0 problem where the STAX Monitor window was not getting
    focus when it starts (Bug #1098).
  - Fixed Java 1.4.0 problem with STAX Monitor table row heights (Bug #1096).
  + Added support to the STAX Monitor to retry the initial releasing of a held
    job when submitting a new job, in order to support slower STAX Service 
    machines (Support #100095).
  + Added new mode 'stdout' for process' &lt;stderr> element to support the new
    STDERRTOSTDOUT option which allows stderr to be redirected to the same file
    as stdout (Support #100096).
  + Added new parameter TEST to EXECUTE request which allows validating an
    XML Job File/Data and its parameters without actually submitting the job
    for execution (Support #100030).
  + Added a TEST button to the STAX Monitor's Start New Job panel, which 
    allows validating an XML Job File and its parameters without actually
    submitting the job for execution (Support #100030).
  + Added logging overall testcase totals for a job in the STAX Job Log
    (Support #100093).
  - Fixed bug where the log and message elementss if attribute was not being
    evaluated and checked if true before evaluating other values, like 
    message (Bug #1141).  
  - Fixed bug where STAFResult was not being set to "None" for the &lt;process>
    element (Bug #1143).
  - Fixed bug where a STAXPythonEvaluationException occurred when assigning
    a process's STAXResult to return file output containing one or more
    single quotes (Bug #1144).
  
-------------------------------------------------------------------------------

Version 1.1.0: 03/12/2002

  + Added &lt;returnstdout>, &lt;returnstderr>, and &lt;returnfile> elements
    to the &lt;process> element.
  + Added a mode attribute to the &lt;testcase> element which indicates if 
    information about the testcase is to be reported even if no tcstatus 
    elements have been encountered (Support #100091).
  - Display an error when Other Function in the STAX Monitor's Start New
    Job panel is selected but a function name is not specified (Bug #1036).
  + Changed the &lt;process> element's &lt;sequence> to &lt;process-action> with
    an if attribute (Support #100090).
  - Fixed STAX Monitor event mismatches (Bug #1031).
  + Updated sample1.xml to define function arguments and use new scope
    and requires attributes for a function.
  + Enhanced function scoping by introducing a 'local' function scope,
    adding the ability to pass arguments to a function, to define
    the arguments that can be passed to a function, and to return from
    a function. (Support #100032)
  + Updated the STAX Event generation information to make it more extensible
    (Support # 100086).
  + Added support for polling the STAX Monitor Process monitor information 
    more frequently (Support #100060).
  + Added support for interacting with processes (Support #100045)
  + Added a "mode" attribute to the &lt;command> element to indicate whether 
    the process's command should be executed with the shell option 
    (Support #100084) 
  + Updated the STAX Monitor command line options to use dashes 
    (Support #100082).
  + Added an Arguments field on the STAX Monitor's Start New Job panel, which
    allows you to pass arguments to the Start Function (Support #10083).
  + Added an &lt;import> element which allows importing of functions from 
    another STAX XML Job File (Support #10028).
  + Added static handle support to the STAX Job Monitor, so SET ALLOWMULTIREG
    is no longer required in the STAF.cfg file for the STAX Job Monitor 
    machine (Support #100073).
  + Added a "jobparms" command line parameter to the STAX Job Monitor.  This 
    tells the STAX Monitor to submit a new STAX Job with the jobParmFiles
    options and monitor the job, if applicable (Support #100075).
  - Fixed intermittent problem where the first row in STAX Monitor tables is
    slightly smaller in height than the other rows (Support #100048). 
  + Added a "closeonend" command line parameter to the STAX Job Monitor.  If 
    specified with the "job" or "jobparms" parameters, the STAX Job Monitor 
    will be closed when the job ends (Support #100077).
  + Added an "if" attribute to the message and log elements that can be used
    conditionally determine whether a message should be displayed/logged.
  + In the STAX Monitor's Start New Job dialog, the most recently accessed job
    parameter files are listed in the File menubar, and selecting a menu item
    will load all of it's job parameters (Support #100066).
  - Fixed problem where on Hold, Commands remain in the STAX Monitor view until 
    the block is released (Bug #887).
  - Fixed NullPointerException on paralleliterate/iterate if list being
    iterated is empty or None.  Now raising a STAXEmptyList signal with a
    default signalhandler that sends/logs a message and continues (Bug #948).
  = Modified STAX User's Guide to install Jython 2.1 (available 12/30/2001)
    instead of Jython 2.1 alpha/beta versions.  
  + Added elapsed time for Processes and Commands in the STAX Job Monitor tree 
    (Support #100057).
  + Added a confirmation dialog when deleting all scripts from the STAX Job  
    Monitor's Start New Job panel (Support #100062).
  + Added support to the STAX Job Monitor so that it can be started to Monitor  
    a particular job (Support #100058).
  + Added support for saving the STAX Job Monitor Start New Job parameters to a
    file so that they can later be reloaded (Support #100059).
  + Added a "Clear All Parameters" button to the STAX Job Monitor Start New
    Job panel which will clear all of the parameter fields (Support #100065).
  + Added elapsed time for Jobs in the main STAX Job Monitor panel (Support 
    #100056).
  + Added Testcase Pass/Fail totals to the STAX Job Monitor (Support #100061).
  - Fixed &lt;other> option for the &lt;process> element (Bug #940).
  - Fixed problem updating monitor properties when not root (Bug #886).
  - Fixed Process and STAFCmd so that they use the real host name if local is
    specified (Bug #890).

-------------------------------------------------------------------------------

Version 1.0.4: 12/13/2001

  + Modified the STAX Monitor's Message table to always scroll to the most
    recent message unless the user has scrolled to earlier messages.
  - Fixed STAX Monitor problem with monitoring already started jobs on Unix
    STAX servers (Testcase parsing error)
  - Added a println for TestProcess to display its loop count.
  - Fixed problem with local machine names when STAX Monitor is using a
    non-local STAX service machine
  - Fixed bug where the STAX Service fails to initialize on Linux systems with
    STAF 2.3 installed (get an ArrayOutOfBounds Exception).

-------------------------------------------------------------------------------

Version 1.03: 10/23/2001

  - Fixed bug where using a different case for the a process's &lt;location> vs 
    the case of the hostname resulted in STAX not knowing that the process
    completed.
  = Added a log warning message if a STAX process/request END message arrived 
    but there was no listener.
  - Fixed bug where the STAX Monitor was not saving the Start New Job data on 
    Linux systems.  
  - Fixed a STAX Monitor SplitPane sizing problem (where you always had to 
    move the dividers to see the Processes, etc) on Linux systems.
  + Added a confirmation dialog when attempting to terminate a job from the main
    STAX Monitor dialog.  
  
-------------------------------------------------------------------------------

Version 1.02: 10/04/2001

  - Fixed bug where an OutOfMemoryError occurred for long-running STAX jobs with
    many processes and/or stafcmds.
  - Fixed bug where a java.lang.NullPointerException occurred during the
    initialization of the STAX Service when trying to create a Python
    Interpreter when it needs to process an updated jar file.

-------------------------------------------------------------------------------

Version 1.01: 09/18/2001

  - Fixed bug where STAFProc did not display some errors that may occur when 
    the STAX Service is first initialized.  In STAX Version 1.0, this resulted 
    in RC 7 being returned when you performed any STAX command, such as "STAF 
    local STAX HELP" because the STAX Service did not really initialize 
    successfully.  The initialization errors are now displayed.  

-------------------------------------------------------------------------------

Version 1.0:  09/17/2001

  + Enhanced the following elements:
      + More variations are now allowed in the ordering of elements within 
        a &lt;process> element.
      + &lt;catch> element's exception attribute is now evaluated by Python 
        instead of being a literal
  + Enhanced the STAX Job Monitor.  This included:
      + Providing an "Active Jobs" first panel.
      + Redesigning the layout of the panel to submit a job.
      + Allowing you to start monitoring a job that is already running
      + Using a popup window to hold, terminate, or release a block when
        you right mouse click on it
      + Using better icons to differentiate blocks, processes, and stafcmds
      + Changing the color of the block icon to red when held, yellow when
        held by parent, and green when running
  + Enhanced the output of the QUERY JOB &lt;Job ID> PROCESS &lt;Location:Handle>.
    All of the process elements are now displayed and in a different order.
  + Made the number of physical threads used by the STAX service 
    configurable and defaulted the thread number to 5 instead of 2. 
  + Changed the names of a couple of STAX Variables:
      + STAXXmlFile    => STAXJobXMLFile
      + STAXXmlMachine => STAXJobXMLMachine 
  + Added a few new STAX Variables:
      + STAXJobSourceMachine
      + STAXJobSourceHandle
      + STAXJobSourceHandleName

-------------------------------------------------------------------------------

Version 1.0 Beta 4: 08/16/1998

  + Added the following elements:
     + &lt;try> / &lt;catch>
     + &lt;throw>
     + &lt;rethrow>
     + &lt;timer>
  + Changed the following elements:
     + &lt;eval> element name changed to &lt;script>
     + Made many updates to the elements that can be contained in the
       &lt;process> element.  This included combining some of the elements
       and adding a "if" attribute to all of the optional process elements.
  + Enhanced the STAX Job Monitor.  This included adding new colors and 
    the automatic changing of block names in the tree from green to red 
    when a block is held.
  + Added the ability to list/query processes and stafcmds to the LIST JOB
    and QUERY JOB requests.  
  + On the execute request, changed the EVAL parameter name to SCRIPT.

-------------------------------------------------------------------------------

Version 1.0 Beta 3: 08/01/2001

  + Many enhancements have been made to the STAX Monitor, including the ability 
    to submit a new job.  Also, many updates were made to the user interface.
  + Added a sample file, sample1.xml, that demonstrates some of the 
    capabilities of STAX.
  + No longer need to copy stax.dtd to the STAF\BIN directory.  Remove it if
    you installed Beta 1 or 2.
  + Changed the following elements:
    + &lt;function id="..."> changed to &lt;function name="...">
    + &lt;defaultfunction> is now called &lt;defaultcall>
    + &lt;eval> elements can now be contained by elements other than just &lt;stax>
      and &lt;sequence>
    + &lt;process> and &lt;stafcmd> now have an optional name attribute
    + &lt;testcasestatus status="pass" message="..."/> has been changed to
      &lt;tcstatus result="'pass'">...&lt;/tcstatus>
    + &lt;if expression="..."> and &lt;elseif expression="..."> have been changed to
      &lt;if expr="..."> and &lt;elseif expr="...">
    + &lt;iterate itemvar="..." in="..." indexvar="..."> has been changed to
      &lt;iterate var="..." in="..." indexvar="...">
    + &lt;paralleliterate itemvar="..." in="..." indexvar="..."> has been changed to
      &lt;paralleliterate var="..." in="..." indexvar="...">
    + &lt;loop indexvar="..."> has been changed to
      &lt;loop var="...">
  + Added support for the following elements:
    + &lt;log>
    + &lt;break>
    + &lt;continue>
    + &lt;nop>

-------------------------------------------------------------------------------

Version 1.0 Beta 2: 06/28/2001

  - Fixed bug some events got "lost" because STAF got bogged down with all the
    events generates by STAX.  Fixed by changing to use ReqSync instead of 
    ReqFireAndForget when generating events.

-------------------------------------------------------------------------------

Version 1.0 Beta 1: 06/26/2001

  + Initial release

-------------------------------------------------------------------------------
</PRE>

</td>
</tr>

<!-- end of text for page -->

<?php
require "bottom.php";
?>

</body>
</html>
