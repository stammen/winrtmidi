@ECHO OFF

REM
REM Version is read from the VERSION file.
REM
REM Command-line parameter specifies either "signed" or a prerelease string to append to the version
REM
REM Say VERSION contains "0.0.3" then:
REM
REM build-nupkg                     <-- generates package with version 0.0.3
REM build-nupkg build-140912-1100   <-- generates package with version 0.0.3-build-140912-1100
REM
REM The "signed" flag is intended for internal Microsoft use. This generates
REM a package without a prerelease version number, and pulling the binaries
REM from a different location to usual. This script does not actually
REM perform signing: the expectation is that previous tooling has already
REM signed the files and copied them to the right place for it to pick up.
REM

PUSHD "%~dp0"

echo "%~dp0"
 
WHERE /Q nuget >NUL
IF %ERRORLEVEL% NEQ 0 ( 
    ECHO nuget not found. Download it from www.nuget.org and put somewhere on the PATH.
    GOTO END
)

SET /p VERSION=<VERSION

IF "%1" == "signed" (
    SET BIN=obj\signed
    SET OUTDIR=bin\signed
) else (
    IF NOT "%1" == "" (
        SET VERSION=%VERSION%-%1
    ) else (
        SET VERSION=%VERSION%
    )
    SET BIN="%~dp0..\"
    SET OUTDIR=bin\unsigned
)

IF NOT EXIST %OUTDIR% (
    md %OUTDIR%
)
 
IF NOT EXIST obj (
    md obj
)

git -C ..\ rev-parse --verify HEAD > winrtmidi.githash.txt

echo %BIN%

SET NUGET_ARGS=^
    -nopackageanalysis ^
    -outputdirectory %OUTDIR% ^
    -version %VERSION% ^
    -properties bin=%BIN%

nuget pack winrtmidi.nuspec %NUGET_ARGS%
IF %ERRORLEVEL% NEQ 0 GOTO END

:END

POPD
EXIT /B %ERRORLEVEL%
