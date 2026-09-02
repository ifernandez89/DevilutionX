import io, os, struct, tarfile, shutil, pycdlib, gzip, time, zlib, urllib.request

SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
BASE_ISO    = os.path.join(SCRIPT_DIR, "tinycore.iso")
OUTPUT_ISO  = os.path.join(SCRIPT_DIR, "tinycore-retro.iso")
TCZ_DIR     = os.path.join(SCRIPT_DIR, "tinycore_build", "cde", "optional")
FREEDOOM_DIR= os.path.join(SCRIPT_DIR, "tinycore_build", "freedoom_files", "freedoom-0.13.0")
DOOM_FILES_DIR = os.path.join(SCRIPT_DIR, "tinycore_build", "doom_files")

def ensure_assets():
    os.makedirs(TCZ_DIR, exist_ok=True)
    os.makedirs(DOOM_FILES_DIR, exist_ok=True)
    
    # 1. Base ISO
    if not os.path.exists(BASE_ISO) or os.path.getsize(BASE_ISO) < 1000000:
        alt_iso = os.path.join(SCRIPT_DIR, "tinycore_build", "TinyCore-base.iso")
        if os.path.exists(alt_iso) and os.path.getsize(alt_iso) > 1000000:
            shutil.copyfile(alt_iso, BASE_ISO)
        else:
            url = "http://tinycorelinux.net/14.x/x86/release/TinyCore-current.iso"
            print(f"[*] Descargando ISO base desde {url}...")
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            with urllib.request.urlopen(req, timeout=30) as resp, open(BASE_ISO, "wb") as f:
                shutil.copyfileobj(resp, f)

    # 2. TCZ Packages for Tiny Core 14.x (GLIBC 2.36)
    packages = [
        "dosbox.tcz", "sdl-sound.tcz", "SDL.tcz",
        "libvorbis.tcz", "libogg.tcz", "libmad.tcz",
        "libasound.tcz", "libpng.tcz", "tree.tcz",
        "emelfm.tcz", "gtk1.tcz", "glib1.tcz"
    ]
    for pkg in packages:
        pkg_path = os.path.join(TCZ_DIR, pkg)
        # libasound.tcz en 14.x mide ~413 KB, en 15.x mide ~500 KB
        need_download = not os.path.exists(pkg_path) or (pkg == "libasound.tcz" and os.path.getsize(pkg_path) > 450000)
        if need_download:
            url = f"http://tinycorelinux.net/14.x/x86/tcz/{pkg}"
            print(f"[*] Descargando {pkg} desde 14.x repo...")
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            with urllib.request.urlopen(req, timeout=30) as resp, open(pkg_path, "wb") as f:
                shutil.copyfileobj(resp, f)

    # 3. DOOM Binaries
    for fname, url_name in [('doom.exe', 'DOOM.exe'), ('doom1.wad', 'DOOM1.WAD')]:
        dest = os.path.join(DOOM_FILES_DIR, fname)
        if not os.path.exists(dest) or os.path.getsize(dest) < 10000:
            url = f"https://raw.githubusercontent.com/angea/doom-poly/main/{url_name}"
            print(f"[*] Descargando {fname}...")
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            with urllib.request.urlopen(req, timeout=30) as resp, open(dest, "wb") as f:
                shutil.copyfileobj(resp, f)

def make_simple_png(width=32, height=32, color=(255, 0, 0, 255)):
    def chunk(tag, d):
        return struct.pack('>I', len(d)) + tag + d + struct.pack('>I', zlib.crc32(tag + d) & 0xffffffff)
    ihdr = struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)
    r, g, b, a = color
    pixel = struct.pack('BBBB', r, g, b, a)
    raw = b''.join(b'\x00' + pixel * width for _ in range(height))
    idat = zlib.compress(raw)
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', ihdr) + chunk(b'IDAT', idat) + chunk(b'IEND', b'')

