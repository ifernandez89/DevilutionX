#!/usr/bin/env python3
"""
build_minixp_iso.py
Genera una imagen ISO booteable (ISO9660 + Joliet + El Torito) para Mini Windows XP
compatible con el emulador WebAssembly v86 / SeaBIOS.

Uso:
    python tools/build_minixp_iso.py [directorio_origen] [archivo_destino_iso] [boot_sector_bin_opcional]

Requisitos:
    pip install pycdlib
"""

import os
import sys
import argparse

try:
    import pycdlib
except ImportError:
    print("[!] Error: pycdlib no está instalado.")
    print("[!] Por favor instálalo ejecutando: pip install pycdlib")
    sys.exit(1)


def find_boot_file(source_dir):
    """Busca automáticamente el cargador de arranque en el directorio de Windows XP."""
    candidates = [
        os.path.join(source_dir, "I386", "SETUPLDR.BIN"),
        os.path.join(source_dir, "i386", "setupldr.bin"),
        os.path.join(source_dir, "MINIXP", "SETUPLDR.BIN"),
        os.path.join(source_dir, "minixp", "setupldr.bin"),
        os.path.join(source_dir, "SETUPLDR.BIN"),
        os.path.join(source_dir, "setupldr.bin"),
        os.path.join(source_dir, "BOOT.BIN"),
        os.path.join(source_dir, "boot.bin"),
        os.path.join(source_dir, "HBCD", "boot.bin"),
    ]
    for c in candidates:
        if os.path.isfile(c):
            return c
    return None


def build_iso(source_dir, output_iso_path, boot_bin=None):
    source_dir = os.path.abspath(source_dir)
    output_iso_path = os.path.abspath(output_iso_path)

    if not os.path.isdir(source_dir):
        print(f"[!] Error: El directorio de origen '{source_dir}' no existe.")
        sys.exit(1)

    os.makedirs(os.path.dirname(output_iso_path), exist_ok=True)

    print("=" * 65)
    print(f"[*] Iniciando empaquetado ISO para Mini Windows XP")
    print(f"[*] Origen:  {source_dir}")
    print(f"[*] Destino: {output_iso_path}")
    print("=" * 65)

    # Identificar archivo de arranque
    if not boot_bin:
        boot_bin = find_boot_file(source_dir)

    if boot_bin and os.path.isfile(boot_bin):
        print(f"[✓] Archivo de arranque detectado: {boot_bin}")
    else:
        print("[!] Advertencia: No se detectó un archivo de arranque explícito.")
        print("    Se generará la ISO de datos estándar.")

    iso = pycdlib.PyCdlib()
    # Joliet level 3 para compatibilidad con nombres largos de archivo de Windows
    iso.new(interchange_level=3, joliet=3, rock_ridge='1.09', vol_ident="MINIXP")

    # Si encontramos o especificamos un sector de arranque, añadir El Torito
    boot_iso_path = None
    if boot_bin and os.path.isfile(boot_bin):
        boot_iso_path = "/BOOT.BIN;1"
        try:
            iso.add_file(
                boot_bin,
                iso_path=boot_iso_path,
                joliet_path="/boot.bin"
            )
            iso.add_eltorito(
                boot_iso_path,
                boot_load_seg=0x07C0,
                load_cookies=True,
                media_type=pycdlib.pycdlib.ELTORITO_MEDIA_TYPE_NOEMUL
            )
            print("[✓] Catálogo El Torito registrado (No-Emulation Mode).")
        except Exception as e:
            print(f"[!] Error al configurar El Torito: {e}")

    added_count = 0
    # Recorrer la estructura de carpetas
    for root, dirs, files in os.walk(source_dir):
        rel_dir = os.path.relpath(root, source_dir)
        
        # Crear directorios en la ISO
        if rel_dir != ".":
            # Formato ISO9660 requiere rutas en mayúsculas y barras normales
            iso_dir = "/" + rel_dir.replace("\\", "/").upper()
            joliet_dir = "/" + rel_dir.replace("\\", "/")
            try:
                iso.add_directory(iso_dir, joliet_path=joliet_dir)
            except Exception:
                pass

        # Agregar archivos
        for fname in files:
            full_path = os.path.join(root, fname)
            rel_path = os.path.relpath(full_path, source_dir)

            # Si este archivo fue agregado como el BOOT.BIN inicial, no duplicarlo
            if boot_bin and os.path.abspath(full_path) == os.path.abspath(boot_bin):
                continue

            iso_file_path = "/" + rel_path.replace("\\", "/").upper() + ";1"
            joliet_file_path = "/" + rel_path.replace("\\", "/")

            try:
                iso.add_file(full_path, iso_path=iso_file_path, joliet_path=joliet_file_path)
                added_count += 1
            except Exception as e:
                print(f"[!] Aviso al empaquetar {rel_path}: {e}")

    print(f"[*] Total archivos empaquetados: {added_count}")
    print(f"[*] Guardando imagen ISO final...")
    iso.write(output_iso_path)
    iso.close()

    size_mb = os.path.getsize(output_iso_path) / (1024 * 1024)
    print("=" * 65)
    print(f"[✓] ISO generada exitosamente: {output_iso_path} ({size_mb:.2f} MB)")
    print("=" * 65)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generador de ISO booteable para Mini XP en WebAssembly")
    parser.add_argument("source", nargs="?", default="./MiniXp", help="Directorio con los archivos de Windows XP")
    parser.add_argument("output", nargs="?", default="./Packaging/minixp-wasm/minixp.iso", help="Ruta de la ISO de salida")
    parser.add_argument("--boot", default=None, help="Ruta al binario del sector de arranque (opcional)")
    args = parser.parse_args()

    build_iso(args.source, args.output, args.boot)
