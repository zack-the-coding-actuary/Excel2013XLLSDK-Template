@echo off
setlocal

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

rem Portable MSVC toolchain (no full VS install on this machine) - x64 only
call "C:\Users\Zachary Perez\AppData\Local\portable\msvc\msvc-14.44.17.14_sdk-26100\activate.cmd"

set INC=/I"include"

rem Include all source files here, enter space + src\FILENAME.c(pp) for each.
rem C and C++ sources are compiled separately below so each gets its own
rem language mode and /std version rather than forcing everything to C++.
set SRC_CPP=src\XLCALL.CPP
set SRC_C=src\main.c src\helper.c

set LIB_XLCALL=lib\x64\XLCALL32.LIB user32.lib
set COMMON=/nologo /W4 /DWIN32 /D_WINDOWS /D_USRDLL
set CPPFLAGS=/TP /std:c++20
set CFLAGS=/TC /std:c17

rem Pin the x64 compiler explicitly rather than relying on PATH order
rem (activate.cmd's PATH happens to list Hostx64\x64 first, but don't depend on that).
set CL_X64="C:\Users\Zachary Perez\AppData\Local\portable\msvc\msvc-14.44.17.14_sdk-26100\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe"

rem Likewise pin LIB to only the x64 dirs rather than the merged x64+x86 order.
set VCROOT=C:\Users\Zachary Perez\AppData\Local\portable\msvc\msvc-14.44.17.14_sdk-26100
set LIB=%VCROOT%\VC\Tools\MSVC\14.44.35207\lib\x64;%VCROOT%\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;%VCROOT%\Windows Kits\10\Lib\10.0.26100.0\um\x64

rem Replace PROJ_NAME with final build file
if /I "%CONFIG%"=="Release" (
    mkdir obj\Release 2>nul
    mkdir bin\Release 2>nul

    %CL_X64% /c %COMMON% %CPPFLAGS% /O2 /GL /MT /DNDEBUG %INC% %SRC_CPP% /Fo"obj\Release\\"
    if errorlevel 1 exit /b 1

    %CL_X64% /c %COMMON% %CFLAGS% /O2 /GL /MT /DNDEBUG %INC% %SRC_C% /Fo"obj\Release\\"
    if errorlevel 1 exit /b 1

    %CL_X64% /LD %COMMON% obj\Release\*.obj ^
        /Fe"bin\Release\PROJ_NAME_x64.xll" ^
        /link /MACHINE:X64 /SUBSYSTEM:WINDOWS /LTCG /OPT:REF /OPT:ICF /DEF:"src\main.def" %LIB_XLCALL%
    if errorlevel 1 exit /b 1
) else (
    mkdir obj\Debug 2>nul
    mkdir bin\Debug 2>nul

    %CL_X64% /c %COMMON% %CPPFLAGS% /Zi /Od /MTd /D_DEBUG %INC% %SRC_CPP% /Fo"obj\Debug\\" /Fd"obj\Debug\PROJ_NAME_x64.pdb"
    if errorlevel 1 exit /b 1

    %CL_X64% /c %COMMON% %CFLAGS% /Zi /Od /MTd /D_DEBUG %INC% %SRC_C% /Fo"obj\Debug\\" /Fd"obj\Debug\PROJ_NAME_x64.pdb"
    if errorlevel 1 exit /b 1

    %CL_X64% /LD %COMMON% obj\Debug\*.obj ^
        /Fe"bin\Debug\PROJ_NAME_x64.xll" ^
        /link /MACHINE:X64 /SUBSYSTEM:WINDOWS /DEBUG /DEF:"src\main.def" %LIB_XLCALL%
    if errorlevel 1 exit /b 1
)
