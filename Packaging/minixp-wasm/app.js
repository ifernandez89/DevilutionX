/**
 * Retro Virtual PC & MiniXP — WebAssembly Runtime Controller
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
const btnToggleTouch = document.getElementById("btn_toggle_touch");
const touchGamepad = document.getElementById("touch_gamepad");
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

        let serialBuffer = "";
        function handleSerialChar(c) {
            if (c === "\n") {
                if (serialBuffer.trim().length > 0) {
                    logDiagnostic(`[TTY/COM1] ${serialBuffer}`, "info");
                }
                serialBuffer = "";
            } else if (c !== "\r") {
                serialBuffer += c;
            }
        }

        emulator.add_listener("serial0-output-char", function(char) {
            handleSerialChar(typeof char === "string" ? char : String.fromCharCode(char));
        });

        emulator.add_listener("ide-read-start", function() {
            triggerDiskActivity(16);
        });

        emulator.add_listener("ide-read-end", function() {
            triggerDiskActivity(16);
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

// Touch Gamepad Toggle
if (btnToggleTouch && touchGamepad) {
    btnToggleTouch.addEventListener("click", () => {
        const isHidden = touchGamepad.style.display === "none";
        touchGamepad.style.display = isHidden ? "flex" : "none";
        btnToggleTouch.classList.toggle("btn-active", isHidden);
        logDiagnostic(`Controles táctiles ${isHidden ? 'activados' : 'desactivados'}.`, "info");
    });
}

// Dispatch Synthetic Keyboard Events to v86
function triggerKey(key, type) {
    const event = new KeyboardEvent(type, {
        key: key,
        code: key,
        bubbles: true,
        cancelable: true
    });
    window.dispatchEvent(event);
}

// Wire Touch Buttons
document.querySelectorAll(".touch-gamepad button").forEach(btn => {
    const key = btn.getAttribute("data-key");
    if (!key) return;

    btn.addEventListener("touchstart", (e) => {
        e.preventDefault();
        btn.classList.add("pressed");
        triggerKey(key, "keydown");
    });

    btn.addEventListener("touchend", (e) => {
        e.preventDefault();
        btn.classList.remove("pressed");
        triggerKey(key, "keyup");
    });

    btn.addEventListener("mousedown", (e) => {
        btn.classList.add("pressed");
        triggerKey(key, "keydown");
    });

    btn.addEventListener("mouseup", (e) => {
        btn.classList.remove("pressed");
        triggerKey(key, "keyup");
    });
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
