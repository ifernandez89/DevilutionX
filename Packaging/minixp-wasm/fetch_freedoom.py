import urllib.request
import zipfile
import io
import os

url = "https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip"
target_dir = os.path.join(os.path.dirname(__file__), "tinycore_build", "freedoom_files")
os.makedirs(target_dir, exist_ok=True)

try:
    print(f"[*] Descargando Freedoom v0.13.0 desde {url}...")
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        data = resp.read()
        print(f"[OK] Freedoom descargado ({len(data)} bytes). Descomprimiendo...")
        zf = zipfile.ZipFile(io.BytesIO(data))
        zf.extractall(target_dir)
        print("[OK] Archivos de Freedoom extraídos:", zf.namelist())
except Exception as e:
    print(f"[!] Error descargando Freedoom: {e}")
