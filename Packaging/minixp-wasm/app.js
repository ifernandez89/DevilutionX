/**
 * Retro Virtual PC & Tiny Core Linux — WebAssembly Runtime Controller
 */

let emulator = null;
let isRunning = false;
let isPaused = false;
let bytesReadTotal = 0;
let diskLedTimer = null;

// DOM Elements
const selectOsProfile = document.getElementById("select_os_profile");
const hwProfileText = document.getElementById("hw_profile_text");
const btnStart = document.getElementById("btn_start");
const btnPause = document.getElementById("btn_pause");
const btnReset = document.getElementById("btn_reset");
const btnSaveState = document.getElementById("btn_save_state");
const btnLoadStateTrigger = document.getElementById("btn_load_state_trigger");
const inputLoadState = document.getElementById("input_load_state");
const btnFullscreen = document.getElementById("btn_fullscreen");
const statusDot = document.getElementById("status_dot");
const statusText = document.getElementById("status_text");
const screenWrapper = document.getElementById("screen_wrapper");
const screenContainer = document.getElementById("screen_container");
const diskLed = document.getElementById("disk_led");
const diskStatusText = document.getElementById("disk_status_text");
const diagnosticConsole = document.getElementById("diagnostic_console");
const consoleLog = document.getElementById("console_log");
const btnToggleLog = document.getElementById("btn_toggle_log");
const btnClearLog = document.getElementById("btn_clear_log");

// Gamepad UI Elements
const btnToggleGamepad = document.getElementById("btn_toggle_gamepad");
const gamepadOverlay = document.getElementById("gamepad_overlay");
const btnToggleFkeys = document.getElementById("btn_toggle_fkeys");
const gamepadFkeysBar = document.getElementById("gamepad_fkeys_bar");
const btnToggleProfile = document.getElementById("btn_toggle_profile");

// ROM & Computer Mode UI Elements
const btnLoadRomTrigger = document.getElementById("btn_load_rom_trigger");
const inputLoadRom = document.getElementById("input_load_rom");
const computerModePill = document.getElementById("computer_mode_pill");
const computerModeLabel = document.getElementById("computer_mode_label");

// Platform Detection Subsystem
const PlatformDetector = {
    isMobile() {
        const ua = navigator.userAgent || "";
        const mobileUa = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(ua);
        const touchPoints = (navigator.maxTouchPoints || 0) > 1;
        const smallScreen = window.innerWidth <= 768;
        return (mobileUa || (touchPoints && smallScreen));
    },
    isDesktopPC() {
        return !this.isMobile() && (window.matchMedia ? window.matchMedia("(pointer: fine)").matches : true);
    }
};

// NES ROM & UNIF Inspector Subsystem
const NesRomInspector = {
    inspect(buffer) {
        if (!buffer || buffer.byteLength < 16) {
            return { isValid: false, isComputer: false, format: "UNKNOWN", mapper: 0, title: "Desconocido" };
        }

        const bytes = new Uint8Array(buffer);
        const header4 = String.fromCharCode(bytes[0], bytes[1], bytes[2], bytes[3]);

        // UNIF Container Format Check
        if (header4 === "UNIF") {
            let offset = 32;
            let mapr = "";
            let name = "";
            let prgSize = 0;
            let hasBattery = false;

            while (offset + 8 <= bytes.length) {
                const chunkId = String.fromCharCode(bytes[offset], bytes[offset+1], bytes[offset+2], bytes[offset+3]);
                const chunkLen = bytes[offset+4] | (bytes[offset+5] << 8) | (bytes[offset+6] << 16) | (bytes[offset+7] << 24);
                const chunkDataStart = offset + 8;
                const chunkDataEnd = Math.min(chunkDataStart + chunkLen, bytes.length);

                if (chunkId === "MAPR") {
                    mapr = new TextDecoder().decode(bytes.slice(chunkDataStart, chunkDataEnd)).trim().replace(/\0/g, '');
                } else if (chunkId === "NAME") {
                    name = new TextDecoder().decode(bytes.slice(chunkDataStart, chunkDataEnd)).trim().replace(/\0/g, '');
                } else if (chunkId.startsWith("PRG")) {
                    prgSize += chunkLen;
                } else if (chunkId === "BATR") {
                    hasBattery = true;
                }

                offset += 8 + chunkLen;
            }

            const isEdu = mapr.includes("UNL-EDU2000") || mapr.includes("EDU2000") || name.toLowerCase().includes("educational") || name.toLowerCase().includes("vivaz");

            return {
                isValid: true,
                format: "UNIF",
                mapperName: mapr || "UNL-EDU2000",
                mapperNumber: 329, // NES 2.0 Mapper 329
                title: name || (isEdu ? "Educational Computer 2000 (PC Vivaz)" : "UNIF ROM"),
                isComputer: isEdu,
                prgSize: prgSize,
                hasBattery: hasBattery
            };
        }

        // iNES / NES 2.0 Format Check
        if (header4 === "NES\x1A") {
            const mapperLow = (bytes[6] >> 4) | (bytes[7] & 0xF0);
            const isNes2 = (bytes[7] & 0x0C) === 0x08;
            let mapper = mapperLow;
            if (isNes2) {
                mapper |= (bytes[8] & 0x0F) << 8;
            }

            const isComputerMapper = (mapper === 329 || mapper === 177 || mapper === 35);

            return {
                isValid: true,
                format: isNes2 ? "NES 2.0" : "iNES",
                mapperNumber: mapper,
                mapperName: `Mapper ${mapper}`,
                title: isComputerMapper ? "PC Vivaz / Educational Computer (Mapper 329)" : "NES Game ROM",
                isComputer: isComputerMapper,
                prgSize: bytes[4] * 16384,
                hasBattery: (bytes[6] & 0x02) !== 0
            };
        }

        return { isValid: false, isComputer: false, format: "RAW", mapper: 0, title: "Archivo Genérico" };
    }
};

