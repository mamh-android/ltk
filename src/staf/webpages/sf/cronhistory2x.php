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

-------------------------------------------------------------------------------

Version 1.2.6: 03/29/2006

  - Fixed problem triggering Cron requests when using the WEEKDAY option
    (Bug #1452535)

-------------------------------------------------------------------------------

Version 1.2.5: 08/15/2005

  - Fixed problem where new line characters were being stripped from option
    values (Bug #1246353)

-------------------------------------------------------------------------------

Version 1.2.4: 05/05/2005

  = Removed the zxJDBC code from our distribution of Jython (Bug #1118221)
  - Changed resolve variable method to check for occurence of "{" anywhere
    in a string instead of just in the first position (Bug #1151440)
  - Fixed various problems with a REGISTER request (Bug #1156800)
  - Changed license from GPL to CPL for all source code (Bug #1149491)  
  
-------------------------------------------------------------------------------

Version 1.2.3: 05/11/2004

  - Fixed bug where certain registrations would cause the STAF command to
    be executed every minute, even though the MINUTE option had not been
    specified in the registration (Bug #916714)

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
