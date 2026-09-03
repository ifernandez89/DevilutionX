// NIGHTMARE Neural HD — Phase 5 In-Browser WebGPU Neural Inference Engine
// Uses onnxruntime-web (WebGPU Execution Provider) with Dynamic Degradation Fallback

class NightmareNeuralInference {
    constructor(harness) {
        this.harness = harness;
        this.session = null;
        this.isLoaded = false;
        this.isInferencing = false;
        this.width = 640;
        this.height = 480;

        // Dynamic Degradation System
        this.frameTimeHistory = [];
        this.highLoadCounter = 0;
        this.lowLoadCounter = 0;
        this.currentQualityLevel = 'HIGH'; // 'HIGH' (Neural 2x), 'MEDIUM' (WebGPU Shaders 1x), 'LOW' (Original 1996)
        this.onQualityChangeCallback = null;

        // Reusable input tensor buffer (1 x 6 x 480 x 640)
        this.inputBuffer = new Float32Array(1 * 6 * this.height * this.width);
    }

    async init(modelPath = 'models/nightmare_neural_hd.onnx') {
        if (typeof ort === 'undefined') {
            console.warn("[NIGHTMARE Neural] ONNX Runtime Web not found. Running in WebGPU Shader mode.");
            return false;
        }

        try {
            console.log(`[NIGHTMARE Neural] Loading Neural Model from ${modelPath}...`);
            
            // Configure ONNX Runtime to use WebGPU Execution Provider with fallback to WASM
            const options = {
                executionProviders: ['webgpu', 'wasm'],
                graphOptimizationLevel: 'all',
            };

            this.session = await ort.InferenceSession.create(modelPath, options);
            this.isLoaded = true;
            console.log("[NIGHTMARE Neural] Model successfully loaded on WebGPU execution provider!");
            return true;
        } catch (err) {
            console.warn("[NIGHTMARE Neural] Could not load ONNX model directly (will use high-quality WebGPU Shader fallback):", err.message);
            return false;
        }
    }

    async runInference(rgbView, depthView, lightView, semanticView) {
        if (!this.isLoaded || !this.session || this.isInferencing) {
            return null;
        }

        this.isInferencing = true;
        const numPixels = this.width * this.height;

        // Pack 6 input channels into NCHW tensor layout: [1, 6, 480, 640]
        const channelOffset = numPixels;
        for (let i = 0; i < numPixels; i++) {
            // RGB (Channels 0, 1, 2)
            this.inputBuffer[i] = rgbView[i * 4 + 0] / 255.0;
            this.inputBuffer[channelOffset + i] = rgbView[i * 4 + 1] / 255.0;
            this.inputBuffer[channelOffset * 2 + i] = rgbView[i * 4 + 2] / 255.0;

            // Logical Depth (Channel 3)
            this.inputBuffer[channelOffset * 3 + i] = depthView[i];

            // Lightmap (Channel 4)
            this.inputBuffer[channelOffset * 4 + i] = lightView[i] / 255.0;

            // Semantic Class ID (Channel 5)
            this.inputBuffer[channelOffset * 5 + i] = semanticView[i] / 255.0;
        }

        try {
            const inputTensor = new ort.Tensor('float32', this.inputBuffer, [1, 6, this.height, this.width]);
            const feeds = { gbuffer_input: inputTensor };
            const results = await this.session.run(feeds);
            const outputTensor = results.enhanced_2x_output;

            this.isInferencing = false;
            return outputTensor; // Shape: [1, 3, 960, 1280]
        } catch (err) {
            console.error("[NIGHTMARE Neural] Inference pass failed:", err);
            this.isInferencing = false;
            return null;
        }
    }

    updateDynamicDegradation(frametimeMs) {
        this.frameTimeHistory.push(frametimeMs);
        if (this.frameTimeHistory.length > 30) {
            this.frameTimeHistory.shift();
        }

        const avgFrametime = this.frameTimeHistory.reduce((a, b) => a + b, 0) / this.frameTimeHistory.length;

        // If frametime exceeds 20.0ms continuously (> 60 frames under heavy load)
        if (avgFrametime > 20.0) {
            this.highLoadCounter++;
            this.lowLoadCounter = 0;

            if (this.highLoadCounter > 60) {
                if (this.currentQualityLevel === 'HIGH') {
                    this.setQualityLevel('MEDIUM', 'Frametime > 20ms: Auto-Degrading to WebGPU Shaders');
                } else if (this.currentQualityLevel === 'MEDIUM') {
                    this.setQualityLevel('LOW', 'Frametime > 25ms: Auto-Degrading to Original 1996');
                }
                this.highLoadCounter = 0;
            }
        } else if (avgFrametime < 14.0) {
            // If system has plenty of headroom (< 14.0ms), restore high quality
            this.lowLoadCounter++;
            this.highLoadCounter = 0;

            if (this.lowLoadCounter > 120) {
                if (this.currentQualityLevel === 'LOW') {
                    this.setQualityLevel('MEDIUM', 'Performance stabilized: Restoring WebGPU Shaders');
                } else if (this.currentQualityLevel === 'MEDIUM' && this.isLoaded) {
                    this.setQualityLevel('HIGH', 'Performance optimal: Restoring Neural HD 2x');
                }
                this.lowLoadCounter = 0;
            }
        }
    }

    setQualityLevel(level, reason = '') {
        this.currentQualityLevel = level;
        console.log(`[NIGHTMARE Dynamic Fallback] Quality switched to ${level}. ${reason}`);
        if (this.onQualityChangeCallback) {
            this.onQualityChangeCallback(level, reason);
        }
    }
}

// Export for WebGPU Harness
window.NightmareNeuralInference = NightmareNeuralInference;