// NES Keyboard Matrix & Mouse Driver Subsystem (Educational Computer / Subor Architecture)
const NesComputerInputAdapter = {
    active: false,
    keyboardMatrix: new Uint8Array(13), // 13 rows matrix for full Subor/Edu keyboard
    mouse: { x: 0, y: 0, lastX: 0, lastY: 0, deltaX: 0, deltaY: 0, leftBtn: false, rightBtn: false, strobeLatch: 0, shiftReg: 0 },
    listenersAttached: false,

    // Subor / Educational Matrix Key Definitions [Row, BitMask]
    KEY_MATRIX_MAP: {
        "KeyA": [1, 0x02], "KeyB": [7, 0x04], "KeyC": [8, 0x04], "KeyD": [2, 0x04], "KeyE": [2, 0x02],
        "KeyF": [2, 0x08], "KeyG": [3, 0x04], "KeyH": [3, 0x08], "KeyI": [4, 0x04], "KeyJ": [4, 0x08],
        "KeyK": [5, 0x04], "KeyL": [5, 0x08], "KeyM": [6, 0x04], "KeyN": [7, 0x08], "KeyO": [4, 0x02],
        "KeyP": [5, 0x02], "KeyQ": [1, 0x04], "KeyR": [2, 0x01], "KeyS": [1, 0x08], "KeyT": [3, 0x02],
        "KeyU": [4, 0x01], "KeyV": [8, 0x08], "KeyW": [1, 0x01], "KeyX": [8, 0x02], "KeyY": [3, 0x01],
        "KeyZ": [8, 0x01], "Digit0": [5, 0x01], "Digit1": [0, 0x02], "Digit2": [0, 0x04], "Digit3": [0, 0x08],
        "Digit4": [1, 0x10], "Digit5": [2, 0x10], "Digit6": [3, 0x10], "Digit7": [4, 0x10], "Digit8": [5, 0x10],
        "Digit9": [6, 0x01], "Enter": [7, 0x02], "Space": [9, 0x04], "Backspace": [6, 0x02], "Escape": [0, 0x01],
        "ShiftLeft": [0, 0x10], "ShiftRight": [0, 0x10], "ControlLeft": [9, 0x08], "ControlRight": [9, 0x08],
        "ArrowUp": [10, 0x01], "ArrowDown": [10, 0x02], "ArrowLeft": [10, 0x04], "ArrowRight": [10, 0x08]
    },

    init() {
        if (this.listenersAttached) return;

        // PC Physical Keyboard Capture
        window.addEventListener("keydown", (e) => {
            if (!this.active || !PlatformDetector.isDesktopPC()) return;
            const mapping = this.KEY_MATRIX_MAP[e.code];
            if (mapping) {
                const [row, mask] = mapping;
                this.keyboardMatrix[row] |= mask;
                e.preventDefault();
            }
        });

        window.addEventListener("keyup", (e) => {
            if (!this.active || !PlatformDetector.isDesktopPC()) return;
            const mapping = this.KEY_MATRIX_MAP[e.code];
            if (mapping) {
                const [row, mask] = mapping;
                this.keyboardMatrix[row] &= ~mask;
                e.preventDefault();
            }
        });

        // PC Physical Mouse Capture
        screenContainer.addEventListener("mousemove", (e) => {
            if (!this.active || !PlatformDetector.isDesktopPC()) return;
            this.mouse.deltaX += (e.movementX || 0);
            this.mouse.deltaY += (e.movementY || 0);
            this.mouse.x = e.offsetX;
            this.mouse.y = e.offsetY;
        });

        screenContainer.addEventListener("mousedown", (e) => {
            if (!this.active || !PlatformDetector.isDesktopPC()) return;
            if (e.button === 0) this.mouse.leftBtn = true;
            if (e.button === 2) this.mouse.rightBtn = true;
        });

        screenContainer.addEventListener("mouseup", (e) => {
            if (!this.active || !PlatformDetector.isDesktopPC()) return;
            if (e.button === 0) this.mouse.leftBtn = false;
            if (e.button === 2) this.mouse.rightBtn = false;
        });

        screenContainer.addEventListener("contextmenu", (e) => {
            if (this.active && PlatformDetector.isDesktopPC()) {
                e.preventDefault();
            }
        });

        this.listenersAttached = true;
    },

    setActive(state, romInfo) {
        this.active = state;
        if (state && PlatformDetector.isDesktopPC()) {
            if (computerModePill) {
                computerModePill.style.display = "inline-flex";
                if (computerModeLabel) {
                    computerModeLabel.textContent = `💻 Modo PC Vivaz Activo (${romInfo ? romInfo.title : "Teclado + Ratón"})`;
                }
            }
            // Auto-hide mobile touch gamepad on desktop PC when in Computer Mode
            if (gamepadOverlay) {
                gamepadOverlay.style.display = "none";
            }
            if (btnToggleGamepad) {
                btnToggleGamepad.style.display = "none";
            }
            logDiagnostic(`[COMPUTER MODE] Activado protocolo exclusivo PC Vivaz: Teclado QWERTY completo + Ratón serial $4016/$4017.`, "success");
        } else {
            if (computerModePill) {
                computerModePill.style.display = "none";
            }
            if (btnToggleGamepad) {
                btnToggleGamepad.style.display = "inline-flex";
            }
            // Reset matrix and mouse
            this.keyboardMatrix.fill(0);
            this.mouse.leftBtn = false;
            this.mouse.rightBtn = false;
            this.mouse.deltaX = 0;
            this.mouse.deltaY = 0;
        }
    }
};

