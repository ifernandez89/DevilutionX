/**
 * Tiny Core Linux — WebAssembly Runtime Controller
 */

let emulator = null;
let isRunning = false;
let isPaused = false;
let bytesReadTotal = 0;
let diskLedTimer = null;

// DOM Elements
const selectOs = document.getElementById("select_os");
const btnStart = document.getElementById("btn_start");
const btnPause = document.getElementById("btn_pause");
const btnReset = document.getElementById("btn_reset");
const btnSaveState = document.getElementById("btn_save_state");
const btnLoadStateTrigger = document.getElementById("btn_load_state_trigger");
const inputLoadState = document.getElementById("input_load_state");
const btnCustomIsoTrigger = document.getElementById("btn_custom_iso_trigger");
const inputCustomIso = document.getElementById("input_custom_iso");
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

function formatTime() {
    const d = new Date();
    return d.toTimeString().split(' ')[0];
}

const MAX_LOG_LINES = 100;
let pendingLogs = [];
let logFlushScheduled = false;

function flushLogs() {
    logFlushScheduled = false;
    if (!consoleLog || pendingLogs.length === 0) return;

    const frag = document.createDocumentFragment();
    for (const item of pendingLogs) {
        const line = document.createElement("div");
        line.className = `log-line log-${item.type}`;
        line.textContent = `[${item.time}] ${item.msg}`;
        frag.appendChild(line);
    }
    pendingLogs = [];

    consoleLog.appendChild(frag);

    // Trim excessive log nodes to avoid memory/DOM bloat
    while (consoleLog.childNodes.length > MAX_LOG_LINES) {
        consoleLog.removeChild(consoleLog.firstChild);
    }

    consoleLog.scrollTop = consoleLog.scrollHeight;
}

function logDiagnostic(msg, type = "info") {
    pendingLogs.push({ msg, type, time: formatTime() });
    if (!logFlushScheduled) {
        logFlushScheduled = true;
        requestAnimationFrame(flushLogs);
    }
}

function updateStatus(text, stateClass) {
    statusText.textContent = text;
    statusDot.className = "status-dot " + (stateClass || "");
}

let lastDiskUpdate = 0;
function triggerDiskActivity(sectors = 1) {
    if (!diskLed) return;
    bytesReadTotal += sectors * 2048;

    const now = Date.now();
    if (now - lastDiskUpdate < 150) return; // Throttle DOM updates
    lastDiskUpdate = now;

    const mb = (bytesReadTotal / (1024 * 1024)).toFixed(1);
    diskLed.classList.add("active");
    diskStatusText.textContent = `CD-ROM: Leyendo (${mb} MB)`;

    clearTimeout(diskLedTimer);
    diskLedTimer = setTimeout(() => {
        diskLed.classList.remove("active");
        diskStatusText.textContent = `CD-ROM: Inactivo (${mb} MB transferidos)`;
    }, 400);
}

function createV86Instance(config) {
    const V86Class = window.V86Starter || window.V86 || (typeof V86Starter !== "undefined" ? V86Starter : (typeof V86 !== "undefined" ? V86 : null));
    if (!V86Class) {
        throw new Error("El motor v86 no se ha podido cargar. Por favor recarga la página.");
    }
    return new V86Class(config);
}

