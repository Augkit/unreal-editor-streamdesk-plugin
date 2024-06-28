@echo off
%1 mshta vbscript:CreateObject("Shell.Application").ShellExecute("cmd.exe","/c %~s0 ::","","runas",1)(window.close)&&exit
cd /d "%~dp0"
mklink /D "C:\Users\%USERNAME%\AppData\Roaming\Elgato\StreamDeck\Plugins\com.augkit.unreal-editor.sdPlugin" "%cd%\..\streamdesk-plugin\com.augkit.unreal-editor.sdPlugin"