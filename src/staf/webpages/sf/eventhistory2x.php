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

-------------------------------------------------------------------------------

Version 1.3.5: 12/07/2005

  - Changed to return an error if the PROPERTY value specified for a GENERATE
    request does not have format <Name>[=<Value>] (Bug #1273518) 

-------------------------------------------------------------------------------

Version 1.3.4: 08/15/2005

  = Changed to not use enum as a Java variable name so can compile using 
    Java 5.0 since enum is now a Java keyword (Bug #1241613)

-------------------------------------------------------------------------------

Version 1.3.3: 05/05/2005

  - Changed resolve variable method to check for occurence of "{" anywhere
    in a string instead of just in the first position (Bug #1151440)
  - Changed license from GPL to CPL for all source code (Bug #1149491)
  - Changed to not resolve variables in PROPERTY option values on a GENERATE
    request as documented (Bug #1159350)  

-------------------------------------------------------------------------------

Version 1.3.2: 01/25/2005

  + Added un-register of the service handle during term() (Feature #966079)
  - Fixed Event service trust levels to match recommended trust levels
    (Bug #1032759)

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
