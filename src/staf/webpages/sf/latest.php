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

<center><a name=top><h1>Programmatically retrieving the latest STAF releases</h1></center>

<p>
The following file is provided to allow you to programmatically determine the
latest release of STAF, STAX, and the other STAF services:
<p>
<a href="http://staf.sourceforge.net/current/latest.xml">
http://staf.sourceforge.net/current/latest.xml</a>

<p>
The following STAX job demonstrates how to programmatically determine the latest
STAF releases that are available on http://staf.sourceforge.net.
<p>
It will read the http://staf.sourceforge.net/current/latest.xml file,
parse the contents, and write the release data to the STAX Monitor and
the STAX Job User log.  It will also demonstrate how to use this data
to download the latest win32 STAF installer executable.
<p>
This STAX job requires the HTTP service to be configured on the STAX
service machine.
<p>
<a href="http://staf.sourceforge.net/current/getLatest.xml">
http://staf.sourceforge.net/current/getLatest.xml</a>

<!-- end of text for page -->

<?php
require "bottom.php";
?>

</body>
</html>





