#!/usr/bin/env python3
"""
build_iso_pycdlib.py
Empaqueta Mini Windows XP usando pycdlib con BOOT.BIN como cargador El Torito de 2048 bytes.
Calcula dinámicamente el LBA de XP.BIN en la ISO generada y recompila BOOT.BIN con el LBA exacto.
"""

import os
import sys
import subprocess
import pycdlib

def create_boot_bin(xp_lba):
    node_cmd = f"const {{ createMiniXpBootSector }} = require('./tools/generate_boot_sector'); const fs = require('fs'); fs.writeFileSync('MiniXp/BOOT.BIN', createMiniXpBootSector({xp_lba}));"
    subprocess.run(["node", "-e", node_cmd], check=True)

def build_iso(source_dir, output_iso):
    source_dir = os.path.abspath(source_dir)
    output_iso = os.path.abspath(output_iso)

    print("=" * 65)
    print("[*] Empaquetando Mini Windows XP con pycdlib + Direct NTLDR Bootloader")
    print(f"[*] Origen:  {source_dir}")
    print(f"[*] Destino: {output_iso}")
    print("=" * 65)

    os.makedirs(os.path.dirname(output_iso), exist_ok=True)

    # 1. Pasar 1: Generar BOOT.BIN dummy y construir ISO inicial para determinar LBA exacto de XP.BIN
    create_boot_bin(100)

    iso = pycdlib.PyCdlib()
    iso.new(interchange_level=3, joliet=3, vol_ident="MINIXP")

    # Agregar directorios
    for root, dirs, files in os.walk(source_dir):
        rel_root = os.path.relpath(root, source_dir).replace('\\', '/')
        if rel_root != '.':
            iso_dir = "/" + rel_root.upper()
            joliet_dir = "/" + rel_root
            try:
                iso.add_directory(iso_path=iso_dir, joliet_path=joliet_dir)
            except Exception:
                pass

    # Agregar BOOT.BIN como bootloader El Torito
    boot_file = os.path.join(source_dir, "BOOT.BIN")
    iso.add_file(boot_file, iso_path="/BOOT.BIN;1", joliet_path="/BOOT.BIN")
    iso.add_eltorito("/BOOT.BIN;1", boot_load_size=4, media_name='noemul', bootable=True)

    # Agregar todos los demás archivos
    for root, dirs, files in os.walk(source_dir):
        rel_root = os.path.relpath(root, source_dir).replace('\\', '/')
        for f in files:
            fp = os.path.join(root, f)
            rel_f = os.path.relpath(fp, source_dir).replace('\\', '/')
            if rel_f.upper() == "BOOT.BIN":
                continue

            iso_f = "/" + rel_f.upper() + ";1"
            joliet_f = "/" + rel_f
            try:
                iso.add_file(fp, iso_path=iso_f, joliet_path=joliet_f)
            except Exception as e:
                pass

    # Obtener el LBA de XP.BIN
    xp_file_entry = iso.get_record(iso_path="/HBCD/XP/XP.BIN;1")
    xp_lba = xp_file_entry.extent_location()
    print(f"[+] LBA exacto determinado para XP.BIN: {xp_lba}")

    # Recompilar BOOT.BIN con el LBA exacto
    create_boot_bin(xp_lba)

    # Actualizar BOOT.BIN en la ISO
    iso.write(output_iso)
    iso.close()

    # Re-escribir BOOT.BIN actualizado en la ISO terminada en su LBA correspondiente
    iso2 = pycdlib.PyCdlib()
    iso2.open(output_iso)
    boot_record = iso2.get_record(iso_path="/BOOT.BIN;1")
    boot_lba = boot_record.extent_location()
    iso2.close()

    with open(boot_file, "rb") as fboot:
        boot_data = fboot.read()

    with open(output_iso, "r+b") as fiso:
        fiso.seek(boot_lba * 2048)
        fiso.write(boot_data)

    final_size = os.path.getsize(output_iso) / (1024 * 1024)
    print("=" * 65)
    print(f"[OK] ISO generada exitosamente con pycdlib: {output_iso} ({final_size:.2f} MB)")
    print("=" * 65)

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base_dir, "MiniXp")
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base_dir, "Packaging", "minixp-wasm", "minixp.iso")
    build_iso(src, out)
