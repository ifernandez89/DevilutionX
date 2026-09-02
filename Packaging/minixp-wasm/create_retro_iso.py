#!/usr/bin/env python3
"""
create_retro_iso.py
Construye la imagen 'tinycore-retro.iso' completa y autónoma:
- Integra DOSBox, SDL, Tree, EmelFM
- Crea la persistencia mydata.tgz con /home/tc (Desktop, Games, Documents, Tools, Wbar)
- Configura onboot.lst para carga automática
- Produce una ISO booteable lista para v86.
"""

import os
import sys
import io
import tarfile
import pycdlib
import shutil

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(SCRIPT_DIR, "tinycore_build")
BASE_ISO_PATH = os.path.join(BUILD_DIR, "TinyCore-base.iso")
OUTPUT_ISO_PATH = os.path.join(SCRIPT_DIR, "tinycore-retro.iso")
TCZ_SRC_DIR = os.path.join(BUILD_DIR, "cde", "optional")
FREEDOOM_DIR = os.path.join(BUILD_DIR, "freedoom_files", "freedoom-0.13.0")

def create_mydata_tar():
    print("[*] Generando mydata.tgz con estructura /home/tc...")
    tar_stream = io.BytesIO()
    
    with tarfile.open(fileobj=tar_stream, mode="w:gz") as tar:
        def add_file(path_in_tar, content_bytes, mode=0o644):
            ti = tarfile.TarInfo(name=path_in_tar)
            ti.size = len(content_bytes)
            ti.mode = mode
            ti.mtime = 1700000000
            ti.uid = 1001 # tc user
            ti.gid = 50   # staff
            ti.uname = "tc"
            ti.gname = "staff"
            tar.addfile(ti, io.BytesIO(content_bytes))

        def add_dir(path_in_tar, mode=0o755):
            ti = tarfile.TarInfo(name=path_in_tar)
            ti.type = tarfile.DIRTYPE
            ti.mode = mode
            ti.mtime = 1700000000
            ti.uid = 1001
            ti.gid = 50
            ti.uname = "tc"
            ti.gname = "staff"
            tar.addfile(ti)

        # Directorios base
        add_dir("home/tc")
        add_dir("home/tc/Desktop")
        add_dir("home/tc/Desktop/Documents")
        add_dir("home/tc/Desktop/Games")
        add_dir("home/tc/Desktop/Tools")
        add_dir("home/tc/games")
        add_dir("home/tc/games/doom")
        add_dir("home/tc/tools")
        add_dir("home/tc/documents")
        add_dir("home/tc/.wbar.d")

        # 1. README y Documentos
        readme_txt = """===========================================================
    BIENVENIDO A TU RETRO VIRTUAL PC (Tiny Core Linux + v86)
===========================================================

Esta máquina virtual incluye un entorno retro completo optimizado:

* DOOM / Freedoom:
  Lanzador 1-click en el escritorio y dock inferior.
  
* File Manager (EmelFM):
  Explorador de archivos gráfico de dos paneles.
  
* DOSBox:
  Emulador DOS para ejecutar cualquier juego clásico en /home/tc/games.
  
* Tree (CLI):
  Inspecciona el sistema de archivos desde la terminal:
  $ tree ~/Desktop
  $ tree ~/games

Controles Doom en v86:
- Flechas / WASD : Movimiento
- Ctrl / Espacio : Disparo / Abrir puertas
- 1-7 : Cambio de armas
- Esc : Menú principal
===========================================================
""".encode("utf-8")
        add_file("home/tc/Desktop/README.txt", readme_txt)
        add_file("home/tc/documents/README.txt", readme_txt)

        guide_txt = """GUIA DE HERRAMIENTAS Y JUEGOS RETRO
----------------------------------
1. Para agregar tus propios juegos DOS (.EXE/.WAD):
   Colocalos dentro de /home/tc/games/ y ejecutalos con:
   dosbox -c "mount c /home/tc/games/tu_juego" -c "c:" -c "juego.exe"

2. EmelFM (File Manager):
   Permite copiar, mover y explorar archivos de forma visual.
   
3. Tree:
   Ejecuta 'tree' en la terminal para un diagrama en arbol de tus carpetas.
""".encode("utf-8")
        add_file("home/tc/Desktop/Documents/Retro_PC_Guide.txt", guide_txt)
        add_file("home/tc/documents/Retro_PC_Guide.txt", guide_txt)

        # 2. Scripts Lanzadores
        # Launcher Doom
        doom_script = """#!/bin/sh
clear
echo "=================================================="
echo "    INICIANDO DOOM / FREEDOOM EN RETRO PC...     "
echo "=================================================="
echo ""
echo "Recursos IWAD cargados desde /home/tc/games/doom/"

# Iniciar DOSBox con parametros optimizados para v86
if [ -f /home/tc/games/doom/freedoom1.wad ]; then
    echo "Lanzando Freedoom Phase 1..."
fi

dosbox \
  -c "cycles=fixed 15000" \
  -c "mount c /home/tc/games/doom" \
  -c "c:" \
  -c "dir" \
  -c "echo Listo para jugar. Ejecuta tu binario DOS o presiona Ctrl+F9 para salir."
""".encode("utf-8")
        add_file("home/tc/games/doom/play_doom.sh", doom_script, mode=0o755)
        add_file("home/tc/Desktop/Games/Play_Doom.sh", doom_script, mode=0o755)

        # Launcher Tree Explorer
        tree_script = """#!/bin/sh
aterm -geometry 80x28+100+100 -title "Filesystem Tree Explorer" -e sh -c "
clear
echo '============================================'
echo '       ESTRUCTURA DE TU RETRO PC            '
echo '============================================'
echo ''
tree /home/tc
echo ''
echo 'Presiona ENTER para cerrar...'
read enter
"
""".encode("utf-8")
        add_file("home/tc/tools/tree_explorer.sh", tree_script, mode=0o755)
        add_file("home/tc/Desktop/Tools/Tree_Explorer.sh", tree_script, mode=0o755)

        # Launcher File Manager
        fm_script = """#!/bin/sh
emelfm /home/tc &
""".encode("utf-8")
        add_file("home/tc/tools/file_manager.sh", fm_script, mode=0o755)
        add_file("home/tc/Desktop/Tools/File_Manager.sh", fm_script, mode=0o755)

        # 3. Copiar Freedoom WAD si existe
        if os.path.exists(FREEDOOM_DIR):
            fd1 = os.path.join(FREEDOOM_DIR, "freedoom1.wad")
            if os.path.exists(fd1):
                print(f"[*] Agregando freedoom1.wad ({os.path.getsize(fd1):,} bytes) a mydata.tgz...")
                with open(fd1, "rb") as f:
                    add_file("home/tc/games/doom/freedoom1.wad", f.read())

        # 4. Configurar wbar (Dock inferior de Tiny Core)
        wbar_cfg = """i: /usr/local/share/pixmaps/aterm.png
t: Terminal
c: aterm &

i: /usr/local/share/pixmaps/emelfm.png
t: File Manager
c: emelfm &

i: /usr/local/share/pixmaps/wbar/dosbox.png
t: DOSBox
c: dosbox &

i: /usr/local/share/pixmaps/wbar/doom.png
t: Play Doom
c: /home/tc/games/doom/play_doom.sh &
""".encode("utf-8")
        add_file("home/tc/.wbar", wbar_cfg)

        # 5. .xsession bienvenida
        xsession_patch = """# Custom Retro PC Welcome
aterm -geometry 70x16+50+50 -title "Retro PC Welcome" -e sh -c "
clear
cat /home/tc/Desktop/README.txt
echo ''
echo 'Disfruta tu Retro Virtual PC! Presiona ENTER...'
read enter
" &
""".encode("utf-8")
        add_file("home/tc/.xsession_welcome", xsession_patch, mode=0o755)

    tar_bytes = tar_stream.getvalue()
    print(f"[OK] mydata.tgz generado ({len(tar_bytes):,} bytes).")
    return tar_bytes

