#!/usr/bin/env python3
"""
build_minixp_iso.py
Wrapper en Python que invoca la herramienta de empaquetado ISO en Node.js (tools/build_minixp_iso.js)
con el cargador directo de 16-bits de NTLDR sin dependencias de ISOLINUX.
"""

import os
import sys
import subprocess

def build_iso(source_dir, output_iso):
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    js_builder = os.path.join(base_dir, "tools", "build_minixp_iso.js")

    cmd = ["node", js_builder, source_dir, output_iso]
    print(f"[*] Ejecutando cargador ISO de Node.js: {' '.join(cmd)}")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print("[!] Error en la generación de la ISO.")
        sys.exit(result.returncode)

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base_dir, "MiniXp")
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base_dir, "Packaging", "minixp-wasm", "minixp.iso")
    build_iso(src, out)
