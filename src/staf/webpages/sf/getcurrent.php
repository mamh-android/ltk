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

<center><a name=top><h1>Download STAF V3</h1></center>
<p>
The latest releases for STAF V3 can be downloaded from this page.
Earlier releases of STAF can be accessed from the
<a href="http://sourceforge.net/project/showfiles.php?group_id=33142">
"See All Releases"</a> page.
<p>
Note that we no longer support STAF V2.  But, you can download the latest
releases for STAF V2 from <a href="getcurrent2x.php">Download STAF V2"</a>.
<p>
See the <a href="history.php">STAF V3 History</a> page for STAF V3
history information.
<br>
See the <a href="current/STAFOverview.html">STAF V3 Overview</a>
if you are migrating from STAF V2 to get more information on major
new enhancements in STAF V3.
<br>
See the <a href="current/STAFGS.pdf">Getting Started with STAF V3 Guide</a>
for more information about how to get started using STAF V3.
<br>
See the <a href="latest.php">Programmatically retrieving the latest STAF
releases</a> page for more information about how to programmatically retrieve the
latest STAF releases.
<p>
<h3>Binaries</h3>
<p>
The recommended installation method for STAF is to use InstallAnywhere (IA).
However, if we haven't yet provided an InstallAnywhere file, use the GNU zipped
or compressed tar file, if provided, to do a STAFInst install.
<p>
See the <a href="current/STAFInstall.pdf">STAF V3 Installation Guide</a> for more
detailed instructions for installing STAF V3.
<p>
The latest STAF V3 release is STAF V3.4.15.  Files which are not at the latest
release level are marked with an asterisk (*).
<!--
Files which are not at the latest release level are marked with an
asterisk (*).
-->
<p>
Note that AMD64 is also commonly known as x86-64 or x64 or Opteron 64-bit.
AMD64 should not be confused with the Intel Itanium architecture, also known as IA-64,
which is not compatible on the native instruction set level with the AMD64 architecture.
It should also not be confused with the PowerPC 64-bit architecture, also known as PPC64.
<p>
<table border="1">
<tr>
  <td><b>Operating System (Architecture)</td>
  <td><b>Java<br>Support</b>
  <td><b>Release</b></td>
  <td><b>File</b></td>
  <td><b>File format</b></td>
  <td><b>File size</b></td>
