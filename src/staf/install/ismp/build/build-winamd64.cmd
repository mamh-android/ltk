
staf local monitor log message "Starting to execute build-win32.cmd"

del C:\STAF_ISMP_InputFiles\build\*.* /s /q
rmdir C:\STAF_ISMP_InputFiles\build /s /q

staf local monitor log message "Adding required empty directories"

REM add required empty LINUX build directories
md C:\STAF_ISMP_InputFiles\build\bin\linux
md C:\STAF_ISMP_InputFiles\build\lib\base\linux
md C:\STAF_ISMP_InputFiles\build\lib-ipv4\linux
md C:\STAF_ISMP_InputFiles\build\lib-ipv6\linux
md C:\STAF_ISMP_InputFiles\build\codepage\linux
md C:\STAF_ISMP_InputFiles\build\lib-log\linux
md C:\STAF_ISMP_InputFiles\build\lib-mon\linux
md C:\STAF_ISMP_InputFiles\build\lib-pool\linux
md C:\STAF_ISMP_InputFiles\build\lib-zip\linux
md C:\STAF_ISMP_InputFiles\build\include\linux
md C:\STAF_ISMP_InputFiles\build\lib-java\linux
md C:\STAF_ISMP_InputFiles\build\lib-jstaf\linux
md C:\STAF_ISMP_InputFiles\build\lib-rexx\linux
md C:\STAF_ISMP_InputFiles\build\lib-tcl\linux
md C:\STAF_ISMP_InputFiles\build\lib-python\linux
md C:\STAF_ISMP_InputFiles\build\bin-perl\linux
md C:\STAF_ISMP_InputFiles\build\lib-perl58\linux
md C:\STAF_ISMP_InputFiles\build\lib-perl56\linux
md C:\STAF_ISMP_InputFiles\build\lib-perl50\linux
md C:\STAF_ISMP_InputFiles\build\codepage\linux-optional

REM add required empty AIX build directories
md C:\STAF_ISMP_InputFiles\build\bin\aix
md C:\STAF_ISMP_InputFiles\build\lib\base\aix
md C:\STAF_ISMP_InputFiles\build\lib-ipv4\aix
md C:\STAF_ISMP_InputFiles\build\lib-ipv6\aix
md C:\STAF_ISMP_InputFiles\build\codepage\aix
md C:\STAF_ISMP_InputFiles\build\lib-log\aix
md C:\STAF_ISMP_InputFiles\build\lib-mon\aix
md C:\STAF_ISMP_InputFiles\build\lib-pool\aix
md C:\STAF_ISMP_InputFiles\build\lib-zip\aix
md C:\STAF_ISMP_InputFiles\build\include\aix
md C:\STAF_ISMP_InputFiles\build\lib-java\aix
md C:\STAF_ISMP_InputFiles\build\lib-jstaf\aix
md C:\STAF_ISMP_InputFiles\build\lib-rexx\aix
md C:\STAF_ISMP_InputFiles\build\codepage\aix-optional

REM add required empty Solaris build directories
md C:\STAF_ISMP_InputFiles\build\bin\solaris
md C:\STAF_ISMP_InputFiles\build\lib\base\solaris
md C:\STAF_ISMP_InputFiles\build\lib-ipv4\solaris
md C:\STAF_ISMP_InputFiles\build\lib-ipv6\solaris
md C:\STAF_ISMP_InputFiles\build\codepage\solaris
md C:\STAF_ISMP_InputFiles\build\lib-log\solaris
md C:\STAF_ISMP_InputFiles\build\lib-mon\solaris
md C:\STAF_ISMP_InputFiles\build\lib-pool\solaris
md C:\STAF_ISMP_InputFiles\build\lib-zip\solaris
md C:\STAF_ISMP_InputFiles\build\include\solaris
md C:\STAF_ISMP_InputFiles\build\lib-java\solaris
md C:\STAF_ISMP_InputFiles\build\lib-jstaf\solaris
md C:\STAF_ISMP_InputFiles\build\codepage\solaris-optional

