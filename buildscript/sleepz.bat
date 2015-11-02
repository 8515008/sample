:checkfiles
dir "h:\REMOVE - TO START SIGNING"
if %errorlevel%==0 goto returnmain

ping localhost -n 15

goto checkfiles

:returnmain