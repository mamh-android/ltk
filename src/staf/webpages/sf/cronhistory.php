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

<center><h1>Cron Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for Cron Service

  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another

  Notes:
  1) To get more information on each bug, use the following url, replacing
     Ticket# with the Bug#: https://sourceforge.net/p/staf/bugs/Ticket#/
     For example, to get more information on Bug #2982317, go to:
       https://sourceforge.net/p/staf/bugs/2982317/
  2) To get more information on each feature, use the following url, replacing
     Ticket# with the Feature#: https://sourceforge.net/p/staf/feature-requests/Ticket#/
     For example, to get more information on Feature #1867258, go to:
       https://sourceforge.net/p/staf/feature-requests/1867258/
  3) If you specify an old ticket number (i.e. created before June 2013 when
     the STAF project was upgraded to the new SourceForge developer platform),
     you'll be automatically redirected to the link with the new ticket number.

-------------------------------------------------------------------------------

Version 3.4.0: 06/30/2011

  + Changed to use Jython 2.5.2 instead of Jython 2.1 (Feature #1867258)
    Note: Cron now requires Java 1.5 or later.
  = Changed to take advantage of some new features in Java 1.5 such as Generics
    (Feature #2961492)

-------------------------------------------------------------------------------

Version 3.3.8: 11/01/2010

  + Added a STAFCronID Python variable containing the ID of the triggered
    registration and added more documentation about using Python in the
    service's user guide (Feature #3095056)
    
-------------------------------------------------------------------------------

Version 3.3.7: 09/30/2010

  - Changed the MonitorThread to exit if the QUEUE GET WAIT request it submits
    in a loop fails so it doesn't run continuously consuming lots of CPU
    (Bug #2982317)
  - Improved error handling when an error occurs writing registration data to
    the cron.ser file by logging and returning an error to let you know that
    registration data in memory is out of sync (Bug #3031791)

-------------------------------------------------------------------------------

Version 3.3.6: 12/15/2009

  = Changed so that no longer use deprecated Java methods (Bug #1505690)
  - Improved error messages for invalid command requests and added exception
    catching when handling a service request (Bug #2895347)

-------------------------------------------------------------------------------

Version 3.3.5: 04/23/2009

  - Fixed the STAF unmarshall method for Jython so that it no longer causes an
    error when invalid marshalled data is input.  This required setting a new
    Jython version, 2.1-staf-v3.3, in the manifest (Bug #2515811)
  - Fixed a problem sharing the same PythonInterpreter across threads, added a
    DEBUG parameter to provide more information if a time trigger issue occurs,
    and restructured the code (Bug #2594306)
  - Improved performance for processing queued messages for STAF service
    request completions.  Updated to require STAF V3.3.3 or later on the Cron
    service machine since it now uses the QUEUE GET ALL request (Bug #2771636)
  - Fixed a memory leak when processing a registration that submits a STAF
    service request to a remote machine (Bug #2771642)

-------------------------------------------------------------------------------

Version 3.3.4: 12/10/2008

  - Fixed a process completion notification problem that can occur if a
    PROCESS START request is registered without the WAIT option and the
    process completes very quickly (Bug #2265304)
  - Fixed a RC 48 (Does Not Exist) problem viewing the service log via the
    CronUI when the service name entered on the CronUI does not have the same
    case as used when registering the service (Bug #2393318)

-------------------------------------------------------------------------------

Version 3.3.3: 09/24/2008

  - Changed the LIST LONG request to mask private data in the Prepare Script
    value (Bug #1938715)
    
-------------------------------------------------------------------------------

Version 3.3.2: 02/26/2008

  + Changed STAF license from the Common Public License (CPL) 1.0 to the
    Eclipse Public License (EPL) 1.0 (Feature #1893042)  

-------------------------------------------------------------------------------

Version 3.3.1: 01/08/2008

  - Changed to not resolve STAF variables in the REGISTER request's MACHINE,
    SERVICE, and REQUEST options and added the OLDVARRESOLUTION service
    parameter for backward compatibility (Bug #1844449)
  - Added a check when triggering/submitting a registered STAF service request
    to see if the submit request failed (Bug #1862787)
  - Fixed "The java class is not found: com.ibm.staf.STAFException" error
    when starting CronUI (Bug #1866950)

-------------------------------------------------------------------------------

Version 3.3.0: 11/20/2007

  = Renamed Manifest.mf to MANIFEST.MF so can build this service on Unix
    machines (Bug #1732343)
  - Fixed problem handling numeric time interval values with leading
    zeros (Bug #1764804)
  - Fixed problem where editing a CronUI registration with Python errors
    could cause the original registration to be lost (Bug #1802869)
  + Added ability to enable/disable Cron registrations
    (Feature #1821696)
  = Updated CronUI to automatically wrap the text specified for tool tips
    (Bug #1822409)
  + Added ability to start the CronUI by executing "java -jar STAFCron.jar"
    (Feature #1827098)

-------------------------------------------------------------------------------

Version 3.2.2: 04/23/2007

  - Added a missing space separator for some options in a REGISTER request
    (Bug #1675955)
  - Added examples for how to use the PYTHONREQUEST and PREPARE options on a
    REGISTER request and another CronUI registration example (Bug #1675198)
    
-------------------------------------------------------------------------------

Version 3.2.1: 11/09/2006

  = Updated the manifest to specify Jython version 2.1-staf-v3.  This version
    changed the marshall and formatObject methods in Lib/STAFMarshalling.py to
    significantly improve their performance (Bug #1559277)

-------------------------------------------------------------------------------

Version 3.2.0: 08/31/2006

  = Updated the manifest to specify Jython version 2.1-staf-v2.  This version
    changed the marshall method in Lib/STAFMarshalling.py to check for objects
    with a callable stafMarshall method (Bug #1408579)
  + Added a ONCE option to the REGISTER command that indicates that the STAF
    command should only be executed one time (Feature #1490943)
  + Added a DESCRIPTION option to the REGISTER command (Feature #1505561)
  - Fixed problem where Python errors in the request would kill the service's
    timer thread, and added Python compilation of the request during
    registration (Bug #1505089)
  - Redesigned the content of the Cron service's log records (Bug #1513006)
  + Added a TRIGGER request that allows a registered STAF command to be
    executed at any time (Feature #1501230)
  - Sort the results of a LIST request by ID (Bug #1536239)
  + Added a SHORT option to the LIST request (Feature #1536307)
  - Changed to use Pass/Fail log levels based on the RC the STAF
    commands/processes return (Bug #1538796)
  + Added a new User Interface, CronUI (which replaces CronRegister) to
    simplify interaction with the Cron service, including the ability to
    edit registrations, copy registrations, etc. (Feature #1493217)

-------------------------------------------------------------------------------

Version 3.1.1: 03/29/2006

  - Fixed problem triggering Cron requests when using the WEEKDAY option
    (Bug #1452535)

-------------------------------------------------------------------------------

Version 3.1.0: 09/29/2005

  + Provided the ability to mask passwords and other sensitive data
    (Feature #622392)
  + Updated to require STAF V3.1.0 or later using the new compareSTAFVersion()
    method since utilizing new privacy methods (Feature #1292268)  
   
-------------------------------------------------------------------------------

Version 3.0.0: 04/21/2005

  - Improved error message provided for RC 25 (Access Denied) for all requests
    (Bug #1054858)
  = Removed the zxJDBC code from our distribution of Jython (Bug #1118221)
  - Changed to use STAFUtil's common resolve variable methods (Bug #1151440)
  - Changed to require trust level 4 for a UNREGISTER request (Bug #1156340)
  - Fixed various problems with a REGISTER request (Bug #1156800)
  - Changed license from GPL to CPL for all source code (Bug #1149491)  
   
-------------------------------------------------------------------------------

Version 3.0.0 Beta 7: 12/13/2004

  = Recompiled
  
-------------------------------------------------------------------------------

Version 3.0.0 Beta 6: 11/19/2004

  - Updated Cron to free process handles when complete (Bug #988247)

-------------------------------------------------------------------------------

Version 3.0.0 Beta 5a: 11/09/2004

  + Changed to load STAFMarshalling Python module needed by STAX
    (Feature #740150)
    
-------------------------------------------------------------------------------

Version 3.0.0 Beta 5: 10/30/2004

  + Changed to return STAFResult from init/term methods (Feature #584049)
  + Updated to handle new queue type for queued messages (Feature #1044711)
  + Updated to handle new marshalled messages in queued messages and to create
    a marshalled map class for its LIST result and added a LONG option to the
    LIST request (Feature #740150)
  + Changed to use new STAFServiceInterfaceLevel30 (Feature #550251)
    
-------------------------------------------------------------------------------

Version 3.0.0 Beta 4: 09/29/2004

  + Updated to handle new marshalled result format for a QUEUE GET request
    (Feature #740150)
    
-------------------------------------------------------------------------------

Version 3.0.0 Beta 3: 06/28/2004

  - Fixed bug where certain registrations would cause the STAF command to
    be executed every minute, even though the MINUTE option had not been
    specified in the registration (Bug #916714)
   
-------------------------------------------------------------------------------

Version 3.0.0 Beta 2: 04/29/2004

  + Updated to use STAFServiceInterfaceLevel5, only supported in STAF V3.0.0,
    and to use new syntax for the VAR service (Feature #464843) 
  + Updated to use the new info.writeLocation field so that it now writes the
    cron.ser file to directory &lt;info.writeLocation>/service/&lt;serviceName>
    instead of writing the file to &lt;System.getProperty(user.home)>/crondata
    (Feature #592875)
    
-------------------------------------------------------------------------------

Version 1.2.2: 03/13/2004

  - Fixed bug where serialized registration data from Cron versions
    prior to 1.2.0 was not being read in during service initialization
    (Bug #915200)

-------------------------------------------------------------------------------

Version 1.2.1: 02/27/2004

  + Instrumented the Cron service's handling of LIST requests to record
    diagnostics data to help prepare for the migration to STAF V3.0.
    The Cron service requires STAF V2.6.0 or later as a result of this change.
    (Feature #853620)  

-------------------------------------------------------------------------------

Version 1.2.0: 02/02/2004

  + Added logging to the Cron service (Feature #838763)

-------------------------------------------------------------------------------

Version 1.1.5: 11/20/2003
    
  - Fixed Cron service bug where the LIST command was returning an extra
    semi-colon if the HOURS option was not specified in the registration
    (Bug #845894, available in Cron service V1.1.5)

-------------------------------------------------------------------------------

Version 1.1.4: 11/29/2003

  - Changed to require a trust level of 5 for the REGISTER command 
    (Bug #837274)
  - Fixed bug where it would run an Hour request every minute within the hour
    (Bug #838757)
  - Fixed bug where the LIST command was not including the PREPARE script
    (Bug #845449)
    
-------------------------------------------------------------------------------

Version 1.1.3: 09/29/2003

  - Fixed problem where Cron was not passing back the correct message when 
    encountering a parsing error (Bug #814808)

-------------------------------------------------------------------------------

Version 1.1.2: 08/31/2003  

  - Fixed problem where Cron service was not ANDing all of the time options 
    (Bug #779348)

-------------------------------------------------------------------------------

Previous History is not available

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
