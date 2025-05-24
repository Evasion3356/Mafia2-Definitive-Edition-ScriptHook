@echo off
set MAFIAPATH=D:\SteamLibrary\steamapps\common\Mafia II Definitive Edition
set mypath=%~dp0

copy /Y /B "%mypath%\build\Release\dxgi.dll" "%MAFIAPATH%\pc\dxgi.dll" /B 
del /F /Q "%MAFIAPATH%\pc\dxgi.pdb