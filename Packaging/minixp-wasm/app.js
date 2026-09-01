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
    const osTitle = isTinyCore ? "Tiny Core Linux" : "MiniXP";

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

// Auto-start emulator on load
window.addEventListener("DOMContentLoaded", () => {
    logDiagnostic("Página cargada. Auto-iniciando emulador...", "info");
    initEmulator();
});
