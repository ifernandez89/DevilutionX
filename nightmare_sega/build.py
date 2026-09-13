"""
build.py - Master Build Script for NIGHTMARE (Sega Genesis)
1. Runs asset generators
2. Compiles 68000 assembly using vasmm68k_mot
3. Calculates and patches Genesis ROM checksum
4. Outputs both .bin (flat binary) and .smd (Super Magic Drive interleaved)
5. Copies outputs to emulator directory and user ROM folder
"""

import os
import subprocess
import shutil

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
TOOLS_DIR = os.path.join(ROOT_DIR, 'tools')
SRC_DIR = os.path.join(ROOT_DIR, 'src')
BUILD_DIR = os.path.join(ROOT_DIR, 'build')
VASM_EXE = os.path.join(TOOLS_DIR, 'vasmm68k_mot.exe')

GENS_DIR = os.path.join(ROOT_DIR, '..', 'gens-win32-bin-2.14')
USER_SEGA_ROMS_DIR = r"I:\Juegos\Hours of Fun\Roms\Sega"

def bin_to_smd(bin_data):
    """
    Converts raw binary Genesis ROM to Super Magic Drive (SMD) interleaved format.
    SMD structure:
    - 512 bytes SMD header
    - 16 KB interleaved blocks:
        Each 16384-byte block is split:
        First 8192 bytes = odd bytes of the 16KB block
        Next 8192 bytes = even bytes of the 16KB block
    """
    smd_data = bytearray()
    
    # 512 bytes header
    num_blocks = len(bin_data) // 16384
    hdr = bytearray(512)
    hdr[0] = num_blocks & 0xFF
    hdr[1] = 0x03
    hdr[2] = 0x00
    hdr[8] = 0xAA
    hdr[9] = 0xBB
    hdr[10] = 0x06
    smd_data.extend(hdr)

    # Interleaved blocks
    for b in range(0, len(bin_data), 16384):
        block = bin_data[b:b+16384]
        # pad block to 16KB if needed
        if len(block) < 16384:
            block = block.ljust(16384, b'\x00')
        
        even_bytes = block[0::2]
        odd_bytes = block[1::2]
        smd_data.extend(odd_bytes)
        smd_data.extend(even_bytes)

    return bytes(smd_data)

def calculate_checksum(rom_data):
    """Calculates Genesis header checksum (sum of words from 0x200 to end)."""
    checksum = 0
    for i in range(0x200, len(rom_data), 2):
        word = (rom_data[i] << 8) | rom_data[i+1]
        checksum = (checksum + word) & 0xFFFF
    return checksum

def main():
    print("=== BUILDING NIGHTMARE: ASHENMOOR (SEGA GENESIS) ===")
    os.makedirs(BUILD_DIR, exist_ok=True)

    # 1. Generate Assets
    print("-> Generating tiles & palettes...")
    subprocess.run([sys.executable, os.path.join(TOOLS_DIR, 'make_assets.py')], check=True)

    # 2. Generate Maps
    print("-> Generating dungeon levels & spawns...")
    subprocess.run([sys.executable, os.path.join(TOOLS_DIR, 'make_maps.py')], check=True)

    # 3. Assemble with VASM
    bin_output = os.path.join(BUILD_DIR, 'nightmare_demo.bin')
    main_asm = os.path.join(SRC_DIR, 'main.asm')

    print(f"-> Assembling {main_asm} with vasmm68k_mot...")
    cmd = [VASM_EXE, '-spaces', '-Fbin', '-nosym', '-o', bin_output, main_asm]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print("VASM ERROR:")
        print(res.stderr or res.stdout)
        sys.exit(1)
    print("-> Assembly successful!")

    # 4. Patch Checksum
    with open(bin_output, 'rb') as f:
        rom_data = bytearray(f.read())

    cs = calculate_checksum(rom_data)
    print(f"-> Calculated Genesis Checksum: 0x{cs:04X}")
    rom_data[0x18E] = (cs >> 8) & 0xFF
    rom_data[0x18F] = cs & 0xFF

    with open(bin_output, 'wb') as f:
        f.write(rom_data)

    # 5. Generate SMD format
    print("-> Converting to Super Magic Drive (.smd) format...")
    smd_data = bin_to_smd(rom_data)
    smd_output = os.path.join(BUILD_DIR, 'nightmare_demo.smd')
    with open(smd_output, 'wb') as f:
        f.write(smd_data)

    # 6. Copy outputs to project root, Gens emulator folder, and ROMs folder
    targets = [
        os.path.join(ROOT_DIR, 'nightmare_demo.smd'),
        os.path.join(ROOT_DIR, 'nightmare_demo.bin'),
        os.path.join(GENS_DIR, 'nightmare_demo.smd'),
        os.path.join(GENS_DIR, 'nightmare_demo.bin'),
    ]
    if os.path.exists(USER_SEGA_ROMS_DIR):
        targets.append(os.path.join(USER_SEGA_ROMS_DIR, 'nightmare_demo.smd'))
        targets.append(os.path.join(USER_SEGA_ROMS_DIR, 'nightmare_demo.bin'))

    print("-> Deploying ROM copies:")
    for t in targets:
        src = smd_output if t.endswith('.smd') else bin_output
        shutil.copy2(src, t)
        print(f"   [OK] {t}")

    print("\n=== BUILD COMPLETE! ===")
    print(f"ROM Size: {len(rom_data) // 1024} KB")
    print(f"Playable in Gens emulator: {os.path.join(GENS_DIR, 'nightmare_demo.smd')}")

if __name__ == '__main__':
    import sys
    main()
