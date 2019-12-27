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

<center><h1>FSExt Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for FSExt Service  
  
  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 1.2.4: 05/05/2005

  + Added un-register of the service handle during term() (Feature #966079)
  - Changed license from GPL to CPL for all source code (Bug #1149491)  
  
-------------------------------------------------------------------------------

Version 1.2.3: 02/27/2004

  + Instrumented the FSExt service's handling of COMPAREDIR, FILECONTAINS, and
    LINECONTAINS requests to record diagnostics data to help prepare for the
    migration to STAF V3.0.  The FSExt service requires STAF V2.6.0 or later as
    a result of this change. (Feature #853620)  

-------------------------------------------------------------------------------

Version 1.2.2: 10/09/2003

  - Fixed problem where FSExt was returning an incorrect error message when
    receiving an invalid command request (Bug #820920)

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
