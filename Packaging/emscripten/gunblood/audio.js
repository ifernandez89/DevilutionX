/**
 * Gunblood Web Audio Procedural Synthesizer
 * 100% Zero-Dependency, Instantaneous, Zero-Latency Western Sound FX
 */

class SoundEngine {
    constructor() {
        this.ctx = null;
        this.muted = false;
        this.initialized = false;
    }

    init() {
        if (this.initialized) return;
        const AudioContext = window.AudioContext || window.webkitAudioContext;
        if (AudioContext) {
            this.ctx = new AudioContext();
            this.initialized = true;
        }
    }

    resume() {
        if (this.ctx && this.ctx.state === 'suspended') {
            this.ctx.resume();
        }
    }

    // Play powerful gunshot
    playGunshot(isEnemy = false) {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        // 1. Noise transient explosion
        const bufferSize = this.ctx.sampleRate * 0.4;
        const buffer = this.ctx.createBuffer(1, bufferSize, this.ctx.sampleRate);
        const data = buffer.getChannelData(0);
        for (let i = 0; i < bufferSize; i++) {
            data[i] = (Math.random() * 2 - 1) * Math.exp(-i / (this.ctx.sampleRate * 0.08));
        }

        const noise = this.ctx.createBufferSource();
        noise.buffer = buffer;

        const noiseFilter = this.ctx.createBiquadFilter();
        noiseFilter.type = 'lowpass';
        noiseFilter.frequency.setValueAtTime(isEnemy ? 1800 : 3200, t);
        noiseFilter.frequency.exponentialRampToValueAtTime(120, t + 0.35);

        const noiseGain = this.ctx.createGain();
        noiseGain.gain.setValueAtTime(isEnemy ? 0.7 : 1.0, t);
        noiseGain.gain.exponentialRampToValueAtTime(0.001, t + 0.38);

        noise.connect(noiseFilter);
        noiseFilter.connect(noiseGain);
        noiseGain.connect(this.ctx.destination);
        noise.start(t);

        // 2. Punchy Sub Kick for body
        const osc = this.ctx.createOscillator();
        const oscGain = this.ctx.createGain();
        osc.type = 'triangle';
        osc.frequency.setValueAtTime(isEnemy ? 140 : 180, t);
        osc.frequency.exponentialRampToValueAtTime(30, t + 0.25);

        oscGain.gain.setValueAtTime(isEnemy ? 0.6 : 0.9, t);
        oscGain.gain.exponentialRampToValueAtTime(0.001, t + 0.25);

        osc.connect(oscGain);
        oscGain.connect(this.ctx.destination);
        osc.start(t);
        osc.stop(t + 0.26);

        // 3. Occasional ricochet on gunshot
        if (Math.random() > 0.45) {
            setTimeout(() => this.playRicochet(), 80 + Math.random() * 120);
        }
    }

    // Bullet ricochet / whiz sound
    playRicochet() {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();

        osc.type = 'sine';
        const startFreq = 2200 + Math.random() * 1800;
        const endFreq = 400 + Math.random() * 400;

        osc.frequency.setValueAtTime(startFreq, t);
        osc.frequency.exponentialRampToValueAtTime(startFreq * 1.5, t + 0.04);
        osc.frequency.exponentialRampToValueAtTime(endFreq, t + 0.28);

        gain.gain.setValueAtTime(0.25, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + 0.3);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start(t);
        osc.stop(t + 0.32);
    }

    // Revolver Hammer Cock
    playCock() {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        // Two sharp clicks in quick succession
        [0, 0.045].forEach((offset, idx) => {
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            osc.type = 'square';
            osc.frequency.setValueAtTime(idx === 0 ? 1200 : 1800, t + offset);
            osc.frequency.exponentialRampToValueAtTime(400, t + offset + 0.02);

            gain.gain.setValueAtTime(0.2, t + offset);
            gain.gain.exponentialRampToValueAtTime(0.001, t + offset + 0.025);

            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start(t + offset);
            osc.stop(t + offset + 0.03);
        });
    }

    // Cylinder click on empty
    playDryFire() {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'triangle';
        osc.frequency.setValueAtTime(600, t);
        osc.frequency.exponentialRampToValueAtTime(100, t + 0.03);

        gain.gain.setValueAtTime(0.3, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + 0.04);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start(t);
        osc.stop(t + 0.05);
    }

    // Duel Bell / Church Chime (High Noon)
    playBell(pitch = 520) {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        [1, 2.02, 3.15, 4.2].forEach((ratio, i) => {
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            osc.type = 'sine';
            osc.frequency.setValueAtTime(pitch * ratio, t);

            const vol = 0.35 / (i + 1);
            gain.gain.setValueAtTime(vol, t);
            gain.gain.exponentialRampToValueAtTime(0.0001, t + 2.5);

            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start(t);
            osc.stop(t + 2.6);
        });
    }

    // Heartbeat during countdown
    playHeartbeat() {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        [0, 0.12].forEach((offset, i) => {
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            osc.type = 'sine';
            osc.frequency.setValueAtTime(80 - i * 15, t + offset);
            osc.frequency.exponentialRampToValueAtTime(35, t + offset + 0.1);

            gain.gain.setValueAtTime(0.4, t + offset);
            gain.gain.exponentialRampToValueAtTime(0.001, t + offset + 0.12);

            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start(t + offset);
            osc.stop(t + offset + 0.14);
        });
    }

    // Flesh impact / blood splatter
    playHitFlesh(isHeadshot = false) {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = isHeadshot ? 'sawtooth' : 'triangle';
        osc.frequency.setValueAtTime(isHeadshot ? 450 : 250, t);
        osc.frequency.exponentialRampToValueAtTime(40, t + 0.15);

        gain.gain.setValueAtTime(isHeadshot ? 0.6 : 0.4, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + 0.18);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start(t);
        osc.stop(t + 0.2);
    }

    // Foul / Cheating Buzzer
    playFoul() {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;

        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(130, t);
        osc.frequency.setValueAtTime(110, t + 0.15);

        gain.gain.setValueAtTime(0.4, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + 0.4);

        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start(t);
        osc.stop(t + 0.45);
    }

    // Victory Western Guitar Fanfare
    playVictory() {
        if (this.muted || !this.ctx) return;
        this.resume();
        const t = this.ctx.currentTime;
        const chords = [261.63, 329.63, 392.00, 523.25, 659.25]; // C major western strum

        chords.forEach((freq, idx) => {
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            osc.type = 'triangle';
            osc.frequency.setValueAtTime(freq, t + idx * 0.05);

            gain.gain.setValueAtTime(0.25, t + idx * 0.05);
            gain.gain.exponentialRampToValueAtTime(0.0001, t + 1.8 + idx * 0.05);

            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start(t + idx * 0.05);
            osc.stop(t + 2.0);
        });
    }
}

window.soundEngine = new SoundEngine();
