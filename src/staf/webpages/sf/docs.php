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

<center><a name=top><h1>STAF/STAX V3 Documentation</h1></center>

<p>
The following documentation is provided for the current versions of STAF V3 and STAX V3.
If you want to view documentation for STAF V2, go to the <a href="docs2x.php">
STAF V2 Documentation</a> page instead.
<p>
<h3>STAF V3 Documentation</h2>
<ul>
  <li><a href="current/STAFFAQ.htm">STAF V3 Frequently Asked Questions (FAQ)</a><br>
  Contains frequently asked questions and answers about STAF and STAX.  You should check the
  FAQ before submitting a question to a STAF Help forum or mailing list.
  <p>
  <li><a href="current/STAFGS.pdf">Getting Started with STAF V3 (PDF)</a><br>
  Gets you started using STAF.  It guides you through many common tasks that are performed when using
  STAF, including a detailed examination of a demo which shows how you can instrument and leverage 
  STAF in your testing.
  <p>
  <li><a href="current/STAFInstall.pdf">STAF V3 Installation Guide (PDF)</a><br>
  Contains detailed information on how to install STAF.
  <p>
  <li><a href="current/STAFUG.htm">STAF V3 User's Guide (HTML)</a><br>
  Contains detailed information on how to set up STAF and use STAF commands and Services.  Contains
  in-depth information including service command syntax and return codes.
  <p>
  <li><a href="current/STAFDiag.htm">STAF Diagnostics Guide (HTML)</a><br>
  This document describes common techniques to debug problems when running STAF. 
  <p>
  <li><a href="current/STAFCMDS.htm">STAF V3 Service Command Reference (HTML)</a><br>
  Contains a quick reference to the syntax of the service commands.
  <p>
  <li><a href="current/STAFRC.htm">STAF V3 API Return Code Reference (HTML)</a><br>
  Contains a quick reference to the STAF return codes.
  <p>
  <li><a href="current/STAFJava.htm">STAF Java User's Guide (HTML)</a><br>
  This document describes STAF's V3 support for the Java language.  It includes
  information on the core STAF Java APIs as well as the wrappers provided for 
  the Monitor and Log services.
  <p>
  <li><a href="current/STAFPerl.htm">STAF Perl User's Guide (HTML)</a><br>
  This document describes STAF's V3 support for the Perl language.  It includes
  information on the core STAF Perl APIs as well as the wrappers provided for
  the Monitor and Log services.
  <p>
  <li><a href="current/STAFPython.htm">STAF Python User's Guide (HTML)</a><br>
  This document describes STAF's V3 support for the Python language.  It includes
  information on the core STAF Python APIs as well as the wrappers provided for
  the Monitor and Log services.
  <p>
  <li><a href="current/STAFTcl.htm">STAF Tcl User's Guide (HTML)</a><br>
  This document describes STAF's V3 support for the Tcl language.  It includes
  information on the core STAF Tcl APIs as well as the wrappers provided for 
  the Monitor and Log services.
  <p>
  <li><a href="current/STAFAnt.htm">STAF Ant Task User's Guide (HTML)</a><br>
  This document describes how to use the STAF Ant Task to call into the
  STAF framework from within an Ant build script.
  <p>
  <li><a href="current/stafdg.html">STAF V3 Developer's Guide (HTML)</a><br>
  Contains detailed information on how to build STAF.  It will walk you through
  the process of obtaining the STAF source code and setting up your build environment.
  STAF developers are the intended audience for this document.
  <p>
  <li><a href="current/stafsdg.html">STAF V3 Service Developer's Guide (HTML)</a><br>
   This document describes how you create STAF services, and includes
   implementation of a sample service in both Java and C++.
  <p>
  <li><a href="current/stafmigrate.html">STAF V3 Migration Guide (HTML)</a><br>
  Contains important information for existing STAF V2 users that should be read before
  migrating to STAF V3.
  <p>
  <li><a href="current/STAFOverview.html">STAF V3 Overview</a><br>
  Contains an overview highlighting some of the major differences between STAF V2 and STAF V3.
</ul>

