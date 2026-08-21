@echo off
setlocal

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

rem Standard VS2019 Build Tools install on this machine - x64 only
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

set INC=/I"include"

rem Include all source files here, enter space + src\FILENAME.c(pp) for each.
rem C and C++ sources are compiled separately below so each gets its own
rem language mode and /std version rather than forcing everything to C++.
set SRC_CPP=src\XLCALL.CPP
set SRC_C=src\main.c src\helper.c src\tls.c

set LIB_XLCALL=lib\x64\XLCALL32.LIB user32.lib
set COMMON=/nologo /W4 /DWIN32 /D_WINDOWS /D_USRDLL
set CPPFLAGS=/TP /std:c++20
set CFLAGS=/TC /std:c17

rem vcvars64.bat already sets up PATH/INCLUDE/LIB for the x64 host/target
rem toolchain, so cl.exe and LIB can be used as-is (no pinning needed).

rem Replace PROJ_NAME with final build file
if /I "%CONFIG%"=="Release" (
    mkdir obj\Release 2>nul
    mkdir bin\Release 2>nul

    cl %COMMON% %CPPFLAGS% /O2 /GL /MT /DNDEBUG %INC% /c %SRC_CPP% /Fo"obj\Release\\"
    if errorlevel 1 exit /b 1

    cl %COMMON% %CFLAGS% /O2 /GL /MT /DNDEBUG %INC% /c %SRC_C% /Fo"obj\Release\\"
    if errorlevel 1 exit /b 1

    cl /LD %COMMON% obj\Release\*.obj ^
        /Fe"bin\Release\PROJ_NAME_x64.xll" ^
        /link /MACHINE:X64 /SUBSYSTEM:WINDOWS /LTCG /OPT:REF /OPT:ICF /DEF:"src\main.def" %LIB_XLCALL%
    if errorlevel 1 exit /b 1
) else (
    mkdir obj\Debug 2>nul
    mkdir bin\Debug 2>nul

    cl %COMMON% %CPPFLAGS% /Zi /Od /MTd /D_DEBUG %INC% /c %SRC_CPP% /Fo"obj\Debug\\" /Fd"obj\Debug\PROJ_NAME_x64.pdb"
    if errorlevel 1 exit /b 1

    cl %COMMON% %CFLAGS% /Zi /Od /MTd /D_DEBUG %INC% /c %SRC_C% /Fo"obj\Debug\\" /Fd"obj\Debug\PROJ_NAME_x64.pdb"
    if errorlevel 1 exit /b 1

    cl /LD %COMMON% obj\Debug\*.obj ^
        /Fe"bin\Debug\PROJ_NAME_x64.xll" ^
        /link /MACHINE:X64 /SUBSYSTEM:WINDOWS /DEBUG /DEF:"src\main.def" %LIB_XLCALL%
    if errorlevel 1 exit /b 1
)
