
staf local monitor log message "Starting to execute build-linux.cmd"

del C:\STAF_ISMP_InputFiles\build\*.* /s /q
rmdir C:\STAF_ISMP_InputFiles\build /s /q

staf local monitor log message "Adding required empty directories"

REM add required empty WIN32 build directories
md C:\STAF_ISMP_InputFiles\build\bin\win32
md C:\STAF_ISMP_InputFiles\build\lib\base\win32
md C:\STAF_ISMP_InputFiles\build\codepage\win32
md C:\STAF_ISMP_InputFiles\build\bin-log\win32
md C:\STAF_ISMP_InputFiles\build\bin-mon\win32
md C:\STAF_ISMP_InputFiles\build\bin-pool\win32
md C:\STAF_ISMP_InputFiles\build\bin-zip\win32
md C:\STAF_ISMP_InputFiles\build\include\win32
md C:\STAF_ISMP_InputFiles\build\bin-java\win32
md C:\STAF_ISMP_InputFiles\build\bin-jstaf\win32
md C:\STAF_ISMP_InputFiles\build\bin-rexx\win32
md C:\STAF_ISMP_InputFiles\build\lib-rexx\win32
md C:\STAF_ISMP_InputFiles\build\bin-tcl\win32
md C:\STAF_ISMP_InputFiles\build\bin-python\win32
md C:\STAF_ISMP_InputFiles\build\bin-perl\win32
md C:\STAF_ISMP_InputFiles\build\lib-perl56\win32
md C:\STAF_ISMP_InputFiles\build\lib-perl58\win32
md C:\STAF_ISMP_InputFiles\build\docs\common-python

REM add required empty AIX build directories
md C:\STAF_ISMP_InputFiles\build\bin\aix
md C:\STAF_ISMP_InputFiles\build\lib\base\aix
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
md C:\STAF_ISMP_InputFiles\build\codepage\hpux
md C:\STAF_ISMP_InputFiles\build\lib-log\hpux
md C:\STAF_ISMP_InputFiles\build\lib-mon\hpux
md C:\STAF_ISMP_InputFiles\build\lib-pool\hpux
md C:\STAF_ISMP_InputFiles\build\lib-zip\hpux
md C:\STAF_ISMP_InputFiles\build\include\hpux
md C:\STAF_ISMP_InputFiles\build\lib-java\hpux
md C:\STAF_ISMP_InputFiles\build\lib-jstaf\hpux
md C:\STAF_ISMP_InputFiles\build\codepage\hpux-optional

staf local monitor log message "Copying linux files to input file directory"

md C:\STAF_ISMP_InputFiles\build
md C:\STAF_ISMP_InputFiles\build\bin\linux

REM BIN LINUX files
copy C:\STAF_ISMP_InputFiles\linux\bin\STAF C:\STAF_ISMP_InputFiles\build\bin\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\STAFProc C:\STAF_ISMP_InputFiles\build\bin\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\STAFReg C:\STAF_ISMP_InputFiles\build\bin\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\GenWL.cmd C:\STAF_ISMP_InputFiles\build\bin\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\CSTAF C:\STAF_ISMP_InputFiles\build\bin\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\FmtLog C:\STAF_ISMP_InputFiles\build\bin\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\STAF.cmd C:\STAF_ISMP_InputFiles\build\bin\linux\*.*

REM LIB BASE LINUX files
md C:\STAF_ISMP_InputFiles\build\lib\base\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAF.so C:\STAF_ISMP_InputFiles\build\lib\base\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAFDSLS.so C:\STAF_ISMP_InputFiles\build\lib\base\linux\*.*
copy "C:\STAF_ISMP_InputFiles\linux\lib\libstdc++.so.6" C:\STAF_ISMP_InputFiles\build\lib\base\linux\*.*
copy "C:\STAF_ISMP_InputFiles\linux\lib\libgcc_s.so.1" C:\STAF_ISMP_InputFiles\build\lib\base\linux\*.*

REM CODEPAGE LINUX files
md C:\STAF_ISMP_InputFiles\build\codepage\linux
copy C:\STAF_ISMP_InputFiles\linux\codepage\alias.txt C:\STAF_ISMP_InputFiles\build\codepage\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\codepage\ibm-437.bin C:\STAF_ISMP_InputFiles\build\codepage\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\codepage\ibm-850.bin C:\STAF_ISMP_InputFiles\build\codepage\linux\*.*

