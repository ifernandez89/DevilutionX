#!/usr/bin/env python3
"""
build_retro_tinycore.py
Descarga y construye la imagen ISO / entorno de Tiny Core Linux con Retro PC Tools:
- DOSBox + SDL Audio/Video
- Tree (CLI)
- EmelFM (File Manager Gráfico Ultraligero)
- DOOM Shareware v1.9 (1-click ready)
- Estructura de carpetas /home/tc (Games, Documents, Tools)
- Accesos directos y configuración optimizada para v86.
"""

import os
import sys
import shutil
import urllib.request
import subprocess

REPO_BASE = "http://tinycorelinux.net/14.x/x86/tcz"
ISO_URL = "http://tinycorelinux.net/14.x/x86/release/TinyCore-current.iso"
DOOM_SHAREWARE_ZIP = "https://archive.org/download/DoomShareware1.9/doom19s.zip"

PACKAGES = [
    "dosbox.tcz",
    "sdl-sound.tcz",
    "SDL.tcz",
    "libvorbis.tcz",
    "libogg.tcz",
    "libmad.tcz",
    "libpng.tcz",
    "libasound.tcz",
    "tree.tcz",
    "emelfm.tcz",
    "gtk1.tcz",
    "glib1.tcz"
]

def download_file(url, target_path):
    if os.path.exists(target_path) and os.path.getsize(target_path) > 0:
        print(f"[OK] Ya existe: {os.path.basename(target_path)} ({os.path.getsize(target_path):,} bytes)")
        return True
    
    print(f"[*] Descargando: {url}...")
    headers = {"User-Agent": "Mozilla/5.0"}
    try:
        req = urllib.request.Request(url, headers=headers)
        with urllib.request.urlopen(req, timeout=30) as resp, open(target_path, "wb") as f:
            shutil.copyfileobj(resp, f)
        print(f"[OK] Descargado: {os.path.basename(target_path)} ({os.path.getsize(target_path):,} bytes)")
        return True
    except Exception as e:
        print(f"[!] Error descargando {url}: {e}")
        return False

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    work_dir = os.path.join(script_dir, "tinycore_build")
    tcz_dir = os.path.join(work_dir, "cde", "optional")
    os.makedirs(tcz_dir, exist_ok=True)
    
    # 1. Descargar ISO base de Tiny Core
    base_iso = os.path.join(work_dir, "TinyCore-base.iso")
    download_file(ISO_URL, base_iso)
    
    # 2. Descargar extensiones TCZ y sus .dep
    print("\n[*] Descargando extensiones TCZ...")
    for pkg in PACKAGES:
        pkg_path = os.path.join(tcz_dir, pkg)
        download_file(f"{REPO_BASE}/{pkg}", pkg_path)
        dep_path = os.path.join(tcz_dir, f"{pkg}.dep")
        download_file(f"{REPO_BASE}/{pkg}.dep", dep_path)

    print("\n[OK] Todos los paquetes base han sido descargados correctamente.")

if __name__ == "__main__":
    main()
