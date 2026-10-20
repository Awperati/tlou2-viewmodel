@echo off
rem Loads the first-person mod and then TLOU2Viewmodel into the game, from this folder.
rem Start the game normally (not as administrator), then run this, or run this first: it waits.
cd /d "%~dp0"
if not exist "TLOU2FirstPerson.asi" (
  echo TLOU2FirstPerson.asi is missing. Put this folder's files next to the first-person mod. See README.
  pause
  exit /b 1
)
fp_launcher.exe --asi TLOU2FirstPerson.asi --asi TLOU2Viewmodel.dll
