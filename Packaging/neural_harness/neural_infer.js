// NIGHTMARE Neural HD — Phase 5 In-Browser WebGPU Neural Inference Engine v2
// Multi-Scale ONNX Inference (2x: 960p, 3x: 1440p, 4x: 1920p/4K) with Silhouette Lock 2.0
// and Multi-Tier Dynamic Degradation Fallback (EXTREME_4X -> ULTRA_3X -> HD_2X -> SHADERS -> RETRO)

class NightmareNeuralInference {
    constructor(harness) {
        this.harness = harness;
        this.sessions = {}; // Cache of loaded ONNX sessions by scale
        this.currentScale = 2; // Default scale: 2x (HD), 3x (Ultra), 4x (Extreme)
        this.isLoaded = false;
        this.isInferencing = false;
        this.width = 640;
        this.height = 480;

        // Dynamic Degradation System
        this.frameTimeHistory = [];
        this.highLoadCounter = 0;
        this.lowLoadCounter = 0;
        this.currentQualityLevel = 'HD_2X'; // 'EXTREME_4X', 'ULTRA_3X', 'HD_2X', 'MEDIUM' (Shaders), 'LOW' (1996)
        this.targetQualityLevel = 'ULTRA_3X'; // Preferred quality requested by user
        this.onQualityChangeCallback = null;

        // Reusable input tensor buffer (1 x 6 x 480 x 640)
        this.inputBuffer = new Float32Array(1 * 6 * this.height * this.width);
    }

    async init(initialScale = 2) {
        this.currentScale = initialScale;
        return await this.loadModelForScale(this.currentScale);
    }

    async loadModelForScale(scale) {
        if (typeof ort === 'undefined') {
            console.warn("[NIGHTMARE Neural v2] ONNX Runtime Web not found. Running in WebGPU Shader mode.");
            return false;
        }

        if (this.sessions[scale]) {
            this.currentScale = scale;
            this.isLoaded = true;
            return true;
        }

        const modelPaths = [
            `models/nightmare_neural_${scale}x.onnx`,
            `models/nightmare_neural_hd.onnx` // Fallback for 2x
        ];

        for (const modelPath of modelPaths) {
            try {
                console.log(`[NIGHTMARE Neural v2] Attempting to load ONNX Model (${scale}x) from ${modelPath}...`);
                const options = {
                    executionProviders: ['webgpu', 'wasm'],
                    graphOptimizationLevel: 'all',
                };

                const session = await ort.InferenceSession.create(modelPath, options);
                this.sessions[scale] = session;
                this.currentScale = scale;
                this.isLoaded = true;
                console.log(`[NIGHTMARE Neural v2] Model ${scale}x successfully loaded on WebGPU execution provider!`);
                return true;
            } catch (err) {
                console.warn(`[NIGHTMARE Neural v2] Could not load ${modelPath}: ${err.message}`);
            }
        }

        console.warn(`[NIGHTMARE Neural v2] Model ${scale}x not available. WebGPU procedural shaders will be used as high-fidelity fallback.`);
        return false;
    }

    async setScale(scale) {
        if (scale === this.currentScale && this.sessions[scale]) return true;
        console.log(`[NIGHTMARE Neural v2] Switching scale to ${scale}x...`);
        const ok = await this.loadModelForScale(scale);
        if (ok) {
            this.currentScale = scale;
            if (scale === 4) this.currentQualityLevel = 'EXTREME_4X';
            else if (scale === 3) this.currentQualityLevel = 'ULTRA_3X';
            else this.currentQualityLevel = 'HD_2X';
            this.targetQualityLevel = this.currentQualityLevel;
            if (this.onQualityChangeCallback) {
                this.onQualityChangeCallback(this.currentQualityLevel, `Switched scale to ${scale}x`);
            }
        }
        return ok;
    }

    async runInference(rgbView, depthView, lightView, semanticView) {
        const session = this.sessions[this.currentScale];
        if (!this.isLoaded || !session || this.isInferencing) {
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
            const results = await session.run(feeds);

            // Handle output names for 2x, 3x, 4x or legacy
            const outputTensor = results[`enhanced_${this.currentScale}x_output`] || 
                                 results.enhanced_2x_output || 
                                 Object.values(results)[0];

            this.isInferencing = false;
            return outputTensor; // Shape: [1, 3, 480*scale, 640*scale]
        } catch (err) {
            console.error(`[NIGHTMARE Neural v2] Inference pass failed at ${this.currentScale}x:`, err);
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

        // Progressive degradation ladder when under heavy GPU load (> 20ms = < 50 FPS)
        if (avgFrametime > 20.0) {
            this.highLoadCounter++;
            this.lowLoadCounter = 0;

            if (this.highLoadCounter > 45) {
                if (this.currentQualityLevel === 'EXTREME_4X') {
                    this.setScale(3);
                    this.setQualityLevel('ULTRA_3X', 'Frametime > 20ms: Auto-Degrading from 4x to 3x');
                } else if (this.currentQualityLevel === 'ULTRA_3X') {
                    this.setScale(2);
                    this.setQualityLevel('HD_2X', 'Frametime > 20ms: Auto-Degrading from 3x to 2x');
                } else if (this.currentQualityLevel === 'HD_2X') {
                    this.setQualityLevel('MEDIUM', 'Frametime > 20ms: Auto-Degrading to WebGPU Shaders');
                } else if (this.currentQualityLevel === 'MEDIUM') {
                    this.setQualityLevel('LOW', 'Frametime > 25ms: Auto-Degrading to Original 1996');
                }
                this.highLoadCounter = 0;
            }
        } else if (avgFrametime < 13.0) {
            // Restore towards target quality when GPU headroom is ample (< 13ms = > 75 FPS)
            this.lowLoadCounter++;
            this.highLoadCounter = 0;

            if (this.lowLoadCounter > 100) {
                if (this.currentQualityLevel === 'LOW') {
                    this.setQualityLevel('MEDIUM', 'Performance stabilized: Restoring WebGPU Shaders');
                } else if (this.currentQualityLevel === 'MEDIUM') {
                    this.setScale(2);
                    this.setQualityLevel('HD_2X', 'Performance optimal: Restoring Neural HD 2x');
                } else if (this.currentQualityLevel === 'HD_2X' && (this.targetQualityLevel === 'ULTRA_3X' || this.targetQualityLevel === 'EXTREME_4X')) {
                    this.setScale(3);
                    this.setQualityLevel('ULTRA_3X', 'High performance headroom: Restoring Ultra HD 3x');
                } else if (this.currentQualityLevel === 'ULTRA_3X' && this.targetQualityLevel === 'EXTREME_4X') {
                    this.setScale(4);
                    this.setQualityLevel('EXTREME_4X', 'Maximum performance headroom: Restoring Extreme 4x');
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