// Initialize input adapter listeners
NesComputerInputAdapter.init();

// OS Configurations Registry
const OS_PROFILES = {
    pcvivaz: {
        name: "🖥️ NES PC Vivaz (Educational Computer 2000 • UNL-EDU2000)",
        memory_size: 128 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,
        boot_order: 0x123,
        media_type: "cdrom",
        media_url: "tinycore-retro.iso",
        isNesComputer: true,
        rom_url: "roms/pcvivaz-unif.nes",
        acpi: false
    },
    nes_game: {
        name: "🎮 NES Clásica (Modo Gamepad Estándar)",
        memory_size: 128 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,
        boot_order: 0x123,
        media_type: "cdrom",
        media_url: "tinycore-retro.iso",
        isNesComputer: false,
        acpi: false
    },
    tinycore_retro: {
        name: "Retro PC (Tiny Core Linux 15.x + DOSBox + Doom + Tree + EmelFM)",
        memory_size: 256 * 1024 * 1024,      // 256 MB RAM
        vga_memory_size: 8 * 1024 * 1024,     // 8 MB VRAM VESA
        boot_order: 0x123,                    // CD-ROM
        media_type: "cdrom",
        media_url: "tinycore-retro.iso",
        acpi: false
    },
    tinycore: {
        name: "Tiny Core Linux (Base GUI LiveCD)",
        memory_size: 128 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,
        boot_order: 0x123,
        media_type: "cdrom",
        media_url: "tinycore.iso",
        acpi: false
    },
    win31: {
        name: "Windows 3.11 for Workgroups (Virtual HD C:)",
        memory_size: 64 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,
        boot_order: 0x213,                    // Hard Drive
        media_type: "hda",
        media_url: "win31.img",
        acpi: false
    },
    windows30: {
        name: "Windows 3.0 (Virtual HD C:)",
        memory_size: 32 * 1024 * 1024,
        vga_memory_size: 4 * 1024 * 1024,
        boot_order: 0x213,
        media_type: "hda",
        media_url: "windows30.img",
        acpi: false
    },
    freedos: {
        name: "FreeDOS 1.3 (Floppy A:)",
        memory_size: 32 * 1024 * 1024,
        vga_memory_size: 4 * 1024 * 1024,
        boot_order: 0x312,
        media_type: "fda",
        media_url: "freedos.img",
        acpi: false
    },
    kolibri: {
        name: "KolibriOS (Floppy A:)",
        memory_size: 64 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,
        boot_order: 0x312,
        media_type: "fda",
        media_url: "kolibri.img",
        acpi: false
    },
    minixp: {
        name: "Mini Windows XP (Live CD)",
        memory_size: 256 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,
        boot_order: 0x123,
        media_type: "cdrom",
        media_url: "minixp.iso",
        acpi: false
    }
};

