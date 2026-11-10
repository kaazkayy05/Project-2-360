@echo off
cd /d "%~dp0"
where py >nul 2>&1
if %errorlevel%==0 (
    py server.py
) else (
    python server.py
)
if errorlevel 1 (
    echo.
    echo If you see a compiler error, install MinGW-w64 g++ and add it to PATH.
    echo Make sure this folder is in your GitHub project next to the backend folder.
    pause
)
