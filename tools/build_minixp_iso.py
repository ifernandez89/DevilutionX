#!/usr/bin/env python3
"""
build_minixp_iso.py
Genera una imagen ISO booteable (ISO9660 + Joliet + El Torito) para Mini Windows XP
compatible con el emulador WebAssembly v86 / SeaBIOS.

Soporta sanitización de nombres de archivo ISO9660 (8.3) y Joliet Level 3.
"""

import os
import sys
import re

try:
    import pycdlib
except ImportError:
    print("[!] Error: pycdlib no está instalado.")
    print("[!] Por favor instálalo ejecutando: pip install pycdlib")
    sys.exit(1)


def sanitize_8_3(name):
    name = name.upper()
    if '.' in name:
        base, ext = name.rsplit('.', 1)
    else:
        base, ext = name, ''
    base = re.sub(r'[^A-Z0-9_]', '_', base)[:8]
    ext = re.sub(r'[^A-Z0-9_]', '_', ext)[:3]
    if not base:
        base = 'FILE'
    return f"{base}.{ext}" if ext else base


def sanitize_joliet(name):
    # Joliet Level 3 soporta nombres de hasta 64 caracteres Unicode
    if len(name) <= 64:
        return name
    if '.' in name:
        base, ext = name.rsplit('.', 1)
        ext = '.' + ext[:10]
        base = base[:64 - len(ext)]
        return base + ext
    return name[:64]


def find_boot_file(source_dir):
    """Busca el archivo cargador de arranque de Windows XP / WinPE."""
    candidates = [
        os.path.join(source_dir, "i386", "setupldr.bin"),
        os.path.join(source_dir, "I386", "SETUPLDR.BIN"),
        os.path.join(source_dir, "minixp", "setupldr.bin"),
        os.path.join(source_dir, "MINIXP", "SETUPLDR.BIN"),
        os.path.join(source_dir, "setupldr.bin"),
        os.path.join(source_dir, "SETUPLDR.BIN"),
        os.path.join(source_dir, "boot.bin"),
        os.path.join(source_dir, "BOOT.BIN"),
        os.path.join(source_dir, "HBCD", "boot.bin"),
    ]
    for c in candidates:
        if os.path.isfile(c):
            return c
    return None