<h3>STAX V3 Documentation</h2>
<ul>
  <p>
  <li><a href="current/staxgs.pdf">Getting Started with STAX V3 (PDF)</a><br>
  Gets you started using STAX.  It guides you through many common tasks that are performed when using
  STAX, including getting started running a STAX job and to create a STAX XML document.
  <p>
  <li>STAX V3 User's Guide<br>
  Contains detailed information on how to configure and use the STAX service.  Contains
  in-depth information including syntax for creating STAX XML documents, how to use the
  STAX Monitor, and examples of STAX XML documents.
    <ul compact>
      <li><a href="current/STAX/staxug.html">STAX V3 User's Guide (HTML)</a> (View)
      <li><a href="current/staxug.pdf">STAX V3 User's Guide (PDF)</a> (Download/View)
    </ul>
  <p>
  <li><a href="current/STAXDoc.pdf">STAXDoc User's Guide (PDF)</a><br>
  STAXDoc is used to generate documentation for your STAX xml files.
  As you grow your library of STAX functions, you will probably find it useful
  to document the STAX functions to make it easier to reuse them and share
  them with other test groups.
  <p>
  <li><a href="current/staxdg.html">STAX V3 Extensions Developer's Guide (HTML)</a><br>
   This document describes how you create extensions for STAX.&nbsp;
   Extensions to the STAX service can be written which define new elements
   that can be used in a STAX xml file. In addition, extensions to the STAX
   Monitor can be written which define new plug-in views which can be displayed
   via the STAX Monitor. 
</ul>


<h3>Other STAF Services Documentation</h2>
<ul>
  <p>
  <li><a href="current/cron.pdf">Cron Service User's Guide</a><br>
  Contains detailed information on how to configure and use the Cron service.
  The Cron service allows you to register STAF commands that will be executed
  at a specified time interval. 
  <p>
  <li><a href="current/email.html">Email Service User's Guide</a><br>
  Contains detailed information on how to configure and use the Email service.
  The Email service allows you to send an email message to a list of recipients.
  <p>
  <li><a href="current/event.pdf">Event Service User's Guide</a><br>
  Contains detailed information on how to configure and use the Event service.
  The Event service allows process communication based on events occuring. 
  <p>
  <li><a href="current/eventmanager.pdf">EventManager Service User's Guide</a><br>
  Contains detailed information on how to configure and use the EventManager service.
  The EventManager service allows you to register with the Event service in order
  to execute STAF Commands. 
  <p>
  <li><a href="current/FSExt.html">FSExt Service User's Guide</a><br>
  Contains detailed information on how to configure and use the FSExt service.
  The FSExt service provides some tools to perform extended file system requests
  (in addition to the FS service provided by core STAF).
  <p>
  <li><a href="current/Http.html">HTTP Service User's Guide</a><br>
  Contains detailed information on how to configure and use the HTTP service.
  The HTTP service allows you to quickly and easily make HTTP requests.
  These requests can be grouped together in a session. Performing requests in
  a session provides the ability simulate a browsing experience. 
  <p>
  <li><a href="current/NamedCounterService.html">NamedCounter Service User's Guide</a><br>
  Contains detailed information on how to configure and use the NamedCounter service.
  The NamedCounter service provides the ability to dynamically manage counters based on a name.
  <p>
  <li><a href="current/Namespace.pdf">Namespace Service User's Guide</a><br>
  Contains detailed information on how to configure and use the Namespace service.
  The Namespace service provides a namespace hierarchy for storing and retrieving
  a persistent repository of variables. 
  <p>
  <li><a href="current/sxe.html">SXE Service User's Guide</a><br>
  Contains detailed information on how to configure and use the SXE service.
  The SXE service provides a simple STAF service to execute a list of STAF
  commands specified in a file. 
  <p>
  <li><a href="current/Timer.html">Timer Service User's Guide</a><br>
  Contains detailed information on how to configure and use the Timer service.
  The Timer service allows a process on one machine to periodically
  receive a notification message from the same or another machine.
  This can be used to have one machine periodically execute a process on
  another machine, as a "heartbeat" to monitor the activity of a machine,
  or any other purpose where a periodic timer may be useful.
</ul>

<h3>STAF Technical Papers</h3>
<ul>
  <li><a href="http://www.research.ibm.com/journal/sj/411/rankin.html">STAF Technical
  Overview</a><br>
  This paper describes how STAF evolved to address the issues of reuse and
  automation for test organizations.  It also describes the design of STAF 
  and how STAF was employed to automate a resource-intensive test suite used
  by an actual testing organization within IBM.
  <p>
  <li><a href="http://www.research.ibm.com/journal/sj/411/williams.html">STCL 
  Test Tools Architecture</a><br>
  This paper discusses the requirements for a Test Tools architecture formulated by
  Software Test Community Leaders (STCL) in IBM.  STAF is discussed in this paper.
 </ul>

<p>

</td>
</tr>

<!-- end of text for page -->

<?php
require "bottom.php";
?>

</body>
</html>