REM add required empty hpux build directories
md C:\STAF_ISMP_InputFiles\build\bin\hpux
md C:\STAF_ISMP_InputFiles\build\lib\base\hpux
md C:\STAF_ISMP_InputFiles\build\lib-ipv4\hpux
md C:\STAF_ISMP_InputFiles\build\lib-ipv6\hpux
md C:\STAF_ISMP_InputFiles\build\codepage\hpux
md C:\STAF_ISMP_InputFiles\build\lib-log\hpux
md C:\STAF_ISMP_InputFiles\build\lib-mon\hpux
md C:\STAF_ISMP_InputFiles\build\lib-pool\hpux
md C:\STAF_ISMP_InputFiles\build\lib-zip\hpux
md C:\STAF_ISMP_InputFiles\build\include\hpux
md C:\STAF_ISMP_InputFiles\build\lib-java\hpux
md C:\STAF_ISMP_InputFiles\build\lib-jstaf\hpux
md C:\STAF_ISMP_InputFiles\build\codepage\hpux-optional

staf local monitor log message "Copying win32 files to input file directory"

md C:\STAF_ISMP_InputFiles\build
md C:\STAF_ISMP_InputFiles\build\bin\win32

REM BIN WIN32 files
REM copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAF.dll C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\LICENSE.htm C:\STAF_ISMP_InputFiles\build\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFLIPC.dll C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
REM copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFOpenSSL.dll C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
REM copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFTCP.dll C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFDSLS.dll C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFEXECPROXY.dll C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAF.exe C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFProc.ico C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFProc.exe C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFReg.exe C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFLoop.exe C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFExecProxy.exe C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\FmtLog.exe C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFDefault.crt C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFDefault.key C:\STAF_ISMP_InputFiles\build\bin\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\CAList.crt C:\STAF_ISMP_InputFiles\build\bin\win32\*.*

REM BIN WIN32 IPv-specific files
md C:\STAF_ISMP_InputFiles\build\bin-ipv4\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\IPv4\STAF.dll C:\STAF_ISMP_InputFiles\build\bin-ipv4\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\IPv4\STAFTCP.dll C:\STAF_ISMP_InputFiles\build\bin-ipv4\win32\*.*
md C:\STAF_ISMP_InputFiles\build\bin-ipv6\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\IPv6\STAF.dll C:\STAF_ISMP_InputFiles\build\bin-ipv6\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\IPv6\STAFTCP.dll C:\STAF_ISMP_InputFiles\build\bin-ipv6\win32\*.*

REM LIB BASE WIN32 files
md C:\STAF_ISMP_InputFiles\build\lib\base\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\STAF.lib C:\STAF_ISMP_InputFiles\build\lib\base\win32\*.*

REM CODEPAGE WIN32 files (need to always copy all files for a workaround)
md C:\STAF_ISMP_InputFiles\build\codepage\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\codepage\*.* C:\STAF_ISMP_InputFiles\build\codepage\win32\*.*

REM Log Service WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-log\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFLog.dll C:\STAF_ISMP_InputFiles\build\bin-log\win32\*.*

REM Monitor Service WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-mon\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFMon.dll C:\STAF_ISMP_InputFiles\build\bin-mon\win32\*.*

REM Respool service WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-pool\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFPool.dll C:\STAF_ISMP_InputFiles\build\bin-pool\win32\*.*

REM Zip service WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-zip\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFZip.dll C:\STAF_ISMP_InputFiles\build\bin-zip\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFZlib.dll C:\STAF_ISMP_InputFiles\build\bin-zip\win32\*.*

