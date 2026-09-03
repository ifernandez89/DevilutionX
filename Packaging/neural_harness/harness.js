// NIGHTMARE Neural HD — WebGPU Harness Controller & Diagnostics (Fases 0 a 6)

class NightmareHarness {
    constructor() {
        this.canvas = document.getElementById('webgpu-canvas');
        this.width = 640;
        this.height = 480;
        this.device = null;
        this.context = null;
        this.pipeline = null;
        this.uniformBuffer = null;
        this.bindGroup = null;

        // Textures
        this.texRGB = null;
        this.texDepth = null;
        this.texLight = null;
        this.texSemantic = null;
        this.sampler = null;

        // Shader parameters & Biomes
        this.state = {
            renderMode: 2, // 0: Original, 1: Enhanced, 2: Split A/B, 3: Depth, 4: Light, 5: Semantic, 6: Normals, 7: Neural HD 2x
            splitPos: 0.5,
            lightIntensity: 1.4,
            bonfireFlicker: 1.0,
            contactShadowStrength: 1.0,
            waterSpecular: 1.2,
            dungeonBiome: 0, // 0: Town, 1: Cathedral, 2: Catacombs, 3: Caves, 4: Hell, 5: Crypt, 6: Hive
            bonfirePos: [320, 240],
        };

        // Neural Inference Engine (Phase 5)
        this.neuralEngine = new NightmareNeuralInference(this);

        // Frame Pacing & Performance Metrics
        this.frameTimes = [];
        this.lastFrameTime = performance.now();
        this.fpsEl = document.getElementById('metric-fps');
        this.frameTimeEl = document.getElementById('metric-frametime');
        this.jitterEl = document.getElementById('metric-jitter');
        this.statusBadgeEl = document.getElementById('status-badge');
        this.qualityBadgeEl = document.getElementById('metric-quality');

        this.isDraggingSplit = false;
    }

    async init() {
        if (!navigator.gpu) {
            this.showError("WebGPU is not supported in this browser. Please use a WebGPU-enabled browser (Chrome/Edge with WebGPU enabled).");
            return false;
        }

        try {
            const adapter = await navigator.gpu.requestAdapter();
            if (!adapter) {
                this.showError("Failed to find a compatible WebGPU adapter.");
                return false;
            }

            this.device = await adapter.requestDevice();
            this.context = this.canvas.getContext('webgpu');
            const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

            this.context.configure({
                device: this.device,
                format: presentationFormat,
                alphaMode: 'opaque'
            });

            // Load WGSL shader
            const shaderResp = await fetch('shaders/tristram_enhancer.wgsl');
            const shaderCode = await shaderResp.text();

            const shaderModule = this.device.createShaderModule({
                code: shaderCode
            });

            // Create sampler
            this.sampler = this.device.createSampler({
                magFilter: 'nearest',
                minFilter: 'nearest',
            });

            // Create Uniform Buffer (64 bytes aligned)
            this.uniformBuffer = this.device.createBuffer({
                size: 64,
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
            });

            // Initialize G-Buffer Textures with Sample Scene
            this.initGBuffers();

            // Create Pipeline Layout and Pipeline
            this.pipeline = this.device.createRenderPipeline({
                layout: 'auto',
                vertex: {
                    module: shaderModule,
                    entryPoint: 'vs_main',
                },
                fragment: {
                    module: shaderModule,
                    entryPoint: 'fs_main',
                    targets: [{
                        format: presentationFormat,
                    }]
                },
                primitive: {
                    topology: 'triangle-list',
                }
            });

            this.updateBindGroup();
            this.setupUI();

            // Init Neural Engine (Non-blocking)
            this.neuralEngine.init();
            this.neuralEngine.onQualityChangeCallback = (level, reason) => {
                if (this.qualityBadgeEl) {
                    this.qualityBadgeEl.textContent = level;
                }
            };

            this.startRenderLoop();

            this.statusBadgeEl.textContent = "WebGPU Active (60 FPS)";
            this.statusBadgeEl.className = "status-badge active";
            return true;
        } catch (err) {
            console.error("WebGPU Init Error:", err);
            this.showError(`WebGPU Initialization Error: ${err.message}`);
            return false;
        }
    }

    initGBuffers() {
        const desc = (format) => ({
            size: [this.width, this.height],
            format: format,
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST,
        });

        this.texRGB = this.device.createTexture(desc('rgba8unorm'));
        this.texDepth = this.device.createTexture(desc('r32float'));
        this.texLight = this.device.createTexture(desc('r8unorm'));
        this.texSemantic = this.device.createTexture(desc('r8unorm'));

        this.generateBiomeGBuffers(this.state.dungeonBiome);
    }

