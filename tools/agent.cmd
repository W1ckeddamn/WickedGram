@echo off
setlocal enabledelayedexpansion

:: 1. Look for native compiled agent.exe (Option 1)
if exist "%~dp0agent.exe" (
    "%~dp0agent.exe" %*
    exit /b %errorlevel%
)
if exist "%~dp0..\agent.exe" (
    "%~dp0..\agent.exe" %*
    exit /b %errorlevel%
)
if exist "%~dp0..\out\Debug\agent.exe" (
    "%~dp0..\out\Debug\agent.exe" %*
    exit /b %errorlevel%
)
if exist "%~dp0..\out\Release\agent.exe" (
    "%~dp0..\out\Release\agent.exe" %*
    exit /b %errorlevel%
)

:: 2. If python is available, run python CLI
where py >nul 2>&1
if %errorlevel% equ 0 (
    py "%~dp0agent_cli.py" %*
    exit /b %errorlevel%
)
where python >nul 2>&1
if %errorlevel% equ 0 (
    python "%~dp0agent_cli.py" %*
    exit /b %errorlevel%
)

:: 3. Zero-dependency fallback: native Windows curl.exe (Option 3)
set "CURL_EXE=%SystemRoot%\System32\curl.exe"
if not exist "%CURL_EXE%" set "CURL_EXE=curl.exe"

set "AGENT_PORT=40404"
set "AGENT_TOKEN="

for %%F in (
    "%~dp0..\tdata\wickedgram_agent.json"
    "D:\WickedGram-dev\tdata\wickedgram_agent.json"
    "tdata\wickedgram_agent.json"
    "%APPDATA%\Telegram Desktop\tdata\wickedgram_agent.json"
) do (
    if not defined AGENT_TOKEN if exist "%%~fF" (
        for /f "tokens=2 delims=:, " %%A in ('findstr /i "\"port\"" "%%~fF"') do set "AGENT_PORT=%%~A"
        for /f "tokens=2 delims=:, " %%A in ('findstr /i "\"token\"" "%%~fF"') do set "AGENT_TOKEN=%%~A"
    )
)

if defined WICKEDGRAM_AGENT_PORT set "AGENT_PORT=%WICKEDGRAM_AGENT_PORT%"
if defined WICKEDGRAM_AGENT_TOKEN set "AGENT_TOKEN=%WICKEDGRAM_AGENT_TOKEN%"

if "%~1"=="" goto help
if "%~1"=="-h" goto help
if "%~1"=="--help" goto help

if "%~1"=="status" (
    "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" "http://127.0.0.1:!AGENT_PORT!/api/status"
    echo.
    exit /b 0
)

if "%~1"=="chats" (
    if "%~2"=="list" (
        "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" "http://127.0.0.1:!AGENT_PORT!/api/chats?limit=50"
        echo.
        exit /b 0
    )
    if "%~2"=="get" (
        "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" "http://127.0.0.1:!AGENT_PORT!/api/chats/%~3"
        echo.
        exit /b 0
    )
)

if "%~1"=="messages" (
    if "%~2"=="get" (
        "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" "http://127.0.0.1:!AGENT_PORT!/api/chats/%~3/messages?limit=30"
        echo.
        exit /b 0
    )
    if "%~2"=="send" (
        "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" -H "Content-Type: application/json" -d "{\"text\":\"%~4\"}" "http://127.0.0.1:!AGENT_PORT!/api/chats/%~3/messages"
        echo.
        exit /b 0
    )
    if "%~2"=="read" (
        "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" -X POST "http://127.0.0.1:!AGENT_PORT!/api/chats/%~3/read"
        echo.
        exit /b 0
    )
)

if "%~1"=="regdate" (
    "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" "http://127.0.0.1:!AGENT_PORT!/api/user/regdate?user_id=%~2"
    echo.
    exit /b 0
)

if "%~1"=="typing" (
    "%CURL_EXE%" -s -H "Authorization: Bearer !AGENT_TOKEN!" -X POST "http://127.0.0.1:!AGENT_PORT!/api/chats/%~2/typing"
    echo.
    exit /b 0
)

:help
echo WickedGram AI Agent CLI
echo Usage: agent.cmd [command] [args...]
echo Commands:
echo   status                          Check bridge status
echo   chats list                      List dialogs
echo   chats get ^<chat_id^>             Get chat details
echo   messages get ^<chat_id^>          Get messages
echo   messages send ^<chat_id^> "text"  Send message
echo   messages read ^<chat_id^>         Mark read
echo   typing ^<chat_id^>                Send typing status
echo   regdate ^<user_id^>               Calculate registration date
exit /b 0
