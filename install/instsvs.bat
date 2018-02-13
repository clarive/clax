@echo off

if not exist %~dp0\clax.exe goto errorPath

cd %~dp0\
sc query clax > NUL
if ERRORLEVEL 1060 goto installService

:stopService
  @echo stop the clax service
  @echo.
  for /F "tokens=3 delims=: " %%H in ('sc query clax ^| findstr "        STATE"') do (
    if /I "%%H" NEQ "STOPPED" (
      sc stop clax
    )
  )

  for /F "tokens=3 delims=: " %%H in ('sc query clax ^| findstr "        STATE"') do (
    if /I "%%H" == "STOPPED" (
      goto deleteService
    )
  )
  goto stopService

:deleteService
  for /F "tokens=3 delims=: " %%H in ('sc query clax ^| findstr "        STATE"') do (
    if /I "%%H" == "STOPPED" (
      @echo delete the clax service
      @echo.
      sc delete clax
    )
  )

:installService
  @echo install the clax service
  @echo.
  sc create "clax" binPath= "%~dp0\clax.exe -l %~dp0\clax.log -c %~dp0\clax.ini" DisplayName= "Clarive Agent"
  sc description clax "This service controls the clarive agent (Clax)..."
  sc config clax start= Auto

  if %1. == . goto configFirewall
  if %2. == . goto help
  sc config clax obj= %1% password= %2%

:configFirewall
  netsh advfirewall firewall delete rule name="Clarive Agent" program="%~dp0\clax.exe"
  netsh advfirewall firewall add rule name="Clarive Agent" dir=in action=allow program="%~dp0\clax.exe" enable=yes
  REM netsh advfirewall firewall add rule name="Clarive Agent" dir=in action=allow program="%~dp0\clax.exe" enable=yes remoteip=10.100.90.13 profile=domain
  REM netsh advfirewall firewall add rule name="Clarive Agent" dir=in action=allow program="%~dp0\clax.exe" enable=yes remoteip=10.100.90.13 profile=private

:startService
  @echo install the clax service
  @echo.
  for /F "tokens=3 delims=: " %%H in ('sc query clax ^| findstr "        STATE"') do (
    if /I "%%H" == "STOPPED" (
      sc start clax
    )
  )

  for /F "tokens=3 delims=: " %%H in ('sc query clax ^| findstr "        STATE"') do (
    if /I "%%H" == "RUNNING" (
      @echo The Clarive Agent Service is installed.
    )
  )
  goto remark

:help
  @echo.
  @echo No parameters are required running the installation of the Clarive Agent.

:remark
  @echo.
  @echo But if you need to start your service with a specific account then restart
  @echo this batch file with following parameters: username and password.
  @echo.
  @echo Usage:  instsvs "<domain\username>" "<password>"
  @echo.
  @pause
  goto end


:errorPath
  @echo.
  @echo You haven't unzipped the clax.zip file in the correct folder.
  @echo Please unzip the file to %~dp0\ and then run this batch.
  @echo.
  goto help

:end
  exit
