@echo off
setlocal EnableDelayedExpansion

REM ===========================================================
REM  notify.bat - send a push notification via ntfy.sh
REM
REM  Usage:
REM    notify.bat --msg "content here"
REM    notify.bat --msg "Disk is full" --title "Backup" --priority high --tags warning
REM
REM  Options:
REM    --msg,      -m  <text>    message body (required)
REM    --title,    -t  <text>    notification title
REM    --priority, -p  <level>   min | low | default | high | urgent
REM    --tags,     -g  <list>    comma-separated, e.g. warning,skull
REM    --server,   -s  <url>     override server URL
REM    --topic,    -o  <name>    override topic
REM    --help,     -h            show this help
REM
REM  Note: a literal percent sign in your message must be
REM  written as %%  (batch eats single %).
REM ===========================================================

set "NTFY_SERVER=https://ntfy.sh"
set "NTFY_TOPIC=razz-ai-status-push-c6ei4he9rj5p"

set "MSG="
set "TITLE="
set "PRIORITY="
set "TAGS="

if "%~1"=="" goto usage

:parse
if "%~1"=="" goto build
if /i "%~1"=="--msg"      (set "MSG=%~2"          & shift & shift & goto parse)
if /i "%~1"=="-m"         (set "MSG=%~2"          & shift & shift & goto parse)
if /i "%~1"=="--title"    (set "TITLE=%~2"        & shift & shift & goto parse)
if /i "%~1"=="-t"         (set "TITLE=%~2"        & shift & shift & goto parse)
if /i "%~1"=="--priority" (set "PRIORITY=%~2"     & shift & shift & goto parse)
if /i "%~1"=="-p"         (set "PRIORITY=%~2"     & shift & shift & goto parse)
if /i "%~1"=="--tags"     (set "TAGS=%~2"         & shift & shift & goto parse)
if /i "%~1"=="-g"         (set "TAGS=%~2"         & shift & shift & goto parse)
if /i "%~1"=="--server"   (set "NTFY_SERVER=%~2"  & shift & shift & goto parse)
if /i "%~1"=="-s"         (set "NTFY_SERVER=%~2"  & shift & shift & goto parse)
if /i "%~1"=="--topic"    (set "NTFY_TOPIC=%~2"   & shift & shift & goto parse)
if /i "%~1"=="-o"         (set "NTFY_TOPIC=%~2"   & shift & shift & goto parse)
if /i "%~1"=="--help"     goto usage
if /i "%~1"=="-h"         goto usage
echo [notify] Unknown argument: %~1
echo.
goto usage

:build
if not defined MSG (
    echo [notify] Error: --msg is required.
    echo.
    goto usage
)
if not defined TITLE set "TITLE=%COMPUTERNAME%"

set "HEADERS=-H "Title: !TITLE!""
if defined PRIORITY set "HEADERS=!HEADERS! -H "Priority: !PRIORITY!""
if defined TAGS     set "HEADERS=!HEADERS! -H "Tags: !TAGS!""

curl -s -S -f !HEADERS! -d "!MSG!" "!NTFY_SERVER!/!NTFY_TOPIC!" >nul
if errorlevel 1 (
    echo [notify] FAILED to send notification.
    exit /b 1
)

echo [notify] Sent: !MSG!
exit /b 0

:usage
echo Usage: notify.bat --msg "content here" [options]
echo.
echo   --msg,      -m  ^<text^>   message body ^(required^)
echo   --title,    -t  ^<text^>   notification title
echo   --priority, -p  ^<level^>  min ^| low ^| default ^| high ^| urgent
echo   --tags,     -g  ^<list^>   comma-separated tags, e.g. warning,skull
echo   --server,   -s  ^<url^>    override server URL
echo   --topic,    -o  ^<name^>   override topic
echo   --help,     -h           show this help
exit /b 2
