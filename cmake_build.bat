@ECHO off

REM This batch file must be in same folder as CMakeLists.txt

set __VCVARSALL_TARGET_ARCH=x64
set __VCVARSALL_HOST_ARCH=x64
set PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64


CD %~dp0
SET build_dir=build
rem Visual Studio generator does not support generating compile_commands.json
rem The Ninja generator produces the json file but generates other errors on Windows


REM *************************
REM    CONFIGURE AND BUILD
REM *************************
CD %~dp0
SET build_dir=build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S. -G "Visual Studio 17 2022" -B%build_dir%
IF "%~2"=="RELEASE" (cmake --build %build_dir% -j) ELSE (cmake --build %build_dir% -j --config Release)
IF %ERRORLEVEL% NEQ 0 (GOTO BUILD_FAILED) 


REM *************************
REM     RUN APPLICATION
REM *************************
rem IF "%~2"=="RELEASE" (cmake --build %build_dir% -j) ELSE (cmake --build %build_dir% -j --config Release)
IF "%~1"=="" exit 0
SET app_name=%~dp0%build_dir%\Release\%1.exe
ECHO Run application: %app_name%
START %app_name%
EXIT 0


:BUILD_FAILED
ECHO Build Failed!
EXIT 1
