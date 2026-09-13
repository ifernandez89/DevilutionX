"""
make_assets.py - Asset and Tile Generator for NIGHTMARE (Sega Genesis)
Generates 4-bit indexed 8x8 tiles and converts them to 68000 assembly data.
Each 8x8 tile = 32 bytes (4 bits per pixel, 2 pixels per byte, 8 rows of 4 bytes).
"""

import os
import sys

def pack_tile(pixels):
    """
    pixels: list of 64 integers (0-15) representing an 8x8 tile (row by row).
    Returns 32 bytes.
    """
    assert len(pixels) == 64
    tile_bytes = bytearray()
    for row in range(8):
        for col in range(0, 8, 2):
            p0 = pixels[row * 8 + col] & 0x0F
            p1 = pixels[row * 8 + col + 1] & 0x0F
            tile_bytes.append((p0 << 4) | p1)
    return tile_bytes

def parse_ascii_tile(ascii_art, char_map):
    """
    ascii_art: string with 8 lines of 8 chars.
    char_map: dict mapping char to palette index (0-15).
    """
    lines = [line.strip() for line in ascii_art.strip().split('\n') if line.strip()]
    assert len(lines) == 8, f"Expected 8 lines, got {len(lines)}"
    pixels = []
    for line in lines:
        assert len(line) == 8, f"Line '{line}' must be 8 chars"
        for ch in line:
            pixels.append(char_map.get(ch, 0))
    return pack_tile(pixels)