REM Log Service LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-log\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAFLog.so C:\STAF_ISMP_InputFiles\build\lib-log\linux\*.*

REM Monitor Service LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-mon\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAFMon.so C:\STAF_ISMP_InputFiles\build\lib-mon\linux\*.*

REM Respool service LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-pool\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAFPool.so C:\STAF_ISMP_InputFiles\build\lib-pool\linux\*.*

REM Zip service LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-zip\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAFZip.so C:\STAF_ISMP_InputFiles\build\lib-zip\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\libSTAFZlib.so C:\STAF_ISMP_InputFiles\build\lib-zip\linux\*.*

REM C++ LINUX files
md C:\STAF_ISMP_InputFiles\build\include\linux
copy C:\STAF_ISMP_InputFiles\linux\include\STAFOSTypes.h C:\STAF_ISMP_InputFiles\build\include\linux\*.*
md C:\STAF_ISMP_InputFiles\build\include\common
copy C:\STAF_ISMP_InputFiles\win32\include\STAF.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAF_fstream.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAF_iostream.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFDynamicLibrary.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFDynamicLibraryInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFError.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFEventSem.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFEventSemInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFException.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFMutexSem.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFMutexSemInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFRefPtr.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFString.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFStringInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFThread.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFTimestamp.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFTimestampInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFUtil.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFFileSystem.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFFileSystemInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFLogService.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFMonitorService.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFResPoolService.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFProcess.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFProcessInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFTrace.h C:\STAF_ISMP_InputFiles\build\include\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFTraceInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\common\*.*

REM JAVA LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-java\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libJSTAF.so C:\STAF_ISMP_InputFiles\build\lib-java\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\libJSTAFSH.so C:\STAF_ISMP_InputFiles\build\lib-java\linux\*.*

REM JSTAF LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-jstaf\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\JSTAF.zip C:\STAF_ISMP_InputFiles\build\lib-jstaf\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\JSTAF.jar C:\STAF_ISMP_InputFiles\build\lib-jstaf\linux\*.*

REM REXX LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-rexx\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libRXSTAF.so C:\STAF_ISMP_InputFiles\build\lib-rexx\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\libRxThread.so C:\STAF_ISMP_InputFiles\build\lib-rexx\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFLog.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFMon.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFPool.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFUtil.rxl C:\STAF_ISMP_InputFiles\build\lib-rexx\linux\*.*

REM TCL LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-tcl\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libTCLSTAF.so C:\STAF_ISMP_InputFiles\build\lib-tcl\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\pkgIndex.tcl C:\STAF_ISMP_InputFiles\build\lib-tcl\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFLog.tcl C:\STAF_ISMP_InputFiles\build\lib-tcl\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFMon.tcl C:\STAF_ISMP_InputFiles\build\lib-tcl\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\STAFUtil.tcl C:\STAF_ISMP_InputFiles\build\lib-tcl\linux\*.*

REM TCL DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common-tcl
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFTcl.htm C:\STAF_ISMP_InputFiles\build\docs\common-tcl\*.*

REM PERL BIN LINUX files
md C:\STAF_ISMP_InputFiles\build\bin-perl\linux
copy C:\STAF_ISMP_InputFiles\linux\bin\PLSTAF.pm C:\STAF_ISMP_InputFiles\build\bin-perl\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\STAFLog.pm C:\STAF_ISMP_InputFiles\build\bin-perl\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\bin\STAFMon.pm C:\STAF_ISMP_InputFiles\build\bin-perl\linux\*.*

REM PERL58 LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-perl58\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\perl58\libPLSTAF.so C:\STAF_ISMP_InputFiles\build\lib-perl58\linux\*.*

REM PERL56 LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-perl56\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\perl56\libPLSTAF.so C:\STAF_ISMP_InputFiles\build\lib-perl56\linux\*.*

REM PERL50 LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-perl50\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\perl50\libPLSTAF.so C:\STAF_ISMP_InputFiles\build\lib-perl50\linux\*.*

REM PERL DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common-perl
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFPerl.htm C:\STAF_ISMP_InputFiles\build\docs\common-perl\*.*

