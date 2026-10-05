/**
 * PC Vivaz / Educational Computer 2000 - NES Core & Peripheral Emulator
 * Supports: NES 2.0 Mapper 329 / iNES Mapper 177 / UNIF UNL-EDU2000
 * Exact Hardware Peripheral Emulation:
 *   - PC Vivaz / Subor 3-Byte Serial Mouse Protocol ($4016 Strobe / $4017 Serial)
 *   - Subor 9-Row Keyboard Multiplexer Matrix ($4016 Strobe / $4017 D1..D4)
 *   - 1024KB PRG-ROM (32x 32KB banks), 32KB PRG-RAM (4x 8KB banks), 8KB CHR-RAM
 */

class PCVivazEmulator {
    constructor(canvas, options = {}) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.imageData = this.ctx.createImageData(256, 240);
        this.buf32 = new Uint32Array(this.imageData.data.buffer);

        // Options
        this.sampleRate = options.sampleRate || 44100;
        this.onFPS = options.onFPS || null;
        this.onSaveState = options.onSaveState || null;

        // Mouse sensitivity
        this.mouseSensitivity = 1.0;

        // Palette (standard NES RGB palette)
        this.initPalette();

        // 6502 CPU State
        this.cpu = {
            a: 0, x: 0, y: 0, sp: 0xFD, pc: 0,
            p: 0x24, // IRQ disabled, unused bit set
            cycles: 0
        };

        // PPU State
        this.ppu = {
            v: 0, t: 0, x: 0, w: 0,
            ctrl: 0, mask: 0, status: 0, oamAddr: 0,
            oam: new Uint8Array(256),
            vram: new Uint8Array(2048),
            palette: new Uint8Array(32),
            buffer: 0,
            scanline: 0, cycle: 0,
            frame: 0,
            nmiTriggered: false
        };

        // Memory & Mapper 329 State
        this.prgRom = null; // Uint8Array 1MB
        this.prgRam = new Uint8Array(32768); // 32KB battery backed SRAM
        this.chrRam = new Uint8Array(16384); // 16KB CHR RAM
        this.cpuRam = new Uint8Array(2048);  // 2KB NES internal RAM

        this.prgBank = 0;      // 32KB bank (0..31)
        this.prgRamBank = 0;   // 8KB bank (0..3)
        this.mirroring = 0;    // 0: Vertical, 1: Horizontal
        this.chrMode16k = 0;

        // Subor / PC Vivaz Mouse State
        this.mouse = {
            enabled: true,
            rawDeltaX: 0,
            rawDeltaY: 0,
            btnLeft: false,
            btnRight: false,
            // Packets (3 bytes)
            packets: [0, 0, 0],
            packetIdx: 0,
            bitIdx: 0,
            strobe: 0,
            latched: false,
            // Delay counter simulation
            lastStrobeCycle: 0
        };

        // Subor / PC Vivaz Keyboard Matrix State
        // 9 rows of 8 bits each (active low or active high depending on key state)
        // 0 = released, 1 = pressed
        this.keyboard = {
            matrix: new Uint8Array(9),
            row: 0,
            colMode: 0 // 0: low nibble ($04), 1: high nibble ($06)
        };

        // Standard Joypad fallback
        this.joypad = {
            buttons: [0, 0], // Player 1, Player 2
            shift: [0, 0],
            strobe: 0
        };

        // Audio APU
        this.initAPU();

        // Execution control
        this.running = false;
        this.animationFrameId = null;
        this.lastFrameTime = performance.now();
        this.fpsCount = 0;
        this.fpsTimer = performance.now();

