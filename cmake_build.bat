@ECHO off

REM This batch file must be in same folder as CMakeLists.txt

rem @call %comspec% /k "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"


REM *************************
REM    CONFIGURE AND BUILD
REM *************************
CD %~dp0
SET build_dir=build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S. -B%build_dir% -GNinja

IF "%~2"=="RELEASE" (cmake --build %build_dir% -j) ELSE (cmake --build %build_dir% -j --config Release)
IF %ERRORLEVEL% NEQ 0 (GOTO BUILD_FAILED) 


REM *************************
REM     RUN APPLICATION
REM *************************
IF "%~1"=="" exit 0
SET app_name=%~dp0%build_dir%\%1.exe
ECHO Run application: %app_name%
START %app_name%
EXIT 0


:BUILD_FAILED
ECHO Build Failed!
EXIT 1
