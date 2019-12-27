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

<center><h1>Namespace Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for Namespace Service

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

Version 1.0.3: 09/28/2011

  + Added the ability to set the location for the Namespaces xml file via
    parameters when registering the service and added a LIST SETTINGS request
    to show the Directory and File Name settings (Feature #3394743)

-------------------------------------------------------------------------------

Version 1.0.2: 12/15/2009

  - Improved error messages for invalid command requests and added exception
    catching when handling a service request (Bug #2895347)

-------------------------------------------------------------------------------

Version 1.0.1: 02/26/2008

  - Fixed a typo in the description of the CREATE request in the User's Guide
    (Bug #1680098)
  + Changed STAF license from the Common Public License (CPL) 1.0 to the
    Eclipse Public License (EPL) 1.0 (Feature #1893042)  
  
-------------------------------------------------------------------------------

Version 1.0.0: 02/06/2006

  + Released Namespace service on SourceForge
  
-------------------------------------------------------------------------------

Version 1.0.0: 10/21/2005

  + Released first version of the Namespace service internally
    (Feature #1312313)

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