        // Subor Keyboard Scancode / Key Map
        this.initKeyMap();
    }

    initPalette() {
        const nesColors = [
            0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00,
            0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000,
            0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
            0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000,
            0xFFFEFF, 0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22,
            0xBCBE00, 0x88D800, 0x5CE430, 0x45E082, 0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
            0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5, 0xF7D8A5,
            0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000
        ];
        this.paletteRGBA = new Uint32Array(64);
        for (let i = 0; i < 64; i++) {
            const rgb = nesColors[i];
            const r = (rgb >> 16) & 0xFF;
            const g = (rgb >> 8) & 0xFF;
            const b = rgb & 0xFF;
            this.paletteRGBA[i] = (0xFF << 24) | (b << 16) | (g << 8) | r;
        }
    }

    initAPU() {
        try {
            const AudioContext = window.AudioContext || window.webkitAudioContext;
            if (AudioContext) {
                this.audioCtx = new AudioContext({ sampleRate: this.sampleRate });
                this.audioBufferSize = 2048;
                this.audioBuffer = new Float32Array(this.audioBufferSize);
                this.audioBufferIdx = 0;

                // Simple APU Channel state
                this.apu = {
                    pulse1: { enabled: false, period: 0, timer: 0, duty: 0, vol: 0, step: 0 },
                    pulse2: { enabled: false, period: 0, timer: 0, duty: 0, vol: 0, step: 0 },
                    triangle: { enabled: false, period: 0, timer: 0, step: 0 },
                    noise: { enabled: false, period: 0, timer: 0, shift: 1, vol: 0 },
                    volume: 0.5
                };
            }
        } catch (e) {
            console.warn('Web Audio not available:', e);
        }
    }

    initKeyMap() {
        // Mapping of standard KeyboardEvent.code / key to Subor Matrix [row, bit]
        // Row 0: 4, G, F, C, F2, E, 5, V
        // Row 1: 2, D, S, END, F1, W, 3, X
        // Row 2: INSERT, BACKSPACE, NEXT(PageDown), RIGHT, F8, PAGEUP, DELETE, HOME
        // Row 3: 9, I, L, COMMA, F5, O, 0, PERIOD
        // Row 4: ], ENTER, UP, LEFT, F7, [, \, DOWN
        // Row 5: Q, CAPSLOCK, Z, TAB, ESC, A, 1, LCONTROL
        // Row 6: 7, Y, K, M, F4, U, 8, J
        // Row 7: -, SEMICOLON, APOS, SLASH, F6, P, =, RSHIFT/Shift
        // Row 8: R, T, H, N, F3, SPACE, 6, B

        this.keyToMatrix = {
            'Digit4': [0, 0], 'KeyG': [0, 1], 'KeyF': [0, 2], 'KeyC': [0, 3], 'F2': [0, 4], 'KeyE': [0, 5], 'Digit5': [0, 6], 'KeyV': [0, 7],
            'Digit2': [1, 0], 'KeyD': [1, 1], 'KeyS': [1, 2], 'End': [1, 3], 'F1': [1, 4], 'KeyW': [1, 5], 'Digit3': [1, 6], 'KeyX': [1, 7],
            'Insert': [2, 0], 'Backspace': [2, 1], 'PageDown': [2, 2], 'ArrowRight': [2, 3], 'F8': [2, 4], 'PageUp': [2, 5], 'Delete': [2, 6], 'Home': [2, 7],
            'Digit9': [3, 0], 'KeyI': [3, 1], 'KeyL': [3, 2], 'Comma': [3, 3], 'F5': [3, 4], 'KeyO': [3, 5], 'Digit0': [3, 6], 'Period': [3, 7],
            'BracketRight': [4, 0], 'Enter': [4, 1], 'NumpadEnter': [4, 1], 'ArrowUp': [4, 2], 'ArrowLeft': [4, 3], 'F7': [4, 4], 'BracketLeft': [4, 5], 'Backslash': [4, 6], 'ArrowDown': [4, 7],
            'KeyQ': [5, 0], 'CapsLock': [5, 1], 'KeyZ': [5, 2], 'Tab': [5, 3], 'Escape': [5, 4], 'KeyA': [5, 5], 'Digit1': [5, 6], 'ControlLeft': [5, 7], 'ControlRight': [5, 7],
            'Digit7': [6, 0], 'KeyY': [6, 1], 'KeyK': [6, 2], 'KeyM': [6, 3], 'F4': [6, 4], 'KeyU': [6, 5], 'Digit8': [6, 6], 'KeyJ': [6, 7],
            'Minus': [7, 0], 'Semicolon': [7, 1], 'Quote': [7, 2], 'Slash': [7, 3], 'F6': [7, 4], 'KeyP': [7, 5], 'Equal': [7, 6], 'ShiftLeft': [7, 7], 'ShiftRight': [7, 7],
            'KeyR': [8, 0], 'KeyT': [8, 1], 'KeyH': [8, 2], 'KeyN': [8, 3], 'F3': [8, 4], 'Space': [8, 5], 'Digit6': [8, 6], 'KeyB': [8, 7]
        };
    }

    handleKeyDown(code) {
        const mapping = this.keyToMatrix[code];
        if (mapping) {
            const [row, bit] = mapping;
            this.keyboard.matrix[row] |= (1 << bit);
        }

        // Standard Joypad mapping
        if (code === 'KeyZ' || code === 'KeyK') this.joypad.buttons[0] |= 0x01; // A
        if (code === 'KeyX' || code === 'KeyJ') this.joypad.buttons[0] |= 0x02; // B
        if (code === 'ShiftLeft' || code === 'KeyC') this.joypad.buttons[0] |= 0x04; // Select
        if (code === 'Enter' || code === 'KeyV') this.joypad.buttons[0] |= 0x08; // Start
        if (code === 'ArrowUp') this.joypad.buttons[0] |= 0x10;
        if (code === 'ArrowDown') this.joypad.buttons[0] |= 0x20;
        if (code === 'ArrowLeft') this.joypad.buttons[0] |= 0x40;
        if (code === 'ArrowRight') this.joypad.buttons[0] |= 0x80;
    }

    handleKeyUp(code) {
        const mapping = this.keyToMatrix[code];
        if (mapping) {
            const [row, bit] = mapping;
            this.keyboard.matrix[row] &= ~(1 << bit);
        }

        if (code === 'KeyZ' || code === 'KeyK') this.joypad.buttons[0] &= ~0x01;
        if (code === 'KeyX' || code === 'KeyJ') this.joypad.buttons[0] &= ~0x02;
        if (code === 'ShiftLeft' || code === 'KeyC') this.joypad.buttons[0] &= ~0x04;
        if (code === 'Enter' || code === 'KeyV') this.joypad.buttons[0] &= ~0x08;
        if (code === 'ArrowUp') this.joypad.buttons[0] &= ~0x10;
        if (code === 'ArrowDown') this.joypad.buttons[0] &= ~0x20;
        if (code === 'ArrowLeft') this.joypad.buttons[0] &= ~0x40;
        if (code === 'ArrowRight') this.joypad.buttons[0] &= ~0x80;
    }

    handleMouseMove(movementX, movementY) {
        // Accumulate movement with sensitivity scaling
        this.mouse.rawDeltaX += Math.round(movementX * this.mouseSensitivity);
        this.mouse.rawDeltaY += Math.round(movementY * this.mouseSensitivity);
    }

    handleMouseDown(button) {
        if (button === 0) this.mouse.btnLeft = true;
        if (button === 2) this.mouse.btnRight = true;
    }

    handleMouseUp(button) {
        if (button === 0) this.mouse.btnLeft = false;
        if (button === 2) this.mouse.btnRight = false;
    }

    loadRom(buffer) {
        const bytes = new Uint8Array(buffer);
        console.log(`Loading ROM, size: ${bytes.length} bytes`);

        // Check if UNIF format
        if (bytes[0] === 0x55 && bytes[1] === 0x4E && bytes[2] === 0x49 && bytes[3] === 0x46) {
            this.loadUNIF(bytes);
        } else if (bytes[0] === 0x4E && bytes[1] === 0x45 && bytes[2] === 0x53 && bytes[3] === 0x1A) {
            this.loadINES(bytes);
        } else if (bytes.length === 1048576) {
            // Raw 1MB PRG
            this.prgRom = bytes;
            this.mirroring = 0;
            console.log('Loaded Raw 1MB PRG-ROM');
        } else {
            throw new Error('Unsupported ROM format');
        }

        this.reset();
    }

    loadUNIF(bytes) {
        let offset = 32;
        let prgData = null;
        let mirror = 0;

        while (offset < bytes.length) {
            const tag = String.fromCharCode(bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3]);
            const size = bytes[offset + 4] | (bytes[offset + 5] << 8) | (bytes[offset + 6] << 16) | (bytes[offset + 7] << 24);
            offset += 8;

            if (tag === 'PRG0') {
                prgData = bytes.slice(offset, offset + size);
            } else if (tag === 'MIRR') {
                mirror = bytes[offset];
            } else if (tag === 'MAPR') {
                const mapr = String.fromCharCode(...bytes.slice(offset, offset + size)).replace(/\0/g, '');
                console.log(`UNIF MAPR: ${mapr}`);
            }
            offset += size;
        }

        if (!prgData) throw new Error('No PRG chunk found in UNIF');
        this.prgRom = prgData;
        this.mirroring = (mirror === 1) ? 1 : 0;
        console.log(`Loaded UNIF UNL-EDU2000 ROM: ${this.prgRom.length} bytes, Mirroring: ${this.mirroring}`);
    }

    loadINES(bytes) {
        const prgSize = bytes[4] * 16384;
        const mapper = (bytes[7] & 0xF0) | (bytes[6] >> 4) | ((bytes[8] & 0x0F) << 8);
        const hasBattery = (bytes[6] & 0x02) !== 0;
        this.mirroring = (bytes[6] & 0x01) ? 0 : 1; // Bit 0: 0=horiz, 1=vert

        const headerSize = (bytes[7] & 0x0C) === 0x08 ? 16 : 16;
        this.prgRom = bytes.slice(headerSize, headerSize + prgSize);
        console.log(`Loaded iNES/NES2.0 Mapper ${mapper}, PRG: ${this.prgRom.length} bytes, Battery: ${hasBattery}`);
    }

    loadSRAM(bytes) {
        if (bytes && bytes.length === 32768) {
            this.prgRam.set(bytes);
            console.log('Loaded 32KB SRAM (Battery RAM)');
        }
    }

    getSRAM() {
        return new Uint8Array(this.prgRam);
    }

    reset() {
        this.cpu.a = 0;
        this.cpu.x = 0;
        this.cpu.y = 0;
        this.cpu.sp = 0xFD;
        this.cpu.p = 0x24;
        this.cpu.cycles = 0;

        this.prgBank = 0;
        this.prgRamBank = 0;

        // Reset Vector in Bank 0 ($8000..$FFFF)
        this.cpu.pc = this.read16(0xFFFC);
        console.log(`CPU Reset Vector: 0x${this.cpu.pc.toString(16).toUpperCase()}`);

        this.ppu.v = 0;
        this.ppu.t = 0;
        this.ppu.x = 0;
        this.ppu.w = 0;
        this.ppu.ctrl = 0;
        this.ppu.mask = 0;
        this.ppu.status = 0;
        this.ppu.scanline = 0;
        this.ppu.cycle = 0;
        this.ppu.frame = 0;
    }

    // CPU Memory Map
    read8(addr) {
        addr &= 0xFFFF;
        if (addr < 0x2000) {
            // Internal 2KB RAM mirrored
            return this.cpuRam[addr & 0x07FF];
        } else if (addr < 0x4000) {
            // PPU Registers mirrored
            return this.readPPU(addr & 0x2007);
        } else if (addr === 0x4016) {
            // Joypad 1 standard read
            const val = (this.joypad.shift[0] & 0x80) ? 1 : 0;
            this.joypad.shift[0] = (this.joypad.shift[0] << 1) & 0xFF;
            return val | 0x40;
        } else if (addr === 0x4017) {
            // PC Vivaz / Subor Mouse & Keyboard Multi-read Port!
            return this.readPort4017();
        } else if (addr >= 0x6000 && addr < 0x8000) {
            // Switchable 8KB PRG-RAM (WRAM) bank (0..3)
            const ramOffset = (this.prgRamBank * 0x2000) + (addr - 0x6000);
            return this.prgRam[ramOffset & 0x7FFF];
        } else if (addr >= 0x8000) {
            // Switchable 32KB PRG-ROM bank (0..31)
            const prgOffset = (this.prgBank * 0x8000) + (addr - 0x8000);
            return this.prgRom[prgOffset % this.prgRom.length];
        }
        return 0;
    }

    write8(addr, val) {
        addr &= 0xFFFF;
        val &= 0xFF;
        if (addr < 0x2000) {
            this.cpuRam[addr & 0x07FF] = val;
        } else if (addr < 0x4000) {
            this.writePPU(addr & 0x2007, val);
        } else if (addr === 0x4014) {
            // OAM DMA
            const base = val << 8;
            for (let i = 0; i < 256; i++) {
                this.ppu.oam[this.ppu.oamAddr] = this.read8(base + i);
                this.ppu.oamAddr = (this.ppu.oamAddr + 1) & 0xFF;
            }
            this.cpu.cycles += 513;
        } else if (addr === 0x4016) {
            // PC Vivaz / Subor Strobe & Peripheral Control
            this.writePort4016(val);
        } else if (addr >= 0x4000 && addr <= 0x4017) {
            this.writeAPU(addr, val);
        } else if (addr >= 0x6000 && addr < 0x8000) {
            // 8KB PRG-RAM (WRAM) bank write
            const ramOffset = (this.prgRamBank * 0x2000) + (addr - 0x6000);
            this.prgRam[ramOffset & 0x7FFF] = val;
        } else if (addr >= 0x8000) {
            // Mapper 329 / 177 Register Write: WWMP PPPP
            // PPPP (bits 0..4): 32KB PRG-ROM Bank
            // M (bit 5): Mirroring (0: Vertical, 1: Horizontal)
            // WW (bits 6..7): 8KB PRG-RAM Bank
            this.prgBank = val & 0x1F;
            this.mirroring = (val & 0x20) ? 1 : 0;
            this.prgRamBank = (val >> 6) & 0x03;
        }
    }

    read16(addr) {
        return this.read8(addr) | (this.read8(addr + 1) << 8);
    }

    read16Bug(addr) {
        // 6502 indirect JMP page wrap bug
        const lo = this.read8(addr);
        const hi = this.read8((addr & 0xFF00) | ((addr + 1) & 0x00FF));
        return lo | (hi << 8);
    }

    // Peripheral E/S Handlers ($4016 / $4017)
    writePort4016(val) {
        // Standard Joypad strobe
        if ((this.joypad.strobe & 1) && !(val & 1)) {
            this.joypad.shift[0] = this.joypad.buttons[0];
            this.joypad.shift[1] = this.joypad.buttons[1];
        }
        this.joypad.strobe = val;

        // PC Vivaz / Subor Keyboard Matrix Control:
        // val == 0x05: Reset row counter to 0
        // val == 0x04: Select low nibble of current row (columns 0..3)
        // val == 0x06: Select high nibble of current row (columns 4..7) and increment row
        if (val === 0x05) {
            this.keyboard.row = 0;
        } else if (val === 0x04) {
            this.keyboard.colMode = 0; // Low nibble
        } else if (val === 0x06) {
            this.keyboard.colMode = 1; // High nibble
        }

        // PC Vivaz / Educational Computer 2000 Mouse Latch:
        // When val == 0x01 (Strobe high), latch mouse motion into 3-byte packets
        if (val & 0x01) {
            this.mouse.strobe = 1;
            this.mouse.lastStrobeCycle = this.cpu.cycles;
            this.latchMousePackets();
        } else {
            // Strobe low (val == 0x06 or 0x00): Ready to clock serial bits
            if (this.mouse.strobe === 1) {
                this.mouse.strobe = 0;
                this.mouse.bitIdx = 0;
            }
        }
    }

    latchMousePackets() {
        // Clamp deltas to 5-bit magnitude (0..31)
        let dx = Math.max(-31, Math.min(31, this.mouse.rawDeltaX));
        let dy = Math.max(-31, Math.min(31, this.mouse.rawDeltaY));

        // Consume raw deltas
        this.mouse.rawDeltaX = 0;
        this.mouse.rawDeltaY = 0;

        const signX = (dx < 0);
        const signY = (dy < 0);
        const magX = Math.abs(dx);
        const magY = Math.abs(dy);

        // Packet 1 ($B6 in ROM):
        // Bit 0..1: 0x01 (Packet 1 ID)
        // Bit 7: Left Click
        // Bit 6: Right Click
        // Bit 5: Sign X (1 = negative)
        // Bit 4: High bit of magnitude X ($10)
        // Bit 3: Sign Y (1 = negative)
        // Bit 2: High bit of magnitude Y ($10)
        let p1 = 0x01;
        if (this.mouse.btnLeft) p1 |= 0x80;
        if (this.mouse.btnRight) p1 |= 0x40;
        if (signX) p1 |= 0x20;
        if (magX >= 16) p1 |= 0x10;
        if (signY) p1 |= 0x08;
        if (magY >= 16) p1 |= 0x04;

        // Packet 2 ($B7 in ROM):
        // Bit 0..1: 0x02 (Packet 2 ID)
        // Bit 2..5: Lower 4 bits of Delta X: (magX & 0x0F) << 2
        let p2 = 0x02 | ((magX & 0x0F) << 2);

        // Packet 3 ($B8 in ROM):
        // Bit 0..1: 0x03 (Packet 3 ID)
        // Bit 2..5: Lower 4 bits of Delta Y: (magY & 0x0F) << 2
        let p3 = 0x03 | ((magY & 0x0F) << 2);

        this.mouse.packets = [p1, p2, p3];
        this.mouse.bitIdx = 0;
        this.mouse.latched = true;
    }

    readPort4017() {
        let result = 0x00;

        // 1. Mouse Serial Stream on Bit 0:
        // The 6502 reads 8 bits per byte using ROR A -> ROL $B1 (MSB first!)
        if (this.mouse.enabled && this.mouse.latched) {
            const currentByte = this.mouse.packets[this.mouse.packetIdx];
            const bit = (currentByte >> (7 - (this.mouse.bitIdx & 7))) & 1;
            result |= bit; // Place on Bit 0

            this.mouse.bitIdx++;
            if (this.mouse.bitIdx >= 8) {
                this.mouse.bitIdx = 0;
                this.mouse.packetIdx = (this.mouse.packetIdx + 1) % 3;
            }
        }

        // 2. Subor Keyboard Matrix on Bits 1..4 (D1..D4):
        const row = Math.min(8, this.keyboard.row);
        const rowByte = this.keyboard.matrix[row];

        if (this.keyboard.colMode === 0) {
            // Low nibble (bits 0..3) placed on D1..D4
            result |= (rowByte & 0x0F) << 1;
        } else {
            // High nibble (bits 4..7) placed on D1..D4
            result |= ((rowByte >> 4) & 0x0F) << 1;
            // Advance row counter
            this.keyboard.row = (this.keyboard.row + 1) % 9;
        }

        return result;
    }

    // PPU Emulation
    readPPU(addr) {
        if (addr === 0x2002) {
            // PPUSTATUS
            const res = this.ppu.status;
            this.ppu.status &= ~0x80; // Clear VBlank flag on read
            this.ppu.w = 0;           // Reset write toggle
            return res;
        } else if (addr === 0x2004) {
            return this.ppu.oam[this.ppu.oamAddr];
        } else if (addr === 0x2007) {
            // PPUDATA
            let res = this.ppu.buffer;
            this.ppu.buffer = this.readVRAM(this.ppu.v);
            if (this.ppu.v >= 0x3F00) {
                res = this.ppu.buffer; // Palette RAM reads immediately
            }
            this.ppu.v += (this.ppu.ctrl & 0x04) ? 32 : 1;
            this.ppu.v &= 0x7FFF;
            return res;
        }
        return 0;
    }

    writePPU(addr, val) {
        if (addr === 0x2000) {
            // PPUCTRL
            this.ppu.ctrl = val;
            this.ppu.t = (this.ppu.t & 0xF3FF) | ((val & 0x03) << 10);
        } else if (addr === 0x2001) {
            // PPUMASK
            this.ppu.mask = val;
        } else if (addr === 0x2003) {
            this.ppu.oamAddr = val;
        } else if (addr === 0x2004) {
            this.ppu.oam[this.ppu.oamAddr] = val;
            this.ppu.oamAddr = (this.ppu.oamAddr + 1) & 0xFF;
        } else if (addr === 0x2005) {
            // PPUSCROLL
            if (this.ppu.w === 0) {
                this.ppu.t = (this.ppu.t & 0xFFE0) | (val >> 3);
                this.ppu.x = val & 0x07;
                this.ppu.w = 1;
            } else {
                this.ppu.t = (this.ppu.t & 0x8C1F) | ((val & 0xF8) << 2) | ((val & 0x07) << 12);
                this.ppu.w = 0;
            }
        } else if (addr === 0x2006) {
            // PPUADDR
            if (this.ppu.w === 0) {
                this.ppu.t = (this.ppu.t & 0x00FF) | ((val & 0x3F) << 8);
                this.ppu.w = 1;
            } else {
                this.ppu.t = (this.ppu.t & 0xFF00) | val;
                this.ppu.v = this.ppu.t;
                this.ppu.w = 0;
            }
        } else if (addr === 0x2007) {
            // PPUDATA
            this.writeVRAM(this.ppu.v, val);
            this.ppu.v += (this.ppu.ctrl & 0x04) ? 32 : 1;
            this.ppu.v &= 0x7FFF;
        }
    }

    readVRAM(addr) {
        addr &= 0x3FFF;
        if (addr < 0x2000) {
            return this.chrRam[addr & 0x1FFF];
        } else if (addr < 0x3F00) {
            return this.ppu.vram[this.mirrorVRAM(addr)];
        } else {
            let palAddr = addr & 0x1F;
            if (palAddr === 0x10 || palAddr === 0x14 || palAddr === 0x18 || palAddr === 0x1C) {
                palAddr -= 0x10;
            }
            return this.ppu.palette[palAddr];
        }
    }

    writeVRAM(addr, val) {
        addr &= 0x3FFF;
        if (addr < 0x2000) {
            this.chrRam[addr & 0x1FFF] = val;
        } else if (addr < 0x3F00) {
            this.ppu.vram[this.mirrorVRAM(addr)] = val;
        } else {
            let palAddr = addr & 0x1F;
            if (palAddr === 0x10 || palAddr === 0x14 || palAddr === 0x18 || palAddr === 0x1C) {
                palAddr -= 0x10;
            }
            this.ppu.palette[palAddr] = val;
        }
    }

    mirrorVRAM(addr) {
        addr = (addr - 0x2000) & 0x0FFF;
        const table = addr >> 10;
        const offset = addr & 0x03FF;
        if (this.mirroring === 0) {
            // Vertical: Table 0 & 2 mapped to 0, Table 1 & 3 mapped to 1
            return ((table & 1) ? 1024 : 0) + offset;
        } else {
            // Horizontal: Table 0 & 1 mapped to 0, Table 2 & 3 mapped to 1
            return ((table & 2) ? 1024 : 0) + offset;
        }
    }

    renderScanline(scanline) {
        if (scanline >= 240) return;

        const showBg = (this.ppu.mask & 0x08) !== 0;
        const showSpr = (this.ppu.mask & 0x10) !== 0;
        const bgPatternBase = (this.ppu.ctrl & 0x10) ? 0x1000 : 0x0000;
        const sprPatternBase = (this.ppu.ctrl & 0x08) ? 0x1000 : 0x0000;
        const sprHeight = (this.ppu.ctrl & 0x20) ? 16 : 8;

        const scanlineOffset = scanline * 256;
        const universalBgColor = this.paletteRGBA[this.ppu.palette[0] & 0x3F];

        // Background Line Rendering
        const bgPriority = new Uint8Array(256);

        if (showBg) {
            const fineY = (this.ppu.v >> 12) & 0x07;
            const coarseY = (this.ppu.v >> 5) & 0x1F;
            const nametableY = (this.ppu.v >> 11) & 0x01;

            for (let tileX = 0; tileX < 33; tileX++) {
                const coarseX = ((this.ppu.v & 0x1F) + tileX) & 0x1F;
                const nametableX = (((this.ppu.v & 0x1F) + tileX) >= 32) ? ((this.ppu.v >> 10) & 0x01) ^ 1 : ((this.ppu.v >> 10) & 0x01);

                const ntAddr = 0x2000 | (nametableY << 11) | (nametableX << 10) | (coarseY << 5) | coarseX;
                const tileId = this.readVRAM(ntAddr);

                const attrAddr = 0x23C0 | (nametableY << 11) | (nametableX << 10) | ((coarseY >> 2) << 3) | (coarseX >> 2);
                const attrByte = this.readVRAM(attrAddr);
                const attrShift = ((coarseY & 2) ? 4 : 0) | ((coarseX & 2) ? 2 : 0);
                const paletteIdx = (attrByte >> attrShift) & 0x03;

                const tileAddr = bgPatternBase + (tileId << 4) + fineY;
                const plane0 = this.readVRAM(tileAddr);
                const plane1 = this.readVRAM(tileAddr + 8);

                for (let px = 0; px < 8; px++) {
                    const screenX = tileX * 8 + px - this.ppu.x;
                    if (screenX >= 0 && screenX < 256) {
                        const bit = 7 - px;
                        const colorVal = (((plane0 >> bit) & 1) | (((plane1 >> bit) & 1) << 1));
                        if (colorVal !== 0) {
                            const palColor = this.readVRAM(0x3F00 + (paletteIdx << 2) + colorVal);
                            this.buf32[scanlineOffset + screenX] = this.paletteRGBA[palColor & 0x3F];
                            bgPriority[screenX] = colorVal;
                        } else {
                            this.buf32[scanlineOffset + screenX] = universalBgColor;
                        }
                    }
                }
            }
        } else {
            for (let x = 0; x < 256; x++) {
                this.buf32[scanlineOffset + x] = universalBgColor;
            }
        }

        // Sprite Rendering ($0200-$02FF OAM)
        if (showSpr) {
            let spritesOnLine = 0;
            for (let i = 0; i < 64; i++) {
                const oamIdx = i * 4;
                const sprY = this.ppu.oam[oamIdx] + 1;
                const tileIndex = this.ppu.oam[oamIdx + 1];
                const attr = this.ppu.oam[oamIdx + 2];
                const sprX = this.ppu.oam[oamIdx + 3];

                if (scanline >= sprY && scanline < sprY + sprHeight) {
                    spritesOnLine++;
                    if (spritesOnLine > 8) {
                        this.ppu.status |= 0x20; // Sprite overflow flag
                        break;
                    }

                    let rowInTile = scanline - sprY;
                    if (attr & 0x80) rowInTile = (sprHeight - 1) - rowInTile; // Vertical Flip

                    let tileAddr = 0;
                    if (sprHeight === 8) {
                        tileAddr = sprPatternBase + (tileIndex << 4) + rowInTile;
                    } else {
                        const table = (tileIndex & 1) ? 0x1000 : 0x0000;
                        const realTile = (tileIndex & 0xFE) + ((rowInTile >= 8) ? 1 : 0);
                        tileAddr = table + (realTile << 4) + (rowInTile & 7);
                    }

                    const plane0 = this.readVRAM(tileAddr);
                    const plane1 = this.readVRAM(tileAddr + 8);
                    const palBase = 0x3F10 + ((attr & 0x03) << 2);
                    const priorityBehindBg = (attr & 0x20) !== 0;

                    for (let px = 0; px < 8; px++) {
                        const screenX = sprX + px;
                        if (screenX >= 256) continue;

                        const bit = (attr & 0x40) ? px : (7 - px); // Horizontal Flip
                        const colorVal = (((plane0 >> bit) & 1) | (((plane1 >> bit) & 1) << 1));

                        if (colorVal !== 0) {
                            // Sprite 0 hit check
                            if (i === 0 && showBg && bgPriority[screenX] !== 0 && screenX < 255) {
                                this.ppu.status |= 0x40; // Sprite 0 hit
                            }

                            if (!priorityBehindBg || bgPriority[screenX] === 0) {
                                const palColor = this.readVRAM(palBase + colorVal);
                                this.buf32[scanlineOffset + screenX] = this.paletteRGBA[palColor & 0x3F];
                            }
                        }
                    }
                }
            }
        }
    }

    writeAPU(addr, val) {
        if (!this.apu) return;
        if (addr === 0x4000) {
            this.apu.pulse1.duty = (val >> 6) & 3;
            this.apu.pulse1.vol = (val & 0x0F) / 15.0;
        } else if (addr === 0x4002) {
            this.apu.pulse1.period = (this.apu.pulse1.period & 0x700) | val;
        } else if (addr === 0x4003) {
            this.apu.pulse1.period = (this.apu.pulse1.period & 0xFF) | ((val & 7) << 8);
        } else if (addr === 0x4015) {
            this.apu.pulse1.enabled = (val & 1) !== 0;
            this.apu.pulse2.enabled = (val & 2) !== 0;
            this.apu.triangle.enabled = (val & 4) !== 0;
            this.apu.noise.enabled = (val & 8) !== 0;
        }
    }

    // 6502 CPU Instruction Step
    stepCPU() {
        const opcode = this.read8(this.cpu.pc++);
        const cyclesBefore = this.cpu.cycles;

        switch (opcode) {
            // NOP
            case 0xEA: this.cpu.cycles += 2; break;

            // LDA
            case 0xA9: this.cpu.a = this.read8(this.cpu.pc++); this.setZN(this.cpu.a); this.cpu.cycles += 2; break;
            case 0xA5: this.cpu.a = this.read8(this.read8(this.cpu.pc++)); this.setZN(this.cpu.a); this.cpu.cycles += 3; break;
            case 0xB5: this.cpu.a = this.read8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF); this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0xAD: this.cpu.a = this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0xBD: this.cpu.a = this.read8((this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF); this.cpu.pc += 2; this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0xB9: this.cpu.a = this.read8((this.read16(this.cpu.pc) + this.cpu.y) & 0xFFFF); this.cpu.pc += 2; this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0xA1: { const zp = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; const addr = this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8); this.cpu.a = this.read8(addr); this.setZN(this.cpu.a); this.cpu.cycles += 6; break; }
            case 0xB1: { const zp = this.read8(this.cpu.pc++); const addr = (this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8)) + this.cpu.y; this.cpu.a = this.read8(addr & 0xFFFF); this.setZN(this.cpu.a); this.cpu.cycles += 5; break; }

            // LDX
            case 0xA2: this.cpu.x = this.read8(this.cpu.pc++); this.setZN(this.cpu.x); this.cpu.cycles += 2; break;
            case 0xA6: this.cpu.x = this.read8(this.read8(this.cpu.pc++)); this.setZN(this.cpu.x); this.cpu.cycles += 3; break;
            case 0xB6: this.cpu.x = this.read8((this.read8(this.cpu.pc++) + this.cpu.y) & 0xFF); this.setZN(this.cpu.x); this.cpu.cycles += 4; break;
            case 0xAE: this.cpu.x = this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.setZN(this.cpu.x); this.cpu.cycles += 4; break;
            case 0xBE: this.cpu.x = this.read8((this.read16(this.cpu.pc) + this.cpu.y) & 0xFFFF); this.cpu.pc += 2; this.setZN(this.cpu.x); this.cpu.cycles += 4; break;

            // LDY
            case 0xA0: this.cpu.y = this.read8(this.cpu.pc++); this.setZN(this.cpu.y); this.cpu.cycles += 2; break;
            case 0xA4: this.cpu.y = this.read8(this.read8(this.cpu.pc++)); this.setZN(this.cpu.y); this.cpu.cycles += 3; break;
            case 0xB4: this.cpu.y = this.read8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF); this.setZN(this.cpu.y); this.cpu.cycles += 4; break;
            case 0xAC: this.cpu.y = this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.setZN(this.cpu.y); this.cpu.cycles += 4; break;
            case 0xBC: this.cpu.y = this.read8((this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF); this.cpu.pc += 2; this.setZN(this.cpu.y); this.cpu.cycles += 4; break;

            // STA
            case 0x85: this.write8(this.read8(this.cpu.pc++), this.cpu.a); this.cpu.cycles += 3; break;
            case 0x95: this.write8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF, this.cpu.a); this.cpu.cycles += 4; break;
            case 0x8D: this.write8(this.read16(this.cpu.pc), this.cpu.a); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0x9D: this.write8((this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF, this.cpu.a); this.cpu.pc += 2; this.cpu.cycles += 5; break;
            case 0x99: this.write8((this.read16(this.cpu.pc) + this.cpu.y) & 0xFFFF, this.cpu.a); this.cpu.pc += 2; this.cpu.cycles += 5; break;
            case 0x81: { const zp = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; const addr = this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8); this.write8(addr, this.cpu.a); this.cpu.cycles += 6; break; }
            case 0x91: { const zp = this.read8(this.cpu.pc++); const addr = (this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8)) + this.cpu.y; this.write8(addr & 0xFFFF, this.cpu.a); this.cpu.cycles += 6; break; }

            // STX
            case 0x86: this.write8(this.read8(this.cpu.pc++), this.cpu.x); this.cpu.cycles += 3; break;
            case 0x96: this.write8((this.read8(this.cpu.pc++) + this.cpu.y) & 0xFF, this.cpu.x); this.cpu.cycles += 4; break;
            case 0x8E: this.write8(this.read16(this.cpu.pc), this.cpu.x); this.cpu.pc += 2; this.cpu.cycles += 4; break;

            // STY
            case 0x84: this.write8(this.read8(this.cpu.pc++), this.cpu.y); this.cpu.cycles += 3; break;
            case 0x94: this.write8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF, this.cpu.y); this.cpu.cycles += 4; break;
            case 0x8C: this.write8(this.read16(this.cpu.pc), this.cpu.y); this.cpu.pc += 2; this.cpu.cycles += 4; break;

            // Transfers
            case 0xAA: this.cpu.x = this.cpu.a; this.setZN(this.cpu.x); this.cpu.cycles += 2; break; // TAX
            case 0x8A: this.cpu.a = this.cpu.x; this.setZN(this.cpu.a); this.cpu.cycles += 2; break; // TXA
            case 0xA8: this.cpu.y = this.cpu.a; this.setZN(this.cpu.y); this.cpu.cycles += 2; break; // TAY
            case 0x98: this.cpu.a = this.cpu.y; this.setZN(this.cpu.a); this.cpu.cycles += 2; break; // TYA
            case 0xBA: this.cpu.x = this.cpu.sp; this.setZN(this.cpu.x); this.cpu.cycles += 2; break; // TSX
            case 0x9A: this.cpu.sp = this.cpu.x; this.cpu.cycles += 2; break; // TXS

            // Stack
            case 0x48: this.push(this.cpu.a); this.cpu.cycles += 3; break; // PHA
            case 0x68: this.cpu.a = this.pop(); this.setZN(this.cpu.a); this.cpu.cycles += 4; break; // PLA
            case 0x08: this.push(this.cpu.p | 0x30); this.cpu.cycles += 3; break; // PHP
            case 0x28: this.cpu.p = (this.pop() & 0xEF) | 0x20; this.cpu.cycles += 4; break; // PLP

            // JMP / JSR / RTS / RTI
            case 0x4C: this.cpu.pc = this.read16(this.cpu.pc); this.cpu.cycles += 3; break; // JMP abs
            case 0x6C: this.cpu.pc = this.read16Bug(this.read16(this.cpu.pc)); this.cpu.cycles += 5; break; // JMP (ind)
            case 0x20: { const target = this.read16(this.cpu.pc); this.push16(this.cpu.pc + 1); this.cpu.pc = target; this.cpu.cycles += 6; break; } // JSR
            case 0x60: this.cpu.pc = this.pop16() + 1; this.cpu.cycles += 6; break; // RTS
            case 0x40: this.cpu.p = (this.pop() & 0xEF) | 0x20; this.cpu.pc = this.pop16(); this.cpu.cycles += 6; break; // RTI

            // Branches
            case 0x10: this.branch(!(this.cpu.p & 0x80)); break; // BPL
            case 0x30: this.branch(!!(this.cpu.p & 0x80)); break; // BMI
            case 0x50: this.branch(!(this.cpu.p & 0x40)); break; // BVC
            case 0x70: this.branch(!!(this.cpu.p & 0x40)); break; // BVS
            case 0x90: this.branch(!(this.cpu.p & 0x01)); break; // BCC
            case 0xB0: this.branch(!!(this.cpu.p & 0x01)); break; // BCS
            case 0xD0: this.branch(!(this.cpu.p & 0x02)); break; // BNE
            case 0xF0: this.branch(!!(this.cpu.p & 0x02)); break; // BEQ

            // Status flags
            case 0x18: this.cpu.p &= ~0x01; this.cpu.cycles += 2; break; // CLC
            case 0x38: this.cpu.p |= 0x01; this.cpu.cycles += 2; break; // SEC
            case 0x58: this.cpu.p &= ~0x04; this.cpu.cycles += 2; break; // CLI
            case 0x78: this.cpu.p |= 0x04; this.cpu.cycles += 2; break; // SEI
            case 0xB8: this.cpu.p &= ~0x40; this.cpu.cycles += 2; break; // CLV
            case 0xD8: this.cpu.p &= ~0x08; this.cpu.cycles += 2; break; // CLD
            case 0xF8: this.cpu.p |= 0x08; this.cpu.cycles += 2; break; // SED

            // ADC
            case 0x69: this.adc(this.read8(this.cpu.pc++)); this.cpu.cycles += 2; break;
            case 0x65: this.adc(this.read8(this.read8(this.cpu.pc++))); this.cpu.cycles += 3; break;
            case 0x75: this.adc(this.read8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF)); this.cpu.cycles += 4; break;
            case 0x6D: this.adc(this.read8(this.read16(this.cpu.pc))); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0x7D: this.adc(this.read8((this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF)); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0x79: this.adc(this.read8((this.read16(this.cpu.pc) + this.cpu.y) & 0xFFFF)); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0x61: { const zp = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; this.adc(this.read8(this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8))); this.cpu.cycles += 6; break; }
            case 0x71: { const zp = this.read8(this.cpu.pc++); this.adc(this.read8(((this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8)) + this.cpu.y) & 0xFFFF)); this.cpu.cycles += 5; break; }

            // SBC
            case 0xE9: this.sbc(this.read8(this.cpu.pc++)); this.cpu.cycles += 2; break;
            case 0xE5: this.sbc(this.read8(this.read8(this.cpu.pc++))); this.cpu.cycles += 3; break;
            case 0xF5: this.sbc(this.read8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF)); this.cpu.cycles += 4; break;
            case 0xED: this.sbc(this.read8(this.read16(this.cpu.pc))); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xFD: this.sbc(this.read8((this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF)); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xF9: this.sbc(this.read8((this.read16(this.cpu.pc) + this.cpu.y) & 0xFFFF)); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xE1: { const zp = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; this.sbc(this.read8(this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8))); this.cpu.cycles += 6; break; }
            case 0xF1: { const zp = this.read8(this.cpu.pc++); this.sbc(this.read8(((this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8)) + this.cpu.y) & 0xFFFF)); this.cpu.cycles += 5; break; }

            // CMP
            case 0xC9: this.cmp(this.cpu.a, this.read8(this.cpu.pc++)); this.cpu.cycles += 2; break;
            case 0xC5: this.cmp(this.cpu.a, this.read8(this.read8(this.cpu.pc++))); this.cpu.cycles += 3; break;
            case 0xD5: this.cmp(this.cpu.a, this.read8((this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF)); this.cpu.cycles += 4; break;
            case 0xCD: this.cmp(this.cpu.a, this.read8(this.read16(this.cpu.pc))); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xDD: this.cmp(this.cpu.a, this.read8((this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF)); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xD9: this.cmp(this.cpu.a, this.read8((this.read16(this.cpu.pc) + this.cpu.y) & 0xFFFF)); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xC1: { const zp = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; this.cmp(this.cpu.a, this.read8(this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8))); this.cpu.cycles += 6; break; }
            case 0xD1: { const zp = this.read8(this.cpu.pc++); this.cmp(this.cpu.a, this.read8(((this.read8(zp) | (this.read8((zp + 1) & 0xFF) << 8)) + this.cpu.y) & 0xFFFF)); this.cpu.cycles += 5; break; }

            // CPX / CPY
            case 0xE0: this.cmp(this.cpu.x, this.read8(this.cpu.pc++)); this.cpu.cycles += 2; break;
            case 0xE4: this.cmp(this.cpu.x, this.read8(this.read8(this.cpu.pc++))); this.cpu.cycles += 3; break;
            case 0xEC: this.cmp(this.cpu.x, this.read8(this.read16(this.cpu.pc))); this.cpu.pc += 2; this.cpu.cycles += 4; break;
            case 0xC0: this.cmp(this.cpu.y, this.read8(this.cpu.pc++)); this.cpu.cycles += 2; break;
            case 0xC4: this.cmp(this.cpu.y, this.read8(this.read8(this.cpu.pc++))); this.cpu.cycles += 3; break;
            case 0xCC: this.cmp(this.cpu.y, this.read8(this.read16(this.cpu.pc))); this.cpu.pc += 2; this.cpu.cycles += 4; break;

            // AND / ORA / EOR / BIT
            case 0x29: this.cpu.a &= this.read8(this.cpu.pc++); this.setZN(this.cpu.a); this.cpu.cycles += 2; break;
            case 0x25: this.cpu.a &= this.read8(this.read8(this.cpu.pc++)); this.setZN(this.cpu.a); this.cpu.cycles += 3; break;
            case 0x2D: this.cpu.a &= this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0x09: this.cpu.a |= this.read8(this.cpu.pc++); this.setZN(this.cpu.a); this.cpu.cycles += 2; break;
            case 0x05: this.cpu.a |= this.read8(this.read8(this.cpu.pc++)); this.setZN(this.cpu.a); this.cpu.cycles += 3; break;
            case 0x0D: this.cpu.a |= this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0x49: this.cpu.a ^= this.read8(this.cpu.pc++); this.setZN(this.cpu.a); this.cpu.cycles += 2; break;
            case 0x45: this.cpu.a ^= this.read8(this.read8(this.cpu.pc++)); this.setZN(this.cpu.a); this.cpu.cycles += 3; break;
            case 0x4D: this.cpu.a ^= this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.setZN(this.cpu.a); this.cpu.cycles += 4; break;
            case 0x24: { const val = this.read8(this.read8(this.cpu.pc++)); this.cpu.p = (this.cpu.p & 0x3D) | (val & 0xC0) | ((this.cpu.a & val) ? 0 : 0x02); this.cpu.cycles += 3; break; }
            case 0x2C: { const val = this.read8(this.read16(this.cpu.pc)); this.cpu.pc += 2; this.cpu.p = (this.cpu.p & 0x3D) | (val & 0xC0) | ((this.cpu.a & val) ? 0 : 0x02); this.cpu.cycles += 4; break; }

            // INX / DEX / INY / DEY / INC / DEC
            case 0xE8: this.cpu.x = (this.cpu.x + 1) & 0xFF; this.setZN(this.cpu.x); this.cpu.cycles += 2; break;
            case 0xCA: this.cpu.x = (this.cpu.x - 1) & 0xFF; this.setZN(this.cpu.x); this.cpu.cycles += 2; break;
            case 0xC8: this.cpu.y = (this.cpu.y + 1) & 0xFF; this.setZN(this.cpu.y); this.cpu.cycles += 2; break;
            case 0x88: this.cpu.y = (this.cpu.y - 1) & 0xFF; this.setZN(this.cpu.y); this.cpu.cycles += 2; break;
            case 0xE6: { const addr = this.read8(this.cpu.pc++); const res = (this.read8(addr) + 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 5; break; }
            case 0xF6: { const addr = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; const res = (this.read8(addr) + 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 6; break; }
            case 0xEE: { const addr = this.read16(this.cpu.pc); this.cpu.pc += 2; const res = (this.read8(addr) + 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 6; break; }
            case 0xFE: { const addr = (this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF; this.cpu.pc += 2; const res = (this.read8(addr) + 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 7; break; }
            case 0xC6: { const addr = this.read8(this.cpu.pc++); const res = (this.read8(addr) - 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 5; break; }
            case 0xD6: { const addr = (this.read8(this.cpu.pc++) + this.cpu.x) & 0xFF; const res = (this.read8(addr) - 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 6; break; }
            case 0xCE: { const addr = this.read16(this.cpu.pc); this.cpu.pc += 2; const res = (this.read8(addr) - 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 6; break; }
            case 0xDE: { const addr = (this.read16(this.cpu.pc) + this.cpu.x) & 0xFFFF; this.cpu.pc += 2; const res = (this.read8(addr) - 1) & 0xFF; this.write8(addr, res); this.setZN(res); this.cpu.cycles += 7; break; }

            // Shifts: ASL, LSR, ROL, ROR
            case 0x0A: { const c = (this.cpu.a >> 7) & 1; this.cpu.a = (this.cpu.a << 1) & 0xFF; this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(this.cpu.a); this.cpu.cycles += 2; break; }
            case 0x06: { const addr = this.read8(this.cpu.pc++); const val = this.read8(addr); const c = (val >> 7) & 1; const res = (val << 1) & 0xFF; this.write8(addr, res); this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(res); this.cpu.cycles += 5; break; }
            case 0x4A: { const c = this.cpu.a & 1; this.cpu.a = (this.cpu.a >> 1) & 0xFF; this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(this.cpu.a); this.cpu.cycles += 2; break; }
            case 0x46: { const addr = this.read8(this.cpu.pc++); const val = this.read8(addr); const c = val & 1; const res = (val >> 1) & 0xFF; this.write8(addr, res); this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(res); this.cpu.cycles += 5; break; }
            case 0x2A: { const c = (this.cpu.a >> 7) & 1; this.cpu.a = ((this.cpu.a << 1) | (this.cpu.p & 1)) & 0xFF; this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(this.cpu.a); this.cpu.cycles += 2; break; }
            case 0x26: { const addr = this.read8(this.cpu.pc++); const val = this.read8(addr); const c = (val >> 7) & 1; const res = ((val << 1) | (this.cpu.p & 1)) & 0xFF; this.write8(addr, res); this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(res); this.cpu.cycles += 5; break; }
            case 0x6A: { const c = this.cpu.a & 1; this.cpu.a = ((this.cpu.a >> 1) | ((this.cpu.p & 1) << 7)) & 0xFF; this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(this.cpu.a); this.cpu.cycles += 2; break; }
            case 0x66: { const addr = this.read8(this.cpu.pc++); const val = this.read8(addr); const c = val & 1; const res = ((val >> 1) | ((this.cpu.p & 1) << 7)) & 0xFF; this.write8(addr, res); this.cpu.p = (this.cpu.p & ~0x01) | c; this.setZN(res); this.cpu.cycles += 5; break; }

            // BRK
            case 0x00: this.push16(this.cpu.pc + 1); this.push(this.cpu.p | 0x30); this.cpu.p |= 0x04; this.cpu.pc = this.read16(0xFFFE); this.cpu.cycles += 7; break;

            default:
                // Unofficial or unhandled opcode
                this.cpu.cycles += 2;
                break;
        }

        return this.cpu.cycles - cyclesBefore;
    }

    setZN(val) {
        this.cpu.p = (this.cpu.p & ~0x82) | (val === 0 ? 0x02 : (val & 0x80));
    }

    branch(cond) {
        const offset = this.read8(this.cpu.pc++);
        const signedOffset = (offset < 0x80) ? offset : (offset - 256);
        if (cond) {
            this.cpu.pc = (this.cpu.pc + signedOffset) & 0xFFFF;
            this.cpu.cycles += 3;
        } else {
            this.cpu.cycles += 2;
        }
    }

    adc(val) {
        const c = this.cpu.p & 1;
        const sum = this.cpu.a + val + c;
        const overflow = (~(this.cpu.a ^ val) & (this.cpu.a ^ sum) & 0x80) !== 0;
        this.cpu.a = sum & 0xFF;
        this.cpu.p = (this.cpu.p & ~0xC1) | (sum > 0xFF ? 1 : 0) | (overflow ? 0x40 : 0) | (this.cpu.a === 0 ? 2 : (this.cpu.a & 0x80));
    }

    sbc(val) {
        this.adc(val ^ 0xFF);
    }

    cmp(reg, val) {
        const res = reg - val;
        this.cpu.p = (this.cpu.p & ~0x83) | (reg >= val ? 1 : 0) | ((res & 0xFF) === 0 ? 2 : (res & 0x80));
    }

    push(val) {
        this.cpuRam[0x0100 + this.cpu.sp] = val & 0xFF;
        this.cpu.sp = (this.cpu.sp - 1) & 0xFF;
    }

    pop() {
        this.cpu.sp = (this.cpu.sp + 1) & 0xFF;
        return this.cpuRam[0x0100 + this.cpu.sp];
    }

    push16(val) {
        this.push((val >> 8) & 0xFF);
        this.push(val & 0xFF);
    }

    pop16() {
        const lo = this.pop();
        const hi = this.pop();
        return lo | (hi << 8);
    }

    triggerNMI() {
        this.push16(this.cpu.pc);
        this.push((this.cpu.p & ~0x10) | 0x20);
        this.cpu.p |= 0x04; // Set interrupt disable
        this.cpu.pc = this.read16(0xFFFA);
        this.cpu.cycles += 7;
    }

    // Execute one full video frame (262 scanlines)
    renderFrame() {
        // 262 Scanlines in NTSC (0..239 visible, 240 post-render, 241 VBlank NMI, 242..260 VBlank, 261 Pre-render)
        for (let scanline = 0; scanline < 262; scanline++) {
            this.ppu.scanline = scanline;

            if (scanline < 240) {
                this.renderScanline(scanline);
            } else if (scanline === 241) {
                this.ppu.status |= 0x80; // Set VBlank flag
                if (this.ppu.ctrl & 0x80) {
                    this.triggerNMI();
                }
            } else if (scanline === 261) {
                this.ppu.status &= ~0xE0; // Clear VBlank & Sprite 0 hit
            }

            // ~113.66 CPU cycles per scanline (341 PPU dots / 3)
            let scanlineCycles = 0;
            while (scanlineCycles < 114) {
                const cyc = this.stepCPU();
                scanlineCycles += cyc;
            }
        }

        // Draw completed frame buffer to canvas
        this.ctx.putImageData(this.imageData, 0, 0);
        this.ppu.frame++;

        // Calculate FPS
        this.fpsCount++;
        const now = performance.now();
        if (now - this.fpsTimer >= 1000) {
            if (this.onFPS) this.onFPS(this.fpsCount);
            this.fpsCount = 0;
            this.fpsTimer = now;
        }
    }

    start() {
        if (this.running) return;
        this.running = true;
        this.lastFrameTime = performance.now();

        const loop = () => {
            if (!this.running) return;
            this.renderFrame();
            this.animationFrameId = requestAnimationFrame(loop);
        };

        this.animationFrameId = requestAnimationFrame(loop);
    }

    stop() {
        this.running = false;
        if (this.animationFrameId) {
            cancelAnimationFrame(this.animationFrameId);
            this.animationFrameId = null;
        }
    }
}

window.PCVivazEmulator = PCVivazEmulator;
