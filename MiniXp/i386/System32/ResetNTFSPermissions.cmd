@echo off
title ResetNTFSPermissions CACLS
echo This will REPLACE all the NTFS permissions to EVERYONE incuding all subfolders
echo (or run the command manually with /E to Edit ACL instead of replacing it) 
echo ---
echo CACLS %1 /T /C /G Everyone:F
echo ---
CACLS %1 /T /C /G Everyone:F
Pause
