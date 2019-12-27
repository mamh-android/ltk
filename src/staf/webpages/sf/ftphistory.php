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

<center><h1>FTP Service History</h1></center>
<PRE>
-------------------------------------------------------------------------------
History Log for FTP Service  
  
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

Version 1.0.3: 12/07/2010

  + Added the ability to specify the port number to which the TCP connection
    is made on the remote FTP host machine (Feature #3130585)

-------------------------------------------------------------------------------

Version 1.0.2: 12/15/2009

  - Improved error messages for invalid command requests and added exception
    catching when handling a service request (Bug #2895347)

-------------------------------------------------------------------------------

Version 1.0.1: 10/06/2008

  + Made the following enhancements (Feature #819826):
    - Changed to not encode the remote file/directory value to fix a problem
      with using the DIR request to list subdirectories since the "/" in the
      directory path was being encoded.  Also, added a "/" to the end of the
      remote directory if not specified so that the DIR request would work.
    - Added an anonymous login by default if USER and PASSWORD aren't specified
      in a request
    - Added buffering of input/output for better performance
    - Provided better error handling and error messages returned
    - Added more checking to the GET, PUT,and DIR STAF parsers so that a 
      parsing error (RC 7 Invalid Request String) is returned if a required
      option (like HOST, URLPATH, FILE, etc) isn't provided
    - When resolving STAF variables in option values, changed to return the
      STAF return code and error message provided by the STAF variable resolve
      method instead of returning RC 7 (Invalid Request String)
    - Changed the required STAF trust levels to 4 for GET and PUT requests and
      2 for a DIR request
    - Added handling STAF privacy delimiters for the PASSWORD option (e.g.
      !!@password@!!) so that the password does not show up when listing STAF
      service requests via a SERVICE LIST REQUESTS request
    - Changed the FTP service syntax (e.g. TARGET -> URLPATH, LOCAL -> FILE)
    - Renamed Java source files and reorganized a bit to follow our conventions
    
-------------------------------------------------------------------------------

Version 1.0.0: 08/27/2008

  + Initial version submitted via Patch #2079001 by David Lander/Ottawa/IBM.

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
