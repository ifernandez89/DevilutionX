#!/usr/bin/env python3
"""
build_minixp_iso.py
Construye la ISO de Mini Windows XP usando pycdlib (si está disponible) o el generador de Node.js.
Genera una imagen ISO 100% compatible con la especificación El Torito + NTLDR direct boot.
"""

import os
import sys
import subprocess

def build_iso(source_dir, output_iso):
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    pycdlib_builder = os.path.join(base_dir, "tools", "build_iso_pycdlib.py")
    js_builder = os.path.join(base_dir, "tools", "build_minixp_iso.js")

    try:
        import pycdlib
        print("[*] pycdlib detectado. Usando motor oficial pycdlib...")
        cmd = [sys.executable, pycdlib_builder, source_dir, output_iso]
    except ImportError:
        print("[*] pycdlib no disponible. Usando motor Node.js...")
        cmd = ["node", js_builder, source_dir, output_iso]

    result = subprocess.run(cmd)
    if result.returncode != 0:
        print("[!] Error en la generación de la ISO.")
        sys.exit(result.returncode)

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base_dir, "MiniXp")
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base_dir, "Packaging", "minixp-wasm", "minixp.iso")
    build_iso(src, out)
