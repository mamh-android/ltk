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

<center><h1>EventManager Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for EventManager Service

  Legend:
   - Fixes
   + Features           
   = Internal changes
   * Changes required to migrate from one release to another

-------------------------------------------------------------------------------

Version 1.2.7: 03/10/2006

   - Fixed EventManager STAFException during shutdown or when the service was
     dynamically removed (Bug #1447346)

-------------------------------------------------------------------------------

Version 1.2.6: 12/07/2005

  - Fixed problem where the service would get in a bad state if an unexpected
    exception occurred while getting messages from its queue (Bug #1315382)

-------------------------------------------------------------------------------

Version 1.2.5: 08/15/2005

  - Fixed problem where new line characters were being stripped from option
    values (Bug #1246353)

-------------------------------------------------------------------------------

Version 1.2.4: 05/26/2005

  - Fixed problem with LIST request when MACHINE and/or TYPE are not specified
    no registrations would be listed (Bug #1208800)
  - Changed to use "local" instead of the machine host name for the default
    Event service machine for performance reasons (Bug #1208721)

-------------------------------------------------------------------------------

Version 1.2.3: 05/05/2005

  + Added un-register of the service handle during term() (Feature #966079)
  - Updated User's Guide to reference the EM instead of the STAX service
    machine and made the install/configuration section clearer (Bug #989784)
  = Removed the zxJDBC code from our distribution of Jython (Bug #1118221)
  - Changed resolve variable method to check for occurence of "{" anywhere
    in a string instead of just in the first position (Bug #1151440)
  - Changed license from GPL to CPL for all source code (Bug #1149491)  
  
-------------------------------------------------------------------------------

Version 1.2.2: 03/13/2004

  - Fixed bug where serialized registration data from EventManager versions
    prior to 1.2.0 was not being read in during service initialization
    (Bug #915200)

-------------------------------------------------------------------------------

Version 1.2.1: 02/27/2004

  + Instrumented the EventManager service's handling of LIST requests to
    record diagnostics data to help prepare for the migration to STAF V3.0.
    The EventManager service requires STAF V2.6.0 or later as a result of this
    change. (Feature #853620)  

-------------------------------------------------------------------------------

Version 1.2.0: 02/02/2004

  + Added logging to the EventManager service (Feature #838763)

-------------------------------------------------------------------------------

Version 1.1.8: 11/19/2003

  - Changed to require a trust level of 5 for the REGISTER command 
    (Bug #837274)

-------------------------------------------------------------------------------

Version 1.1.7: 09/29/2003

  - Fixed problem where EventManager was not passing back the correct message
    when encountering a parsing error (Bug #814808)

-------------------------------------------------------------------------------

Previous versions included:

  + Added an eventinfo dictionary to the EventManager service (Feature #608058)
  - Display better information if the EventManager service python options 
    contain invalid python code (Bug #608078) 
  - Fixed EventManager service bug where python errors cause queue thread to
    die (Bug #607513) 

-------------------------------------------------------------------------------

Version 1.1.1: 08/12/2002

  - Made the Type and Subtype fields case-insensitive (Bug #594244) 
  
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