def create_font():
    """Generates 8x8 bitmap font tiles (Digits 0-9, Letters A-Z, symbols)."""
    font_tiles = {}
    # Simple clean 8x8 bitmap definitions for UI
    patterns = {
        ' ': ["........"] * 8,
        '0': [".XXXX..", "X....X.", "X...XX.", "X..X.X.", "X.X..X.", "XX...X.", ".XXXX..", "......."],
        '1': ["...XX..", "..XXX..", ".X.XX..", "...XX..", "...XX..", "...XX..", ".XXXXX.", "......."],
        '2': [".XXXX..", "X....X.", ".....X.", "...XX..", ".XX....", "X......", "XXXXXX.", "......."],
        '3': [".XXXX..", "X....X.", ".....X.", "..XXX..", ".....X.", "X....X.", ".XXXX..", "......."],
        '4': ["X....X.", "X....X.", "X....X.", "XXXXXX.", ".....X.", ".....X.", ".....X.", "......."],
        '5': ["XXXXXX.", "X......", "XXXXX..", ".....X.", ".....X.", "X....X.", ".XXXX..", "......."],
        '6': [".XXXX..", "X......", "XXXXX..", "X....X.", "X....X.", "X....X.", ".XXXX..", "......."],
        '7': ["XXXXXX.", ".....X.", "....X..", "...X...", "..X....", "..X....", "..X....", "......."],
        '8': [".XXXX..", "X....X.", "X....X.", ".XXXX..", "X....X.", "X....X.", ".XXXX..", "......."],
        '9': [".XXXX..", "X....X.", "X....X.", ".XXXXX.", ".....X.", ".....X.", ".XXXX..", "......."],
        ':': [".......", "..XX...", "..XX...", ".......", "..XX...", "..XX...", ".......", "......."],
        '-': [".......", ".......", ".......", "XXXXXX.", ".......", ".......", ".......", "......."],
        '/': [".....X.", "....X..", "...X...", "..X....", ".X.....", "X......", ".......", "......."],
        'A': [".XXXX..", "X....X.", "X....X.", "XXXXXX.", "X....X.", "X....X.", "X....X.", "......."],
        'B': ["XXXXX..", "X....X.", "X....X.", "XXXXX..", "X....X.", "X....X.", "XXXXX..", "......."],
        'C': [".XXXX..", "X....X.", "X......", "X......", "X......", "X....X.", ".XXXX..", "......."],
        'D': ["XXXX...", "X...X..", "X....X.", "X....X.", "X....X.", "X...X..", "XXXX...", "......."],
        'E': ["XXXXXX.", "X......", "X......", "XXXXX..", "X......", "X......", "XXXXXX.", "......."],
        'F': ["XXXXXX.", "X......", "X......", "XXXXX..", "X......", "X......", "X......", "......."],
        'G': [".XXXX..", "X....X.", "X......", "X..XXX.", "X....X.", "X....X.", ".XXXX..", "......."],
        'H': ["X....X.", "X....X.", "X....X.", "XXXXXX.", "X....X.", "X....X.", "X....X.", "......."],
        'I': [".XXXX..", "...X...", "...X...", "...X...", "...X...", "...X...", ".XXXX..", "......."],
        'K': ["X....X.", "X...X..", "X..X...", "XXX....", "X..X...", "X...X..", "X....X.", "......."],
        'L': ["X......", "X......", "X......", "X......", "X......", "X......", "XXXXXX.", "......."],
        'M': ["X....X.", "XX..XX.", "X.XX.X.", "X.XX.X.", "X....X.", "X....X.", "X....X.", "......."],
        'N': ["X....X.", "XX...X.", "XX...X.", "X.X..X.", "X..X.X.", "X...XX.", "X....X.", "......."],
        'O': [".XXXX..", "X....X.", "X....X.", "X....X.", "X....X.", "X....X.", ".XXXX..", "......."],
        'P': ["XXXXX..", "X....X.", "X....X.", "XXXXX..", "X......", "X......", "X......", "......."],
        'R': ["XXXXX..", "X....X.", "X....X.", "XXXXX..", "X...X..", "X....X.", "X....X.", "......."],
        'S': [".XXXX..", "X....X.", "X......", ".XXXX..", ".....X.", "X....X.", ".XXXX..", "......."],
        'T': ["XXXXXX.", "...X...", "...X...", "...X...", "...X...", "...X...", "...X...", "......."],
        'U': ["X....X.", "X....X.", "X....X.", "X....X.", "X....X.", "X....X.", ".XXXX..", "......."],
        'V': ["X....X.", "X....X.", "X....X.", "X....X.", ".X..X..", ".X..X..", "..XX...", "......."],
        'W': ["X....X.", "X....X.", "X....X.", "X.XX.X.", "X.XX.X.", "XX..XX.", "X....X.", "......."],
        'Y': ["X....X.", "X....X.", ".X..X..", "..XX...", "...X...", "...X...", "...X...", "......."],
        '!': ["..XX...", "..XX...", "..XX...", "..XX...", ".......", "..XX...", "..XX...", "......."],
    }
    for ch, grid in patterns.items():
        # Ensure 8 lines of 8 chars
        norm_grid = [row.ljust(8, '.')[:8] for row in grid]
        ascii_tile = '\n'.join(norm_grid)
        font_tiles[ch] = parse_ascii_tile(ascii_tile, {'.': 0, 'X': 13}) # color 13 = bright text
    return font_tiles