    generateBiomeGBuffers(biome) {
        const numPixels = this.width * this.height;
        const rgbData = new Uint8Array(numPixels * 4);
        const depthData = new Float32Array(numPixels);
        const lightData = new Uint8Array(numPixels);
        const semanticData = new Uint8Array(numPixels);

        for (let y = 0; y < this.height; y++) {
            for (let x = 0; x < this.width; x++) {
                const idx = y * this.width + x;
                const idx4 = idx * 4;
                const isoZ = ((x + y * 1.2) / (this.width + this.height)) * 0.7 + 0.15;
                depthData[idx] = isoZ;

                let r = 38, g = 48, b = 28;
                let sem = 1; // Floor
                let light = 110;

                if (biome === 0) {
                    // Town (Tristram)
                    r = 38 + ((x ^ y) & 7) * 2;
                    g = 48 + ((x ^ y) & 15) * 3;
                    b = 28 + ((x + y) & 7) * 2;
                    if (x > 530 + Math.sin(y * 0.05) * 15) {
                        r = 18; g = 35; b = 65; sem = 5; light = 90; // River
                    }
                } else if (biome === 1) {
                    // Cathedral
                    r = 45; g = 42; b = 48; light = 130;
                } else if (biome === 2) {
                    // Catacombs (Dark stone + Blood pool)
                    r = 30; g = 28; b = 32; light = 80;
                    if (x > 450 && y > 300) { r = 90; g = 10; b = 15; sem = 5; } // Blood
                } else if (biome === 3 || biome === 4) {
                    // Caves / Hell (Lava pool)
                    r = 40; g = 25; b = 20; light = 140;
                    if (x > 480 || y > 380) { r = 240; g = 80; b = 10; sem = 5; light = 255; } // Molten Lava
                } else if (biome === 5) {
                    // Crypt
                    r = 55; g = 58; b = 62; light = 100;
                } else if (biome === 6) {
                    // Hive
                    r = 35; g = 50; b = 25; light = 110;
                }

                // Walls & Columns
                if (x >= 80 && x <= 240 && y >= 60 && y <= 150) {
                    if (x <= 90 || x >= 230 || y <= 70 || y >= 140) {
                        r = 85; g = 75; b = 65;
                        sem = 2; // Wall
                        depthData[idx] = isoZ - 0.12;
                    }
                }

                // Monsters (Around x: 200, y: 320)
                if (biome > 0) {
                    const distToMonster = Math.hypot(x - 200, y - 320);
                    if (distToMonster < 20) {
                        r = 140; g = 30; b = 30; sem = 7; depthData[idx] = isoZ - 0.09;
                    }
                }

                // Player Hero (around center x: 350, y: 260)
                const distToHero = Math.hypot(x - 350, y - 260);
                if (distToHero < 22) {
                    r = 80; g = 120; b = 180;
                    sem = 3;
                    depthData[idx] = isoZ - 0.1;
                }

                // Light Source
                const distToFire = Math.hypot(x - 320, y - 240);
                if (distToFire < 12) {
                    r = 255; g = 180; b = 50;
                    sem = 6;
                    light = 255;
                } else {
                    const fireLight = Math.max(0, 255 - distToFire * 1.1);
                    light = Math.min(255, light + Math.floor(fireLight * 0.6));
                }

                rgbData[idx4 + 0] = Math.min(255, r);
                rgbData[idx4 + 1] = Math.min(255, g);
                rgbData[idx4 + 2] = Math.min(255, b);
                rgbData[idx4 + 3] = 255;

                lightData[idx] = light;
                semanticData[idx] = sem;
            }
        }

        this.device.queue.writeTexture({ texture: this.texRGB }, rgbData, { bytesPerRow: this.width * 4 }, [this.width, this.height]);
        this.device.queue.writeTexture({ texture: this.texDepth }, depthData, { bytesPerRow: this.width * 4 }, [this.width, this.height]);
        this.device.queue.writeTexture({ texture: this.texLight }, lightData, { bytesPerRow: this.width }, [this.width, this.height]);
        this.device.queue.writeTexture({ texture: this.texSemantic }, semanticData, { bytesPerRow: this.width }, [this.width, this.height]);
    }