// Hardware Scancodes Table (Make / Break)
const SCANCODES = {
    ESC:    { make: [0x01], break: [0x81] },
    TAB:    { make: [0x0F], break: [0x8F] },
    SPACE:  { make: [0x39], break: [0xB9] },
    LCTRL:  { make: [0x1D], break: [0x9D] },
    LALT:   { make: [0x38], break: [0xB8] },
    LSHIFT: { make: [0x2A], break: [0xAA] },
    ENTER:  { make: [0x1C], break: [0x9C] },
    UP:     { make: [0xE0, 0x48], break: [0xE0, 0xC8] },
    DOWN:   { make: [0xE0, 0x50], break: [0xE0, 0xD0] },
    LEFT:   { make: [0xE0, 0x4B], break: [0xE0, 0xCB] },
    RIGHT:  { make: [0xE0, 0x4D], break: [0xE0, 0xCD] },
    W:      { make: [0x11], break: [0x91] },
    A:      { make: [0x1E], break: [0x9E] },
    S:      { make: [0x1F], break: [0x9F] },
    D:      { make: [0x20], break: [0xA0] },
    F1:     { make: [0x3B], break: [0xBB] },
    F2:     { make: [0x3C], break: [0xBC] },
    F3:     { make: [0x3D], break: [0xBD] },
    F4:     { make: [0x3E], break: [0xBE] },
    F5:     { make: [0x3F], break: [0xBF] },
    "1":    { make: [0x02], break: [0x82] },
    "2":    { make: [0x03], break: [0x83] },
    "3":    { make: [0x04], break: [0x84] },
    "4":    { make: [0x05], break: [0x85] },
    Y:      { make: [0x15], break: [0x95] },
    N:      { make: [0x31], break: [0xB1] }
};

// Control Profiles
const CONTROL_PROFILES = [
    { name: "🎮 WASD", dpad: { UP: "W", DOWN: "S", LEFT: "A", RIGHT: "D" } },
    { name: "🏹 Flechas", dpad: { UP: "UP", DOWN: "DOWN", LEFT: "LEFT", RIGHT: "RIGHT" } }
];
let currentProfileIdx = 0;

// Input Mapper Engine
const InputMapper = {
    activeKeys: new Set(),

    sendKey(keyName, isDown) {
        if (!emulator) return;

        let scancodeKey = keyName;
        const profile = CONTROL_PROFILES[currentProfileIdx];
        if (profile.dpad[keyName]) {
            scancodeKey = profile.dpad[keyName];
        }

        const codes = SCANCODES[scancodeKey];
        if (codes) {
            try {
                const bytes = isDown ? codes.make : codes.break;
                if (emulator.keyboard_send_scancodes) {
                    emulator.keyboard_send_scancodes(bytes);
                } else if (emulator.v86 && emulator.v86.keyboard_send_scancodes) {
                    emulator.v86.keyboard_send_scancodes(bytes);
                }
            } catch (err) {}
        }

        // Also trigger synthetic event for maximum compatibility
        const evtType = isDown ? "keydown" : "keyup";
        const evt = new KeyboardEvent(evtType, { key: keyName, code: keyName, bubbles: true });
        window.dispatchEvent(evt);
    },

    press(keyName) {
        if (this.activeKeys.has(keyName)) return;
        this.activeKeys.add(keyName);
        this.sendKey(keyName, true);
    },

    release(keyName) {
        if (!this.activeKeys.has(keyName)) return;
        this.activeKeys.delete(keyName);
        this.sendKey(keyName, false);
    },

    releaseAll() {
        for (const key of Array.from(this.activeKeys)) {
            this.release(key);
        }
    }
};