REM PYTHON LIB LINUX files
md C:\STAF_ISMP_InputFiles\build\lib-python\linux
copy C:\STAF_ISMP_InputFiles\linux\lib\libPYSTAF.so C:\STAF_ISMP_InputFiles\build\lib-python\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\PySTAFLog.py C:\STAF_ISMP_InputFiles\build\lib-python\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\PySTAFMon.py C:\STAF_ISMP_InputFiles\build\lib-python\linux\*.*
copy C:\STAF_ISMP_InputFiles\linux\lib\PySTAF.py C:\STAF_ISMP_InputFiles\build\lib-python\linux\*.*

REM PYTHON DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common-python
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFPython.htm C:\STAF_ISMP_InputFiles\build\docs\common-python\*.*

REM DOCS COMMON files
md C:\STAF_ISMP_InputFiles\build\docs\common
copy C:\STAF_ISMP_InputFiles\win32\docs\History C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFCMDS.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFFAQ.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFHome.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFRC.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFUG.htm C:\STAF_ISMP_InputFiles\build\docs\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\docs\STAFGS.pdf C:\STAF_ISMP_InputFiles\build\docs\common\*.*

REM SAMPLES/DEMOS COMMON files
md C:\STAF_ISMP_InputFiles\build\samples\common
xcopy /s C:\STAF_ISMP_InputFiles\win32\samples\*.* C:\STAF_ISMP_InputFiles\build\samples\common\*.*

REM SERVICE DEV INCLUDE COMMON files
md C:\STAF_ISMP_InputFiles\build\include\service-dev\common
copy C:\STAF_ISMP_InputFiles\win32\include\STAFCommandParser.h C:\STAF_ISMP_InputFiles\build\include\service-dev\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFCommandParserInlImpl.cpp C:\STAF_ISMP_InputFiles\build\include\service-dev\common\*.*
copy C:\STAF_ISMP_InputFiles\win32\include\STAFServiceInterface.h C:\STAF_ISMP_InputFiles\build\include\service-dev\common\*.*

REM SERVICE DEV LIB COMMON files
md C:\STAF_ISMP_InputFiles\build\lib\service-dev\common
copy C:\STAF_ISMP_InputFiles\win32\lib\service.def C:\STAF_ISMP_InputFiles\build\lib\service-dev\common\*.*

REM OPTIONAL CODEPAGES LINUX
md C:\STAF_ISMP_InputFiles\build\codepage\linux-optional
copy C:\STAF_ISMP_InputFiles\linux\codepage\*.* C:\STAF_ISMP_InputFiles\build\codepage\linux-optional\*.*

staf local monitor log message "Starting the linux ISMP build"

cd C:\ISMP503
java -cp .;C:\ISMP503\lib\idewizards.jar;C:\ISMP503\lib\ProjectWizard.jar;C:\ISMP503\ppk\win32ppk.jar;C:\ISMP503\ppk\linuxppk.jar;C:\ISMP503\ppk\solarisppk.jar;C:\ISMP503\ppk\hpuxppk.jar;C:\ISMP503\ppk\aixppk.jar;C:\ISMP503\ppk\os2ppk.jar;C:\ISMP503\ppk\cimppk.jar;C:\ISMP503\ppk\as400ppk.jar;C:\ISMP503\ppk\webppk.jar;C:\ISMP503\classes;C:\ISMP503\classes\MyCustomBeans.jar;C:\ISMP503\lib\ide.jar;C:\ISMP503\lib\wizard.jar;C:\ISMP503\lib\product.jar;C:\ISMP503\lib\platform.jar;C:\ISMP503\lib\help.jar;C:\ISMP503\lib\swing.jar;C:\ISMP503\lib\jhall.jar;C:\ISMP503\lib\parser.jar;C:\ISMP503\lib\xt.jar;C:\ISMP503\lib\icebrowserbean.jar;C:\ISMP503\lib\icebrowserlitebean.jar;C:\ISMP503\ppk\macosxppk.jar;C:\ISMP503\ppk\genericunixppk.jar;C:\ISMP503\i18n com.installshield.isje.ISJE .\Projects\STAF2x\STAF2x.xml -build

staf local monitor log message "Copying the linux ISMP binaries to the STAF_ISMP_OutputFiles\disk1 directory"

cd C:\STAF_ISMP_InputFiles

copy C:\STAF_ISMP_OutputFiles\disk1\STAF264-setup-linux.bin C:\STAF_Installers\linux\*.*
copy C:\STAF_ISMP_OutputFiles\disk1\STAF264-setup.jar C:\STAF_Installers\linux\STAF264-setup-linux.jar

staf local monitor log message "Completed execution of build-linux.cmd"







