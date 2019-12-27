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

<center><a name=top><h1>STAF Project Roles</h1></center>

<p>
<font face="arial, helvetica" color="#1e029a" size="4"><b>Overview</b></font>
<p>
The STAF project is a collaborative software development project dedicated
to providing an open source, freely available, multi-platform,
multi-language framework designed around the idea of reusable components,
called services, that provides a testing automation framework with which
you can build your automation solution.  This document describes the
composition of the project and the roles and responsibilities of the 
participants.</p>

<font face="arial, helvetica" color="#1e029a" size="4"><b>Roles in the STAF Project</b></font>
<p>
There are various roles people play in the STAF project.  The more you
contribute, and the higher the quality of your contribution, the more
responsibility you can obtain.</p>

<h3>User</h3>
<p>
<em>Users</em> are the people who use STAF, without contributing code or
documentation to the project.  Users are encouraged to participate through
the forums and mailing lists, asking questions, providing suggestions, and
helping other users.  Users are also encouraged to report problems using
the <a href="http://sourceforge.net/tracker/?group_id=33142&atid=407381">
Bug tracking system</a> and request new features using the
<a href="http://sourceforge.net/tracker/?group_id=33142&atid=407384">
Request For Enhancement (RFE) tracking system</a>.  Anyone can be a user.
</p>

<h3>Contributor</h3>
<p>
A user who contributes back to STAF becomes a <em>contributor</em>.
Contributors are the people who <a href="contributions.php">contribute
enhancements, bug fixes, documentation, or other work</a> that is
incorporated into the system.  A contributor does not have write access
to the source code repository.  Anyone can be a contributor.</p>

<h3>Committer</h3>
<p>
A contributor who gives frequent contributions can be promoted to a
<em>committer</em>.  Committers have write access to (a subset of) the
source code repository, and are developer members of the STAF SourceForge
project.  Committers should only commit code that they have written
themselves (not code obtained from other contributors).  All major
contributions from a committer should be reviewed by one or more core
team members before potential inclusion into the CVS repository.  Part of
the processing of contributions by a committer includes conducting
reasonable due diligence to satisfy themselves that proposed contributions
can be licensed under the terms of the Eclipse Public License (EPL),
Common Public License (CPL), or an equivalent license.</p>
<p>
A contributor can become a committer by the following sequential process:
<ol>
<li>They are nominated by an existing core team member,</li>
<li>At least 2 other core team members support their nomination, and</li>
<li>The STAF Steering Committee approves the nomination by majority vote</li>
</ol>
</p>
<p>
Becoming a committer is a privilege that is earned by contributing and
showing good judgement.  It is a responsibility that should be neither
given nor taken lightly.  Active participation on the forums and mailing
lists is a responsibility of all committers, and is critical to the
success of the project.  Committers are responsible for proactively
reporting problems via the bug tracking system, and annotating problem
reports with status information, explanations, clarifications, or requests
for more information from the submitter.</p>
<p>
At times, committers may go inactive for a variety of reasons.
The project relies on committers who respond to discussions in a
constructive and timely manner.  A committer that is disruptive, does not
participate actively, or has been inactive for an extended period may have
his or her commit status removed by the STAF Steering Committee.</p>

<h3>Core Team Member</h3>
<p>
A committer who gives frequent and valuable contributions can be promoted
to a <em>core team member</em>.  Core team members have write access to
the source code repository, and voting rights allowing them to affect the
future of the project.  The members of the core team are responsible for
virtually all of the day-to-day technical decisions associated with the
project.  They are the gatekeepers, deciding what new code is added to the
system.  All contributions will be processed by one or more core team
members before potential inclusion into the CVS repository.  Part of the
processing of contributions by a core team member includes conducting
reasonable due diligence to satisfy themselves that proposed contributions
can be licensed under the terms of the Eclipse Public License (EPL),
Common Public License (CPL), or an equivalent license.  The core team
member(s) shall use his/her reasonable judgement to determine if the 
contribution can be contributed under the STAF project licensing policy.</p>
<p>
A committer can become a core team member by the following sequential
process:
<ol>
  <li>They are nominated by an existing core team member,</li>
  <li>At least 2 other core team members support their nomination, and</li>
  <li>The STAF Steering Committee approves the nomination by majority vote</li>