def build_retro_iso():
    print(f"\n[*] Abriendo ISO base: {BASE_ISO_PATH}...")
    base_iso = pycdlib.PyCdlib()
    base_iso.open(BASE_ISO_PATH)

    new_iso = pycdlib.PyCdlib()
    new_iso.new(interchange_level=3, joliet=3, rock_ridge='1.09', vol_ident="TINYC_RETRO")

    # Extraer y copiar directorios /BOOT y /CDE
    new_iso.add_directory('/BOOT', rr_name='boot', joliet_path='/boot')
    new_iso.add_directory('/BOOT/ISOLINUX', rr_name='isolinux', joliet_path='/boot/isolinux')
    new_iso.add_directory('/CDE', rr_name='cde', joliet_path='/cde')
    new_iso.add_directory('/CDE/OPTIONAL', rr_name='optional', joliet_path='/cde/optional')

    # 1. Copiar archivos de /BOOT
    for child in base_iso.list_children(iso_path='/BOOT'):
        ident = child.file_identifier().decode('utf-8')
        if ident in ('.', '..', 'ISOLINUX'):
            continue
        clean_name = ident.split(';')[0]
        buf = io.BytesIO()
        base_iso.get_file_from_iso_fp(buf, iso_path=f'/BOOT/{ident}')
        data = buf.getvalue()
        new_iso.add_fp(io.BytesIO(data), len(data), f'/BOOT/{clean_name}',
                       rr_name=clean_name.lower(), joliet_path=f'/boot/{clean_name.lower()}')

    # 2. Copiar archivos de /BOOT/ISOLINUX (con isolinux.bin para boot)
    boot_bin_fp = None
    for child in base_iso.list_children(iso_path='/BOOT/ISOLINUX'):
        ident = child.file_identifier().decode('utf-8')
        if ident in ('.', '..'):
            continue
        clean_name = ident.split(';')[0]
        buf = io.BytesIO()
        base_iso.get_file_from_iso_fp(buf, iso_path=f'/BOOT/ISOLINUX/{ident}')
        data = buf.getvalue()
        new_iso.add_fp(io.BytesIO(data), len(data), f'/BOOT/ISOLINUX/{clean_name}',
                       rr_name=clean_name.lower(), joliet_path=f'/boot/isolinux/{clean_name.lower()}')

    # 3. Copiar extensiones originales de /CDE/OPTIONAL
    existing_tczs = set()
    for child in base_iso.list_children(iso_path='/CDE/OPTIONAL'):
        ident = child.file_identifier().decode('utf-8')
        if ident in ('.', '..'):
            continue
        clean_name = ident.split(';')[0]
        existing_tczs.add(clean_name.lower())
        buf = io.BytesIO()
        base_iso.get_file_from_iso_fp(buf, iso_path=f'/CDE/OPTIONAL/{ident}')
        data = buf.getvalue()
        new_iso.add_fp(io.BytesIO(data), len(data), f'/CDE/OPTIONAL/{clean_name}',
                       rr_name=clean_name, joliet_path=f'/cde/optional/{clean_name}')

    # 4. Agregar nuevas extensiones TCZ descargadas
    new_tcz_list = [
        "dosbox.tcz", "sdl-sound.tcz", "SDL.tcz", "libvorbis.tcz",
        "libogg.tcz", "libmad.tcz", "libpng.tcz", "libasound.tcz",
        "tree.tcz", "emelfm.tcz", "gtk1.tcz", "glib1.tcz"
    ]
    for pkg in new_tcz_list:
        pkg_file = os.path.join(TCZ_SRC_DIR, pkg)
        if os.path.exists(pkg_file):
            clean_name = pkg.upper().replace('-', '_')
            if pkg.lower() not in existing_tczs:
                with open(pkg_file, "rb") as f:
                    data = f.read()
                new_iso.add_fp(io.BytesIO(data), len(data), f'/CDE/OPTIONAL/{clean_name}',
                               rr_name=pkg, joliet_path=f'/cde/optional/{pkg}')
                print(f"[+] Agregada extension: {pkg} ({len(data):,} bytes)")

    # 5. Generar onboot.lst y xbase.lst actualizados
    onboot_entries = [
        "aterm.tcz", "bzip2-lib.tcz", "fltk-1.3.tcz", "flwm.tcz",
        "freetype.tcz", "imlib2-bin.tcz", "imlib2.tcz", "libfontenc.tcz",
        "libICE.tcz", "libjpeg-turbo.tcz", "libpng.tcz", "libSM.tcz",
        "libX11.tcz", "libXau.tcz", "libxcb.tcz", "libXcursor.tcz",
        "libXdmcp.tcz", "libXext.tcz", "libXfixes.tcz", "libXfont.tcz",
        "libXmu.tcz", "libXrender.tcz", "libXt.tcz", "wbar.tcz",
        "Xlibs.tcz", "Xprogs.tcz", "Xvesa.tcz",
        # Nuevas extensiones
        "libasound.tcz", "libmad.tcz", "libogg.tcz", "libvorbis.tcz",
        "SDL.tcz", "sdl-sound.tcz", "dosbox.tcz",
        "tree.tcz",
        "glib1.tcz", "gtk1.tcz", "emelfm.tcz"
    ]
    onboot_content = "\n".join(onboot_entries).encode('utf-8') + b"\n"
    new_iso.add_fp(io.BytesIO(onboot_content), len(onboot_content), '/CDE/ONBOOT.LST',
                   rr_name='onboot.lst', joliet_path='/cde/onboot.lst')
    new_iso.add_fp(io.BytesIO(onboot_content), len(onboot_content), '/CDE/XBASE.LST',
                   rr_name='xbase.lst', joliet_path='/cde/xbase.lst')
    new_iso.add_fp(io.BytesIO(onboot_content), len(onboot_content), '/CDE/COPY2FS.LST',
                   rr_name='copy2fs.lst', joliet_path='/cde/copy2fs.lst')

    # 6. Agregar mydata.tgz a /CDE
    mydata_bytes = create_mydata_tar()
    new_iso.add_fp(io.BytesIO(mydata_bytes), len(mydata_bytes), '/CDE/MYDATA.TGZ',
                   rr_name='mydata.tgz', joliet_path='/cde/mydata.tgz')

    # 7. Configurar arranque El Torito (ISOLINUX)
    new_iso.add_eltorito('/BOOT/ISOLINUX/ISOLINUX.BIN')

    print(f"\n[*] Escribiendo ISO final en {OUTPUT_ISO_PATH}...")
    new_iso.write(OUTPUT_ISO_PATH)
    new_iso.close()
    base_iso.close()
    print(f"[EXITO] ISO 'tinycore-retro.iso' creada exitosamente ({os.path.getsize(OUTPUT_ISO_PATH):,} bytes)!")

if __name__ == "__main__":
    build_retro_iso()