function formatTime() {
    const d = new Date();
    return d.toTimeString().split(' ')[0];
}

function logDiagnostic(msg, type = "info") {
    if (!consoleLog) return;
    const line = document.createElement("div");
    line.className = `log-line log-${type}`;
    line.textContent = `[${formatTime()}] ${msg}`;
    consoleLog.appendChild(line);
    consoleLog.scrollTop = consoleLog.scrollHeight;
}

function updateStatus(text, stateClass) {
    statusText.textContent = text;
    statusDot.className = "status-dot " + (stateClass || "");
}

function triggerDiskActivity(sectors = 1) {
    if (!diskLed) return;
    bytesReadTotal += sectors * 2048;
    const mb = (bytesReadTotal / (1024 * 1024)).toFixed(1);
    diskLed.classList.add("active");
    diskStatusText.textContent = `Disco: Leyendo (${mb} MB)`;

    clearTimeout(diskLedTimer);
    diskLedTimer = setTimeout(() => {
        diskLed.classList.remove("active");
        diskStatusText.textContent = `Disco: Inactivo (${mb} MB transferidos)`;
    }, 400);
}

function createV86Instance(config) {
    const V86Class = window.V86Starter || window.V86 || (typeof V86Starter !== "undefined" ? V86Starter : (typeof V86 !== "undefined" ? V86 : null));
    if (!V86Class) {
        throw new Error("El motor v86 no se ha podido cargar. Por favor recarga la página.");
    }
    return new V86Class(config);
}