function initEmulator(customCdromBuffer = null) {
    if (emulator) {
        try { emulator.destroy(); } catch (e) {}
        emulator = null;
    }

    bytesReadTotal = 0;
    updateStatus("Cargando BIOS y WebAssembly...", "paused");

    const selectedIso = selectOs ? selectOs.value : "tinycore.iso";
    const isTinyCore = selectedIso.includes("tinycore") || (!customCdromBuffer && selectedIso === "tinycore.iso");
    const memSize = isTinyCore ? 256 * 1024 * 1024 : 512 * 1024 * 1024;
    const osTitle = isTinyCore ? "Tiny Core Linux" : "Linux Live";

    logDiagnostic(`Iniciando emulador x86 (${memSize / (1024 * 1024)} MB RAM, SeaBIOS, VGABIOS)...`, "info");

    const cacheBuster = Date.now();
    const config = {
        wasm_path: "v86.wasm?v=" + cacheBuster,
        memory_size: memSize,
        vga_memory_size: 8 * 1024 * 1024,
        screen_container: screenContainer,
        bios: { url: "seabios.bin?v=" + cacheBuster },
        vga_bios: { url: "vgabios.bin?v=" + cacheBuster },
        acpi: !isTinyCore, // TinyCore boots much cleaner and faster without ACPI overhead in v86
        autostart: true,
        network_relay_url: null,
    };

    if (customCdromBuffer) {
        logDiagnostic("Cargando ISO personalizada desde búfer local...", "info");
        config.cdrom = { buffer: customCdromBuffer };
    } else {
        logDiagnostic(`Cargando ${selectedIso} (${osTitle})...`, "info");
        config.cdrom = { url: selectedIso + "?v=" + cacheBuster };
    }

    try {
        emulator = createV86Instance(config);

        emulator.add_listener("download-progress", function(e) {
            if (e && e.file_name) {
                const loadedMb = (e.loaded / (1024 * 1024)).toFixed(1);
                const totalMb = e.total ? (e.total / (1024 * 1024)).toFixed(1) + " MB" : "";
                logDiagnostic(`Descargando ${e.file_name}: ${loadedMb} MB / ${totalMb}`, "disk");
            }
        });

        emulator.add_listener("emulator-ready", function() {
            isRunning = true;
            isPaused = false;
            updateStatus(`Ejecutando ${osTitle}`, "running");
            logDiagnostic(`CPU virtual x86 inicializada (${memSize / (1024 * 1024)} MB RAM). Arrancando SeaBIOS...`, "success");
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
                logDiagnostic(`Modo de video gráfico VESA/VGA activado (${osTitle}).`, "success");
                updateStatus(`${osTitle} Gráfico`, "running");
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
                    // Only log meaningful system messages
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

        // Telemetría suave y no invasiva de CPU
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
            } catch (ex) {
                // ignore
            }
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
        console.error("Error al iniciar emulador v86:", err);
        updateStatus("Error: " + err.message, "");
        logDiagnostic("Error: " + err.message, "error");
    }
}

// OS Select Dropdown
if (selectOs) {
    selectOs.addEventListener("change", () => {
        if (selectOs.value === "custom") {
            btnCustomIsoTrigger.click();
        } else {
            initEmulator();
        }
    });
}

// Start Button
btnStart.addEventListener("click", () => {
    initEmulator();
});

// Pause / Resume Button
btnPause.addEventListener("click", () => {
    if (!emulator || !isRunning) return;

    if (isPaused) {
        emulator.run();
        isPaused = false;
        btnPause.innerHTML = '<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="4" height="16"></rect></svg> Pausar';
        updateStatus("Ejecutando Tiny Core Linux", "running");
        logDiagnostic("Emulación reanudada.", "info");
    } else {
        emulator.stop();
        isPaused = true;
        btnPause.innerHTML = '<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg> Reanudar';
        updateStatus("Emulación Pausada", "paused");
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

// Save State (Snapshot to File)
btnSaveState.addEventListener("click", () => {
    if (!emulator || !isRunning) return;

    updateStatus("Generando Snapshot...", "paused");
    logDiagnostic("Guardando estado completo de la memoria RAM a archivo...", "info");

    emulator.save_state(function(error, state_data) {
        if (error) {
            alert("Error al guardar estado: " + error.message);
            updateStatus("Ejecutando Tiny Core Linux", "running");
            logDiagnostic("Fallo al guardar estado: " + error.message, "error");
            return;
        }

        const blob = new Blob([state_data], { type: "application/octet-stream" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = `tinycore_state_${Date.now()}.bin`;
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
            const config = {
                wasm_path: "v86.wasm",
                memory_size: 512 * 1024 * 1024,
                vga_memory_size: 16 * 1024 * 1024,
                screen_container: screenContainer,
                bios: { url: "bios/seabios.bin" },
                vga_bios: { url: "bios/vgabios.bin" },
                acpi: true,
                initial_state: { buffer: reader.result },
                autostart: true,
            };
            emulator = createV86Instance(config);
            updateStatus("Estado restaurado", "running");
            logDiagnostic("Emulador iniciado con snapshot restaurado.", "success");
            btnStart.disabled = true;
            btnPause.disabled = false;
            btnReset.disabled = false;
            btnSaveState.disabled = false;
        }
    };
    reader.readAsArrayBuffer(file);
});

// Load Custom ISO Trigger
btnCustomIsoTrigger.addEventListener("click", () => {
    inputCustomIso.click();
});

inputCustomIso.addEventListener("change", (e) => {
    const file = e.target.files[0];
    if (!file) return;

    updateStatus(`Cargando ${file.name}...`, "paused");
    logDiagnostic(`Cargando ISO local: ${file.name} (${(file.size / (1024 * 1024)).toFixed(1)} MB)...`, "info");
    const reader = new FileReader();
    reader.onload = function() {
        initEmulator(reader.result);
    };
    reader.readAsArrayBuffer(file);
});

// Drag and Drop ISO Support
screenWrapper.addEventListener("dragover", (e) => {
    e.preventDefault();
    screenWrapper.classList.add("dragover");
});

screenWrapper.addEventListener("dragleave", () => {
    screenWrapper.classList.remove("dragover");
});

screenWrapper.addEventListener("drop", (e) => {
    e.preventDefault();
    screenWrapper.classList.remove("dragover");

    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
        const file = e.dataTransfer.files[0];
        if (file.name.endsWith(".iso") || file.name.endsWith(".img")) {
            updateStatus(`Arrastrado: ${file.name}`, "paused");
            logDiagnostic(`ISO arrastrada: ${file.name} (${(file.size / (1024 * 1024)).toFixed(1)} MB)...`, "info");
            const reader = new FileReader();
            reader.onload = function() {
                initEmulator(reader.result);
            };
            reader.readAsArrayBuffer(file);
        } else if (file.name.endsWith(".bin")) {
            const reader = new FileReader();
            reader.onload = function() {
                if (emulator) {
                    emulator.restore_state(reader.result);
                    logDiagnostic("Snapshot arrastrado restaurado.", "success");
                }
            };
            reader.readAsArrayBuffer(file);
        }
    }
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

// Capture mouse on click & dismiss overlay hint
const hintOverlay = document.getElementById("hint_overlay");
const dismissHint = () => {
    if (hintOverlay) {
        hintOverlay.classList.add("fade-out");
        setTimeout(() => {
            if (hintOverlay.parentNode) hintOverlay.remove();
        }, 500);
    }
};

// Auto-hide hint after 4 seconds
setTimeout(dismissHint, 4000);

screenContainer.addEventListener("click", () => {
    dismissHint();
    if (emulator && emulator.lock_mouse) {
        emulator.lock_mouse();
    }
});

// ==========================================================================
// InputMapper & Mobile Retro Gamepad Controller
// ==========================================================================

const SCANCODES = {
    "ESC": { make: [0x01], break: [0x81] },
    "1": { make: [0x02], break: [0x82] },
    "2": { make: [0x03], break: [0x83] },
    "3": { make: [0x04], break: [0x84] },
    "4": { make: [0x05], break: [0x85] },
    "5": { make: [0x06], break: [0x86] },
    "TAB": { make: [0x0F], break: [0x8F] },
    "ENTER": { make: [0x1C], break: [0x9C] },
    "LCTRL": { make: [0x1D], break: [0x9D] },
    "LALT": { make: [0x38], break: [0xB8] },
    "LSHIFT": { make: [0x2A], break: [0xAA] },
    "SPACE": { make: [0x39], break: [0xB9] },
    "F1": { make: [0x3B], break: [0xBB] },
    "F2": { make: [0x3C], break: [0xBC] },
    "F3": { make: [0x3D], break: [0xBD] },
    "F4": { make: [0x3E], break: [0xBE] },
    "F5": { make: [0x3F], break: [0xBF] },
    "W": { make: [0x11], break: [0x91] },
    "A": { make: [0x1E], break: [0x9E] },
    "S": { make: [0x1F], break: [0x9F] },
    "D": { make: [0x20], break: [0xA0] },
    "E": { make: [0x12], break: [0x92] },
    "Q": { make: [0x10], break: [0x90] },
    "Y": { make: [0x15], break: [0x95] },
    "N": { make: [0x31], break: [0xB1] },
    // Extended Hardware Arrow Scancodes
    "ARROW_UP": { make: [0xE0, 0x48], break: [0xE0, 0xC8] },
    "ARROW_LEFT": { make: [0xE0, 0x4B], break: [0xE0, 0xCB] },
    "ARROW_RIGHT": { make: [0xE0, 0x4D], break: [0xE0, 0xCD] },
    "ARROW_DOWN": { make: [0xE0, 0x50], break: [0xE0, 0xD0] },
};

const CONTROL_PROFILES = [
    {
        id: "WASD",
        label: "🎮 WASD",
        dpad: { UP: "W", DOWN: "S", LEFT: "A", RIGHT: "D" },
        actions: { FIRE: "LCTRL", USE: "SPACE", RUN: "LSHIFT", ENTER: "ENTER" }
    },
    {
        id: "ARROWS",
        label: "🏹 Flechas",
        dpad: { UP: "ARROW_UP", DOWN: "ARROW_DOWN", LEFT: "ARROW_LEFT", RIGHT: "ARROW_RIGHT" },
        actions: { FIRE: "LCTRL", USE: "SPACE", RUN: "LSHIFT", ENTER: "ENTER" }
    },
    {
        id: "RPG",
        label: "🧙 Roguelike",
        dpad: { UP: "ARROW_UP", DOWN: "ARROW_DOWN", LEFT: "ARROW_LEFT", RIGHT: "ARROW_RIGHT" },
        actions: { FIRE: "ENTER", USE: "SPACE", RUN: "ESC", ENTER: "TAB" }
    }
];

let currentProfileIndex = 0;
const activePressedKeys = new Set();

function sendScancodeMake(keyName) {
    const sc = SCANCODES[keyName];
    if (sc && sc.make && emulator && isRunning && !isPaused) {
        emulator.keyboard_send_scancodes(sc.make);
        if (navigator.vibrate) {
            try { navigator.vibrate(20); } catch (e) {}
        }
    }
}

function sendScancodeBreak(keyName) {
    const sc = SCANCODES[keyName];
    if (sc && sc.break && emulator && isRunning && !isPaused) {
        emulator.keyboard_send_scancodes(sc.break);
    }
}

function handleInputPress(keyName, el) {
    if (!keyName || activePressedKeys.has(keyName)) return;
    activePressedKeys.add(keyName);
    if (el) el.classList.add("pressed");
    sendScancodeMake(keyName);
}

function handleInputRelease(keyName, el) {
    if (!keyName || !activePressedKeys.has(keyName)) return;
    activePressedKeys.delete(keyName);
    if (el) el.classList.remove("pressed");
    sendScancodeBreak(keyName);
}

function initInputMapper() {
    const gamepadOverlay = document.getElementById("gamepad_overlay");
    const btnToggleGamepad = document.getElementById("btn_toggle_gamepad");
    const btnToggleProfile = document.getElementById("btn_toggle_profile");
    const btnToggleFkeys = document.getElementById("btn_toggle_fkeys");
    const gamepadFkeysBar = document.getElementById("gamepad_fkeys_bar");

    if (!gamepadOverlay) return;

    // Toggle Gamepad visibility
    if (btnToggleGamepad) {
        btnToggleGamepad.addEventListener("click", () => {
            gamepadOverlay.classList.toggle("collapsed");
        });
    }

    // Toggle F-Keys visibility
    if (btnToggleFkeys && gamepadFkeysBar) {
        btnToggleFkeys.addEventListener("click", () => {
            const isHidden = gamepadFkeysBar.style.display === "none";
            gamepadFkeysBar.style.display = isHidden ? "flex" : "none";
            btnToggleFkeys.classList.toggle("active", isHidden);
        });
    }

    // Toggle Profile (WASD -> Flechas -> Roguelike)
    const updateProfileUI = () => {
        const cur = CONTROL_PROFILES[currentProfileIndex];
        if (btnToggleProfile) btnToggleProfile.textContent = cur.label;
    };

    if (btnToggleProfile) {
        btnToggleProfile.addEventListener("click", () => {
            currentProfileIndex = (currentProfileIndex + 1) % CONTROL_PROFILES.length;
            updateProfileUI();
            logDiagnostic(`Perfil de control activo: ${CONTROL_PROFILES[currentProfileIndex].label}`, "info");
        });
    }

    // Bind utility & F-key buttons (.pad-key-btn with data-key)
    document.querySelectorAll(".pad-key-btn[data-key]").forEach((btn) => {
        const keyName = btn.getAttribute("data-key");
        const onStart = (e) => { e.preventDefault(); e.stopPropagation(); handleInputPress(keyName, btn); };
        const onEnd = (e) => { e.preventDefault(); e.stopPropagation(); handleInputRelease(keyName, btn); };

        btn.addEventListener("touchstart", onStart, { passive: false });
        btn.addEventListener("touchend", onEnd, { passive: false });
        btn.addEventListener("touchcancel", onEnd, { passive: false });
        btn.addEventListener("mousedown", onStart);
        btn.addEventListener("mouseup", onEnd);
        btn.addEventListener("mouseleave", onEnd);
    });

    // Bind D-Pad directional buttons
    document.querySelectorAll(".dpad-btn[data-dpad]").forEach((btn) => {
        const dir = btn.getAttribute("data-dpad");
        const getMappedKey = () => CONTROL_PROFILES[currentProfileIndex].dpad[dir];

        const onStart = (e) => {
            e.preventDefault();
            e.stopPropagation();
            const key = getMappedKey();
            handleInputPress(key, btn);
        };
        const onEnd = (e) => {
            e.preventDefault();
            e.stopPropagation();
            const key = getMappedKey();
            handleInputRelease(key, btn);
        };

        btn.addEventListener("touchstart", onStart, { passive: false });
        btn.addEventListener("touchend", onEnd, { passive: false });
        btn.addEventListener("touchcancel", onEnd, { passive: false });
        btn.addEventListener("mousedown", onStart);
        btn.addEventListener("mouseup", onEnd);
        btn.addEventListener("mouseleave", onEnd);
    });

    // Bind Action buttons (FIRE, USE, RUN, ENTER)
    document.querySelectorAll(".action-btn[data-action]").forEach((btn) => {
        const actionType = btn.getAttribute("data-action");
        const getMappedKey = () => CONTROL_PROFILES[currentProfileIndex].actions[actionType] || actionType;

        const onStart = (e) => {
            e.preventDefault();
            e.stopPropagation();
            const key = getMappedKey();
            handleInputPress(key, btn);
        };
        const onEnd = (e) => {
            e.preventDefault();
            e.stopPropagation();
            const key = getMappedKey();
            handleInputRelease(key, btn);
        };

        btn.addEventListener("touchstart", onStart, { passive: false });
        btn.addEventListener("touchend", onEnd, { passive: false });
        btn.addEventListener("touchcancel", onEnd, { passive: false });
        btn.addEventListener("mousedown", onStart);
        btn.addEventListener("mouseup", onEnd);
        btn.addEventListener("mouseleave", onEnd);
    });
}

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

// AudioContext Gesture Unlocker (Chrome/Firefox/Edge Autoplay policy)
function unlockAudio() {
    try {
        const AudioCtx = window.AudioContext || window.webkitAudioContext;
        if (AudioCtx) {
            const ctx = new AudioCtx();
            if (ctx.state === 'suspended') {
                ctx.resume();
            }
        }
    } catch (e) {}
    document.removeEventListener("click", unlockAudio);
    document.removeEventListener("keydown", unlockAudio);
}
document.addEventListener("click", unlockAudio, { once: true });
document.addEventListener("keydown", unlockAudio, { once: true });

// Auto-start emulator on load & initialize mobile controls
window.addEventListener("DOMContentLoaded", () => {
    logDiagnostic("Página cargada. Auto-iniciando emulador...", "info");
    initInputMapper();
    initEmulator();
});
