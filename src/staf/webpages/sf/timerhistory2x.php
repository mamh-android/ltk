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

<center><h1>Timer Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for Timer Service

  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 1.1.1: 05/05/2005

  = Fixed typo in Timer Service User's Guide to reference the correct jar file
  - Changed license from GPL to CPL for all source code (Bug #1149491)  
  
-------------------------------------------------------------------------------

Version 1.1.0: 02/27/2004

  = Changed from old STAFServiceInterfaceLevel1 to STAFServiceInterfaceLevel4
    (Feature #853620)
  + Registered and unregistered help for the Timer service's error codes
    (Feature (#853620)
  + Instrumented the Timer service's handling of LIST requests to record
    diagnostics data to help prepare for the migration to STAF V3.0.
    The Timer service requires STAF V2.6.0 or later as a result of this change.
    (Feature #853620)  

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