function initEmulator(customBuffer = null) {
    if (emulator) {
        emulator.destroy();
        emulator = null;
    }

    bytesReadTotal = 0;
    const profileKey = selectOsProfile ? selectOsProfile.value : "pcvivaz";
    const profile = OS_PROFILES[profileKey] || OS_PROFILES.pcvivaz;

    if (hwProfileText) {
        hwProfileText.textContent = profile.name;
    }

    // Determine if current configuration is Computer Mode (PC Vivaz)
    let isComputerMode = !!profile.isNesComputer;
    let romInfo = null;

    if (customBuffer) {
        romInfo = NesRomInspector.inspect(customBuffer);
        if (romInfo.isValid) {
            isComputerMode = romInfo.isComputer;
            logDiagnostic(`[ROM INSPECTOR] Archivo cargado: ${romInfo.title} (${romInfo.format}, ${romInfo.mapperName}, ${(romInfo.prgSize/1024).toFixed(0)} KB PRG, Batería: ${romInfo.hasBattery ? 'SÍ' : 'NO'})`, "success");
        }
    } else if (profile.isNesComputer) {
        romInfo = { title: "Educational Computer 2000 (PC Vivaz)", format: "UNIF", mapperName: "UNL-EDU2000 (Mapper 329)", isComputer: true };
    }

    // Configure Input Mode: ONLY PC Desktop gets Computer Mode (QWERTY + Mouse)
    if (isComputerMode && PlatformDetector.isDesktopPC()) {
        NesComputerInputAdapter.setActive(true, romInfo);
        logDiagnostic(`[INPUT ROUTER] Plataforma detectada: PC Desktop. Modo Computadora activado (QWERTY + Ratón). Gamepad oculto.`, "info");
    } else {
        NesComputerInputAdapter.setActive(false);
        if (PlatformDetector.isMobile()) {
            logDiagnostic(`[INPUT ROUTER] Plataforma detectada: Móvil / Táctil. Modo Gamepad táctil estándar activo.`, "info");
        } else {
            logDiagnostic(`[INPUT ROUTER] Modo Gamepad estándar activo para juegos NES clásicos.`, "info");
        }
    }

    updateStatus(`Cargando ${profile.name.split(' ')[0]}...`, "paused");
    
    const cacheBuster = Date.now();
    const config = {
        wasm_path: "v86.wasm?v=" + cacheBuster,
        memory_size: profile.memory_size,
        vga_memory_size: profile.vga_memory_size,
        screen_container: screenContainer,
        bios: { url: "bios/seabios.bin?v=" + cacheBuster },
        vga_bios: { url: "bios/vgabios.bin?v=" + cacheBuster },
        boot_order: profile.boot_order,
        disable_speaker: false,
        autostart: true,
        network_relay_url: null,
    };

    if (customBuffer) {
        config.cdrom = { buffer: customBuffer };
        logDiagnostic("Iniciando con imagen / ROM personalizada cargada por el usuario...", "info");
    } else {
        config[profile.media_type] = { url: `${profile.media_url}?v=${cacheBuster}` };
        logDiagnostic(`Iniciando ${profile.name}...`, "info");
    }

    try {
        const ramMb = (config.memory_size / (1024 * 1024)).toFixed(0);
        const vramMb = (config.vga_memory_size / (1024 * 1024)).toFixed(0);

        logDiagnostic(`[CONFIG] ${ramMb} MB RAM • ${vramMb} MB VRAM • Medio: ${profile.media_type.toUpperCase()} (${profile.media_url})`, "info");

        emulator = createV86Instance(config);

        emulator.add_listener("download-progress", function(e) {
            if (e && e.file_name) {
                const loadedMb = (e.loaded / (1024 * 1024)).toFixed(1);
                const totalMb = e.total ? (e.total / (1024 * 1024)).toFixed(1) + " MB" : "";
                logDiagnostic(`Descargando ${e.file_name}: ${loadedMb} MB / ${totalMb}`, "disk");
            }
        });

        emulator.add_listener("download-error", function(e) {
            logDiagnostic(`[ERROR DE DESCARGA] Falló la carga de: ${e.file_name || e.url || 'recurso'}`, "error");
            updateStatus("Error de descarga", "paused");
        });

        emulator.add_listener("emulator-ready", function() {
            isRunning = true;
            isPaused = false;
            updateStatus("En ejecución", "running");
            logDiagnostic(`CPU virtual inicializada (${ramMb} MB RAM). Arrancando SeaBIOS...`, "success");
            btnStart.disabled = true;
            btnPause.disabled = false;
            btnReset.disabled = false;
            btnSaveState.disabled = false;
        });

        emulator.add_listener("screen-set-mode", function(is_graphical) {
            const canvas = screenContainer.querySelector("canvas");
            const textDiv = screenContainer.querySelector("div");
            if (is_graphical) {
                if (canvas) canvas.style.display = "block";
                if (textDiv) textDiv.style.display = "none";
                logDiagnostic("Modo gráfico VESA/VGA activado (Escritorio GUI).", "success");
                updateStatus("Modo Gráfico", "running");
            } else {
                if (canvas) canvas.style.display = "none";
                if (textDiv) textDiv.style.display = "block";
                logDiagnostic("Modo de texto BIOS/Terminal activo.", "info");
            }
        });

        let serialLineBuffer = "";
        emulator.add_listener("serial0-output-char", function(char) {
            const c = typeof char === "string" ? char : String.fromCharCode(char);
            if (c === "\n") {
                if (serialLineBuffer.trim().length > 0) {
                    if (serialLineBuffer.includes("Linux") || serialLineBuffer.includes("Boot") || serialLineBuffer.includes("TC")) {
                        logDiagnostic(`[Kernel] ${serialLineBuffer.trim()}`, "info");
                    }
                }
                serialLineBuffer = "";
            } else if (c !== "\r" && serialLineBuffer.length < 120) {
                serialLineBuffer += c;
            }
        });

        emulator.add_listener("ide-read-start", function() {
            triggerDiskActivity(16);
        });

        emulator.add_listener("ide-read-end", function() {
            triggerDiskActivity(16);
        });

        let lastInstructions = 0;
        const telemetryInterval = setInterval(() => {
            if (!emulator || !isRunning || isPaused) return;
            try {
                if (emulator.v86 && emulator.v86.cpu) {
                    const currentInstr = emulator.v86.cpu.instruction_counter || 0;
                    const diff = currentInstr - lastInstructions;
                    lastInstructions = currentInstr;
                    if (diff > 0 && diskStatusText && !diskLed.classList.contains("active")) {
                        const mips = (diff / 1000000).toFixed(1);
                        diskStatusText.textContent = `CPU x86: Procesando (${mips} MIPS)`;
                    }
                }
            } catch (ex) {}
        }, 1000);

        emulator.add_listener("emulator-stopped", function() {
            isRunning = false;
            isPaused = false;
            clearInterval(telemetryInterval);
            updateStatus("Emulador Detenido", "");
            logDiagnostic("Emulador detenido.", "warn");
            btnStart.disabled = false;
            btnPause.disabled = true;
            btnReset.disabled = true;
            btnSaveState.disabled = true;
        });

    } catch (err) {
        logDiagnostic(`Error crítico al inicializar v86: ${err.message}`, "error");
        updateStatus("Error de inicio", "paused");
    }
}

