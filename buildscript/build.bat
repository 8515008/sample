@echo "Welcome to lenovo SHAREit build console"

@SET MAJOR_VER=3
@SET MINOR_VER=0
@SET PKGBUILD_VER=0
@SET PATCH_VER=%date:~5,2%%date:~8,2%
@set PKGVERSION=%MAJOR_VER%.%MINOR_VER%.%PKGBUILD_VER%.%PATCH_VER%

dir isonbuildsever.tag
if %errorlevel%==0 goto skipconfirm
@echo "Are you sure you had updated the version?????????????????????????????????????????"
@echo %PKGVERSION%
pause
@echo "Are you sure?????????????????????????????????????????"
pause

:skipconfirm
@set BINARY_PATH=..\bin\Release
@set PROJECT_PATH=..\..\output
@set BIN4INST_PATH=..\..\output\bin4installer
@set OUT_INST_PATH=..\..\rel_inst
rem "%VS120COMNTOOLS%\vsDevCmd.bat"
move .\*.sh ..\..\cricket\
C:\Windows\Microsoft.NET\Framework\v4.0.30319\msbuild ..\Shareit.Foundation.sln /t:Rebuild /p:VersionNumber="%PKGVERSION%";Configuration=Release;TargetFrameworkVersion=v3.5;Platform="x86" > .\build.log
if %errorlevel% NEQ 0 goto FAILED

del %PROJECT_PATH%\* /S /Q
md %PROJECT_PATH%
md %BIN4INST_PATH%
md %OUT_INST_PATH%
copy ..\Shareit.Resource\Image\logo.ico %PROJECT_PATH%\ /S /Y
xcopy * %PROJECT_PATH%\ /S /Y
xcopy %BINARY_PATH%\* %BIN4INST_PATH% /S /Y
del %BIN4INST_PATH%\NLog.xml /F
del %BIN4INST_PATH%\Shareit.Foundation.pdb /F
del %BIN4INST_PATH%\Shareit.pdb /F
del %BIN4INST_PATH%\Shareit.Resource.pdb /F
del %BIN4INST_PATH%\Shareit.vshost.exe /F
del %BIN4INST_PATH%\Shareit.vshost.exe.config /F
del %BIN4INST_PATH%\Shareit.vshost.exe.manifest /F
del %BIN4INST_PATH%\ShareitUpdater.pdb /F
del %BIN4INST_PATH%\ShareitUpdater.vshost.exe /F
del %BIN4INST_PATH%\ShareitUpdater.vshost.exe.config /F
del %BIN4INST_PATH%\ShareitUpdater.vshost.exe.manifest /F

netsh wlan connect ssid=lenovo name=lenovo
if %errorlevel% NEQ 0 goto FAILED
ping localhost -n 10
dir h:
if %errorlevel%==0 goto copyfiles
net use h: \\10.176.36.40\bc_in robbie123 /user:share-user-robbie
if %errorlevel% NEQ 0 goto FAILED

:copyfiles
copy %BIN4INST_PATH%\Shareit.exe h:\ /Y
copy %BIN4INST_PATH%\Shareit.Foundation.dll h:\ /Y
copy %BIN4INST_PATH%\Shareit.Resource.dll h:\ /Y
copy %BIN4INST_PATH%\ShareitUpdater.exe h:\ /Y
xcopy %BIN4INST_PATH%\zh-hans h:\zh-hans /S /Y /I

del "h:\REMOVE - TO START SIGNING"

call .\sleepz.bat > temp.log

dir i:
if %errorlevel%==0 goto copyfiles2
net use i: \\10.176.36.40\bc_out robbie123 /user:share-user-robbie
if %errorlevel% NEQ 0 goto FAILED

:copyfiles2
copy i:\Shareit.exe %BIN4INST_PATH% /Y
copy i:\Shareit.Foundation.dll %BIN4INST_PATH% /Y
copy i:\Shareit.Resource.dll %BIN4INST_PATH% /Y
copy i:\ShareitUpdater.exe %BIN4INST_PATH% /Y
xcopy i:\zh-hans %BIN4INST_PATH%\zh-hans /S /Y /I

del i:\Shareit.exe /f
del i:\Shareit.Foundation.dll /f
del i:\Shareit.Resource.dll /f
del i:\ShareitUpdater.exe /f
del i:\zh-hans /f /q

"C:\Program Files (x86)\Inno Setup 5\iscc.exe" /FSHAREit%PKGVERSION% /O+ /DPKGVERSION=%PKGVERSION% /DCHANNEL=DEBUG /DUPGRADECHANNEL=DEBUG "%PROJECT_PATH%\shareit.iss"
if %errorlevel% NEQ 0 goto FAILED

for %%i in (DEV KCWEB LEFACTORY THIRDPART 360 baidu tencent yesky TVSU OTHERS POKKI LENOVOSUPPORT MedionP MedionW ironsrc) do "C:\Program Files (x86)\Inno Setup 5\iscc.exe" /Fshareit%%i /Q /DPKGVERSION=%PKGVERSION% /DCHANNEL=%%i /DUPGRADECHANNEL=%%i "%PROJECT_PATH%\shareit.iss"
)

rem !!!!!!!!!!!!!!!!记得把系统时区里的格式改成yyyy-mm-dd!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
@set dst=%date:~0,4%%date:~5,2%%date:~8,2%
del %OUT_INST_PATH%\%dst%\* /S /Q
md %OUT_INST_PATH%\%dst%
rem xcopy %PROJECT_PATH%\Output\* %OUT_INST_PATH%\%dst%\ /S /Y

xcopy %PROJECT_PATH%\Output\* h: /S /Y

del "h:\REMOVE - TO START SIGNING"

call .\sleepz.bat > temp.log

for %%i in (DEV KCWEB LEFACTORY THIRDPART 360 baidu tencent yesky TVSU OTHERS POKKI LENOVOSUPPORT MedionP MedionW ironsrc) do move /Y i:\shareit%%i.exe %OUT_INST_PATH%\%dst%
)

move i:\SHAREit%PKGVERSION%.exe %OUT_INST_PATH%\%dst%
move %OUT_INST_PATH%\%dst%\SHAREit%PKGVERSION%.exe ..\..\cricket\

dir isonbuildsever.tag
if %errorlevel%==0 goto END
explorer %OUT_INST_PATH%\%dst%\

goto END
:FAILED
powershell .\sendmail.ps1 -flag fail
net use h: /delete /y
net use i: /delete /y
goto OK
:END
net use h: /delete /y
net use i: /delete /y
powershell .\sendmail.ps1 -flag pass

:OK