def create_dungeon_tiles():
    """Generates dungeon wall, floor, stairs, and decoration tiles (Palette 0)."""
    tiles = []
    
    # 0: Empty / Void
    tiles.append(parse_ascii_tile("""
........
........
........
........
........
........
........
........
""", {'.': 0}))

    # 1: Stone Floor 1 (clean stone tile)
    tiles.append(parse_ascii_tile("""
========
=......=
=.----.=
=.-..-.=
=.-..-.=
=.----.=
=......=
========
""", {'.': 3, '-': 4, '=': 2}))

    # 2: Stone Floor 2 (cracked stone)
    tiles.append(parse_ascii_tile("""
========
=..C...=
=.C--..=
=..C.-.=
=...C-.=
=.--.C.=
=......=
========
""", {'.': 3, '-': 4, '=': 2, 'C': 1}))

    # 3: Stone Floor 3 (blood stained)
    tiles.append(parse_ascii_tile("""
========
=......=
=.B---.=
=.BB.--=
=..BB.-=
=.---B.=
=......=
========
""", {'.': 3, '-': 4, '=': 2, 'B': 10}))

    # 4: Wall Top (solid dark stone brick edge)
    tiles.append(parse_ascii_tile("""
########
########
###**###
#******#
#*####*#
#******#
########
########
""", {'#': 2, '*': 4}))

    # 5: Wall Front Face (gothic stonework blocks)
    tiles.append(parse_ascii_tile("""
********
*######*
*#....#*
*#....#*
********
*######*
*#....#*
********
""", {'*': 5, '#': 2, '.': 3}))

    # 6: Wall with Torch (flickering flame)
    tiles.append(parse_ascii_tile("""
*..FF..*
*#FYYF#*
*#YYYY#*
*#FYYF#*
***WW***
*#WWWW#*
*##WW##*
********
""", {'*': 5, '#': 2, '.': 3, 'W': 7, 'F': 12, 'Y': 13}))

    # 7: Stairs Down (dark abyss opening with steps)
    tiles.append(parse_ascii_tile("""
========
=SSSSSS=
=S====S=
=SssssS=
=Ss..sS=
=Ss..sS=
=SssssS=
========
""", {'=': 5, 'S': 2, 's': 1, '.': 0}))

    # 8: Altar / Pillar Base
    tiles.append(parse_ascii_tile("""
..****..
.******.
.######.
.######.
.######.
.######.
.******.
..****..
""", {'*': 4, '#': 2, '.': 0}))

    # 9: Runic Floor Tile (Boss Arena)
    tiles.append(parse_ascii_tile("""
=RR..RR=
R..RR..R
R.RRRR.R
..RRRR..
..RRRR..
R.RRRR.R
R..RR..R
=RR..RR=
""", {'=': 2, '.': 3, 'R': 10})) # Red runes in blood

    return tiles