def create_cpio_archive(files_dict):
    out = io.BytesIO()
    ino = 100
    for path, (content, mode) in files_dict.items():
        ino += 1
        name_bytes = path.encode('utf-8') + b'\x00'
        name_len = len(name_bytes)
        file_len = len(content)
        header = f"070701{ino:08X}{mode:08X}{1001:08X}{50:08X}{1:08X}{int(time.time()):08X}{file_len:08X}{3:08X}{1:08X}{0:08X}{0:08X}{name_len:08X}{0:08X}".encode('ascii')
        out.write(header)
        out.write(name_bytes)
        pad1 = (4 - ((110 + name_len) % 4)) % 4
        out.write(b'\x00' * pad1)
        out.write(content)
        pad2 = (4 - (file_len % 4)) % 4
        out.write(b'\x00' * pad2)
        
    trailer_name = b'TRAILER!!!\x00'
    trailer_hdr = f"070701{0:08X}{0:08X}{0:08X}{0:08X}{1:08X}{0:08X}{0:08X}{0:08X}{0:08X}{0:08X}{0:08X}{len(trailer_name):08X}{0:08X}".encode('ascii')
    out.write(trailer_hdr)
    out.write(trailer_name)
    pad_trailer = (4 - ((110 + len(trailer_name)) % 4)) % 4
    out.write(b'\x00' * pad_trailer)
    return out.getvalue()