// OS Profile Selector Event
if (selectOsProfile) {
    selectOsProfile.addEventListener("change", () => {
        logDiagnostic(`Cambiando a perfil: ${selectOsProfile.value}`, "warn");
        initEmulator();
    });
}

// Start Button
btnStart.addEventListener("click", () => {
    if (!isRunning) {
        initEmulator();
    }
});

// Pause / Resume Button
btnPause.addEventListener("click", () => {
    if (!emulator || !isRunning) return;

    if (isPaused) {
        emulator.run();
        isPaused = false;
        btnPause.innerHTML = `<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="16"></rect></svg> Pausar`;
        updateStatus("En ejecución", "running");
        logDiagnostic("Emulación reanudada.", "info");
    } else {
        emulator.stop();
        isPaused = true;
        btnPause.innerHTML = `<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg> Reanudar`;
        updateStatus("Pausado", "paused");
        logDiagnostic("Emulación pausada.", "warn");
    }
});

// Reset Button
btnReset.addEventListener("click", () => {
    if (!emulator) return;
    if (confirm("¿Deseas reiniciar la máquina virtual?")) {
        logDiagnostic("Reiniciando máquina virtual...", "warn");
        emulator.restart();
        updateStatus("Reiniciando...", "paused");
    }
});

// Gamepad UI Toggles & Actions
if (btnToggleGamepad && gamepadOverlay) {
    btnToggleGamepad.addEventListener("click", () => {
        const isHidden = gamepadOverlay.style.display === "none";
        gamepadOverlay.style.display = isHidden ? "flex" : "none";
        btnToggleGamepad.classList.toggle("btn-active", isHidden);
    });
}

if (btnToggleFkeys && gamepadFkeysBar) {
    btnToggleFkeys.addEventListener("click", () => {
        const isHidden = gamepadFkeysBar.style.display === "none";
        gamepadFkeysBar.style.display = isHidden ? "flex" : "none";
        btnToggleFkeys.classList.toggle("active", isHidden);
    });
}

if (btnToggleProfile) {
    btnToggleProfile.addEventListener("click", () => {
        currentProfileIdx = (currentProfileIdx + 1) % CONTROL_PROFILES.length;
        btnToggleProfile.textContent = CONTROL_PROFILES[currentProfileIdx].name;
    });
}

// Attach Touch & Mouse Handlers to Gamepad Buttons
function attachKeyButton(elem, keyName) {
    if (!elem) return;

    const startHandler = (e) => {
        e.preventDefault();
        elem.classList.add("pressed");
        InputMapper.press(keyName);
    };

    const endHandler = (e) => {
        e.preventDefault();
        elem.classList.remove("pressed");
        InputMapper.release(keyName);
    };

    elem.addEventListener("touchstart", startHandler, { passive: false });
    elem.addEventListener("touchend", endHandler, { passive: false });
    elem.addEventListener("touchcancel", endHandler, { passive: false });
    elem.addEventListener("mousedown", startHandler);
    elem.addEventListener("mouseup", endHandler);
    elem.addEventListener("mouseleave", endHandler);
}

document.querySelectorAll("[data-key]").forEach(btn => {
    attachKeyButton(btn, btn.getAttribute("data-key"));
});

document.querySelectorAll("[data-dpad]").forEach(btn => {
    attachKeyButton(btn, btn.getAttribute("data-dpad"));
});

const ACTION_MAP = {
    FIRE: "LCTRL",
    USE: "SPACE",
    RUN: "LSHIFT",
    ENTER: "ENTER"
};

document.querySelectorAll("[data-action]").forEach(btn => {
    const act = btn.getAttribute("data-action");
    const key = ACTION_MAP[act] || act;
    attachKeyButton(btn, key);
});