def build_iso(source_dir, output_iso, boot_file=None):
    source_dir = os.path.abspath(source_dir)
    output_iso = os.path.abspath(output_iso)

    print("=" * 65)
    print(f"[*] Iniciando empaquetado ISO booteable para Mini Windows XP")
    print(f"[*] Origen:  {source_dir}")
    print(f"[*] Destino: {output_iso}")
    print("=" * 65)
    
    if not os.path.exists(source_dir):
        print(f"[!] Error: La carpeta origen '{source_dir}' no existe.")
        sys.exit(1)

    os.makedirs(os.path.dirname(output_iso), exist_ok=True)
    
    if not boot_file:
        boot_file = find_boot_file(source_dir)

    iso = pycdlib.PyCdlib()
    iso.new(interchange_level=3, joliet=3, rock_ridge='1.09', vol_ident="MINIXP")

    # 1. Configurar El Torito si existe archivo de boot
    boot_iso_path = None
    if boot_file and os.path.isfile(boot_file):
        print(f"[+] Archivo de arranque detectado: {boot_file}")
        boot_iso_path = "/BOOT.BIN;1"
        try:
            iso.add_file(
                boot_file,
                iso_path=boot_iso_path,
                joliet_path="/boot.bin"
            )
            iso.add_eltorito(
                boot_iso_path,
                boot_load_seg=0x07C0,
                load_cookies=True,
                media_type=pycdlib.pycdlib.ELTORITO_MEDIA_TYPE_NOEMUL
            )
            print("[+] Catálogo El Torito registrado (No-Emulation Boot).")
        except Exception as e:
            print(f"[!] Aviso al configurar El Torito: {e}")

    dir_map = {'': {'iso': '', 'joliet': ''}}

    # 2. Recolectar y ordenar directorios por profundidad
    dirs = []
    for root, _, _ in os.walk(source_dir):
        rel = os.path.relpath(root, source_dir).replace('\\', '/')
        if rel != '.':
            dirs.append(rel)

    dirs.sort(key=lambda d: len(d.split('/')))

    used_iso_dirs = {}

    for d in dirs:
        parts = d.split('/')
        parent_rel = '/'.join(parts[:-1])
        parent_iso = dir_map[parent_rel]['iso']
        parent_joliet = dir_map[parent_rel]['joliet']
        
        raw = parts[-1]
        clean_iso = sanitize_8_3(raw)
        clean_joliet = sanitize_joliet(raw)
        
        if parent_iso not in used_iso_dirs:
            used_iso_dirs[parent_iso] = set()
        
        cand = clean_iso
        idx = 1
        while cand in used_iso_dirs[parent_iso]:
            cand = f"{clean_iso[:5]}_{idx}"
            idx += 1
        used_iso_dirs[parent_iso].add(cand)
        
        curr_iso = f"{parent_iso}/{cand}" if parent_iso else f"/{cand}"
        curr_joliet = f"{parent_joliet}/{clean_joliet}" if parent_joliet else f"/{clean_joliet}"
        
        dir_map[d] = {'iso': curr_iso, 'joliet': curr_joliet}
        try:
            iso.add_directory(iso_path=curr_iso, joliet_path=curr_joliet)
        except Exception as e:
            print(f"[!] Error al agregar directorio {d}: {e}")

    # 3. Agregar archivos
    used_iso_files = {}
    added_count = 0
    for root, _, files in os.walk(source_dir):
        rel = os.path.relpath(root, source_dir).replace('\\', '/')
        if rel == '.':
            rel = ''
        parent_iso = dir_map[rel]['iso']
        parent_joliet = dir_map[rel]['joliet']
        
        if parent_iso not in used_iso_files:
            used_iso_files[parent_iso] = set()
            
        for f in files:
            file_path = os.path.join(root, f)

            # Si este archivo fue agregado como el BOOT.BIN inicial, no duplicarlo
            if boot_file and os.path.abspath(file_path) == os.path.abspath(boot_file):
                continue

            clean_iso = sanitize_8_3(f)
            clean_joliet = sanitize_joliet(f)
            
            cand = clean_iso
            idx = 1
            while cand in used_iso_files[parent_iso]:
                if '.' in clean_iso:
                    b, ext = clean_iso.rsplit('.', 1)
                    cand = f"{b[:4]}_{idx}.{ext}"
                else:
                    cand = f"{clean_iso[:5]}_{idx}"
                idx += 1
            used_iso_files[parent_iso].add(cand)
            
            iso_p = (f"{parent_iso}/{cand}" if parent_iso else f"/{cand}") + ";1"
            joliet_p = f"{parent_joliet}/{clean_joliet}" if parent_joliet else f"/{clean_joliet}"
            
            try:
                iso.add_file(file_path, iso_path=iso_p, joliet_path=joliet_p)
                added_count += 1
                if added_count % 100 == 0:
                    print(f"    -> Procesados {added_count} archivos...")
            except Exception as e:
                print(f"[!] Error al agregar archivo {file_path}: {e}")

    print(f"[*] Escribiendo archivo ISO ({added_count} archivos, {len(dirs)} directorios)...")
    iso.write(output_iso)
    iso.close()
    
    size_mb = os.path.getsize(output_iso) / (1024 * 1024)
    print("=" * 65)
    print(f"[+] ISO generada exitosamente: {output_iso} ({size_mb:.2f} MB)")
    print("=" * 65)


if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(base_dir, "MiniXp")
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(base_dir, "Packaging", "minixp-wasm", "minixp.iso")
    boot = sys.argv[3] if len(sys.argv) > 3 else None
    
    build_iso(src, out, boot)