def create_sorceress_sprites():
    """
    Generates Sorceress sprites (Palette 1).
    Feminine silhouette with purple robe, golden hair, arcane staff.
    Dimensions: 16x24 pixels (2x3 tiles).
    Generates: Down Idle, Down Walk, Up Walk, Left Walk, Right Walk, Cast.
    """
    sprites = []
    
    # Color map for Sorceress:
    # . = transparent (0)
    # H = blonde hair (3, 4)
    # S = skin face/hands (1, 2)
    # R = purple robe (5, 6, 7)
    # G = gold trim (8)
    # W = wood staff (9)
    # C = crystal gem (10, 11)
    # B = boots (12)
    # O = black outline (15)
    cmap = {
        '.': 0, 'S': 1, 's': 2, 'H': 3, 'h': 4,
        'R': 6, 'r': 5, 'L': 7, 'G': 8, 'W': 9,
        'C': 11, 'c': 10, 'B': 12, 'O': 15
    }

    # Helper to slice 16x24 ASCII into 6 8x8 tiles (column-major order as Genesis hardware expects: Tile(0,0), Tile(0,1), Tile(0,2), Tile(1,0), Tile(1,1), Tile(1,2))
    def parse_16x24(ascii_lines):
        lines = [line.strip() for line in ascii_lines.strip().split('\n') if line.strip()]
        assert len(lines) == 24
        for row in lines:
            assert len(row) == 16, f"Row length {len(row)} != 16: '{row}'"
        
        # Extract 2 columns of 3 tiles each
        result_tiles = []
        for col_idx in [0, 8]:
            for row_idx in [0, 8, 16]:
                tile_ascii = []
                for r in range(8):
                    tile_ascii.append(lines[row_idx + r][col_idx:col_idx + 8])
                result_tiles.append(parse_ascii_tile('\n'.join(tile_ascii), cmap))
        return result_tiles

    # Frame 1: Sorceress Facing Down (Front)
    front_idle = """
....OO....OO....
...OHHHO.OHHHO..
..OHHHHHOHHHHO..
..OHHSSHHHHSSHO.
...OSSOSSSOSS...
....SSSSSSSS....
....OSSssSSO....
...ORRRRRRRRO...
...ORRRRRRRRO...
...ORRRGGRRRO...
...ORRRGGRRRO...
..WORRRLRRROW...
..CORRRRRRRROC..
..cORRRRRRRROc..
..WORRRRRRRROW..
...ORRRRRRRRO...
...ORRR..RRRO...
...ORRR..RRRO...
...ORRR..RRRO...
...OBB....BBO...
...OBBO..OBBO...
....OO....OO....
................
................
"""
    # Frame 2: Sorceress Walking Down Step
    front_walk = """
....OO....OO....
...OHHHO.OHHHO..
..OHHHHHOHHHHO..
..OHHSSHHHHSSHO.
...OSSOSSSOSS...
....SSSSSSSS....
....OSSssSSO....
...ORRRRRRRRO...
...ORRRRRRRRO...
...ORRRGGRRRO...
...ORRRGGRRRO...
..WORRRLRRROW...
..CORRRRRRRROC..
..cORRRRRRRROc..
..WORRRRRRRROW..
...ORRRRRRRRO...
...ORRR..RRRO...
...ORRR..RRRO...
...OBBO..RRRO...
...OBBO..BBOO...
....OO...OBBO...
..........OO....
................
................
"""

    # Frame 3: Sorceress Facing Up (Back)
    back_walk = """
....OO....OO....
...OHHHHHHHHHO..
..OHHHHHHHHHHO..
..OHHHHHHHHHHO..
...OHHHHHHHHO...
...OHHHHHHHHO...
....OHHHHHHO....
...ORRRRRRRRO...
...ORRRRRRRRO...
...ORRRGGRRRO...
...ORRRGGRRRO...
..WORRRRRRRROW..
..CORRRRRRRROC..
..cORRRRRRRROc..
..WORRRRRRRROW..
...ORRRRRRRRO...
...ORRR..RRRO...
...ORRR..RRRO...
...OBBO..RRRO...
...OBBO..BBO....
....OO...OBBO...
..........OO....
................
................
"""

    # Frame 4: Sorceress Facing Side (Right)
    side_walk = """
......OO........
....OHHHHO......
...OHHHHHHO.....
...OHHSSSHHO....
...OHSSSSHO.....
....OSSSSS......
....OSSssO......
...ORRRRRO......
..WORRRRRO......
..CORRGGGRO.....
..cORRLRRRO.....
..WORRRRRRO.....
...ORRRRRRO.....
...ORRRRRRO.....
...ORRRRRRO.....
...ORRRRRRO.....
...ORRR.RRO.....
...ORRR.RRO.....
...OBBO.RRO.....
...OBBO.BBO.....
....OO..OBBO....
.........OO.....
................
................
"""

    # Frame 5: Sorceress Casting Spell (Staff Raised!)
    cast_spell = """
..CC..OO........
..cc.OHHHHO.....
..WWOHHHHHHO....
...WOHHSSSHHO...
...WOHSSSSHO....
...W.OSSSSS.....
...W.OSSssO.....
..W.ORRRRRO.....
..W.ORRRRRO.....
...WORRGGGRO....
...WORRLRRRO....
...WORRRRRRO....
....ORRRRRRO....
....ORRRRRRO....
....ORRRRRRO....
....ORRRRRRO....
....ORRR.RRO....
....ORRR.RRO....
....OBBO.RRO....
....OBBO.BBO....
.....OO..OBBO...
..........OO....
................
................
"""

    for anim in [front_idle, front_walk, back_walk, side_walk, cast_spell]:
        sprites.extend(parse_16x24(anim))
    
    return sprites