def build_perfect_retro_iso():
    ensure_assets()
    print(f"[*] Abriendo ISO base: {BASE_ISO}")
    base = pycdlib.PyCdlib()
    base.open(BASE_ISO)
    
    buf_iso_bin = io.BytesIO()
    base.get_file_from_iso_fp(buf_iso_bin, iso_path='/BOOT/ISOLINUX/ISOLINUX.BIN;1')
    isolinux_bin_data = buf_iso_bin.getvalue()
    print(f"[*] isolinux.bin original extraído: {len(isolinux_bin_data)} bytes")
    
    new_iso = pycdlib.PyCdlib()
    new_iso.new(interchange_level=3, joliet=3, rock_ridge='1.09', vol_ident="TinyCore")
    
    new_iso.add_directory('/BOOT', rr_name='boot', joliet_path='/boot')
    new_iso.add_directory('/BOOT/ISOLINUX', rr_name='isolinux', joliet_path='/boot/isolinux')
    new_iso.add_directory('/CDE', rr_name='cde', joliet_path='/cde')
    new_iso.add_directory('/CDE/OPTIONAL', rr_name='optional', joliet_path='/cde/optional')
    
    # Preparar archivos custom para inyectar directo en rootfs / core.gz
    # Así existen en /home/tc desde el instante 0 sin depender de mydata.tgz
    dosbox_safe_conf = b"""[sdl]
fullscreen=false
fulldouble=false
fullresolution=original
windowresolution=original
output=surface
autolock=true
sensitivity=100
waitonerror=true
priority=higher,normal
usescancodes=false

[dosbox]
machine=svga_s3
memsize=16

[render]
frameskip=1
aspect=false
scaler=none

[cpu]
core=normal
cputype=auto
cycles=10000
cycleup=1000
cycledown=1000

[mixer]
nosound=true
rate=11025
blocksize=1024
prebuffer=20

[sblaster]
sbtype=none

[speaker]
pcspeaker=false

[autoexec]
mount c /home/tc/games/doom
c:
doom.exe
"""
    dosbox_interactive_conf = b"""[sdl]
fullscreen=false
fulldouble=false
fullresolution=original
windowresolution=original
output=surface
autolock=true
sensitivity=100
waitonerror=true
priority=higher,normal
usescancodes=false

[dosbox]
machine=svga_s3
memsize=16

[render]
frameskip=1
aspect=false
scaler=none

[cpu]
core=normal
cputype=auto
cycles=10000
cycleup=1000
cycledown=1000

[mixer]
nosound=true
rate=11025
blocksize=1024
prebuffer=20

[sblaster]
sbtype=none

[speaker]
pcspeaker=false

[autoexec]
mount c /home/tc/games/doom
c:
"""
    benchmark_doom_script = b"""#!/bin/sh
CYCLES=${1:-10000}
FRAMESKIP=${2:-0}
AUDIO=${3:-off}
MODE=${4:-timedemo}

echo "====================================================="
echo "   LABORATORIO BENCHMARK: DOOM en DOSBox (Retro PC)  "
echo "====================================================="
echo "Parametros de prueba:"
echo "  * Cycles:    $CYCLES"
echo "  * Frameskip: $FRAMESKIP"
echo "  * Audio:     $AUDIO"
echo "  * Modo:      $MODE"
echo "-----------------------------------------------------"

CONF="/tmp/bench_$CYCLES.conf"

if [ "$AUDIO" = "on" ]; then
  MIXER_CONF="nosound=false
rate=22050
blocksize=1024
prebuffer=20"
  SB_CONF="sbtype=sb16
sbbase=220
irq=7
dma=1"
else
  MIXER_CONF="nosound=true
rate=11025
blocksize=1024
prebuffer=20"
  SB_CONF="sbtype=none"
fi

if [ "$MODE" = "timedemo" ]; then
  EXEC_CMD="doom.exe -timedemo demo1"
else
  EXEC_CMD="doom.exe"
fi

cat << EOF > $CONF
[sdl]
fullscreen=false
fulldouble=false
fullresolution=original
windowresolution=original
output=surface
autolock=true
sensitivity=100
waitonerror=true
priority=higher,normal
usescancodes=false

[dosbox]
machine=svga_s3
memsize=16

[render]
frameskip=$FRAMESKIP
aspect=false
scaler=none

[cpu]
core=normal
cputype=auto
cycles=$CYCLES
cycleup=1000
cycledown=1000

[mixer]
$MIXER_CONF

[sblaster]
$SB_CONF

[speaker]
pcspeaker=false

[autoexec]
mount c /home/tc/games/doom
c:
$EXEC_CMD
EOF

echo "Ejecutando prueba cronometrada..."
START_SEC=$(date +%s)
SDL_VIDEODRIVER=x11 SDL_VIDEO_X11_NODIRECTCOLOR=1 dosbox -conf $CONF
END_SEC=$(date +%s)
ELAPSED=$((END_SEC - START_SEC))
echo "-----------------------------------------------------"
echo "RESULTADO BENCHMARK:"
echo "  * Duracion de prueba: ${ELAPSED} segundos"
echo "  * Perfil: Cycles=$CYCLES | Frameskip=$FRAMESKIP | Audio=$AUDIO"
echo "====================================================="
"""

    bench_layers_script = b"""#!/bin/sh
clear
echo "====================================================="
echo "   RELEVAMIENTO DE CAPAS EN EJECUCION (A0 - A3)      "
echo "====================================================="
echo ""
echo "--- [1] MEMORIA RAM DEL SISTEMA ---"
free -m
echo ""
echo "--- [2] PROCESOS ACTIVOS (TOP CONSUMO) ---"
ps aux | head -n 15
echo ""
echo "--- [3] ESTADO DE CAPAS ACTIVAS ---"
echo -n "  * Capa A1 (Servidor Xvesa):       "
pgrep Xvesa >/dev/null && echo "ACTIVO" || echo "INACTIVO (Modo CLI puro A0)"
echo -n "  * Capa A1 (Window Manager FLWM):  "
pgrep flwm >/dev/null && echo "ACTIVO" || echo "INACTIVO"
echo -n "  * Capa A1 (Dock Wbar):            "
pgrep wbar >/dev/null && echo "ACTIVO" || echo "INACTIVO"
echo -n "  * Capa A2/A3 (Emulador DOSBox):   "
pgrep dosbox >/dev/null && echo "ACTIVO" || echo "INACTIVO"
echo "====================================================="
"""

    laboratorio_menu_script = rb"""#!/bin/sh
aterm -geometry 80x25+30+30 -title "Laboratorio de Benchmarking DOOM" -e sh -c '
while true; do
  clear
  echo "====================================================="
  echo "   LABORATORIO METODOLOGICO DE RENDIMIENTO (DOOM)    "
  echo "====================================================="
  echo " 1) Medir Consumo de Capas (RAM / CPU / Procesos)"
  echo " 2) Test A3: Cycles 5.000  (Timedemo reproducible)"
  echo " 3) Test A3: Cycles 7.500  (Timedemo reproducible)"
  echo " 4) Test A3: Cycles 10.000 (Timedemo reproducible)"
  echo " 5) Test A3: Cycles 15.000 (Timedemo reproducible)"
  echo " 6) Test A3: Cycles 20.000 (Timedemo reproducible)"
  echo " 7) Test A3: Frameskip 0 vs 1 (Comparativa)"
  echo " 8) Test A3: Audio ON vs OFF"
  echo " 9) Test Personalizado (Ingresar ciclos/frameskip)"
  echo " 0) Jugar DOOM Interactivo"
  echo " q) Salir"
  echo "-----------------------------------------------------"
  echo -n "Seleccione una opcion [0-9, q]: "
  read opt
  case "$opt" in
    1) /usr/local/bin/bench_layers; echo ""; echo "Presione ENTER para continuar..."; read dummy ;;
    2) /usr/local/bin/benchmark_doom 5000 0 off timedemo; echo "Presione ENTER..."; read dummy ;;
    3) /usr/local/bin/benchmark_doom 7500 0 off timedemo; echo "Presione ENTER..."; read dummy ;;
    4) /usr/local/bin/benchmark_doom 10000 0 off timedemo; echo "Presione ENTER..."; read dummy ;;
    5) /usr/local/bin/benchmark_doom 15000 0 off timedemo; echo "Presione ENTER..."; read dummy ;;
    6) /usr/local/bin/benchmark_doom 20000 0 off timedemo; echo "Presione ENTER..."; read dummy ;;
    7) 
       echo "--- Corriendo con Frameskip 0 (Todos los cuadros) ---"
       /usr/local/bin/benchmark_doom 10000 0 off timedemo
       echo "--- Corriendo con Frameskip 1 (Salto de 1 cuadro) ---"
       /usr/local/bin/benchmark_doom 10000 1 off timedemo
       echo "Presione ENTER para continuar..."; read dummy
       ;;
    8)
       echo "--- Corriendo con Audio OFF ---"
       /usr/local/bin/benchmark_doom 10000 1 off timedemo
       echo "--- Corriendo con Audio ON (Sound Blaster 16) ---"
       /usr/local/bin/benchmark_doom 10000 1 on timedemo
       echo "Presione ENTER para continuar..."; read dummy
       ;;
    9)
       echo -n "Ingrese Ciclos [ej: 8000]: "
       read c
       echo -n "Ingrese Frameskip [0, 1, 2]: "
       read fs
       echo -n "Audio [on/off]: "
       read aud
       /usr/local/bin/benchmark_doom "${c:-10000}" "${fs:-0}" "${aud:-off}" timedemo
       echo "Presione ENTER para continuar..."; read dummy
       ;;
    0) /usr/local/bin/play_doom ;;
    q|Q) exit 0 ;;
  esac
done
' &
"""

    gamemode_test_script = b"""#!/bin/sh
/usr/local/bin/benchmark_doom 10000 1 off play &
"""

    guia_metodologia_text = rb"""=== METODOLOGIA DE PRUEBAS DE RENDIMIENTO ===
Retro Virtual PC (Tiny Core Linux + DOSBox + DOOM)

OBJETIVO:
  Determinar cientificamente el perfil optimo de juego sin sobrecargar
  el motor WebAssembly ni la CPU anfitriona.

CAPAS ANALIZADAS:
  * A0: Tiny Core CLI (Consumo base sin interfaz grafica)
  * A1: Tiny Core GUI (Overhead de Xvesa y Window Manager)
  * A2: DOSBox Reposo (Overhead del emulador en C:\>)
  * A3: DOSBox + DOOM (Pruebas parametricas cronometradas con -timedemo demo1)
  * A4: Motor Nativo Linux (Chocolate Doom / PrBoom)

COMO REGISTRAR RESULTADOS:
  1. Abrir '1_Laboratorio_Benchmark.sh'
  2. Ejecutar cada prueba (5k, 7.5k, 10k, 15k, 20k)
  3. Anotar los segundos de duracion y la sensacion de fluidez.
  4. La configuracion con menor duracion y mayor suavidad sera la elegida
     para el Game Mode definitivo.
"""

    play_doom_script = b"""#!/bin/sh
cd /home/tc/games/doom
SDL_VIDEODRIVER=x11 SDL_VIDEO_X11_NODIRECTCOLOR=1 dosbox -conf /home/tc/games/doom/dosbox.conf
"""
    run_tree_script = b"""#!/bin/sh
tree /home/tc
"""
    run_dosbox_script = b"""#!/bin/sh
cd /home/tc
SDL_VIDEODRIVER=x11 SDL_VIDEO_X11_NODIRECTCOLOR=1 dosbox -conf /home/tc/.dosbox/dosbox-0.74-3.conf
"""

    doom_desktop = b"""[Desktop Entry]
Type=Application
Name=DOOM
Exec=/usr/local/bin/play_doom
Icon=/usr/local/share/pixmaps/doom.png
Categories=Game;
Terminal=false
"""
    dosbox_desktop = b"""[Desktop Entry]
Type=Application
Name=DOSBox
Exec=/usr/local/bin/dosbox_safe
Icon=/usr/local/share/pixmaps/dosbox.png
Categories=Game;Emulator;
Terminal=false
"""
    tree_desktop = b"""[Desktop Entry]
Type=Application
Name=Tree Explorer
Exec=tree /home/tc
Icon=/usr/local/share/pixmaps/tree.png
Categories=Utility;
Terminal=true
"""

    emelfm_conf = b"""# emelfm configuration
terminal aterm -e %s
filetype sh: sh %f &
filetype wad: /usr/local/bin/play_doom &
filetype txt: aterm -e cat %f &
"""

    # Inyección limpia preservando el entorno de escritorio original de Tiny Core
    injected_files = {
        'home/tc/Desktop': (b'', 0o040755),
        'home/tc/.dosbox': (b'', 0o040755),
        'home/tc/games': (b'', 0o040755),
        'home/tc/games/doom': (b'', 0o040755),
        'etc/skel/Desktop': (b'', 0o040755),
        'etc/skel/.dosbox': (b'', 0o040755),
        'etc/skel/games': (b'', 0o040755),
        'etc/skel/games/doom': (b'', 0o040755),
        'usr/local/share/pixmaps': (b'', 0o040755),
        'usr/local/share/applications': (b'', 0o040755),
        'home/tc/.emelfm': (b'', 0o040755),
        'etc/skel/.emelfm': (b'', 0o040755),
        
        # Scripts de Benchmarking y accesos directos
        'home/tc/Desktop/1_Laboratorio_Benchmark.sh': (laboratorio_menu_script, 0o100755),
        'home/tc/Desktop/2_DOOM_GameMode_Test.sh': (gamemode_test_script, 0o100755),
        'home/tc/Desktop/3_Start_DOSBox.sh': (run_dosbox_script, 0o100755),
        'home/tc/Desktop/4_Tree_Explorer.sh': (run_tree_script, 0o100755),
        'home/tc/Desktop/GUIA_METODOLOGIA.txt': (guia_metodologia_text, 0o100644),

        'etc/skel/Desktop/1_Laboratorio_Benchmark.sh': (laboratorio_menu_script, 0o100755),
        'etc/skel/Desktop/2_DOOM_GameMode_Test.sh': (gamemode_test_script, 0o100755),
        'etc/skel/Desktop/3_Start_DOSBox.sh': (run_dosbox_script, 0o100755),
        'etc/skel/Desktop/4_Tree_Explorer.sh': (run_tree_script, 0o100755),
        'etc/skel/Desktop/GUIA_METODOLOGIA.txt': (guia_metodologia_text, 0o100644),

        'home/tc/games/doom/play_doom.sh': (play_doom_script, 0o100755),
        'home/tc/games/doom/dosbox.conf': (dosbox_safe_conf, 0o100644),
        'home/tc/games/doom/PLAY_DOOM.BAT': (b"@echo off\r\ndoom.exe\r\n", 0o100755),
        'home/tc/games/doom/DOOM.BAT': (b"@echo off\r\ndoom.exe\r\n", 0o100755),
        'home/tc/games/doom/PLAY.BAT': (b"@echo off\r\ndoom.exe\r\n", 0o100755),
        'home/tc/.dosbox/dosbox-0.74-3.conf': (dosbox_interactive_conf, 0o100644),

        'etc/skel/games/doom/play_doom.sh': (play_doom_script, 0o100755),
        'etc/skel/games/doom/dosbox.conf': (dosbox_safe_conf, 0o100644),
        'etc/skel/games/doom/PLAY_DOOM.BAT': (b"@echo off\r\ndoom.exe\r\n", 0o100755),
        'etc/skel/games/doom/DOOM.BAT': (b"@echo off\r\ndoom.exe\r\n", 0o100755),
        'etc/skel/games/doom/PLAY.BAT': (b"@echo off\r\ndoom.exe\r\n", 0o100755),
        'etc/skel/.dosbox/dosbox-0.74-3.conf': (dosbox_interactive_conf, 0o100644),

        # Binarios globales en PATH
        'usr/local/bin/play_doom': (play_doom_script, 0o100755),
        'usr/local/bin/dosbox_safe': (run_dosbox_script, 0o100755),
        'usr/local/bin/benchmark_doom': (benchmark_doom_script, 0o100755),
        'usr/local/bin/bench_layers': (bench_layers_script, 0o100755),

        # Pixmaps e iconos de Wbar
        'usr/local/share/pixmaps/doom.png': (make_simple_png(32, 32, (220, 30, 30, 255)), 0o100644),
        'usr/local/share/pixmaps/dosbox.png': (make_simple_png(32, 32, (30, 100, 220, 255)), 0o100644),
        'usr/local/share/pixmaps/tree.png': (make_simple_png(32, 32, (40, 180, 60, 255)), 0o100644),

        # Desktop entries (para que Wbar y el menú FLWM los integren de forma nativa)
        'usr/local/share/applications/doom.desktop': (doom_desktop, 0o100644),
        'usr/local/share/applications/dosbox.desktop': (dosbox_desktop, 0o100644),
        'usr/local/share/applications/tree.desktop': (tree_desktop, 0o100644),

        # Configuración de asociaciones de EmelFM
        'home/tc/.emelfm/config': (emelfm_conf, 0o100644),
        'etc/skel/.emelfm/config': (emelfm_conf, 0o100644),
    }
    
    # Inyectar binarios reales de DOOM
    doom_files_dir = os.path.join(SCRIPT_DIR, "tinycore_build", "doom_files")
    for bname in ['doom.exe', 'doom1.wad']:
        bpath = os.path.join(doom_files_dir, bname)
        if os.path.exists(bpath):
            with open(bpath, "rb") as f:
                bdata = f.read()
            print(f"[*] Inyectando {bname} ({len(bdata):,} bytes) en initramfs...")
            injected_files[f'home/tc/games/doom/{bname}'] = (bdata, 0o100755)
            injected_files[f'etc/skel/games/doom/{bname}'] = (bdata, 0o100755)

    wad_path = os.path.join(FREEDOOM_DIR, "freedoom1.wad")
    if os.path.exists(wad_path):
        with open(wad_path, "rb") as f:
            wad_data = f.read()
        print(f"[*] Inyectando freedoom1.wad ({len(wad_data):,} bytes) en initramfs...")
        injected_files['home/tc/games/doom/freedoom1.wad'] = (wad_data, 0o100644)
        injected_files['etc/skel/games/doom/freedoom1.wad'] = (wad_data, 0o100644)

    retro_cpio = create_cpio_archive(injected_files)
    retro_cpio_gz = gzip.compress(retro_cpio, compresslevel=6, mtime=0)
    print(f"[*] Initramfs overlay generado: {len(retro_cpio_gz):,} bytes comprimidos")

    # 1. Copiar todos los archivos de /BOOT (VMLINUZ, CORE.GZ con overlay anexado)
    for child in base.list_children(iso_path='/BOOT'):
        ident = child.file_identifier().decode('utf-8')
        if ident in ('.', '..', 'ISOLINUX'):
            continue
        clean = ident.split(';')[0]
        buf = io.BytesIO()
        base.get_file_from_iso_fp(buf, iso_path=f'/BOOT/{ident}')
        data = buf.getvalue()
        
        if clean == 'CORE.GZ':
            # Anexar el overlay directamente a core.gz (soportado nativamente por Linux)
            data = data + retro_cpio_gz
            print(f"[+] /BOOT/CORE.GZ ampliado con overlay: {len(data):,} bytes")
            
        print(f"[+] /BOOT/{clean} ({len(data):,} bytes, magic={data[:8].hex()})")
        new_iso.add_fp(io.BytesIO(data), len(data), f'/BOOT/{clean}',
                       rr_name=clean.lower(), joliet_path=f'/boot/{clean.lower()}')

    # 2. Copiar archivos de /BOOT/ISOLINUX con isolinux.cfg directo
    cfg_direct = b"""DEFAULT tc
PROMPT 0
TIMEOUT 0

LABEL tc
  KERNEL /boot/vmlinuz
  INITRD /boot/core.gz
  APPEND loglevel=3 cde restore=cde

LABEL core
  KERNEL /boot/vmlinuz
  INITRD /boot/core.gz
  APPEND loglevel=3
"""
    for child in base.list_children(iso_path='/BOOT/ISOLINUX'):
        ident = child.file_identifier().decode('utf-8')
        if ident in ('.', '..', 'BOOT.CAT'):
            continue
        clean = ident.split(';')[0]
        if clean == 'ISOLINUX.CFG':
            data = cfg_direct
        else:
            buf = io.BytesIO()
            base.get_file_from_iso_fp(buf, iso_path=f'/BOOT/ISOLINUX/{ident}')
            data = buf.getvalue()
        
        new_iso.add_fp(io.BytesIO(data), len(data), f'/BOOT/ISOLINUX/{clean}',
                       rr_name=clean.lower(), joliet_path=f'/boot/isolinux/{clean.lower()}')

    # 3. Copiar extensiones originales de /CDE/OPTIONAL preservando Rock Ridge exacto
    existing_tcz = set()
    for child in base.list_children(iso_path='/CDE/OPTIONAL'):
        ident = child.file_identifier().decode('utf-8')
        if ident in ('.', '..'):
            continue
        clean = ident.split(';')[0]
        rr_name = child.rock_ridge.name().decode('utf-8') if hasattr(child, 'rock_ridge') and child.rock_ridge else clean
        existing_tcz.add(rr_name.lower())
        buf = io.BytesIO()
        base.get_file_from_iso_fp(buf, iso_path=f'/CDE/OPTIONAL/{ident}')
        data = buf.getvalue()
        new_iso.add_fp(io.BytesIO(data), len(data), f'/CDE/OPTIONAL/{clean}',
                       rr_name=rr_name, joliet_path=f'/cde/optional/{rr_name}')

    # 4. Agregar nuevas extensiones
    packages = [
        "dosbox.tcz", "sdl-sound.tcz", "SDL.tcz",
        "libvorbis.tcz", "libogg.tcz", "libmad.tcz",
        "libasound.tcz", "tree.tcz",
        "emelfm.tcz", "gtk1.tcz", "glib1.tcz"
    ]
    for pkg in packages:
        src = os.path.join(TCZ_DIR, pkg)
        if not os.path.exists(src):
            continue
        if pkg.lower() in existing_tcz:
            continue
        with open(src, "rb") as f:
            data = f.read()
        iso_name = pkg.upper().replace('-', '_')
        new_iso.add_fp(io.BytesIO(data), len(data), f'/CDE/OPTIONAL/{iso_name}',
                       rr_name=pkg, joliet_path=f'/cde/optional/{pkg}')
        print(f"[+] Agregada extension: {pkg} ({len(data):,} bytes)")

    # 5. Listas onboot.lst / xbase.lst / copy2fs.lst
    entries = [
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
        "tree.tcz", "glib1.tcz", "gtk1.tcz", "emelfm.tcz"
    ]
    onboot = ("\n".join(entries) + "\n").encode()
    new_iso.add_fp(io.BytesIO(onboot), len(onboot), '/CDE/ONBOOT.LST',
                   rr_name='onboot.lst', joliet_path='/cde/onboot.lst')
    new_iso.add_fp(io.BytesIO(onboot), len(onboot), '/CDE/XBASE.LST',
                   rr_name='xbase.lst', joliet_path='/cde/xbase.lst')
    new_iso.add_fp(io.BytesIO(onboot), len(onboot), '/CDE/COPY2FS.LST',
                   rr_name='copy2fs.lst', joliet_path='/cde/copy2fs.lst')

    # 6. El Torito
    new_iso.add_eltorito('/BOOT/ISOLINUX/ISOLINUX.BIN')

    print(f"[*] Escribiendo {OUTPUT_ISO}...")
    new_iso.write(OUTPUT_ISO)
    new_iso.close()
    base.close()

    print("[*] Restaurando isolinux.bin original y parcheando boot info table...")
    SECTOR = 2048
    with open(OUTPUT_ISO, "rb") as f:
        raw = bytearray(f.read())

    for s in range(16, 32):
        off = s * SECTOR
        if raw[off] == 0x00 and raw[off+1:off+6] == b'CD001':
            cat_lba = struct.unpack_from('<I', raw, off + 0x47)[0]
            entry_off = cat_lba * SECTOR + 0x20
            boot_lba = struct.unpack_from('<I', raw, entry_off + 8)[0]
            print(f"[PATCH] El Torito Boot Catalog @ {cat_lba}, Boot Image @ LBA {boot_lba}")
            break

    boot_offset = boot_lba * SECTOR
    raw[boot_offset : boot_offset + len(isolinux_bin_data)] = isolinux_bin_data

    isolinux_len = len(isolinux_bin_data)
    checksum = 0
    for i in range(boot_offset + 64, boot_offset + isolinux_len, 4):
        checksum = (checksum + struct.unpack_from('<I', raw, i)[0]) & 0xFFFFFFFF

    bit = boot_offset + 8
    struct.pack_into('<I', raw, bit + 0,  16)
    struct.pack_into('<I', raw, bit + 4,  boot_lba)
    struct.pack_into('<I', raw, bit + 8,  isolinux_len)
    struct.pack_into('<I', raw, bit + 12, checksum)

    print(f"[PATCH] Boot info table: pvd=16, boot_lba={boot_lba}, file_len={isolinux_len}, checksum=0x{checksum:08X}")

    with open(OUTPUT_ISO, "wb") as f:
        f.write(raw)

    print(f"[EXITO] ISO generada: {os.path.getsize(OUTPUT_ISO):,} bytes!")

if __name__ == "__main__":
    build_perfect_retro_iso()