</ol>
</p>
<p>
Becoming a core team member is a privilege that is earned by contributing
and showing good judgement.  It is a responsibility that should be neither
given nor taken lightly.  Active participation on the forums and mailing
lists is a responsibility of all core team members, and is critical to the
success of the project.  Core team members are responsible for proactively
reporting problems in the bug tracking system, and annotating problem reports
with status information, explanations, clarifications, or requests for more
information from the submitter.  The core team also ensures that nightly
builds are run on key supported platforms, performs regression testing before
releasing a new version of STAF or one of its services, and opens defects to
track regression test failures.  A subset of the core team does most of this
monitoring, however all core team members are expected to investigate
regression test failures that might have been caused by a source code change
they committed.</p>
<p>
At times, core team members may go inactive for a variety of reasons.
The project relies on active core team members who respond to discussions
in a constructive and timely manner.  A core team member that is disruptive,
does not participate actively, or has been inactive for an extended period
may have his or her commit status removed by the STAF Steering Committee.</p>

<h4>Current Core Team Members</h4>

The STAF project currently contains three subprojects, each with its own
core team.  Individuals are often members of more than one of the subprojects.
Members of the STAF Steering Committee are members <em>ex officio</em> of all
subproject core teams.  In the tables below, Steering Committee members are
only listed on projects where they actively participate as core team members.

<h5>STAF Core Team</h5>

<table>
<tr><td>
<ul>
  <li><a href="mailto:dave2268@users.sourceforge.net">David Bender</a>,
  IBM Austin</li>
  <li><a href="mailto:slucas@users.sourceforge.net">Sharon Lucas</a>,
  IBM Austin</li>
  <li><a href="mailto:crankin_work@users.sourceforge.net">Charles Rankin</a>,
  IBM Austin</li>
</ul>
</td>
</table>

<h5>STAX Core Team</h5>

<table>
<tr><td>
<ul>
  <li><a href="mailto:dave2268@users.sourceforge.net">David Bender</a>,
  IBM Austin</li>
  <li><a href="mailto:slucas@users.sourceforge.net">Sharon Lucas</a>,
  IBM Austin</li>
  <li><a href="mailto:crankin_work@users.sourceforge.net">Charles Rankin</a>,
  IBM Austin</li>
</ul>
</td>
</table>

<h5>Services Core Team</h5>

<table>
<tr><td>
<UL>
  <li><a href="mailto:dave2268@users.sourceforge.net">David Bender</a>,
  IBM Austin</li>
  <li><a href="mailto:slucas@users.sourceforge.net">Sharon Lucas</a>,
  IBM Austin</li>
</ul>
</td>
</table>

<h3>Steering Committee</h3>
<p>
The STAF Steering Committee (SC) is a small group that is responsible
for the strategic direction and success of the project.  This governing
and advisory body is expected to ensure the project's welfare and guide
its overall direction.  The SC is also authorized to create new
subprojects, each with its own core team.</p>
<p>
The initial STAF SC consisted of the founding core team members.
Thereafter, to become a member of the SC, an individual must be nominated
by a member of the SC, and unanimously approved by all SC members.
The goal is to keep the membership of the SC very small. 
In the unlikely event that a member of the SC becomes disruptive to the
process or ceases to contribute for an extended period, the member may be
removed by unanimous vote of the remaining SC members.</p>

<h4>Current Steering Committee</h4>
<table>
<tr><td>
<ul>
  <li><a href="mailto:dave2268@users.sourceforge.net">David Bender</a>,
  IBM Austin</li>
  <li><a href="mailto:slucas@users.sourceforge.net">Sharon Lucas</a>,
  IBM Austin</li>
  <li><a href="mailto:crankin_work@users.sourceforge.net">Charles Rankin</a>,
  IBM Austin</li>
</ul>
</td>
</table>

</td>
</tr>

<!-- end of text for page -->

<?php
require "bottom.php";
?>

</body>
</html>
