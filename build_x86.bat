@echo off
setlocal

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

rem Portable MSVC toolchain (no full VS install on this machine) - x86 only
call "C:\Users\Zachary Perez\AppData\Local\portable\msvc\msvc-14.44.17.14_sdk-26100\activate.cmd"

set INC=/I"include"

rem Include all source files here, enter space + src\FILENAME.c(pp) for each.
rem C and C++ sources are compiled separately below so each gets its own
rem language mode and /std version rather than forcing everything to C++.
set SRC_CPP=src\XLCALL.CPP
set SRC_C=src\main.c src\helper.c src\tls.c

set LIB_XLCALL=lib\x86\XLCALL32.LIB user32.lib
set COMMON=/nologo /W4 /DWIN32 /D_WINDOWS /D_USRDLL
set CPPFLAGS=/TP /std:c++20
set CFLAGS=/TC /std:c17

rem activate.cmd's PATH lists Hostx64\x64 before Hostx64\x86, so a bare
rem cl.exe here would silently pick the x64 compiler. Pin the x86 cross tools.
set CL_X86="C:\Users\Zachary Perez\AppData\Local\portable\msvc\msvc-14.44.17.14_sdk-26100\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x86\cl.exe"

rem activate.cmd's merged LIB also lists x64 dirs before x86, which makes the
rem linker resolve default libs (kernel32.lib, MSVCRTD.lib, etc.) from the
rem wrong architecture. Override LIB to only the x86 dirs.
set VCROOT=C:\Users\Zachary Perez\AppData\Local\portable\msvc\msvc-14.44.17.14_sdk-26100
set LIB=%VCROOT%\VC\Tools\MSVC\14.44.35207\lib\x86;%VCROOT%\Windows Kits\10\Lib\10.0.26100.0\ucrt\x86;%VCROOT%\Windows Kits\10\Lib\10.0.26100.0\um\x86

rem Replace PROJ_NAME with final build file
if /I "%CONFIG%"=="Release" (
    mkdir obj\Release 2>nul
    mkdir bin\Release 2>nul

    %CL_X86% /c %COMMON% %CPPFLAGS% /O2 /GL /MT /DNDEBUG %INC% %SRC_CPP% /Fo"obj\Release\\"
    if errorlevel 1 exit /b 1

    %CL_X86% /c %COMMON% %CFLAGS% /O2 /GL /MT /DNDEBUG %INC% %SRC_C% /Fo"obj\Release\\"
    if errorlevel 1 exit /b 1

    %CL_X86% /LD %COMMON% obj\Release\*.obj ^
        /Fe"bin\Release\PROJ_NAME_x86.xll" ^
        /link /MACHINE:X86 /SUBSYSTEM:WINDOWS /LTCG /OPT:REF /OPT:ICF /DEF:"src\main.def" %LIB_XLCALL%
    if errorlevel 1 exit /b 1
) else (
    mkdir obj\Debug 2>nul
    mkdir bin\Debug 2>nul

    %CL_X86% /c %COMMON% %CPPFLAGS% /Zi /Od /MTd /D_DEBUG %INC% %SRC_CPP% /Fo"obj\Debug\\" /Fd"obj\Debug\PROJ_NAME_x86.pdb"
    if errorlevel 1 exit /b 1

    %CL_X86% /c %COMMON% %CFLAGS% /Zi /Od /MTd /D_DEBUG %INC% %SRC_C% /Fo"obj\Debug\\" /Fd"obj\Debug\PROJ_NAME_x86.pdb"
    if errorlevel 1 exit /b 1

    %CL_X86% /LD %COMMON% obj\Debug\*.obj ^
        /Fe"bin\Debug\PROJ_NAME_x86.xll" ^
        /link /MACHINE:X86 /SUBSYSTEM:WINDOWS /DEBUG /DEF:"src\main.def" %LIB_XLCALL%
    if errorlevel 1 exit /b 1
)
