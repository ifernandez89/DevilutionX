/**
 * Mini Windows XP — WebAssembly Runtime Controller
 */

let emulator = null;
let isRunning = false;
let isPaused = false;

// DOM Elements
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

function updateStatus(text, stateClass) {
    statusText.textContent = text;
    statusDot.className = "status-dot " + (stateClass || "");
}

function initEmulator(customCdromBuffer = null) {
    if (emulator) {
        emulator.destroy();
        emulator = null;
    }

    updateStatus("Cargando BIOS y WebAssembly...", "paused");

    const config = {
        wasm_path: "v86.wasm",
        memory_size: 256 * 1024 * 1024,      // 256 MB RAM
        vga_memory_size: 8 * 1024 * 1024,    // 8 MB VRAM
        screen_container: screenContainer,
        bios: { url: "bios/seabios.bin" },
        vga_bios: { url: "bios/vgabios.bin" },
        autostart: true,
        network_relay_url: null,
    };

    if (customCdromBuffer) {
        config.cdrom = { buffer: customCdromBuffer };
    } else {
        config.cdrom = { url: "minixp.iso" };
    }

    try {
        emulator = new V86Starter(config);

        emulator.add_listener("emulator-ready", function() {
            isRunning = true;
            isPaused = false;
            updateStatus("Ejecutando MiniXP", "running");
            btnStart.disabled = true;
            btnPause.disabled = false;
            btnReset.disabled = false;
            btnSaveState.disabled = false;
        });

        emulator.add_listener("emulator-stopped", function() {
            isRunning = false;
            isPaused = false;
            updateStatus("Emulador Detenido", "");
            btnStart.disabled = false;
            btnPause.disabled = true;
            btnReset.disabled = true;
            btnSaveState.disabled = true;
        });

    } catch (err) {
        console.error("Error al iniciar emulador v86:", err);
        updateStatus("Error: " + err.message, "");
    }
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
        updateStatus("Ejecutando MiniXP", "running");
    } else {
        emulator.stop();
        isPaused = true;
        btnPause.innerHTML = '<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg> Reanudar';
        updateStatus("Emulación Pausada", "paused");
    }
});

// Reset Button
btnReset.addEventListener("click", () => {
    if (!emulator) return;
    if (confirm("¿Deseas reiniciar la máquina virtual?")) {
        emulator.restart();
        updateStatus("Reiniciando...", "paused");
    }
});

// Save State (Snapshot to File)
btnSaveState.addEventListener("click", () => {
    if (!emulator || !isRunning) return;

    updateStatus("Generando Snapshot...", "paused");
    emulator.save_state(function(error, state_data) {
        if (error) {
            alert("Error al guardar estado: " + error.message);
            updateStatus("Ejecutando MiniXP", "running");
            return;
        }

        const blob = new Blob([state_data], { type: "application/octet-stream" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = `minixp_state_${Date.now()}.bin`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        updateStatus("Snapshot Guardado exitosamente", "running");
    });
});

// Load State Trigger
btnLoadStateTrigger.addEventListener("click", () => {
    inputLoadState.click();
});

inputLoadState.addEventListener("change", (e) => {
    const file = e.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = function() {
        if (emulator) {
            emulator.restore_state(reader.result);
            updateStatus("Estado restaurado", "running");
        } else {
            // Iniciar emulador y restaurar estado
            const config = {
                wasm_path: "v86.wasm",
                memory_size: 256 * 1024 * 1024,
                vga_memory_size: 8 * 1024 * 1024,
                screen_container: screenContainer,
                bios: { url: "bios/seabios.bin" },
                vga_bios: { url: "bios/vgabios.bin" },
                initial_state: { buffer: reader.result },
                autostart: true,
            };
            emulator = new V86Starter(config);
            updateStatus("Estado restaurado", "running");
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