REM C++ WIN32 files
md C:\STAF_ISMP_InputFiles\build\include\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFOSTypes.h C:\STAF_ISMP_InputFiles\build\include\win32\*.*
md C:\STAF_ISMP_InputFiles\build\include\common
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAF.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAF_fstream.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAF_iostream.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFDataTypes.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFDataTypesInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFDynamicLibrary.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFDynamicLibraryInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFError.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFEventSem.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFEventSemInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFException.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFMutexSem.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFMutexSemInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFRefPtr.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFString.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFStringInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFThread.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFTimestamp.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFTimestampInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFUtil.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFFileSystem.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFFileSystemInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFLogService.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFMonitorService.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFResPoolService.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFProcess.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFProcessInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFTrace.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFTraceInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*

REM JAVA BIN WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-java\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\JSTAF.dll C:\STAF_ISMP_InputFiles\build\bin-java\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\JSTAFSH.dll C:\STAF_ISMP_InputFiles\build\bin-java\win32\*.*

REM JSTAF BIN WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-jstaf\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\JSTAF.zip C:\STAF_ISMP_InputFiles\build\bin-jstaf\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\JSTAF.jar C:\STAF_ISMP_InputFiles\build\bin-jstaf\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFAnt.jar C:\STAF_ISMP_InputFiles\build\bin-jstaf\win32\*.*

REM REXX BIN WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-rexx\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\RXSTAF.dll C:\STAF_ISMP_InputFiles\build\bin-rexx\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\RxThread.dll C:\STAF_ISMP_InputFiles\build\bin-rexx\win32\*.*

REM REXX LIB WIN32 files
md C:\STAF_ISMP_InputFiles\build\lib-rexx\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\STAFCPar.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\STAFLog.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\STAFMon.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\STAFPool.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\STAFUtil.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\win32\*.*


REM TCL BIN WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-tcl\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\pkgIndex.tcl C:\STAF_ISMP_InputFiles\build\bin-tcl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\bin\staf\STAF.tcl C:\STAF_ISMP_InputFiles\build\bin-tcl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\bin\staf\STAFLog.tcl C:\STAF_ISMP_InputFiles\build\bin-tcl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\bin\staf\STAFMon.tcl C:\STAF_ISMP_InputFiles\build\bin-tcl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\bin\staf\STAFUtil.tcl C:\STAF_ISMP_InputFiles\build\bin-tcl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\bin\staf\TCLSTAF.dll C:\STAF_ISMP_InputFiles\build\bin-tcl\win32\*.*

REM TCL DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common-tcl
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFTcl.htm C:\STAF_ISMP_InputFiles\build\docs\common-tcl\*.*

REM PERL BIN WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-perl\win32
md C:\STAF_ISMP_InputFiles\build\lib-perl56\win32
md C:\STAF_ISMP_InputFiles\build\lib-perl58\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\PLSTAF.pm C:\STAF_ISMP_InputFiles\build\bin-perl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFLog.pm C:\STAF_ISMP_InputFiles\build\bin-perl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAFMon.pm C:\STAF_ISMP_InputFiles\build\bin-perl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAF.pl C:\STAF_ISMP_InputFiles\build\bin-perl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\STAF2.pl C:\STAF_ISMP_InputFiles\build\bin-perl\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\perl56\PLSTAF.dll C:\STAF_ISMP_InputFiles\build\lib-perl56\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\perl58\PLSTAF.dll C:\STAF_ISMP_InputFiles\build\lib-perl58\win32\*.*

REM PERL DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common-perl
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFPerl.htm C:\STAF_ISMP_InputFiles\build\docs\common-perl\*.*

REM PYTHON BIN WIN32 files
md C:\STAF_ISMP_InputFiles\build\bin-python\win32
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\PYSTAF.pyd C:\STAF_ISMP_InputFiles\build\bin-python\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\PySTAFLog.py C:\STAF_ISMP_InputFiles\build\bin-python\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\PySTAFMon.py C:\STAF_ISMP_InputFiles\build\bin-python\win32\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\bin\PySTAF.py C:\STAF_ISMP_InputFiles\build\bin-python\win32\*.*

REM PYTHON DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common-python
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFPython.htm C:\STAF_ISMP_InputFiles\build\docs\common-python\*.*