    updateBindGroup() {
        this.bindGroup = this.device.createBindGroup({
            layout: this.pipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: { buffer: this.uniformBuffer } },
                { binding: 1, resource: this.sampler },
                { binding: 2, resource: this.texRGB.createView() },
                { binding: 3, resource: this.texDepth.createView() },
                { binding: 4, resource: this.texLight.createView() },
                { binding: 5, resource: this.texSemantic.createView() },
            ]
        });
    }

    updateUniforms(timeSec) {
        const uData = new ArrayBuffer(64);
        const uViewU32 = new Uint32Array(uData);
        const uViewF32 = new Float32Array(uData);

        uViewU32[0] = this.state.renderMode;
        uViewF32[1] = this.state.splitPos;
        uViewF32[2] = this.state.lightIntensity;
        uViewF32[3] = this.state.bonfireFlicker;
        uViewF32[4] = this.state.contactShadowStrength;
        uViewF32[5] = this.state.waterSpecular;
        uViewF32[6] = timeSec;
        uViewU32[7] = this.state.dungeonBiome;
        uViewF32[8] = this.width;
        uViewF32[9] = this.height;
        uViewF32[10] = this.state.bonfirePos[0];
        uViewF32[11] = this.state.bonfirePos[1];

        this.device.queue.writeBuffer(this.uniformBuffer, 0, uData);
    }

    render(now) {
        const timeSec = now * 0.001;
        const deltaMs = now - this.lastFrameTime;
        this.lastFrameTime = now;

        this.updateMetrics(deltaMs);
        this.updateUniforms(timeSec);

        const commandEncoder = this.device.createCommandEncoder();
        const textureView = this.context.getCurrentTexture().createView();

        const renderPass = commandEncoder.beginRenderPass({
            colorAttachments: [{
                view: textureView,
                clearValue: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
                loadOp: 'clear',
                storeOp: 'store',
            }]
        });

        renderPass.setPipeline(this.pipeline);
        renderPass.setBindGroup(0, this.bindGroup);
        renderPass.draw(3, 1, 0, 0);
        renderPass.end();

        this.device.queue.submit([commandEncoder.finish()]);

        requestAnimationFrame((t) => this.render(t));
    }

    updateMetrics(deltaMs) {
        this.frameTimes.push(deltaMs);
        if (this.frameTimes.length > 60) {
            this.frameTimes.shift();
        }

        const avgDelta = this.frameTimes.reduce((a, b) => a + b, 0) / this.frameTimes.length;
        const fps = 1000.0 / Math.max(avgDelta, 0.001);

        const variance = this.frameTimes.reduce((acc, dt) => acc + Math.pow(dt - avgDelta, 2), 0) / this.frameTimes.length;
        const jitter = Math.sqrt(variance);

        this.fpsEl.textContent = `${fps.toFixed(1)} FPS`;
        this.frameTimeEl.textContent = `${avgDelta.toFixed(2)} ms`;
        this.jitterEl.textContent = `Δ ${jitter.toFixed(2)} ms`;

        if (jitter < 2.0 && avgDelta < 18.0) {
            this.jitterEl.style.color = '#4ade80';
        } else if (jitter < 4.0) {
            this.jitterEl.style.color = '#facc15';
        } else {
            this.jitterEl.style.color = '#f87171';
        }

        // Dynamic Degradation check
        if (this.neuralEngine) {
            this.neuralEngine.updateDynamicDegradation(avgDelta);
        }
    }

    startRenderLoop() {
        requestAnimationFrame((t) => this.render(t));
    }

    setupUI() {
        // Mode Selector Buttons
        document.querySelectorAll('.mode-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.mode-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                this.state.renderMode = parseInt(btn.dataset.mode, 10);
            });
        });

        // Biome Selector Buttons
        document.querySelectorAll('.biome-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.biome-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                this.state.dungeonBiome = parseInt(btn.dataset.biome, 10);
                this.generateBiomeGBuffers(this.state.dungeonBiome);
            });
        });

        // Sliders
        const bindSlider = (id, prop, scale = 1.0) => {
            const el = document.getElementById(id);
            if (el) {
                el.addEventListener('input', (e) => {
                    this.state[prop] = parseFloat(e.target.value) * scale;
                });
            }
        };

        bindSlider('slider-split', 'splitPos');
        bindSlider('slider-light', 'lightIntensity');
        bindSlider('slider-flicker', 'bonfireFlicker');
        bindSlider('slider-shadow', 'contactShadowStrength');
        bindSlider('slider-specular', 'waterSpecular');

        this.canvas.addEventListener('mousedown', (e) => {
            if (this.state.renderMode === 2) {
                this.isDraggingSplit = true;
                this.updateSplitFromEvent(e);
            }
        });

        window.addEventListener('mousemove', (e) => {
            if (this.isDraggingSplit) {
                this.updateSplitFromEvent(e);
            }
        });

        window.addEventListener('mouseup', () => {
            this.isDraggingSplit = false;
        });

        this.canvas.addEventListener('click', (e) => {
            if (this.state.renderMode !== 2 || !this.isDraggingSplit) {
                const rect = this.canvas.getBoundingClientRect();
                const scaleX = this.width / rect.width;
                const scaleY = this.height / rect.height;
                this.state.bonfirePos = [
                    (e.clientX - rect.left) * scaleX,
                    (e.clientY - rect.top) * scaleY
                ];
            }
        });
    }

    updateSplitFromEvent(e) {
        const rect = this.canvas.getBoundingClientRect();
        const pos = Math.max(0.0, Math.min(1.0, (e.clientX - rect.left) / rect.width));
        this.state.splitPos = pos;
        const slider = document.getElementById('slider-split');
        if (slider) slider.value = pos.toString();
    }

    showError(msg) {
        const errEl = document.getElementById('error-overlay');
        if (errEl) {
            errEl.style.display = 'flex';
            errEl.textContent = msg;
        }
    }
}

window.addEventListener('DOMContentLoaded', () => {
    const harness = new NightmareHarness();
    harness.init();
});