def create_monster_sprites():
    """
    Generates Monster sprites (Palette 2).
    1. Skeleton Devotee: 16x24 (6 tiles)
    2. Plague Crawler / Ghoul: 16x16 (4 tiles)
    """
    cmap = {
        '.': 0, 'B': 1, 'b': 2, 'D': 3, 'd': 4,
        'E': 5, 'M': 6, 'm': 7, 'F': 8, 'f': 9,
        'R': 10, 'P': 11, 'O': 15
    }
    
    def parse_16x24(ascii_lines):
        lines = [line.strip() for line in ascii_lines.strip().split('\n') if line.strip()]
        assert len(lines) == 24
        result_tiles = []
        for col_idx in [0, 8]:
            for row_idx in [0, 8, 16]:
                tile_ascii = []
                for r in range(8):
                    tile_ascii.append(lines[row_idx + r][col_idx:col_idx + 8])
                result_tiles.append(parse_ascii_tile('\n'.join(tile_ascii), cmap))
        return result_tiles

    def parse_16x16(ascii_lines):
        lines = [line.strip() for line in ascii_lines.strip().split('\n') if line.strip()]
        assert len(lines) == 16
        result_tiles = []
        for col_idx in [0, 8]:
            for row_idx in [0, 8]:
                tile_ascii = []
                for r in range(8):
                    tile_ascii.append(lines[row_idx + r][col_idx:col_idx + 8])
                result_tiles.append(parse_ascii_tile('\n'.join(tile_ascii), cmap))
        return result_tiles

    skeleton_front = """
....OOOOOO......
...OBBBBBBO.....
..OBBD..DBBO....
..OBEE..EEBO....
..OBDD..DDBO....
...OBBBBBBO.....
....ODD..DDO....
...OMMMMMMMO....
...OMMMMMMMO....
...OMMMMMMMO....
..mOMMMMMMMOm...
..m.ODDDDDDO.m..
..m..OB..BO..m..
..m..OB..BO..m..
..m..OB..BO..m..
.....OB..BO.....
.....OD..DDO....
.....OD..DDO....
.....OB..BBO....
.....OB..BBO....
....OBB..BBO....
....OOOOOOOO....
................
................
"""

    skeleton_walk = """
....OOOOOO......
...OBBBBBBO.....
..OBBD..DBBO....
..OBEE..EEBO....
..OBDD..DDBO....
...OBBBBBBO.....
....ODD..DDO....
...OMMMMMMMO....
...OMMMMMMMO....
...OMMMMMMMO....
..mOMMMMMMMOm...
..m.ODDDDDDO.m..
..m..OB..BO..m..
..m..OB..BO..m..
..m..OB..BO..m..
.....OB..BO.....
....OBD..DDO....
....OBD...DDO...
...OBB.....BBO..
...OBB.....BBO..
...OOOO...OOOO..
................
................
................
"""

    ghoul_quad = """
....OOOO........
...OPPPPOO......
..OPPPPPPPO.....
..OPEEEEPPO.....
.OPPPPPPPPPPO...
.OPPOOOOOOPPO...
.OPO......OPO...
OOOO......OOOO..
.OPO......OPO...
.OPPO....OPPO...
.OPPPO..OPPPO...
..OPPO..OPPO....
..OPO....OPO....
.OOPO....OPOO...
.OOOO....OOOO...
................
"""

    tiles = []
    tiles.extend(parse_16x24(skeleton_front))
    tiles.extend(parse_16x24(skeleton_walk))
    tiles.extend(parse_16x16(ghoul_quad))
    return tiles

