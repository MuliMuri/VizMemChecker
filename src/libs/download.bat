@echo off



@rem ======================================
@rem ================Config================
@rem ======================================

set PROXY=
set RE_DOWNLOAD=
set DIR=
set URL_CAPSTONE=https://github.com/capstone-engine/capstone/archive/refs/tags/5.0-rc2.zip



@rem ======================================
@rem ==============Parse_Args==============
@rem ======================================

:parse_args
if "%~1" == "" goto :break

if "%~1" == "-proxy" (
    set PROXY=%~2
    shift
)

if "%~1" == "-dir" (
    set DIR=%~2
    shift
)

if "%~1" == "-r" (
    set RE_DOWNLOAD=1
)

shift
goto :parse_args

:break



@rem ======================================
@rem ===============Download===============
@rem ======================================

if not exist "%DIR%/packs" (
    mkdir "%DIR%/packs"
)

if not exist "%DIR%/packs/capstone.zip" (
    curl -x "%PROXY%" -o "%DIR%/packs/capstone.zip" %URL_CAPSTONE% -L
) else if "%RE_DOWNLOAD%" == "1" (
    powershell rm -Force -Recurse "%DIR%/packs/capstone"
    curl -x "%PROXY%" -o "%DIR%/packs/capstone.zip" %URL_CAPSTONE% -L
)

if exist "%DIR%/packs/capstone.zip" (
    powershell Expand-Archive -Force -Path "%DIR%/packs/capstone.zip" -DestinationPath "%DIR%"
    powershell ren "%DIR%/capstone-5.0-rc2" "%DIR%/capstone"
)
