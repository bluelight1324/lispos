@echo off
cd /d "%~dp0"
del /f synout*.* 2>nul
echo Running LALRGen on simple.y... > test_output.txt
"..\build\bin\Release\LALRGen.exe" simple.y >> test_output.txt 2>&1
echo Exit code: %ERRORLEVEL% >> test_output.txt
echo. >> test_output.txt
echo Output files: >> test_output.txt
dir synout*.* >> test_output.txt 2>&1
echo. >> test_output.txt
echo Content of synout0.cpp: >> test_output.txt
type synout0.cpp >> test_output.txt 2>&1