REM DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\History C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFCMDS.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFFAQ.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFHome.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFRC.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFUG.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFGS.pdf C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\docs\STAFDiag.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*

REM SAMPLES/DEMOS COMMON files
md C:\STAF_ISMP_InputFiles\build\samples\common
xcopy /s C:\STAF_ISMP_InputFiles\win32\staf\samples\*.* C:\STAF_ISMP_InputFiles\build\samples\common\*.*

REM SERVICE DEV INCLUDE COMMON files
md C:\STAF_ISMP_InputFiles\build\include\service-dev\common
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFCommandParser.h C:\STAF_ISMP_InputFiles\build\include\service-dev\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFCommandParserInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\service-dev\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\staf\include\STAFServiceInterface.h C:\STAF_ISMP_InputFiles\build\include\service-dev\common\*.*

REM SERVICE DEV LIB COMMON files
md C:\STAF_ISMP_InputFiles\build\lib\service-dev\common
copy C:\STAF_ISMP_InputFiles\win32\staf\lib\service.def C:\STAF_ISMP_InputFiles\build\lib\service-dev\common\*.*

REM OPTIONAL CODEPAGES WIN32 - Note on Windows all codepages are already copied to target system
REM md C:\STAF_ISMP_InputFiles\build\codepage\win32-optional
REM copy C:\STAF_ISMP_InputFiles\win32\staf\codepage\*.* C:\STAF_ISMP_InputFiles\build\codepage\win32-optional\*.*

staf local monitor log message "Starting the win32 ISMP build"

cd C:\IS11.5MP
InstallShieldMultiPlatformCommandLineBuild.exe "C:\IS11.5MP\Projects\STAF\STAF.uip" -build
REM cd C:\InstallShieldX\Universal Installer
REM InstallShieldUniversalCommandLineBuild.exe "C:\InstallShieldX\Universal Installer\Projects\STAF\STAF.uip" -build
REM cd C:\ISMP503
REM java -cp .;C:\ISMP503\lib\idewizards.jar;C:\ISMP503\lib\ProjectWizard.jar;C:\ISMP503\ppk\win32ppk.jar;C:\ISMP503\ppk\linuxppk.jar;C:\ISMP503\ppk\solarisppk.jar;C:\ISMP503\ppk\hpuxppk.jar;C:\ISMP503\ppk\aixppk.jar;C:\ISMP503\ppk\os2ppk.jar;C:\ISMP503\ppk\cimppk.jar;C:\ISMP503\ppk\as400ppk.jar;C:\ISMP503\ppk\webppk.jar;C:\ISMP503\classes;C:\ISMP503\classes\MyCustomBeans.jar;C:\ISMP503\lib\ide.jar;C:\ISMP503\lib\wizard.jar;C:\ISMP503\lib\product.jar;C:\ISMP503\lib\platform.jar;C:\ISMP503\lib\help.jar;C:\ISMP503\lib\swing.jar;C:\ISMP503\lib\jhall.jar;C:\ISMP503\lib\parser.jar;C:\ISMP503\lib\xt.jar;C:\ISMP503\lib\icebrowserbean.jar;C:\ISMP503\lib\icebrowserlitebean.jar;C:\ISMP503\ppk\macosxppk.jar;C:\ISMP503\ppk\genericunixppk.jar;C:\ISMP503\i18n com.installshield.isje.ISJE .\Projects\STAF\STAF.xml -build

staf local monitor log message "Copying the win32 ISMP binaries to the STAF_ISMP_OutputFiles\disk1 directory"

cd C:\STAF_ISMP_InputFiles

copy C:\STAF_ISMP_OutputFiles\disk1\STAF324-setup-win32.exe C:\STAF_Installers\winamd64\STAF324-setup-winamd64.exe
copy C:\STAF_ISMP_OutputFiles\disk1\STAF324-setup.jar C:\STAF_Installers\winamd64\STAF324-setup-winamd64.jar

staf local monitor log message "Completed execution of build-winamd64.cmd"







