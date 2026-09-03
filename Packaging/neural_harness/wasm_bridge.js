// NIGHTMARE Neural HD — WebAssembly to WebGPU Memory Bridge
// Performs direct streaming from WASM Linear Heap to WebGPU Textures with Zero Extra Allocations

class NightmareWASMBridge {
    constructor(webgpuHarness) {
        this.harness = webgpuHarness;
        this.wasmModule = null;
        this.isConnected = false;
        this.width = 640;
        this.height = 480;

        // Cached TypedArray views directly into WASM Linear Memory
        this.viewRGB = null;
        this.viewDepth = null;
        this.viewLight = null;
        this.viewSemantic = null;
    }

    connect(emscriptenModule) {
        if (!emscriptenModule || !emscriptenModule.HEAPU8) {
            console.warn("[NIGHTMARE Bridge] Emscripten module not ready or missing HEAP.");
            return false;
        }

        this.wasmModule = emscriptenModule;
        this.isConnected = true;
        console.log("[NIGHTMARE Bridge] Connected to DevilutionX WASM runtime.");
        return true;
    }

    updatePointers() {
        if (!this.isConnected) return false;

        const getRGBPtr = this.wasmModule._Nightmare_GBuffer_GetRGB;
        const getDepthPtr = this.wasmModule._Nightmare_GBuffer_GetDepth;
        const getLightPtr = this.wasmModule._Nightmare_GBuffer_GetLight;
        const getSemanticPtr = this.wasmModule._Nightmare_GBuffer_GetSemantic;

        if (!getRGBPtr || !getDepthPtr || !getLightPtr || !getSemanticPtr) {
            return false;
        }

        const ptrRGB = getRGBPtr();
        const ptrDepth = getDepthPtr();
        const ptrLight = getLightPtr();
        const ptrSemantic = getSemanticPtr();

        const numPixels = this.width * this.height;
        const heap = this.wasmModule.HEAPU8.buffer;

        // Recreate typed array views only if heap grew or pointers changed
        this.viewRGB = new Uint8Array(heap, ptrRGB, numPixels * 4);
        this.viewDepth = new Float32Array(heap, ptrDepth, numPixels);
        this.viewLight = new Uint8Array(heap, ptrLight, numPixels);
        this.viewSemantic = new Uint8Array(heap, ptrSemantic, numPixels);

        return true;
    }

    syncFrameToWebGPU() {
        if (!this.isConnected || !this.harness.device) return;

        if (!this.viewRGB) {
            if (!this.updatePointers()) return;
        }

        const queue = this.harness.device.queue;

        // Zero-copy streaming straight to GPU Texture storage
        queue.writeTexture(
            { texture: this.harness.texRGB },
            this.viewRGB,
            { bytesPerRow: this.width * 4 },
            [this.width, this.height]
        );

        queue.writeTexture(
            { texture: this.harness.texDepth },
            this.viewDepth,
            { bytesPerRow: this.width * 4 },
            [this.width, this.height]
        );

        queue.writeTexture(
            { texture: this.harness.texLight },
            this.viewLight,
            { bytesPerRow: this.width },
            [this.width, this.height]
        );

        queue.writeTexture(
            { texture: this.harness.texSemantic },
            this.viewSemantic,
            { bytesPerRow: this.width },
            [this.width, this.height]
        );
    }
}

// Export for harness
window.NightmareWASMBridge = NightmareWASMBridge;