</tr>
<tr>
  <td>Windows XP, Vista, Windows 7/8,<br>Windows Server 2003/2008/2008R2 (32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-win32.exe?download">STAF3415-setup-win32.exe</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>89M</td>
</tr>
<tr>
  <td>Windows XP, Vista, Windows 7/8,<br>Windows Server 2003/2008/2008R2 (32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-win32-NoJVM.exe?download">STAF3415-setup-win32-NoJVM.exe</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>18M</td>
</tr>
<tr>
  <td>Windows Server 2003/2008/2008R2/2012, <br>Vista, Windows 7/8 (AMD64, aka x64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-winamd64.exe?download">STAF3415-setup-winamd64.exe</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>105M</td>
</tr>
<tr>
  <td>Windows Server 2003/2008/2008R2/2012, <br>Vista, Windows 7/8 (AMD64, aka x64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-winamd64-NoJVM.exe?download">STAF3415-setup-winamd64-NoJVM.exe</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>23M</td>
</tr>
<!--
<tr>
  <td>Windows Server 2003/2008 (IA64)</td>
  <td>64-bit</td>
  <td>3.4.14*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3414-setup-win64.exe?download">STAF3414-setup-win64.exe</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>56M</td>
</tr>
<tr>
  <td>Windows Server 2003/2008 (IA64)</td>
  <td>64-bit</td>
  <td>3.4.14*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3414-setup-win64-NoJVM.exe?download">STAF3414-setup-win64-NoJVM.exe</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>19M</td>
</tr>
-->
<tr>
  <td>Linux (Intel 32-bit, aka i386 or x86-32)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux.bin?download">STAF3415-setup-linux.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>85M</td>
</tr>
<tr>
  <td>Linux (Intel 32-bit, aka i386 or x86-32)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-NoJVM.bin?download">STAF3415-setup-linux-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>25M</td>
</tr>
<tr>
  <td>Linux (Intel 32-bit, aka i386 or x86-32)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-linux.tar.gz?download">STAF3415-linux.tar.gz</a>
  <td>GNU zipped tar</td>
  <td>7.2M</td>
</tr>
<tr>
  <td>Linux (AMD64, aka x86-64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-amd64.bin?download">STAF3415-setup-linux-amd64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>91M</td>
</tr>
<tr>
  <td>Linux (AMD64, aka x86-64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-amd64-NoJVM.bin?download">STAF3415-setup-linux-amd64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>25M</td>
</tr>
<tr>
  <td>Linux (AMD64, aka x86-64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-linux-amd64.tar.gz?download">STAF3415-linux-amd64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>7.2M</td>
<tr>
  <td>Linux (PPC64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-ppc64-32.bin?download">STAF3415-setup-linux-ppc64-32.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>86M</td>
</tr>
<tr>
  <td>Linux (PPC64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-ppc64-32-NoJVM.bin?download">STAF3415-setup-linux-ppc64-32-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>25M</td>
</tr>
<tr>
  <td>Linux (PPC64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-linux-ppc64-32.tar.gz?download">STAF3415-linux-ppc64-32.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>7.0M</td>
</tr>
<tr>
  <td>Linux (PPC64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-ppc64-64.bin?download">STAF3415-setup-linux-ppc64-64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>96M</td>
</tr>
<tr>
  <td>Linux (PPC64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-linux-ppc64-64-NoJVM.bin?download">STAF3415-setup-linux-ppc64-64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>27M</td>
</tr>
<tr>
  <td>Linux (PPC64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-linux-ppc64-64.tar.gz?download">STAF3415-linux-ppc64-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>7.2M</td>
</tr>
<!--
<tr>
  <td>Linux (IA64)</td>
  <td>64-bit</td>
  <td>3.4.1*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF341-setup-linux-ia64.bin?download">STAF341-setup-linux-ia64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>84M</td>
</tr>
<tr>
  <td>Linux (IA64)</td>
  <td>64-bit</td>
  <td>3.4.1*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF341-setup-linux-ia64-NoJVM.bin?download">STAF341-setup-linux-ia64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>27M</td>
</tr>
<tr>
  <td>Linux (IA64)</td>
  <td>64-bit</td>
  <td>3.4.1*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF341-linux-ia64.tar.gz?download">STAF341-linux-ia64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>7.6M</td>
</tr>
 -->
<tr>
  <td>Linux SLES10+/RHEL5+ on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-zlinux-32.bin?download">STAF3415-setup-zlinux-32.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>81M</td>
</tr>
<tr>
  <td>Linux SLES10+/RHEL5+ on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-zlinux-32-NoJVM.bin?download">STAF3415-setup-zlinux-32-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>24M</td>
</tr>
<tr>
  <td>Linux SLES10+/RHEL5+ on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-zlinux-32.tar.gz?download">STAF3415-zlinux-32.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>6.7M</td>
</tr>
<tr>
  <td>Linux SLES10+/RHEL5+ on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-zlinux-64.bin?download">STAF3415-setup-zlinux-64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>88M</td>
</tr>
<tr>
  <td>Linux SLES10+/RHEL5+ on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-zlinux-64-NoJVM.bin?download">STAF3415-setup-zlinux-64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>25M</td>
</tr>
<tr>
  <td>Linux SLES10+/RHEL5+ on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-zlinux-64.tar.gz?download">STAF3415-zlinux-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>6.9M</td>
</tr>
<!--
<tr>
  <td>Linux SLES8 on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.4.0</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF340-setup-zlinux-32.bin?download">STAF340-setup-zlinux-32.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>65M</td>
</tr>
<tr>
  <td>Linux SLES8 on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.4.0</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF340-setup-zlinux-32-NoJVM.bin?download">STAF340-setup-zlinux-32-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>20M</td>
</tr>
<tr>
  <td>Linux SLES8 on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.4.0</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF340-zlinux-32.tar.gz?download">STAF340-zlinux-32.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>6.3M</td>
</tr>
<tr>
  <td>Linux SLES8 on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.0</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF340-setup-zlinux-64.bin?download">STAF340-setup-zlinux-64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>66M</td>
</tr>
<tr>
  <td>Linux SLES8 on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.0</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF340-setup-zlinux-64-NoJVM.bin?download">STAF340-setup-zlinux-64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>22M</td>
</tr>
<tr>
  <td>Linux SLES8 on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.0</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF340-zlinux-64.tar.gz?download">STAF340-zlinux-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>6.6M</td>
</tr>
<tr>
  <td>Linux SLES9 on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.1.5.1*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF315-zlinux-sles9-32.tar.gz?download">STAF315-zlinux-sles9-32.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>5.1M</td>
</tr>
<tr>
  <td>Linux SLES9 on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.2.0*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF320-zlinux-sles9-64.tar.gz?download">STAF320-zlinux-sles9-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>5.7M</td>
</tr>
<tr>
  <td>Linux SLES10 on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.2.0*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF320-zlinux-sles10-64.tar.gz?download">STAF320-zlinux-sles10-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>5.1M</td>
</tr>
<tr>
  <td>Linux RHEL4 on zSeries (31-bit)</td>
  <td>31-bit</td>
  <td>3.1.5.1*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF315-zlinux-rhel4-32.tar.gz?download">STAF315-zlinux-rhel4-32.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>5.0M</td>
</tr>
<tr>
  <td>Linux RHEL4 on zSeries (64-bit)</td>
  <td>64-bit</td>
  <td>3.2.0*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF320-zlinux-rhel4-64.tar.gz?download">STAF320-zlinux-rhel4-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>5.4M</td>
</tr>
-->
<tr>
  <td>z/OS UNIX 1.4+ (32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-zos.tar.Z?download">STAF3415-zos.tar.Z</a></td>
  <td>Unix compressed file</td>
  <td>12M</td>
</tr>
<tr>
  <td>z/OS UNIX 1.4+ (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-zos64.tar.Z?download">STAF3415-zos64.tar.Z</a></td>
  <td>Unix compressed file</td>
  <td>11M</td>
</tr>
<tr>
  <td>Solaris 10+ (Sparc 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-sparc.bin?download">STAF3415-setup-solaris-sparc.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>122M</td>
</tr>
<tr>
  <td>Solaris 10+ (Sparc 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-sparc-NoJVM.bin?download">STAF3415-setup-solaris-sparc-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>41M</td>
</tr>
<tr>
  <td>Solaris 10+ (Sparc 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-solaris-sparc.tar.gz?download">STAF3415-solaris-sparc.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>13M</td>
</tr>
<tr>
  <td>Solaris 10+ (Sparc 64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-sparc64.bin?download">STAF3415-setup-solaris-sparc64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>145M</td>
</tr>
<tr>
  <td>Solaris 10+ (Sparc 64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-sparc64-NoJVM.bin?download">STAF3415-setup-solaris-sparc64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>51M</td>
</tr>
<tr>
  <td>Solaris 10+ (Sparc 64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-solaris-sparc64.tar.gz?download">STAF3415-solaris-sparc64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>17M</td>
</tr>
<tr>
  <td>Solaris 10+ (AMD Opteron 64-bit, aka x64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-x64.bin?download">STAF3415-setup-solaris-x64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>129M</td>
</tr>
<tr>
  <td>Solaris 10+ (AMD Opteron 64-bit, aka x64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-x64-NoJVM.bin?download">STAF3415-setup-solaris-x64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>46M</td>
</tr>
<tr>
  <td>Solaris 10+ (AMD Opteron 64-bit, aka x64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-solaris-x64.tar.gz?download">STAF3415-solaris-x64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>14M</td>
</tr>
<tr>
  <td>Solaris 10+ (AMD Opteron 64-bit, aka x64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-x64-64.bin?download">STAF3415-setup-solaris-x64-64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>130M</td>
</tr>
<tr>
  <td>Solaris 10+ (AMD Opteron 64-bit, aka x64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-x64-64-NoJVM.bin?download">STAF3415-setup-solaris-x64-64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>47M</td>
</tr>
<tr>
  <td>Solaris 10+ (AMD Opteron 64-bit, aka x64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-solaris-x64-64.tar.gz?download">STAF3415-solaris-x64-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>14M</td>
</tr>
<tr>
  <td>Solaris 10+ (x86 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-x86.bin?download">STAF3415-setup-solaris-x86.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>100</td>
</tr>
<tr>
  <td>Solaris 10+ (x86 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-solaris-x86-NoJVM.bin?download">STAF3415-setup-solaris-x86-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>46M</td>
</tr>
<tr>
  <td>Solaris 10+ (x86 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-solaris-x86.tar.gz?download">STAF3415-solaris-x86.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>14M</td>
</tr>
<tr>
  <td>AIX 6.1+ (32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-aix.bin?download">STAF3415-setup-aix.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>105M</td>
</tr>
<tr>
  <td>AIX 6.1+ (32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-aix-NoJVM.bin?download">STAF3415-setup-aix-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>37M</td>
</tr>
<tr>
  <td>AIX 6.1+ (32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-aix.tar.gz?download">STAF3415-aix.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>11M</td>
</tr>
<tr>
  <td>AIX 6.1+ (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-aix64.bin?download">STAF3415-setup-aix64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>114M</td>
</tr>
<tr>
  <td>AIX 6.1+ (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-aix64-NoJVM.bin?download">STAF3415-setup-aix64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>38M</td>
</tr>
<tr>
  <td>AIX 6.1+ (64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-aix64.tar.gz?download">STAF3415-aix64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>11M</td>
</tr>
<!--
<tr>
  <td>AIX 4.3.3.0+ (32-bit, no IPv6 support)</td>
  <td>32-bit</td>
  <td>3.3.0*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF330-aix433.tar.gz?download">STAF330-aix433.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>9.5</td>
</tr>
-->
<tr>
  <td>HP-UX 11.11+ (PA-RISC 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux.bin?download">STAF3415-setup-hpux.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>88M</td>
<tr>
  <td>HP-UX 11.11+ (PA-RISC 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-NoJVM.bin?download">STAF3415-setup-hpux-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>31M</td>
<tr>
  <td>HP-UX 11.11+ (PA-RISC 32-bit)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-hpux.tar.gz?download">STAF3415-hpux.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>8.1M</td>
</tr>
<tr>
  <td>HP-UX 11.11+ (PA-RISC 64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-parisc64.bin?download">STAF3415-setup-hpux-parisc64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>107M</td>
<tr>
  <td>HP-UX 11.11+ (PA-RISC 64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-parisc64-NoJVM.bin?download">STAF3415-setup-hpux-parisc64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>33M</td>
<tr>
  <td>HP-UX 11.11+ (PA-RISC 64-bit)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-hpux-parisc64.tar.gz?download">STAF3415-hpux-parisc64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>8.0M</td>
</tr>
<tr>
  <td>HP-UX 11.31+ (IA64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-ia64-32.bin?download">STAF3415-setup-hpux-ia64-32.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>126M</td>
<tr>
  <td>HP-UX 11.31+ (IA64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-ia64-32-NoJVM.bin?download">STAF3415-setup-hpux-ia64-32-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>51M</td>
<tr>
  <td>HP-UX 11.31+ (IA64)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-hpux-ia64-32.tar.gz?download">STAF3415-hpux-ia64-32.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>12M</td>
</tr>
<tr>
  <td>HP-UX 11.31+ (IA64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-ia64-64.bin?download">STAF3415-setup-hpux-ia64-64.bin</a></td>
  <td>InstallAnywhere (Bundled JVM)</td>
  <td>175M</td>
<tr>
  <td>HP-UX 11.31+ (IA64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-hpux-ia64-64-NoJVM.bin?download">STAF3415-setup-hpux-ia64-64-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>53M</td>
<tr>
  <td>HP-UX 11.31+ (IA64)</td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-hpux-ia64-64.tar.gz?download">STAF3415-hpux-ia64-64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>12M</td>
</tr>
<tr>
  <td>IBM i 7.1+ (32-bit),<br>
      previously known as i5/OS, OS/400<br>
      Runs in PASE, an AIX runtime environment
  </td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-aix.tar.gz?download">STAF3415-aix.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>11M</td>
</tr>
<tr>
  <td>IBM i 7.1+ (64-bit),<br>
      previously known as i5/OS, OS/400<br>
      Runs in PASE, an AIX runtime environment
  </td>
  <td>64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-aix64.tar.gz?download">STAF3415-aix64.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>11M</td>
</tr>
<tr>
  <td>FreeBSD 7.4+ (i386)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-freebsd-NoJVM.bin?download">STAF3415-setup-freebsd-NoJVM.bin</a></td>
  <td>InstallAnywhere (No JVM)</td>
  <td>25M</td>
<tr>
  <td>FreeBSD 7.4+ (i386)</td>
  <td>32-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-freebsd.tar.gz?download">STAF3415-freebsd.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>6.9M</td>
</tr>
<tr>
  <td>Mac OS X 10.6+ (Universal binary for i386,<br>x86_64, and ppc)</td>
  <td>32/64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-macosx-universal.bin?download">STAF3415-setup-macosx-universal.bin</a></td>
  <td>InstallAnywhere (No JVM) executable</td>
  <td>150M</td>
</tr>
<tr>
  <td>Mac OS X 10.6+ (Universal binary for i386,<br>x86_64, and ppc)</td>
  <td>32/64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-setup-macosx-universal.zip?download">STAF3415-setup-macosx-universal.zip</a></td>
  <td>InstallAnywhere (No JVM) zip</td>
  <td>21M</td>
</tr>
<tr>
  <td>Mac OS X 10.6+ (Universal binary for i386,<br>x86_64, and ppc)</td>
  <td>32/64-bit</td>
  <td>3.4.15</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-macosx-universal.tar.gz?download">STAF3415-macosx-universal.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>14M</td>
</tr>
<!--
<tr>
  <td>Mac OS X 10.4+ (i386)</td>
  <td>32-bit</td>
  <td>3.4.10*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3410-setup-macosx-i386.bin?download">STAF3410-setup-macosx-i386.bin</a></td>
  <td>InstallAnywhere (No JVM) executable</td>
  <td>47M</td>
</tr>
<tr>
  <td>Mac OS X 10.4+ (i386)</td>
  <td>32-bit</td>
  <td>3.4.10*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3410-setup-macosx-i386.zip?download">STAF3410-setup-macosx-i386.zip</a></td>
  <td>InstallAnywhere (No JVM) zip</td>
  <td>17M</td>
</tr>
<tr>
  <td>Mac OS X 10.4+ (i386)</td>
  <td>32-bit</td>
  <td>3.4.10*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3410-macosx-i386.tar.gz?download">STAF3410-macosx-i386.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>12M</td>
</tr>
<tr>
  <td>Mac OS X 10.4+ (PPC)</td>
  <td>32-bit</td>
  <td>3.4.10*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3410-setup-macosx-ppc.bin?download">STAF3410-setup-macosx-ppc.bin</a></td>
  <td>InstallAnywhere (No JVM) executable</td>
  <td>47M</td>
</tr>
<tr>
  <td>Mac OS X 10.4+ (PPC)</td>
  <td>32-bit</td>
  <td>3.4.10*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3410-setup-macosx-ppc.zip?download">STAF3410-setup-macosx-ppc.zip</a></td>
  <td>InstallAnywhere (No JVM) zip</td>
  <td>17M</td>
</tr>
<tr>
  <td>Mac OS X 10.4+ (PPC)</td>
  <td>32-bit</td>
  <td>3.4.10*</td>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3410-macosx-ppc.tar.gz?download">STAF3410-macosx-ppc.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>12M</td>
</tr>
-->
</table>

<p>
<h3>Sample Authenticator</h3>

<table border="1">
<tr><td><b>File</b></td><td><b>File Format</b></td><td><b>File Size</b></td></tr>
<tr>
  <td><a href="http://prdownloads.sourceforge.net/staf/AuthSampleV300.jar?download">AuthSampleV300.jar</a></td>
  <td>Jar file</td>
  <td>4K</td>
</tr>
<tr>
</table>

<p>
<h3>Source code</h3>

<table border="1">
<tr><td><b>File</b></td><td><b>File Format</b></td><td><b>File Size</b></td></tr>
<tr>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-src.tar.gz?download">STAF3415-src.tar.gz</a></td>
  <td>GNU zipped tar</td>
  <td>33M</td>
</tr>
<tr>
  <td><a href="http://prdownloads.sourceforge.net/staf/STAF3415-src.zip?download">STAF3415-src.zip</a></td>
  <td>Zip file</td>
  <td>35M</td>
</tr>
<tr>
</table>
<p>

<p>
<h3>Documentation</h3>

<table border="1">
<tr><td><b>Document</b></td><td><b>File</b></td></tr>
<tr>
  <td><a href="current/STAFGS.pdf">Getting Started with STAF V3</a></td>
  <td>STAFGS.pdf</td>
</tr>
<tr>
  <td><a href="current/STAFV30Overview.ppt">STAF V3 Overview Presentation</a></td>
  <td>STAFV30Overview.ppt</td>
</tr>
<tr>
  <td><a href="current/stafmigrate.html">STAF V3 Migration Guide</a></td>
  <td>stafmigrate.html</td>
</tr>
<tr>
  <td><a href="current/STAFInstall.pdf">STAF V3 Installation Guide</a></td>
  <td>STAFInstall.pdf</td>
</tr>
<tr>
  <td><a href="current/STAFUG.htm">STAF V3 User's Guide</a></td>
  <td>STAFUG.htm</td>
</tr>
<tr>
  <td><a href="current/STAFFAQ.htm">STAF V3 Frequently Asked Questions</a></td>
  <td>STAFFAQ.htm</td>
</tr>
<tr>
  <td><a href="current/STAFCMDS.htm">STAF V3 Service Command Reference</a></td>
  <td>STAFCMDS.htm</td>
</tr>
<tr>
  <td><a href="current/STAFRC.htm">STAF V3 API Return Code Reference</a></td>
  <td>STAFRC.htm</td>
</tr>
<tr>
  <td><a href="current/stafdg.html">STAF V3 Developer's Guide</a></td>
  <td>stafdg.html</td>
</tr>
<tr>
  <td><a href="current/stafsdg.html">STAF V3 Service Developer's Guide</a></td>
  <td>stafsdg.html</td>
</tr>
<tr>
  <td><a href="current/STAFJava.htm">STAF V3 Java User's Guide (HTML)</a></td>
  <td>STAFJava.htm</td>
</tr>
<tr>
  <td><a href="current/STAFPerl.htm">STAF V3 Perl User's Guide (HTML)</a></td>
  <td>STAFPerl.htm</td>
</tr>
<tr>
  <td><a href="current/STAFPython.htm">STAF V3 Python User's Guide (HTML)</a></td>
  <td>STAFPython.htm</td>
</tr>
<tr>
  <td><a href="current/STAFTcl.htm">STAF V3 Tcl User's Guide (HTML)</a></td>
  <td>STAFTcl.htm</td>
</tr>
<tr>
  <td><a href="current/STAFAnt.htm">STAF V3 Ant Task User's Guide (HTML)</a></td>
  <td>STAFAnt.htm</td>
</tr>
</table>
<p>

<!-- end of text for page -->

<?php
require "bottom.php";
?>

</body>
</html>

