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

<center><h1>SXE Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for SXE Service

  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 2.1.3: 05/05/2005

  + Added un-register of the service handle during term() (Feature #966079)
  - Changed license from GPL to CPL for all source code (Bug #1149491)  
  
-------------------------------------------------------------------------------

Version 2.1.2: 02/27/2004

  = Changed from old STAFServiceInterfaceLevel2 to STAFServiceInterfaceLevel4
    (Feature #853620)
  + Unregistered help for the SXE service's error codes
    (Feature (#853620)
  + Instrumented the SXE service's handling of EXECUTE requests to record
    diagnostics data to help prepare for the migration to STAF V3.0.
    The SXE service requires STAF V2.6.0 or later as a result of this change.
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
