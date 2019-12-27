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

<center><h1>Email Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for Email Service  
  
  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 1.1.6: 02/27/2006

  - Added a LINEEND configuration parameter to allow additional line ending
    characters required by some mail servers (Bug #1398788)

-------------------------------------------------------------------------------

Version 1.1.5: 11/29/2005

  - Fixed multi-threading problem in Email service (Bug #1368689)

-------------------------------------------------------------------------------

Version 1.1.4 08/25/2005

  - Fixed problem where new line characters were being stripped from the email
    message (Bug #1246353)
  - Fixed problem where emails were not being sent when using certain SMTP
    servers (Bug #1160687)

-------------------------------------------------------------------------------

Version 1.1.3 05/05/2005

  - Changed resolve variable method to check for occurence of "{" anywhere
    in a string instead of just in the first position (Bug #1151440)
  - Changed license from GPL to CPL for all source code (Bug #1149491)  

-------------------------------------------------------------------------------

Version 1.1.2 02/02/2005

  - Fixed problem where emails were no longer being sent (Bug #1115008)
  - Fixed problem where the email delivery message was not being written to
    the JVM log (Bug #1115028)

-------------------------------------------------------------------------------

Version 1.1.1 09/22/2004

  - Fixed problem where SEND requests would fail if the MAILSERVER was using
    strict checking (Bug #972876)
  - Fixed additional problems where SEND requests would fail if the MAILSERVER
    was using strict checking (Bug #1032993)

-------------------------------------------------------------------------------

Version 1.1.0: 08/23/2003

  - Send complete FROM address in Email service (Bug #791638, available in 
    Email service V1.1.0)
  + Added a FILE option to the Email service, to email the contents of a text 
    file (Feature #792897, available in Email service V1.1.0)
  - Fixed problem where Email service was not correctly resolving option 
    variables (Bug #787072, available in Email service V1.1.0)

-------------------------------------------------------------------------------

Version 1.0.1: 05/02/2003

-------------------------------------------------------------------------------

Version 1.0.0: 10/07/2002

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