def create_boss_sprites():
    """
    Generates 'The Flayer' Boss sprite (Palette 3).
    Dimensions: 32x32 pixels (4x4 hardware tiles = 16 tiles!).
    Massive demon with butcher cleaver, iron horns, spiked armor.
    """
    cmap = {
        '.': 0, 'H': 1, 'h': 2, 'M': 3, 'm': 4,
        'C': 5, 'W': 6, 'D': 7, 'F': 8, 'I': 9,
        'S': 10, 'G': 11, 'R': 12, 'T': 13, 't': 14, 'O': 15
    }

    # 32 lines of 32 characters
    boss_art = """
.......OOOO..........OOOO.......
......OIIIHO........OHIIIO......
.....OIIIIIIO......OIIIIIIO.....
....OIIHHHHIIOOOOOOIIHHHHIIO....
...OIIHHDDDHIIIIIIIIHDDHHIIO....
...OIIHDDDDDDDDDDDDDDDDHHIIO....
....OHDDDDDDDDDDDDDDDDDDHO......
.....ODDDRRRDDRRRDDDRRDDDO......
.....ODDDRFRDDRFRDDDRFRDDO......
.....ODDDRRRDDRRRDDDRRRDDO......
.....ODDDDDDDDGGDDDDDDDDO.......
......ODDDDDDDGGDDDDDDDO........
.....OODDDDDSSGGSSDDDDDOO.......
....OIIODDDSGGGGSSDDDDOIIO......
...OIIIODDGGGGGGGSSDDDOIIIO.....
..OIIIIODDGGGGGGGGSDDDOIIIIO....
..OIIIIODDGGGGGGGGSDDDOIIIIO....
...OIIIODDGGGGGGGSSDDDOIIIO.....
....OIIODDDSSGGSSDDDDDDOIIO.....
.....OODDDDDSSGGSSDDDDDOO.......
......ODDDDDDDGGDDDDDDDO........
......ODDDDDDDRRDDDDDDDO........
......ODDDDDDRRFRDDDDDDO........
......ODDDDDDRFRRDDDDDDO........
.....OODDDDDDRRFRDDDDDDOO.......
....OIIIODDDDRRFRDDDDDOIIIO.....
...OIIIIODDDDRRFRDDDDDOIIIIO....
..OIIIIIIODDDRRFRDDDDOIIIIIIO...
..OIIIIIIODDDRRFRDDDDOIIIIIIO...
...OOOOOOODDDRRFRDDDDOOOOOOO....
..........ODDDRRDDDDO...........
...........OOOOOOOO.............
"""
    lines = [line.strip() for line in boss_art.strip().split('\n') if line.strip()]
    assert len(lines) == 32
    for r in lines:
        assert len(r) == 32, f"Row len {len(r)} != 32"

    tiles = []
    # 4 columns of 4 tiles (column-major order for hardware sprite)
    for col_idx in [0, 8, 16, 24]:
        for row_idx in [0, 8, 16, 24]:
            tile_ascii = []
            for r in range(8):
                tile_ascii.append(lines[row_idx + r][col_idx:col_idx + 8])
            tiles.append(parse_ascii_tile('\n'.join(tile_ascii), cmap))
    return tiles

