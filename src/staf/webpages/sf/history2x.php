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

<center><h1>STAF History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for STAF  
  
  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another
   
-------------------------------------------------------------------------------

Version 2.6.11: 04/18/2006

  - Changed to strip leading whitespace from a request value.  Previously, this
    would result in an RC 7, Invalid Request String (Bug #1407668)
  - Fixed "Can't find STAFInst.mfs" error when running STAFInst from a
    directory other than the STAFInst root (Bug #1427934)
  - Updated Users's Guide to indicate commands that are only valid if submitted
    to the local machine (Bug #522701)
  - Changed STAF C++ command parser to provide an error message along with RC 7
    if :Length: exceeds the length of the data (Bug #464827)
  - Improved error message when registering a Java service using a "bad" JVM
    (Bug #1422950)
  - Updated list of operating systems supported by STAF in STAF User's Guide
    (Bug #1452437)      
  - Added instructions to the STAF User's Guide on how to use the "qsh" shell
    on AS/400 (Bug #1428630)
  - Changed the InstallShield installers so that the default STAF.cfg file will
    be created only if the STAF.cfg file does not already exist (Bug #1368716)
  - Improved description of RC 25 to indicate that it's an insufficient trust
    issue (Bug #1457375)
  - Fixed FS COPY request so that if fails due to being out of space on AIX,
    now get RC 19 (File Write Error) instead of RC 0 (Bug #1461730)
  - Fixed a memory leak when enumerating a directory (Bug #1463861)

-------------------------------------------------------------------------------

Version 2.6.10: 12/07/2005

  - Documented what it means if you specify local for the TOMACHINE option in a
    FS COPY request (Bug #1263436)
  - Removed the default selection for the License Agreement panel in the
    InstallShield installers (Bug #1266242)
  - Fixed FS COPY request so that if a write error occurs copying a file or
    directory (e.g. Disk Full), you now get an RC 19 (File Write Error) instead
    of RC 0 (no error) or RC 22 (Bug #1262633)
  - Added a comment to the Python User's Guide about the error that occurs when
    using environment variable PYTHONCASEOK (Bug #1285055)
  - Documented in the STAF Developer's Guide that building STAF Perl 5.6
    support requires both Perl 5.6 and 5.8 to be installed (Bug #1326247)
  - Fixed a performance problem in the Java STAF Command Parser (Bug #1329463)
  - Fixed problem in ZIP service handling zipfiles containing > 32k entries
    (Bug #1347778)
  - Fixed FS service problem in a CREATE DIRECTORY FULLPATH request if specify
    a directory name that starts with \\computername\sharename on Windows
    (Bug #1305912)
  - Fixed FS COPY hang problem when source file is located on a mapped drive
    and the mapped drive is disconnected (Bug #1353461)
  - Fixed problem where FS DELETE ENTRY RECURSE returns RC 22 on Windows if
    length of an entry exceeded MAXPATH (Bug #1295334)
  - Fixed problem where FS DELETE ENTRY RECURSE returns RC 10 on Windows if
    specify a file instead of a directory for the ENTRY (Bug #788475)
  - Fixed problem registering Java services on HPUX-IA64 (Bug #1371022)
  = Changed the Linux IA-32 build machine to a RedHat 8.0 machine
    (Bug #1374880)
  - Fixed problem where removing services would kill STAFProc on Linux
    (Bug #1070250)
  - Fixed problem where STAFProc was unkillable on Linux (Bug #1195497)

-------------------------------------------------------------------------------

Version 2.6.9: 08/15/2005

  - Fixed problem in the Java Command Parser where :0: was not being handled
    correctly as an option value (Bug #1198553)
  - Fixed "Unrecognized option" error during the Solaris/AIX/HPUX STAF install
    when using the .bin file with the bundled JVM (Bug #1201480)
  - Fixed Python support problem where only one thread could run at a time
    (Bug #1201047)
  - For the Linux AMD64 package, include libstdc++.so.6 instead of
    libstdc++.so.5 (Bug #1216686)
  - Fixed problem in FS service where an entry whose name ends in a period was
    not being handled properly (Bug #1225586)
  - Fixed RC 22 problem using FS service to list or query the root directory
    of a Windows network share such as \\server\service (Bug #1225876)
  = Changed to not use enum as a Java variable name so can compile using 
    Java 5.0 since enum is now a Java keyword (Bug #1241613)
  - Pass NULL to AttachCurrentThread in STAFJavaServiceHelper.cpp to resolve a
    JVM crash with IBM Java 5.0 (Bug #1243199)
  - Fixed ZIP service's LIST and UNZIP requests to support read-only zip files
    instead of returning RC 48 "Does Not Exist" (Bug #1244658)
  - Fixed ZIP service's UNZIP request to return an error if a FILE specified
    does not exist (Bug #1245354)
  - Updated STAF Users's Guide by adding a section on environment variable
    settings for STAF (Bug #1243242)
  - Fixed problem with STAF C++ command parser's instanceName() and
    instanceValue() methods (Bug #1252798)
  - Fixed 99% CPU utilization problem with STAFProc that can occur on Unix
    systems if STAF has been running for 49+ days (Bug #1256803)
  - Fixed intermittent FS Copy hang problem and added recovery code for read
    or write failures (Bug #988110)

-------------------------------------------------------------------------------

Version 2.6.8: 05/05/2005

  - Removed Java 1.1 support in STAFJavaServiceHelper.cpp in order to fix a
    build error on Linux AMD64 (Bug #1114820)
  = Removed the zxJDBC code from our distribution of Jython (Bug #1118221)
  - Fixed problem with PYSTAF.dll not being a valid Windows image
    (Bug #1122905)
  - Added InstallShield support for IBM Java 1.4.2 (Bug #1150221)
  - Added a note to the STAF User's Guide that STAF-enabled programs written
    in C must be linked with the C++ compiler (Bug #1153704)
  - Updated the STAF InstallShield installers to bundle newer JVMs which
    resolve security issues (Bug #1149985)
  - Fixed problem building jython along with dependent service (Bug #1156934)
  - Changed license from GPL to CPL for all source code (Bug #1149491)
  - Added more examples for the PROCESS service's START request in the STAF
    User's Guide (Bug #1160201)
  - Fixed problem where STAFProc hung if line in config file was too long and
    increased the maximum length for a line to 2048 characters (Bug #1160287)
  - Fixed command parser hang if ending double quote not found (Bug #1150901)
  + Added license information to the InstallShield and STAFInst installers
    (Feature #1101944) 
  - Provide libstdc++.so.5 in Linux IA64 and AMD64 builds (Bug #1165597)
  - Fixed STAFProc crash on AIX and HPUX when running ZIP ADD (Bug #1181083)
  - Include LICENSE.htm in all installations (Bug #1184010)
  - Added support for AMD64 on Windows (Feature #915243)
  - Fixed problem where STAF configuration statements that did not have a line
    ending were being ignored (Bug #1192041)
  - Fixed typo in the STAFInst help for the acceptlicense option (Bug #1195499)

-------------------------------------------------------------------------------

Version 2.6.7: 01/26/2005

  - Added support for iso8859-15 as alias for codepage ibm-923 (Bug #1076948)
  - Fixed unzip symbolic link issue (Bug #1084676)
  = Moved Zip archive handling out of STAFZipFile class (Feature #1084669)
  - Fixed Windows install error "The system cannot find the file specified"
    (Bug #1086358)
  - Fixed install error on AIX when using IBM Java 1.4 2 (Bug #1084526)
  - Added install support for Java 1.5.x (Bug #1105514)
  - Clarified the definitions of the "NAME" and "EXT" portions of a filename
    for FS COPY/LIST DIRECTORY commands in the STAF User Guide (Bug #1084739)
  - Fixed PYSTAF ImportError when using the STAF Python library on Linux
    (Bug #974507)
  - Added the machine which could not be connected to the error text for an
    RC 16 to make debugging easier (Bug #1054858)
  - Fixed problem in services/log/PySTAFLog.py example (Bug #1044826)

-------------------------------------------------------------------------------

Version 2.6.6.1: 11/19/2004

  - Fixed "zero bytes when unzipping JAR archives" issue (Bug #1076948)

-------------------------------------------------------------------------------

Version 2.6.6: 11/05/2004

  - Fixed ZIP service inflate file problem for InfoZip archive (Bug #1033654)
  - Changed how line endings in a file are determined during a FS GET FILE
    request (Bug #1040001)
  + Documented queued message changes for STAF V3.0 in the Migration section
    and added HTTP LIST/QUERY/FOLLOW LINK/SUBMIT FORM requests to list of
    requests whose output is changing in STAF V3.0 (Feature #740150)  
  + Improve unzip's performance on large files (Feature #1055682)  
  + Added a "Uninstall STAF" icon in the Programs folder on Windows 
    (Feature #1059631)
  + Added ability to specify a default maximum number of records to return
    when querying a log file if don't specify FIRST/LAST/ALL, etc. and added
    the ALL option to the QUERY request (Feature #1040232)  

-------------------------------------------------------------------------------

Version 2.6.5: 09/14/2004

  - Updated Reg service to work on Windows (Bug #1008888)
  - Fixed problem running zip service causes bus error (core dump)
    (Bug #994218)
  - Fixed problem that zip service can't read permission info in the latest 
    InfoZip archive (Bug #1012202)
  - Fixed problem on Windows when starting a process using the default SHELL
    option to preserve quoting in command/parms (Bug #1025075)

-------------------------------------------------------------------------------

Version 2.6.4: 08/02/2004

  - Fixed problem where the AIX STAF install failed with "Null" error 
    (Bug #965002)
  - Fixed problem where the Linux STAF Python and TCL support was not being 
    installed via STAFInst (Bug #968922)
  + As an aid for migrating to STAF V3.0, instrumented the SEM service to
    record diagnostics data since the syntax of most of its requests will be
    changing in STAF V3.0 (Feature #979770)
  - Updated sample C++ and Java services to use current service interface  
    levels (level 2 for C++ services and level 4 for Java services) and updated
    the STAF Service Developer's Guide (Bug #983563)
  + As an aid for migrating to STAF V3.0, instrumented the MISC service's
    MACHINE request as it is being removed in STAF V3.0 (Feature #550251)  
  + Changed STAFInst so that FmtLog is installed during a Recommended
    installation (Feature #986818)
  - Fixed problems deleting symlinks on a FS DELETE request (Bug #604347)
  - Fixed problems providing correct error information on a FS DELETE request
    (Bug #999677)
  - Fixed problem with environment variables when starting a process on Windows
    (Bug #999053)
  - Improved error message on Unix when starting STAFProc without staf/bin in 
    PATH (Bug #824522)
  - Fixed problem accessing files with a timestamp of Feb. 29, 2000
    (Bug #1000886)

-------------------------------------------------------------------------------

Version 2.6.3.1: 05/28/2004

  - Fixed problem where STAF was not listed in Add/Remove Programs on
    Windows IA64 (Bug #957389)

-------------------------------------------------------------------------------

Version 2.6.3: 05/14/2004

  - Fixed problem where the STAFInst script did not have execute permission
    (Bug #944947)
  + Added support for Windows IA64 (Feature #914308)

-------------------------------------------------------------------------------

Version 2.6.2: 04/29/2004

  - Fixed STAFStringConstruct exception when dealing with ZIP archives whose
    "Extra Field" contains unreadable charactors (Bug #928442)
  - Added ISMP Uninstaller support for IBM Java 1.4.1 (Bug #913707)
  - Fixed codepage makefile problem (Bug #932433)
  - Fixed STAF User Guide error in autoboot install section (Bug #935317)
  - Fixed problem where STAF receives a SIGSEGV 11 and crashes on Unix systems
    when starting a process that uses temporary files for stdout/stderr
    (Bug #881930)
  - Updated Linux build to use GCC 3.3.3 to resolve SIGSEGV problems 
    (Bug #936685)
  + Added support for HP-UX IA64 (both 32-bit and 64-bit) (Feature #914317)

-------------------------------------------------------------------------------

Version 2.6.1: 04/01/2004

  - Resolved variables for LIST TRIGGER/SOURCE request to the DIAG service
    (Bug #914288)
  - Corrected the nested jar file section of the STAF Service Developer's
    Guide to show how to correctly nest the jar files (Bug #913155)
  - Fixed problem where STAFHandle.submit/submit2 core dumps Java if a null
    value is passed to it (Bug #917232)
  - Added a delay for a random time before the next connection retry attempt to
    help avoid RC 16 recv: 111 errors and added a new operational parameter
    CONNECTRETRYDELAY to make the maximum delay time configurable (Bug #915342)
  + Added libstdc++-libc6.2-2.so.3 to Linux build/install (Feature #923476)
  - Fixed Perl problem where all STAF calls were made from the most recently
    created STAF handle (Bug #926738)
  + As an aid for migrating to STAF V3.0, instrumented the VAR service to
    record diagnostics data since the syntax of all of its requests will be
    changing in STAF V3.0 (Feature #464843)
    
-------------------------------------------------------------------------------

Version 2.6.0: 03/03/2004

  + Added information to the STAF User's Guide on how to have STAF 
    automatically start as a Windows service during reboot (Feature #889847)
  - Fixed problem "WsnInitialContextFactoy Class Not Found" (Bug #889770)
  + Added information to the STAF User's Guide on how to have STAF
    automatically start during reboot on Unix (Feature #464848)
  + Removed error messages displayed in STAFProc window if can't register
    with automate.austin.ibm.com (Feature #853521)
  + Provided a new internal Diagnostics service, called DIAG, which allows
    you to record diagnostics data and interact with the diagnostics data
    collected (Feature #893634)
  + Added support for Perl 5.8 on Windows and Linux (Feature #890822)
  + Added a new external Zip service, called ZIP, which allows you to 
    Zip/Unzip/List/Delete PKZip/WinZip/Jar compatible archives
    (Feature #890827)
  + Added support for command separator in STAF global variable pool
    (Feature #556432)
  + As an aid for migrating to STAF V3.0, instrumented STAF requests that will
    be changing in STAF V3.0 to record diagnostics data (Feature #853620)
  + Changed the Windows ISMP installer to be a console launcher, so that
    silent installations will not return until the install actually completes
    (Feature #902942)
  - Fixed problem where infinite event/mutex semaphores would time out
    inadvertently on Solaris, resulting in STAF shutting down (Bug #890837)  
  + Added a notify key to the Process Service (Feature #626917)
  - Fixed problem where a STAFException with large message text causes
    STAFProc to terminate abnormally (Bug #906259)
  + Increased default maximum record size for LOG service from 1024 to
    100,000 bytes (Feature #908645)

-------------------------------------------------------------------------------

Version 2.5.2: 01/27/2004

  - Fixed problem "Could not open file /usr/local/staf/codepage/iso88591.bin"
    (Bug #815979)
  - Fixed error in Log service where level User7 was shown as UseR7
    (Bug #816930)
  - Added notes to STAF Users's Guide silent install section to logout/login
    on Unix and to restart on Windows 95/98/ME systems (Bug #819624)
  - Miscellaneous updates to the STAF Service Developer's Guide (Bug #820959)
  = Created an aix421 package (Bug #821438)
  + Added new operational parameter CONNECTATTEMPTS to specify the maximum
    number of times to attempt to connect to a remote system (Feature #827639)
  - Added a new environment variable called STAF_REPLACE_NULLS used by the
    STAF executable to replace null characters in the result string to prevent
    truncation (Bug #863127)
  - Unregister Help service errors for Log, Respool, and Monitor services
    (Bug #878447)
  - Delete stdout/stderr files if PROCESS START command fails to start the
    process (Bug #885014)
  - Fixed RC:10 error on HP-UX if PROCESS START uses temporary stdout or
    stderr files (Bug #883296)

-------------------------------------------------------------------------------

Version 2.5.1: 09/26/2003

  - Removed libC.a and libC_r.a from AIX packaging (Bug #791557)
  + Added support to start a process using RETURNSTDOUT/ERR without having
    having to specify a STDOUT/STDERR filename (Feature #523404)  
  - Fixed FS COPY DIRECTORY RC 22 problem when copying a directory from a
    STAF 2.5.0 machine to a STAF 2.4.5 or lower machine (Bug #810650)

-------------------------------------------------------------------------------

Version 2.5.0: 07/28/2003

  - Fixed PROCESS START request bug on Unix systems where it returned RC 46
    instead of 0 with option IGNOREDISABLEDAUTH specified (Bug #711634)
  - Added help text for error code 51, Directory Copy Error (Bug #719284)
  - Fixed UTF8 conversion problem when accessing a string that contains DBCS
    characters (e.g. via FS GET FILE), but the system is English (Bug #719998)
  - Fixed a Java submit2 error where the result was incorrectly being converted
    from UTF8 to the current codepage (Bug #723415)
  - Fixed Latin-1 codepage conversion hang problem for strings containing DBCS
    characters (Bug #729827)
  - Fixed how STAF determines the codepage on non-English Linux systems
    (Bug #730469)
  - Added try/catch block for process sendNotification exceptions (Bug #740156)
  - Fixed codepage converter exception found when get a STAX parsing exception
    message containing Chinese or other DBCS/MBCS text (Bug #740164)
  - Added more information to the error message when registering a Java service
    and the java executable is not found in the path (Bug #609975)
  - Fixed FS QUERY request bug where it returned an error if the path specified
    had one or more trailing slashes (Bug #726956)
  - Reduced memory use when returning files via a process start request
    (Bug #711604)
  + Added a new codepage variable called STAF/Config/CodePage (Feature #750306)
  + Added support for Windows 2003 (Feature #749572) 
  - Fixed "JVM not found" error with STAF Jar installation (Bug #725261)
  - Fixed problem where ISMP STAF Uninstall fails with "No suitable JVM found" 
    error (Bug #709711)
  - Added a 0-arg constructor for Java STAFResult (Bug #754377)
  - Fixed bug where FS command line not checking all command options
    (Bug #737123)
  + Added support for converting line ending characters on a FS GET FILE for
    text files and added support for displaying in hex (Feature #526463)
  + Added support for converting line ending characters on a FS COPY FILE/
    DIRECTORY for text files and added support for codepage conversion on 
    text file copies (Feature #526463)
  + Allowed substitution of a userid/password in the shell option used when
    starting a process (Feature #751503)
  + Updated STAF builds to use InstallShield MultiPlatform 5.0 
    (Feature #750249)
  - Removed support for PASSWD and SHADOW as process authentication modes
    (Bug #758214)
  - Removed STAF 1.2 checks during Windows installation (Bug #759558)
  - Fixed bug where STAFProc prevents Windows system shutdown (Bug #737123)
  - Fixed RC 22 problem on Unix systems for PROCESS START (no SHELL option)
    requests containing non-English characters (Bug #675502)
  - Fixed problem creating relative paths using a FS CREATE DIRECTORY request
    on Unix systems (Bug #769141)
  + Added support for z/OS V1.4+ (Feature #463682)
  - Fixed problem where a custom install location could not be specified 
    during a silent STAF installation (Bug #776459)
  - Fixed Chinese codepage mapping error for the line-feed (x0D) character
    discovered on a FS GET/COPY TEXT request (Bug #777196) 
  + Install all language support in a Typical STAF installation
    (Feature #778988)
  - Documented how to get around codepage translation problems on Windows
    systems whose locale (e.g. French) sets the ANSI and OEM codepages to
    different values (Bug #775356)
  - Fixed problem deleting a service jar file on Windows after the Java service
    has been dynamically removed via a SERVICE REMOVE request (Bug #779861)
  
-------------------------------------------------------------------------------

Version 2.4.5: 03/27/2003

  + Added Copy Directory request to FS service (Feature #562568)
  - Fixed RC 22 when sending Async requests to non-existant services
    (Bug #704659)
  - Fixed wrong RC (10 instead of 50) when submitting a FS DELETE request for
    a non-empty directory on Win95 and Solaris (Bug #703776)
  - Fixed FS CREATE DIRECTORY bug where it returned RC 10 even though the
    directory was created if the directory name had a trailing slash and
    FULLPATH was specified (Bug #671971)
  - Fixed RC 4007, Invalid file format, query problem in the Log service and
    improved the Log service's performance (Bug #676437)
  + Changed FS GET FILE required trust level to 4 (Feature #709645)
  - Fixed bug where we used the wrong file pointer when determining file size.
    Also removed old linker flag that was causing exceptions not to be caught
    on Linux PPC-64 (Bug #709723)
  - Fixed typo in STAF Python User's Guide example (Bug #710457)
  - Display RC/Result for all STAFDemo errors (Bug #710535)

-------------------------------------------------------------------------------

Version 2.4.4: 03/11/2003

  - Fixed STDIN option on process service start requests (Bug #658842)
  + Added support to allow retrieval of request start times (Feature #656412)
  - Fixed TODAY option on LOG requests (Bug #613357)
  - Updated STAF User Guide, section "7.2 Option Value Formats", on how to use
    the name of an option as the value of an option (Bug #669975)
  - Fixed wrong RC for the Monitor service's query request (Bug #671443)
  = Updated internal service interface to pass a structure instead of
    individual parameters (Feature #668090)
  - Fixed wildcard matching, used by FS service (Bug #677529)
  - Fixed reference to invalid log levels in STAF User Guide (Bug #681041)
  - Updated PROCESS service help to include RETURNxxx options (Bug #681739)
  - Fixed MONITOR service's QUERY request to resolve variables (Bug #682609)
  - Fixed RESPOOL service's REMOVE ENTRY request to return correct RC if
    the entry is owned (Bug #684081)
  - Fixed DELAY and ECHO services trust level checking (Bug #694472)
  - Updated documentation for HANDLE service to include the [STATIC] option in
    the QUERY ALL request (Bug #698339)
  - Fixed link problem with libJSTAF.sl on HP-UX (Bug #699495)
  - Fixed Windows 95 STAFProc startup problem (Bug #696973)
  - Captured stdout/stderr for the JVM processes for diagnostic purposes when
    STAF Java services encounter a problem (Bug #681081)

-------------------------------------------------------------------------------

Version 2.4.3: 12/10/2002

  - Fixed STAF Perl User's Guide Example 3.2.2 (Bug #640697)
  - Fixed STAF Perl User's Guide Example 4.3.3+ (Bug #640715)
  + Added support for codepage ibm-936 (Feature #647977)
  + Added support for building Perl 5.8 support (Feature #648698)
  - Fixed problem where superfluous threads were being started by STAF
    executable (Bug #648545)
  + Added new log method to the STAFLog Java wrapper API to support specifying
    level as a String, such as "info" or "Error" (Feature #651209)
  - Fixed "Too many open files" error installing the shared_jython
    directory (Bug # 651693)    

-------------------------------------------------------------------------------

Version 2.4.2.2: 11/14/2002

  - Fixed StringIndexOutOfBounds exception which was occurring in STAX if you 
    returned a file containing null characters (Bug #605664)
  - Fixed OutOfMemory error when running Java services (Bug #635794)

-------------------------------------------------------------------------------

Version 2.4.2.1: 10/31/2002

  - Fixed Japanese codepage conversion problem for backslash (Bug #621527)
  - Fixed incomplete shared_jython directory problem (Bug #623800)

-------------------------------------------------------------------------------

Version 2.4.2: 10/08/2002

  + Added a symbolic link libSTAF.a to libSTAF.so on AIX (Feature #601478)
  - Fixed SHLIB_PATH not set on HP-UX (Bug #604180)
  - Fixed bug where STAFProc would start if an invalid configuration file was
    specified (Bug #607048)
  - Fixed bug in Java service jar class loader for STAX XMLParseError "Can't
    find bundle for base name org.apache.xerces.impl.msg.XMLMessages"
    (Bug #614659)
  - Fixed problem with default process stop using method not being used  
    (Bug #617866)
  - Fixed Log service problem where FIRST option returns one more record than 
    specified (Bug #613354)
  - Fixed MBCS codepage conversion problem for backslash (Bug #617232)
  - Added support for additional options (%C, %T, %W, %x, %X) when specifying
    a shell on Windows (Bug #620005)
  - Fixed SET PROCESSAUTHMODE bug on Unix (Bug #620407)
                                                               
-------------------------------------------------------------------------------

Version 2.4.1: 08/23/2002

  - Fixed Java service jar class loader (Bug #597392)
  - Fixed "JVM not found" bug during the jar file ISMP installation 
    (Bug #592783)
  - Fixed ISMP installation exceptions when using Blackdown's Java
    (Bug #580332)
  - Fixed Jar installation failure on Windows XP with Java 1.4 (Bug #598448)
  - Decreased timeout when shutting down STAF (Bug #595269)
  + Provided ability to specify a shell to use when starting a process, and
    to specify a default shell to use via the STAF.cfg file (Feature #565465)
  - Fixed problem tracing to STDERR (Bug #599356)

-------------------------------------------------------------------------------

Version 2.4.0.2: 08/15/2002

  - Fixed typo in User's Guide JSTAF examples (Bug #593272)
  + Added a new trace point, Deprecated, which is causes a trace message to
    be generated for deprecated options that STAF detects (Feature #594218)
  + Provided a port of STAF to PACE on OS/400 (Feature #528694)
  + Fixed Fatal Error on AS400 when loading Java services (Bug #595296)
  + Fixed bug where FmtLog was not being installed during Unix ISMP installs
    (Bug #595652)

-------------------------------------------------------------------------------

Version 2.4.0.1: 08/07/2002

  - Fixed bug on HP-UX which required fully qualified path names for shared
    libraries (Bug #592293)
  - Updated docs to indicate use of SHLIB_PATH on HP-UX (Bug #592296)
  - Fixed problem with HP-UX not keeping reference counts on loaded libraries
    (Bug #592844).
  - Fixed HP Installation bug where JSTAF.jar was not being installed
    (Bug #592141)
  - Fixed HP Installation bug where an incorrect link to /lib/java12/libJSTAF
    was being created (Bug #592171)

-------------------------------------------------------------------------------

Version 2.4.0: 08/05/2002

  - Fixed shared library initialization bug on HP-UX (Bug #590177)
  + Added case insensitive contains for Queue service GET/PEEK/DELETE and
    case sensitive contains for Log service QUERY/PURGE requests (Feature
    #464833)
  + Made process management and tracing APIs part of the OS porting layer
    (Feature #585593)
  - Only list services with an init RC of 0 (Bug #584047)
  - Fixed Unix problem by moving sys/types.h include to top of STAFOSTypes.h
    (Bug #567667)
  - Fixed ucm2bin file converter to find last period in file name (Bug #567424)
  + Updated STAFProc to do a proper shutdown when terminated via SIGTERM,
    SIGINT, SIGQUIT, CTRL+C, and CTRL+Break (Feature #464828)
  - Updated the Java build information for Java 1.2+ in the Developer's
    Guide (Bug #575231)
  + Added TCL build information to the Developer's Guide (Feature #575225)
  - Fixed win32 problems with the TCL makefile (Bug #572864)
  + A stack trace is now returned in the STAFResult bufer when Java services
    throw an exception (feature #464840)
  * The ALLOWMULTIREG configuration setting has been removed.  This setting is
    now permanantly "on".  The configuration file parser will continue to
    recognize the option (but will ignore it) until the V3.0 release of STAF.
  + Made STAFDemo more self-contained (Feature #520493)
  + Enhanced the useability of Java services (Feature #561673)
      1) Ability to load services directly from jar files (i.e., without
         CLASSPATH updates)
      2) Removed need to update dynamic library path for Java services
      3) Java services are now loaded in a JVM separate from STAFProc
      4) Java services may be loaded into isolated JVMs or share the same JVM
  * The options available when registering Java services has changed
    substantially.  Please read section 4.4.3 of the STAF User's Guide for a
    list of the current available options.
  + Added Python build info to Developer's Guide (Feature #572900)
  + Added Perl build info to Developer's Guide (Feature #572860)
  - Fixed Perl process start wait timeout error (Bug #572243)
  - Fixed Perl makefile so that it builds correctly on win32 (Bug #572571)
  - Fixed Windows trap when querying log files (Bug #570293)
  - If the STAFDemo can't start the process, display the RC and result (Bug 
    #569064)
  - Fixed bug where a SEM MUTEX request was being added to the front of the
    pending requests list instead of to the back (Bug #565023)
  + Added support for dynamically adding/removing services (Feature #464868)
  + Added support for Service Loader Services (Feature #464867)
  + Added ONLYHANDLE option to only list handle variables (Feature #464830)
  - Updated Java API docs for static handles (Bug #513446)

-------------------------------------------------------------------------------

Version 2.3.2: 06/03/2002

  + Added support for whitespace around machine and service by stripping
    the whitespaces (Feature #464846) 
  - Fixed bug where CONFIRM option for a LOG PURGE request was not working
    (Bug #523949)
  - Fixed bug where STAFInst fails when run under csh and tcsh (Bug #545577)
  - Fixed invalid reference in User's Guide in Process STOP RC (Bug #513386)
  - Updated Variable Service in User's Guide to clarify you should almost
    always use RESOLVE, not GET, to retrieve a variable value (Bug #517765)
  - Fixed problem where could not escape a left brace, {, in a resolvable
    string. Now, can use a caret, ^, to escape a { or ^. (Bug #562495) 

-------------------------------------------------------------------------------

Version 2.3.1: 04/11/2002

  - Fixed trap/hang on Linux SMP (Bug #538488)
  - Fixed problem starting a process remotely using a statichandlename
    (Bug #505081)
  - Fixed SIGSEGV on Linux PPC 64 when using Java Services (Bug #524502)
  - Fixed problem starting a process with a statichandlename where the var
    parameters were not being set as the primary variable pool (Bug #530537)
  + Added support for Python (Feature #513993)
  + Added trace points for warning and info (Feature #531940)
  - Fixed problem starting a process as a different user on Windows NT/2000/XP
    (Bug #487221)
  - Fixed problem redirecting stdout and stderr to the same file when starting
    a process. Added new option stderrtostdout. (Bug #513452)
  - Fixed problem where the system classpath and the OPTION classpaths for a
    Java service were not being merged when using Java 1.2+.  Added support for
    multiple OPTION J2=-Djava.class.path parameters. (Bug #532645)
  - Fixed problem where a process start request specifying a shell command
    like "date; grep ab ab" would fail because it was trying to verify that
    the first subword is a valid command. Unix only. (Bug #541732)
  - Fixed SIGSEGV on Unix systems when a process start shell command's length
    is 36. (#542679)

-------------------------------------------------------------------------------

Version 2.3.0: 12/13/2001

  - Fixed another multi-thread bug on Solaris
  - Removed superfluous (and erroneous) constant from STAFOSTypes.h
  - Fixed bug running Java STAF applications on HP/UX
  - Fixed OS HANDLE leak (win32 only) (Bug #456606)
  + Added a STAFQueueMessage class to Java support
  - Fixed multi-thread bug with gethostbyname() (Bug #460757)
  - Fixed bug with permissions of files created via the PROCESS service's
    STDOUT[APPEND] and STDERR[APPEND] options (unix only) (Bug #461613)
  + Added support for arbitrary shell commands via a new SHELL option to the
    PROCESS services's START command (Feature #461616)
  - Fixed bug where processes STARTed with STDIN/OUT/ERR redirected could not
    delete the redirection files (win32 only) (Bug #462669)
  - Fixed bug where processes STARTed with STDOUT/STDERR didn't have the files
    properly truncated (Bug #462672)
  + Added support for Irix (Feature #463681)
  - Fixed bug when lots of STAF handles exist (win32 only) (Bug #466975)
  - Fixed bug with multi-handle registration on Win95/98/Me (Bug #466976)
  - Fixed problem running Java services on many JVMs (Bug #464869)
  + Added support for returning stdout, stderr, and arbitrary text files when
    a process completes (Feature #464467)
  - Fixed bug where sometimes got RC 6 in STAFRWSemWriteUnlock (Bug #478357)
  + Converted the ResPool service from REXX to C++ (Feature #464864)
  - Changed several Monitor Service return codes from kSTAFInvalidRequestString
    to kSTAFInvalidValue (Bug #478900)
  - Added WinXP support for STAF/Config/OS/Name variable (Bug #478479)
  + Added a variety of file system related commands to FS service (Feature
    #461618 and #461619)
  - Fixed bug sending process end notifications on Linux (Bug #464807)
  + Added support for using static handles from Java (Feature #464857)
  - Fixed bug where processes started with a command & parms whose length
    was > 1024 caused the buffer to overrun and get a segfault (Bug #491608)
  + Increased performance on Windows by 20%
  + Extended AIX support from 4.3.3+ to 4.2.1+

-------------------------------------------------------------------------------

Version 2.2.0: 06/19/2001

  + Added support for HP-UX
  - Fixed bug where stdin/out/err were not displayed when none of them were
    being redirected (win32 only)
  - Fixed bug where incorrect timestamps were being returned by the file
    system APIs (win32 only)
  - Fixed Handle leak bug (win32 only)
  - Fixed bug in STAFLog.rxl where importing 'All' didn't import STAFInitLog
  + Moved HELP service inside STAFProc so that is always available
  * Due to the above move, you should not try to register the old REXX-based
    HELP service
  + Enhanced HELP service so external services can register their error codes
    with it
  + Updated Log and Monitor services to register error codes with HELP service
  + Added a STAFUtilFormatString function, ala printf(), to simplify creating
    STAF request strings.  This is exposed as the formatString() method on the
    STAFHandle class.
  - Fixed bug in Monitor Service where Machine names were case-sensitive
  - Fixed bug where bad handles were returned to STARTed processes (win32 only)
  - Fixed problem prematurely closing socket connections
  - Fixed multi-threading problem on Solaris
  - Fixed bugs when logging and querying log files using bit-strings
  - Fixed standard/daylight savings time bug with Timestamps
  + Enhanced tracing support.  You can now trace only certain services, trace
    requests to other systems, and trace registrations.  Additionally, more
    "Error" conditions are now traced.
  + Improved FS COPY FILE performance
  * Changed default trust level to 3 (was 2)
  = Changed default INITIALTHREADS to 10 (was 5)
  + Added support for "static" handles.  This allows full integration with
    shell-script applications.
  - Fixed file-locking bug preventing use of Log service on Win95/98/Me
  + General performance improvements: 5-30% on Unix, 30-70% on Win32
  - Fixed bug where pending handles from WAITed on processes were not freed
  - Fixed timing bug which resulted in ghost processes
  = Now officially check whether STAF is already running on win32
  - Fixed bug preventing STAF from working on Win95
  - Fixed bug with HANDLE logs when using remote logging
  - Fixed bug listing machines in log service when using USELONGNAMES

-------------------------------------------------------------------------------

Version 2.1.0: 03/02/2001

  + Enhanced STAF command line handling of quoted parameters.  It should now be
    significantly easier to enter commands containing quoted strings from the
    command line.
  + Added support for asynchronous requests (see STAFSubmit2 in the User's
    Guide)
  + The Log and Monitor services have been rewritten in C++, improving their
    performance substantially and allowing them to operate on all supported
    STAF platforms.
  * Standardized all C/C++ API return codes and exceptions.  Existing C/C++
    applications should work unchanged, but new (or recompiled) applications
    may need some mild cleanup.
  + Added support for starting processes as different userids
  + Added support for redirecting stdin/stdout/stderr on started processes
  + STAFProc's environment variables are now exposed through STAF variables
    of the form STAF/Env/&lt;Name>
  - Fixed bug where environment variables weren't being overwritten when
    starting a process on some unix systems
  - Fixed multi-processor bug on win32
  + Added STAF variables for the STAF version number and the configuration file
    being used
  - Fixed bug in error message handling of Rexx services
  - Fixed bug where Java STAFUtil class wasn't public
  - Fixed bug where Java STAFMonitor class constructor wasn't public
  - Fixed bug preventing execution on WinMe and incorrect identification of
    Win2000
  + Unix shared libraries can now be specified like other platforms (i.e.,
    without the 'lib' and '.so')
  + Reduced unix disk and memory requirements
  + The Service service has been updated so that you may list the service
    requests currently being handled by STAF

-------------------------------------------------------------------------------

Version 2.01: 10/11/2000

  - Fixed bug where STAFCommandParseResultGetOptionValue() returned
    incorrect value for non-existant option (this manifested itself as
    a problem using the Event service)
  - Fixed timing problem with FS copy (this manifested itself as a problem
    submitting jobs to WorkFlow Manager)
  - Fixed service termination order

-------------------------------------------------------------------------------

Version 2.0: 09/13/2000

  + Now supported on Linux and Solaris
  + Removed internal STAF dependency on service implementation language   
  * Service registration in STAF.cfg has changed to support the previous
    enhancement (see the configuration section of the STAF User's Guide)
  + Added support for multiple line configuration statements in STAF.cfg
  + Added support for processes to register multiple times (see the
    discussion of ALLOWMULTIREG in the configuration section of the STAF
    User's Guide)
  + We now provide a C++ STAFHandle class
  * Renamed STAFHandle typedef to STAFHandle_t to support the above C++
    STAFHandle class
  + We now provide a set of C/C++ APIs to handle operating system abstraction
    and UTF8 string support
  + The STAFCommandParser is now available for C/C++ services
  = Removed dependency on Visual Age compiler
  - Fixed bug in ResPool that prevented "in use" resources from being deleted
    with the FORCE option
  + The Win32 version now uses InstallShield to do the installation
  + Added service interface level 2 for Java services
  + STAF now uses UTF-8 internally.  This enables round-trip data integrity
    between different codepages/languages.
  + Each copy of STAF is now automatically registerd with a central system in
    Austin (although you can opt out).  This allows us to better determine
    our user base.
  + Added ability to STOP processes "gracefully".
  + Added ability to start processes with or without a new console window
    (Windows only)
  * Moved to a unified STAF command line executable.  The older STAF.cmd and
    CSTAF are still provided, but will be removed in a future version of STAF.

-------------------------------------------------------------------------------

Version 1.21: 11/03/1999

  + Now supported on AIX
  + Added INITIALTHREADS and THREADGROWTHDELTA options to the STAF.cfg SET
    command
  - Fixed problem where PROCESS FREE ALL (or WORKLOAD) could return the
    wrong number of total processes
  - Fixed internal synchronization problem where a process could be removed
    from the process list before it was added, resulting in an exception

-------------------------------------------------------------------------------

Version 1.20: 05/19/1999

  + Added Java services support
  - Fixed bug when performing a QUEUE GET WAIT across midnight
  + Added GET command to FS service
  + Added STAF/Config/Sep/Line, STAF/Config/Sep/File, and STAF/Config/Sep/Path
    variables
  - Fixed service initialization order bug
  + STAFProc now looks in the current directory and
    {STAF/Config/STAFRoot}\bin for STAF.cfg
  + Now allow external services to accept parameters at Init
  - Fixed SEM EVENT WAIT bug where RC:37 could be returned when there are
    multiple waiters and a POST and RESET are done quickly back to back
  + Now allow QUEUE GET/PEEK to specify multiple PRIORITIES, HANDLES,
    MACHINES, NAMES, and CONTAINS
  + Added PULSE option to EVENT command of SEM service
  + Added a folder and icons for STAFProc and HTML Documents, and a link to
    the STAF Web Site for both WIN and OS/2 platforms
  + Added an install record which is created when STAFInst is run, in order
    to document installation history and parameters
  - Fixed a bug in Log in the Query/Purge code using the BEFORE option
  = Modified Help, Monitor, Log and RLog to conform to the new REXX
    STAF_SERVICE_INTERFACE_LEVEL, new call structure, and new variable
    naming convention: STAF/Service/&lt;serviceName>
  - Fixed bug where machine requesting an FS COPY from another machine was
    required to give the sending machine TRUST LEVEL 4
  + You may now delegate a service to a service with a different name on the
    delegatee machine
  + Added the ability to RESOLVE multiple strings in one call
  + Added the ability to perform VAR service commands on the variable pool
    of a given handle
  + All service command options that resolve variables will now resolve
    from the handle's variable pool before resolving globally
  + Added USEPROCESSVARS option to PROCESS START
  * Service registration in the STAF Configuration File has been changed (and
    simplified).
  + Updated GenWL to support global processes and process references,
    inclusion and exclusion of specific machines and processes, and other
    features (see User's Guide full more information)
  - Fixed bug when first character of length delimited data is a colon
  + Added connection timeout configuration parameter
  * Changed names of preset STAF variables to fit within hierarchical
    naming convention
  * Merged MAXREQUESTS, MAXQUEUESIZE, and USELONGNAMES configuration statements
    into one SET configuration statement
  + Added RESPOOL service to standard distribution


-------------------------------------------------------------------------------

Version 1.11: 07/31/1998

  + Now supported on Win95, Win98, and WinNT 4.0
  + Added STAFProc initialization and termination messages
  - Added termination handler for uncaught exceptions

-------------------------------------------------------------------------------

Version 1.10: 05/19/1998

  * The MACHINE and INTERFACE statements in the STAF configuration file
    are now mutually exclusive
  + Official STAF Web Site created: http://automate.austin.ibm.com/staf/
  + Added an index to the STAF User's Guide
  + Added a Services Command Reference to the STAF User's Guide and web
  + Created a seperate STAF API Return Codes document on web
  + Added additional examples/samples to the STAF User's Guide
  + Created a STAF Frequently Asked Questions web document
  + Added STAF future direction to web site
  + Added tracing to STAF (trace command added to MISC service)
  - Fixed a bug when queuing when neither HANDLE nor NAME is specified
  - Fixed intermittent RC:21 error
  - Fixed DBCS bug reading log files in STAFLog.cmd, this required a
    new log record format (backward compatability maintained)
  - Fixed install bug when updating CONFIG.SYS
  - Fixed install bug when updating STARTUP.CMD
  - Fixed GenWL query bug with monitor
  - Fixed DBCS 0x5C translation bug.  Now converts around "\" character
  - Fixed a bug in external services where if passed a null NAME
    or REQUEST was causing a Rexx error.
  - Fixed internal API Level bug with internalSTAFSubmit()
  + Internal parsing changing to support multiple options within OptionNeed

-------------------------------------------------------------------------------

Version 1.00: 04/14/1998

  NOTE: This version is the first official release of STAF.  No one should
        be running a version of STAF prior to 1.0.  Every attempt will be
        made to keep all subsequent versions of STAF backward compatable
        with 1.0.

  * The Java APIs have been overhauled and are not compatible with the
    earlier Java APIs.  This is primarily due to some name changes.
       You need to import com.ibm.staf.*
       You need to import com.ibm.staf.wrapper.* if you use STAFLog
       Class StafHandle changed to STAFHandle
       Class StafException changed to STAFException
       Class StafLog changed to STAFLog
       The return code from STAFLog.log() changed from int to STAFResult
  + The REXX STAF APIs now default to using the STAFHandle variable, thus, it
    is no longer necessary to specify the handle variable name on
    STAFRegister, nor the handle value on STAFSubmit or STAFUnRegister.
  + It is no longer an error to register an already registered process
  + If you register, unregister, and re-register a STAF STARTed process, you
    will now receive the same handle each time you register
  + The handle information for a STAF STARTed process is now retained until
    the process has been FREEd, instead of STOPed
  - Fixed bug in GenWL where machine and global variables could not contain
    spaces
  + Added an EffectiveMachine global variable
  * The Machine global variable now refers to the long name of the machine
  - During a STAF installation, do not replace STAF.CFG if it already exists
    and STAFCFG is NOT specified in the response file
  + Added AUTOSTART response file option in the STAF installation to
    add a START STAF command in the STARTUP.CMD file
  + Added REPMOD response file option in the STAF installation to
    replace in-use STAF EXE/DLL files

  * Version 1.00 is not compatable with version 0.30

-------------------------------------------------------------------------------

Version 0.30: 03/30/1998

  - Fixed a bug in Monitor where the record format changed and it was
    querying 1 more character than it should.

  + New STAF installation process
  + Added TITLE option to PROCESS START.  This support was also added to GenWL.
  + STAF Variable names are now case insensitive
  + Added timeout option to PROCESS START WAIT
  + Added Active External and Active DLL Services
  + Added levels to all internal APIs for future expansion
  + Added SEM Service
  + Added QUEUE Service
  + Added queueing support
  + Added notifications on STAF START, SHUTDOWN, and PROCESS end
  + Added NLV support

  * Version 0.30 is not compatable with version 0.20
  * STAF Variable names are now case insensitive.

-------------------------------------------------------------------------------

Version 0.20: 03/09/1998

  - Fixed bug writing over sockets with a buffer greater than 4096
  - Fixed error code on recursive variable resolution.  Was incorrectly 0.
  - Fixed intermittent RC:6 calling into STAFProc
  - Fixed broken pipe bug on Warp V3 systems
  - Support for IP addresses instead of names
  - Now handle exceptions on cases where 6 was returned.  Now return better
    error codes and more data on problem.
  - Fixed bug where two services could have the same name.

  + Added access control model
  + Added SERVICE service
  + Added TRUST service
  + Added STAFRLog service (remote logging)
  + Added STAFHelp service
  + Added USELONGNAMES configuration option
  + Added FmtLog utility (format log)
  + Added STAFErr (Rexx STAF Common Error Header)
  + Variable resolution of machine and service on STAFSubmit()
  + Variable resolution in config file
  + PROCESS service variable resolution
  + FS service variable resolution
  + HANDLE service variable resolution
  + Removed forcing of 4000+ return codes on external services
  + STAFLog - Unique variable id, variable resolution, PURGE, STATS
  + STAFMon - Unique variable id, variable resolution
  + REXX Services are now tokenized on startup, enhancing performance
  + Added STAFRoot variable
  + TCP/IP isolation - TCP/IP is no longer required on the machine if the
    TCP/IP interface is not used
  + REXX isolation - REXX is no longer required on the machine if no REXX
    services or APIs are used

  * Version 0.20 is not compatable with version 0.10
  * The VAR service command REMOVE changed to DELETE
  * Access control has been added, you may need to set trust levels.
  * Log configuration variable name changes, reference the STAF User's Guide
  * Monitor configuration variable name changes, reference the STAF User's
    Guide

-------------------------------------------------------------------------------

Version 0.10: 02/16/1998

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
