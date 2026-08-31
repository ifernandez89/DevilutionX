@set aaa=B:\Temp
@set /p aaa=Press ENTER to Extract this file to %aaa% (or type full path):
@B:\Tools\uharc.exe x -t"%aaa%" %1
