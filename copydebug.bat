@echo off
set MAFIAPATH=D:\SteamLibrary\steamapps\common\Mafia II Definitive Edition
set mypath=%~dp0

copy /Y /B "%mypath%\build\Debug\dxgi.dll" "%MAFIAPATH%\pc\dxgi.dll" /B 
copy /Y /B "%mypath%\build\Debug\dxgi.pdb" "%MAFIAPATH%\pc\dxgi.pdb" /B 