#!/usr/bin/env python3
"""
build_minixp_iso.py
Genera una imagen ISO booteable de Mini Windows XP para WebAssembly (v86 / SeaBIOS).
Usa HBCD/XP/XP.BIN como cargador de arranque directo El Torito No-Emulation.
"""

import os
import sys
import pycdlib

def build_iso(source_dir, output_iso):
    source_dir = os.path.abspath(source_dir)
    output_iso = os.path.abspath(output_iso)

    print("=" * 65)
    print("[*] Empaquetando Mini Windows XP Live ISO (Boot directo XP.BIN)")
    print(f"[*] Origen:  {source_dir}")
    print(f"[*] Destino: {output_iso}")
    print("=" * 65)

    os.makedirs(os.path.dirname(output_iso), exist_ok=True)

    iso = pycdlib.PyCdlib()
    iso.new(interchange_level=3, joliet=3, vol_ident="MINIXP")

    # 1. Crear directorios principales
    for d in ["/HBCD", "/HBCD/XP", "/HBCD/BOOT"]:
        try:
            iso.add_directory(d, joliet_path=d)
        except Exception:
            pass

    # 2. Registrar sector de arranque El Torito directo (XP.BIN)
    boot_file = os.path.join(source_dir, "HBCD", "XP", "XP.BIN")
    if not os.path.isfile(boot_file):
        print(f"[!] Error: No se encontro {boot_file}")
        sys.exit(1)

    iso.add_file(
        boot_file,
        iso_path="/HBCD/XP/XP.BIN;1",
        joliet_path="/HBCD/XP/XP.BIN"
    )
    iso.add_eltorito(
        "/HBCD/XP/XP.BIN;1",
        boot_load_size=4,
        media_name='noemul',
        bootable=True
    )
    print("[+] Sector de arranque El Torito registrado con HBCD/XP/XP.BIN")

    # 3. Agregar todos los demás archivos
    added_count = 1
    for root, dirs, files in os.walk(source_dir):
        rel_root = os.path.relpath(root, source_dir).replace('\\', '/')
        if rel_root != '.':
            iso_dir = "/" + rel_root.upper()
            joliet_dir = "/" + rel_root
            try:
                iso.add_directory(iso_path=iso_dir, joliet_path=joliet_dir)
            except Exception:
                pass

        for f in files:
            fp = os.path.join(root, f)
            rel_f = os.path.relpath(fp, source_dir).replace('\\', '/')
            if rel_f.upper() == "HBCD/XP/XP.BIN":
                continue # Ya agregado como bootloader

            iso_f = "/" + rel_f.upper() + ";1"
            joliet_f = "/" + rel_f
            try:
                iso.add_file(fp, iso_path=iso_f, joliet_path=joliet_f)
                added_count += 1
                print(f"  [+] Empaquetado: {rel_f} ({os.path.getsize(fp)} bytes)")
            except Exception as e:
                print(f"  [!] Error en {rel_f}: {e}")

    print(f"[*] Escribiendo imagen ISO ({added_count} archivos)...")
    iso.write(output_iso)
    iso.close()

    size_mb = os.path.getsize(output_iso) / (1024 * 1024)
    print("=" * 65)
    print(f"[OK] ISO generada exitosamente: {output_iso} ({size_mb:.2f} MB)")
    print("=" * 65)

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base_dir, "MiniXp")
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base_dir, "Packaging", "minixp-wasm", "minixp.iso")
    build_iso(src, out)