def create_fx_and_hud_tiles():
    """Generates projectile, hit effect, potion, and HUD orb tiles."""
    tiles = []
    cmap = {
        '.': 0, 'R': 1, 'r': 2, 'B': 3, 'b': 4,
        'C': 5, 'W': 6, 'G': 11, 'Y': 12, 'T': 13, 'O': 15
    }

    # 1. Arcane Bolt Projectile (8x8)
    bolt = """
...WW...
..WCCW..
.WCCCCW.
.WCCCCW.
..WCCW..
...WW...
...WW...
....W...
"""
    tiles.append(parse_ascii_tile(bolt, cmap))

    # 2. Magic Impact Spark (8x8)
    spark = """
.W....W.
..W..W..
...WW...
.WWCCWW.
.WWCCWW.
...WW...
..W..W..
.W....W.
"""
    tiles.append(parse_ascii_tile(spark, cmap))

    # 3. Health Potion Drop (8x8)
    potion = """
...WW...
...WW...
..WRRW..
.WRRRRW.
.WRRRRW.
.WRRRRW.
..WRRW..
...WW...
"""
    tiles.append(parse_ascii_tile(potion, cmap))

    # 4. HUD Life Orb Full (16x16 = 4 tiles: (0,0), (0,1), (1,0), (1,1))
    life_orb = """
.....OOOOOO.....
...OORRRRRROO...
..OORRRRRRRROO..
.OORRRRRRRRRROO.
.ORRRRRRRRRRRRO.
OORRRRRRRRRRRROO
OORRRRRRRRRRRROO
OORRRRRRRRRRRROO
OORRRRRRRRRRRROO
OORRRRRRRRRRRROO
OORRRRRRRRRRRROO
.ORRRRRRRRRRRRO.
.OORRRRRRRRRROO.
..OORRRRRRRROO..
...OORRRRRROO...
.....OOOOOO.....
"""
    lines = [line.strip() for line in life_orb.strip().split('\n') if line.strip()]
    for col_idx in [0, 8]:
        for row_idx in [0, 8]:
            tile_ascii = []
            for r in range(8):
                tile_ascii.append(lines[row_idx + r][col_idx:col_idx + 8])
            tiles.append(parse_ascii_tile('\n'.join(tile_ascii), cmap))

    # 5. HUD Mana Orb Full (16x16 = 4 tiles)
    mana_orb = """
.....OOOOOO.....
...OOBBBBBBOO...
..OOBBBBBBBBOO..
.OOBBBBBBBBBBOO.
.OBBBBBBBBBBBBO.
OOBBBBBBBBBBBBOO
OOBBBBBBBBBBBBOO
OOBBBBBBBBBBBBOO
OOBBBBBBBBBBBBOO
OOBBBBBBBBBBBBOO
OOBBBBBBBBBBBBOO
.OBBBBBBBBBBBBO.
.OOBBBBBBBBBBOO.
..OOBBBBBBBBOO..
...OOBBBBBBOO...
.....OOOOOO.....
"""
    lines = [line.strip() for line in mana_orb.strip().split('\n') if line.strip()]
    for col_idx in [0, 8]:
        for row_idx in [0, 8]:
            tile_ascii = []
            for r in range(8):
                tile_ascii.append(lines[row_idx + r][col_idx:col_idx + 8])
            tiles.append(parse_ascii_tile('\n'.join(tile_ascii), cmap))

    return tiles

def export_tiles_to_asm(output_path):
    """Compiles all tiles into a unified 68k assembly include file."""
    font = create_font()
    dungeon = create_dungeon_tiles()
    sorceress = create_sorceress_sprites()
    monsters = create_monster_sprites()
    boss = create_boss_sprites()
    fx_hud = create_fx_and_hud_tiles()

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("; ================================================================\n")
        f.write("; NIGHTMARE GENESIS — COMPILED TILE PATTERNS\n")
        f.write("; Generated by make_assets.py\n")
        f.write("; ================================================================\n\n")

        all_groups = [
            ("DungeonTiles", dungeon),
            ("FontTiles", list(font.values())),
            ("SorceressTiles", sorceress),
            ("MonsterTiles", monsters),
            ("BossTiles", boss),
            ("FxHudTiles", fx_hud),
        ]

        total_tiles = 0
        for group_name, tiles in all_groups:
            f.write(f"\n; --- Group: {group_name} ({len(tiles)} tiles) ---\n")
            f.write(f"{group_name}:\n")
            for t_idx, tile in enumerate(tiles):
                f.write(f"; Tile {total_tiles} ({group_name} #{t_idx})\n")
                # write 8 longwords (32 bytes)
                for i in range(0, 32, 4):
                    val = (tile[i] << 24) | (tile[i+1] << 16) | (tile[i+2] << 8) | tile[i+3]
                    f.write(f"    dc.l    ${val:08X}\n")
                total_tiles += 1
            f.write(f"{group_name}_End:\n")

        f.write(f"\nTotalTilesCount equ {total_tiles}\n")
    print(f"Exported {total_tiles} tiles to {output_path}")

if __name__ == '__main__':
    out_file = os.path.join(os.path.dirname(__file__), '..', 'assets', 'tiles.inc')
    export_tiles_to_asm(out_file)
