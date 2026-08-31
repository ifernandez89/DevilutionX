#!/usr/bin/env python3
"""
download_assets.py
Descarga los binarios de WebAssembly y BIOS necesarios para el emulador v86
(v86.wasm, libv86.js, seabios.bin, vgabios.bin).
"""

import os
import urllib.request

ASSETS = {
    "libv86.js": "https://cdn.jsdelivr.net/npm/v86@latest/build/libv86.js",
    "v86.wasm": "https://cdn.jsdelivr.net/npm/v86@latest/build/v86.wasm",
    "seabios.bin": "https://raw.githubusercontent.com/copy/v86/master/bios/seabios.bin",
    "vgabios.bin": "https://raw.githubusercontent.com/copy/v86/master/bios/vgabios.bin",
}

def main():
    target_dir = os.path.dirname(os.path.abspath(__file__))
    print(f"[*] Descargando activos de v86 a {target_dir}...")
    headers = {"User-Agent": "Mozilla/5.0"}

    for filename, url in ASSETS.items():
        filepath = os.path.join(target_dir, filename)
        if os.path.exists(filepath) and os.path.getsize(filepath) > 0:
            print(f"[OK] {filename} ya existe ({os.path.getsize(filepath)} bytes).")
            continue

        print(f"[*] Descargando {filename} desde {url}...")
        try:
            req = urllib.request.Request(url, headers=headers)
            with urllib.request.urlopen(req, timeout=30) as response, open(filepath, "wb") as out:
                data = response.read()
                out.write(data)
                print(f"[OK] {filename} guardado correctamente ({len(data)} bytes).")
        except Exception as e:
            print(f"[!] Error descargando {filename}: {e}")

    print("[*] Proceso completado.")

if __name__ == "__main__":
    main()
