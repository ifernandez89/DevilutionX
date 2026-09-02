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

// OS Configurations Registry
const OS_PROFILES = {
    tinycore_retro: {
        name: "Retro PC (Tiny Core Linux 15.x + DOSBox + Doom + Tree + EmelFM)",
        memory_size: 128 * 1024 * 1024,      // 128 MB RAM
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
    const profileKey = selectOsProfile ? selectOsProfile.value : "tinycore_retro";
    const profile = OS_PROFILES[profileKey] || OS_PROFILES.tinycore_retro;

    if (hwProfileText) {
        hwProfileText.textContent = profile.name;
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
        logDiagnostic("Iniciando con imagen personalizada cargada por el usuario...", "info");
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

// Fullscreen Button
btnFullscreen.addEventListener("click", () => {
    if (!document.fullscreenElement) {
        screenWrapper.requestFullscreen().catch(err => {
            alert(`Error al entrar en pantalla completa: ${err.message}`);
        });
    } else {
        document.exitFullscreen();
    }
});

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