// Save State
btnSaveState.addEventListener("click", () => {
    if (!emulator || !isRunning) return;

    updateStatus("Generando Snapshot...", "paused");
    logDiagnostic("Guardando estado de la memoria RAM a archivo...", "info");

    emulator.save_state(function(error, state_data) {
        if (error) {
            alert("Error al guardar estado: " + error.message);
            updateStatus("En ejecución", "running");
            logDiagnostic("Fallo al guardar estado: " + error.message, "error");
            return;
        }

        const blob = new Blob([state_data], { type: "application/octet-stream" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = `retropc_state_${Date.now()}.bin`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        updateStatus("Snapshot Guardado", "running");
        logDiagnostic("Snapshot de RAM guardado exitosamente.", "success");
    });
});

// Load State Trigger
btnLoadStateTrigger.addEventListener("click", () => {
    inputLoadState.click();
});

inputLoadState.addEventListener("change", (e) => {
    const file = e.target.files[0];
    if (!file) return;

    logDiagnostic(`Restaurando snapshot de RAM: ${file.name}...`, "info");
    const reader = new FileReader();
    reader.onload = function() {
        if (emulator) {
            emulator.restore_state(reader.result);
            updateStatus("Estado restaurado", "running");
            logDiagnostic("Estado de RAM restaurado en emulador activo.", "success");
        } else {
            initEmulator();
        }
    };
    reader.readAsArrayBuffer(file);
});

// ROM Loader Trigger & File Input
if (btnLoadRomTrigger && inputLoadRom) {
    btnLoadRomTrigger.addEventListener("click", () => {
        inputLoadRom.click();
    });

    inputLoadRom.addEventListener("change", (e) => {
        const file = e.target.files[0];
        if (!file) return;

        logDiagnostic(`Cargando archivo ROM: ${file.name}...`, "info");
        const reader = new FileReader();
        reader.onload = function() {
            const buffer = reader.result;
            const romInfo = NesRomInspector.inspect(buffer);
            if (romInfo.isComputer) {
                if (selectOsProfile) selectOsProfile.value = "pcvivaz";
            } else {
                if (selectOsProfile) selectOsProfile.value = "nes_game";
            }
            initEmulator(buffer);
        };
        reader.readAsArrayBuffer(file);
    });
}

// Drag and drop ROM files onto screen
if (screenWrapper) {
    screenWrapper.addEventListener("dragover", (e) => {
        e.preventDefault();
        screenWrapper.classList.add("drag-over");
    });

    screenWrapper.addEventListener("dragleave", (e) => {
        e.preventDefault();
        screenWrapper.classList.remove("drag-over");
    });

    screenWrapper.addEventListener("drop", (e) => {
        e.preventDefault();
        screenWrapper.classList.remove("drag-over");
        if (e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files.length > 0) {
            const file = e.dataTransfer.files[0];
            logDiagnostic(`ROM arrastrada al visor: ${file.name}...`, "info");
            const reader = new FileReader();
            reader.onload = function() {
                const buffer = reader.result;
                const romInfo = NesRomInspector.inspect(buffer);
                if (romInfo.isComputer) {
                    if (selectOsProfile) selectOsProfile.value = "pcvivaz";
                } else {
                    if (selectOsProfile) selectOsProfile.value = "nes_game";
                }
                initEmulator(buffer);
            };
            reader.readAsArrayBuffer(file);
        }
    });
}

// Fullscreen Button
if (btnFullscreen && screenWrapper) {
    btnFullscreen.addEventListener("click", () => {
        if (!document.fullscreenElement) {
            screenWrapper.requestFullscreen().catch(err => {
                alert(`Error al entrar en pantalla completa: ${err.message}`);
            });
        } else {
            document.exitFullscreen();
        }
    });
}

// Capture mouse on click
screenContainer.addEventListener("click", () => {
    if (emulator && emulator.lock_mouse) {
        emulator.lock_mouse();
    }
});

// Toggle Log Console
if (btnToggleLog && diagnosticConsole) {
    btnToggleLog.addEventListener("click", () => {
        diagnosticConsole.classList.toggle("collapsed");
    });
}

// Clear Log Console
if (btnClearLog && consoleLog) {
    btnClearLog.addEventListener("click", () => {
        consoleLog.innerHTML = "";
        logDiagnostic("Consola limpiada.", "info");
    });
}

// Auto-start on load
window.addEventListener("DOMContentLoaded", () => {
    logDiagnostic("Página cargada. Iniciando Retro PC...", "info");
    initEmulator();
});
