@echo off
:: %1=MountPoint %2=FileSystem %3=MinSize %4=Reserv %5=Size %6=Unit %7=label %8=Pack 
:: MinSize - The minimum size RamDisk
:: Reserv -  It is reserved under system
set FileSystem=%2
set Pack=%8
:: The rigid size
if "%6"=="M" (
  set "Size=%5"
  goto _1
)
:: Calculation of the size
for /f "usebackq skip=1 tokens=3 delims=: " %%i in (`RamStatus.exe`) do set size=%%i
set /a size=%size%/1024
set /a "size=(%size%-%4)*%5/100"
if %size% lss %3 set size=%3
:_1
::There is no sense to use NTFS for small disks 
if %size% lss 16 set "FileSystem=fat"
:: Packing is incompatible with FAT  
if "%FileSystem%"=="fat" set "Pack= "
imdisk.exe -a -t vm -m %1 -s %size%M -p "/fs:%FileSystem% /q /y /v:%7 %Pack%"