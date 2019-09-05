@echo off
set OOCD_EXE="C:\Program Files\openocd-2007re204\bin\openocd-ftd2xx.exe"
set OOCD_CFG=lpc2xxx_armusbocd_flash.cfg

%OOCD_EXE% %OOCD_DBG% -f %OOCD_CFG%
