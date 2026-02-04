# Microsoft Developer Studio Project File - Name="SYN" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SYN - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SYN.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SYN.mak" CFG="SYN - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SYN - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SYN - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/SYN", OHAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SYN - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "C:\FILES\SYN\REL"
# PROP Intermediate_Dir "C:\FILES\SYN\REL"
# PROP Ignore_Export_Lib 0
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Zi /O1 /Ob0 /D "NDBUG" /D "WIN32" /D "_CONSOLE" /D "STRICT" /D "RELEASE" /D "SYN" /D "NO_MFC" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib gc.lib lib.lib /nologo /subsystem:console /debug /machine:I386 /out:"C:\SYSTEST\BIN/SYN.exe"

!ELSEIF  "$(CFG)" == "SYN - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "C:\FILES\SYN\DBG"
# PROP Intermediate_Dir "C:\FILES\SYN\DBG"
# PROP Ignore_Export_Lib 0
# ADD BASE CPP /nologo /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /Op /D "_CONSOLE" /D "STRICT" /D "_DEBUG" /D "WIN32" /D "SYN" /D "NO_MFC" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib.lib /nologo /subsystem:console /debug /machine:I386 /out:"C:\SYSTEST\BIN/SYN.exe"

!ENDIF 

# Begin Target

# Name "SYN - Win32 Release"
# Name "SYN - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\ACTS.c
# End Source File
# Begin Source File

SOURCE=.\ASSORT.c
# End Source File
# Begin Source File

SOURCE=.\FERR.c
# End Source File
# Begin Source File

SOURCE=.\FIRST.c
# End Source File
# Begin Source File

SOURCE=.\HASH.c
# End Source File
# Begin Source File

SOURCE=.\HASHADD.c
# End Source File
# Begin Source File

SOURCE=.\INPUT.c
# End Source File
# Begin Source File

SOURCE=.\LEXYY.c
# End Source File
# Begin Source File

SOURCE=.\MAIN.c
# End Source File
# Begin Source File

SOURCE=.\ONFERR.c
# End Source File
# Begin Source File

SOURCE=.\PRINTV.c
# End Source File
# Begin Source File

SOURCE=.\PRNT.c
# End Source File
# Begin Source File

SOURCE=..\Sets\SET.c
# End Source File
# Begin Source File

SOURCE=.\SIGNON.c
# End Source File
# Begin Source File

SOURCE=.\STOK.c
# End Source File
# Begin Source File

SOURCE=..\Sets\virtualmem.c
# End Source File
# Begin Source File

SOURCE=.\YYCODE.c
# End Source File
# Begin Source File

SOURCE=.\YYDOLLAR.c
# End Source File
# Begin Source File

SOURCE=.\YYDRIVER.c
# End Source File
# Begin Source File

SOURCE=.\YYOUT.c
# End Source File
# Begin Source File

SOURCE=.\YYPATCH.c
# End Source File
# Begin Source File

SOURCE=.\YYSTATE.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\DEBUG.H
# End Source File
# Begin Source File

SOURCE=.\DEF.H
# End Source File
# Begin Source File

SOURCE=.\HASH.H
# End Source File
# Begin Source File

SOURCE=.\HASPARS.H
# End Source File
# Begin Source File

SOURCE=.\LLOUT.H
# End Source File
# Begin Source File

SOURCE=.\SET.H
# End Source File
# Begin Source File

SOURCE=.\STACK.H
# End Source File
# Begin Source File

SOURCE=..\..\Include\SYNMAIN.H
# End Source File
# Begin Source File

SOURCE=.\test.h
# End Source File
# Begin Source File

SOURCE=.\YYOUT.H
# End Source File
# Begin Source File

SOURCE=.\YYSTACK.H
# End Source File
# Begin Source File

SOURCE=..\..\Include\yystate.h
# End Source File
# Begin Source File

SOURCE=.\YYSTP.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\syn.rc
# End Source File
# End Group
# End Target
# End Project
