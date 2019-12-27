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

<center><h1>HTTP Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for HTTP Service  
  
  Legend:
   - Fixes
   + Features
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 2.0.4: 04/18/2006

  + Updated to use CyberNeko HTML Parser V0.9.5 and Commons HttpClient V3.0
    (Feature #1415860)
  - Fixed NullPointerException that can occur in an HTTP DOGET request when
    a IOException occurs and added Java stack trace to result (Bug #143980)  
    
-------------------------------------------------------------------------------

Version 2.0.3: 12/07/2005

  - Fixed a problem when using the FILE option on a request such as DOGET
    where the result stored in the specified file is corrupted (Bug #1375570)

-------------------------------------------------------------------------------

Version 2.0.2: 05/25/2005

  + Provided better support for relative URLs (Feature #1013094)
  
-------------------------------------------------------------------------------

Version 2.0.1: 05/05/2005

  - Fixed typo in HTTP Service docs where it references the wrong jar file name
  - Fixed problem submitting forms with an INPUT FILE control type
    (Bug #1155345)
  - Changed license from GPL to CPL for all source code (Bug #1149491)
  - Updated to use CyberNeko HTML Parser V0.9.4 and Commons HttpClient V3.0-RC2
    (Bug #1188680)
  - Fixed Null Pointer Exception using CONTENTFILE option (Bug #1193873)

-------------------------------------------------------------------------------

Version 2.0.0: 09/15/2004

  + Added un-register of the service handle during term() (Feature #966079)
  = Switched code base for non-session requests to use a temporary session
  - Fixed TRACE request behavior to display response message body in 
    non-session requests
  - Fixed 404 behavior now return rc 4404 as documentation indicates
  + Added session support (Feature #1004825)
  + Added SSL support (Feature #1004825)
  + Added Basic, Digest, and NTLM Authentication support (Feature #1004825)
  + Added Session persistant headers (Feature #1004825)
  + Added Cookie support (Feature #1004825)
  + Added PARAMETER option to provide support for method parameters 
    (Feature #1011721)
  + Added CONTENTFILE option to improve multi-file post support 
    (Feature #1011721)
  + Added form manipulation support (Feature #1004825)
  + Added link following support  (Feature #1004825)
  - Completed information provided for a QUERY FORM CONTROLNAME request
    (Bug #1013116)

-------------------------------------------------------------------------------

Version 1.0.6: 05/12/2004

  - Changed to return new RC 4004 (IO Exception) instead of RC 38 (Java error)
    when a Java IOException occurs due to not being able to find the page
    requested for REQUEST/DOPOST/DOGET requests.  Also, added code to 
    unregister all HTTP return codes with the HELP service when the HTTP
    service is terminated. (Bug #944926)

-------------------------------------------------------------------------------

Version 1.0.5: 02/27/2004

  + Instrumented the HTTP service's handling of REQUEST/DOPOST/DOGET requests
    to record diagnostics data to help prepare for the migration to STAF V3.0.
    The HTTP service requires STAF V2.6.0 or later as a result of this change.
    (Feature #853620)  

-------------------------------------------------------------------------------

Version 1.0.4: 10/09/2003

  - Fixed problem where HTTP was returning an incorrect error message when
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
