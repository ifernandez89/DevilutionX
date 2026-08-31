@echo off
title ResetAttribs
echo This will clear SYSTEM/HIDDEN/READONLY attributes incuding all subfolders
echo ---
echo ATTRIB -S -H -R %1 /S /D
set /p attansw=Continue? (y/n)
if %attansw%'==y' ATTRIB -S -H -R %1 /S /D
if %attansw%'==Y' ATTRIB -S -H -R %1 /S /D
Pause
