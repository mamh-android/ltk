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

<center><h1>Event Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for Event Service

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

Version 3.1.5: 09/28/2012
   
  + Provided the ability for the event generator to be notified when all
    registrants have acknowledged receiving the event (Feature #3557001)
    
-------------------------------------------------------------------------------

Version 3.1.4: 12/15/2009

  - Improved error messages for invalid command requests and added exception
    catching when handling a service request (Bug #2895347)

-------------------------------------------------------------------------------

Version 3.1.3: 05/29/2009

  - Fixed an event registration problem by performing a case-insensitive match
    of the handle name for the client to prevent duplicate registrations
    (Bug #2778955)
  - Added an Examples section and a section on the EventManager service which
    describes how it allows you to easily manage events (Bug #2786701)

-------------------------------------------------------------------------------

Version 3.1.2: 09/24/2008

  - Fixed how matching by machine and handle name or number is performed when
    submitting an UNREGISTER request and improved the error messages for RC
    4003 and 4004 (Bug #1938024)
  - Fixed how matching by machine and handle name or number is performed when
    submitting an ACKNOWLEDGE request (Bug #1938836)
  - Fixed problem where using the STAX Monitor remotely would slow down STAX
    job execution (Bug #1922591)
  
-------------------------------------------------------------------------------

Version 3.1.1: 02/26/2008

  + Changed STAF license from the Common Public License (CPL) 1.0 to the
    Eclipse Public License (EPL) 1.0 (Feature #1893042)  

-------------------------------------------------------------------------------

Version 3.1.0: 09/29/2005

  - Fixed RC 6 problem that occurred on a GENERATE request if you specify a
    property that only contains a Name instead of Name=Value (Bug #1273518)
  + Provided the ability to mask private data like passwords specified in
    a PROPERTY option value (Feature #622392)
  + Updated to require STAF V3.1.0 or later using the new compareSTAFVersion()
    method since utilizing new privacy methods (Feature #1292268)  

-------------------------------------------------------------------------------

Version 3.0.2: 08/15/2005

  = Changed to not use enum as a Java variable name so can compile using 
    Java 5.0 since enum is now a Java keyword (Bug #1241613)
   
-------------------------------------------------------------------------------

Version 3.0.1: 06/27/2005

  + Added a LIST SETTINGS request to show the operational settings for the
    Event service (Feature #989754)
    
-------------------------------------------------------------------------------

Version 3.0.0: 04/21/2005

  - Fixed Event service trust levels to match recommended trust levels
    (Bug #1032759)
  - Improved error message provided for RC 25 (Access Denied) for all requests
    (Bug #1054858)
  - Changed to use STAFUtil's common resolve variable methods (Bug #1151440)
  - Changed license from GPL to CPL for all source code (Bug #1149491)
  - Changed to not resolve variables in PROPERTY option values on a GENERATE
    request as documented (Bug #1159350)  
    
-------------------------------------------------------------------------------

Version 3.0.0 Beta 7: 12/13/2004

  - Changed to use endpoint instead of machine name when generating queued
    messages (Bug #1079611)
  - Fixed how resolved variables in requests (Bug #1081561)
  
-------------------------------------------------------------------------------

Version 3.0.0 Beta 6: 11/19/2004

  - Removed line separators from message text to display better
  
-------------------------------------------------------------------------------

Version 3.0.0 Beta 5: 10/30/2004

  + Changed to return STAFResult from init/term methods (Feature #584049)
  + Standardized the LIST/QUERY syntax and changed the LIST/QUERY requests to
    return marshalled data (Feature #740150)
  + Updated queued messages to have type STAF/Service/Event (Feature #1044711)
  + Changed the message being queued to be a marshalled map (Feature #740150)
  + Changed to use new STAFServiceInterfaceLevel30 (Feature #550251)
  
-------------------------------------------------------------------------------

Version 3.0.0 Beta 4: 09/29/2004

  - Fixed some variable resolution problems (Bug #986196)

-------------------------------------------------------------------------------

Version 3.0.0 Beta 3: 06/28/2004

  + Added un-register of the service handle during term() (Feature #966079)
  
-------------------------------------------------------------------------------

Version 3.0.0 Beta 2: 04/29/2004

  + Updated to use STAFServiceInterfaceLevel5, only supported in STAF V3.0.0,
    and to use new syntax for the VAR service (Feature #464843) 
  + Updated to use the new info.writeLocation field so that it now writes the
    GenManager.out and RegManager.out files to directory &lt;info.writeLocation>/
    service/&lt;serviceName> instead of writing the files to the current
    directory, the directory where STAFProc was started (Feature #592875)
    
-------------------------------------------------------------------------------

Version 1.3.1: 02/27/2004

  + Instrumented the Event service's handling of QUERY and LIST requests to
    record diagnostics data to help prepare for the migration to STAF V3.0.
    The Event service requires STAF V2.6.0 or later as a result of this change.
    (Feature #853620)  

-------------------------------------------------------------------------------

Version 1.3.0: 01/26/2004

  - Fixed various bugs in the Event Service and improved its performance and
    updated the Event Service User's Guide (Bug #881693)
  - Fixed a performation problem where stale event registration clients for the
    STAX Monitor were not being unregistered (Bug #883240)

-------------------------------------------------------------------------------

Version 1.2.2: 08/19/2003

  - Fixed wrong description for RC 4005 in Event service's User's Guide
    (Bug #789416)

-------------------------------------------------------------------------------

Version 1.2.1: 08/15/2002

  - Increased PriorityQueue size for Event Service (Bug #565501)

-------------------------------------------------------------------------------

No previous history available

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
