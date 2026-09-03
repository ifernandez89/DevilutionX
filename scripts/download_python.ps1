$destDir = "deps\emsdk\python\3.13.3_64bit"
New-Item -ItemType Directory -Force -Path $destDir | Out-Null
$zipPath = "deps\emsdk\python\python-embed-311.zip"
Write-Host "Descargando Python 3.11 embeddable 64-bit..."
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Invoke-WebRequest -Uri "https://www.python.org/ftp/python/3.11.9/python-3.11.9-embed-amd64.zip" -OutFile $zipPath
Write-Host "Extrayendo Python..."
Expand-Archive -Path $zipPath -DestinationPath $destDir -Force
Remove-Item $zipPath -Force
Write-Host "Verificando Python:"
& "$destDir\python.exe" --version
