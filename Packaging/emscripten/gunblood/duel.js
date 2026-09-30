/**
 * Gunblood: Western Quick-Draw Duel Engine
 * High-performance Canvas 2D physics, ragdoll animations & AI system
 */

const OUTLAWS = [
    {
        id: 'ned',
        name: 'Novice Ned',
        title: 'El Aprendiz de Bandido',
        bounty: '$250',
        quote: '¡No me dispares! Apenas aprendí a cargar esta cosa...',
        reactionTime: 620, // ms
        accuracy: 0.45,
        hp: 100,
        colors: { hat: '#8d6e63', shirt: '#a1887f', vest: '#5d4037', pants: '#3e2723', skin: '#ffcc80' }
    },
    {
        id: 'pete',
        name: 'Cactus Pete',
        title: 'El Ermitaño del Cañón',
        bounty: '$500',
        quote: '¡El sol del desierto te cegará antes que mi plomo!',
        reactionTime: 520,
        accuracy: 0.60,
        hp: 100,
        colors: { hat: '#689f38', shirt: '#9ccc65', vest: '#33691e', pants: '#558b2f', skin: '#d7ccc8' }
    },
    {
        id: 'jack',
        name: 'One-Eye Jack',
        title: 'El Ojo del Buitre',
        bounty: '$1,000',
        quote: 'Solo necesito un ojo para mandarte tres metros bajo tierra.',
        reactionTime: 440,
        accuracy: 0.72,
        hp: 110,
        colors: { hat: '#455a64', shirt: '#78909c', vest: '#263238', pants: '#37474f', skin: '#ffb74d' }
    },
    {
        id: 'morales',
        name: 'Bandido Morales',
        title: 'El Terror de la Frontera',
        bounty: '$2,500',
        quote: '¡Ándele gringo! Veamos si tu pulso es tan rápido como tu boca.',
        reactionTime: 370,
        accuracy: 0.80,
        hp: 120,
        colors: { hat: '#d84315', shirt: '#ff7043', vest: '#bf360c', pants: '#4e342e', skin: '#d7a15c' }
    },
    {
        id: 'jane',
        name: 'Calamity Jane',
        title: 'La Dama de la Muerte',
        bounty: '$5,000',
        quote: 'Muchos hombres han dudado de mi velocidad... ninguno sobrevivió.',
        reactionTime: 310,
        accuracy: 0.86,
        hp: 120,
        colors: { hat: '#ad1457', shirt: '#ec407a', vest: '#880e4f', pants: '#4a148c', skin: '#ffe082' }
    },
    {
        id: 'butch',
        name: "Butch 'The Bull'",
        title: 'El Verdugo de Dodge City',
        bounty: '$10,000',
        quote: '¡Trágame si puedes, sheriff! ¡Yo soy la ley aquí!',
        reactionTime: 260,
        accuracy: 0.90,
        hp: 140,
        colors: { hat: '#212121', shirt: '#424242', vest: '#b71c1c', pants: '#212121', skin: '#ffcc80' }
    },
    {
        id: 'sal',
        name: 'Silvertongue Sal',
        title: 'El As de Espadas',
        bounty: '$25,000',
        quote: 'El plomo es la única moneda que nunca se devalúa.',
        reactionTime: 215,
        accuracy: 0.94,
        hp: 130,
        colors: { hat: '#37474f', shirt: '#cfd8dc', vest: '#263238', pants: '#263238', skin: '#ffe082' }
    },
    {
        id: 'diablo',
        name: 'El Diablo Rojo',
        title: 'La Leyenda Inmortal del Oeste',
        bounty: '$100,000',
        quote: 'Has llegado lejos, forastero... pero este es tu último atardecer.',
        reactionTime: 175,
        accuracy: 0.98,
        hp: 150,
        colors: { hat: '#b71c1c', shirt: '#d32f2f', vest: '#1a1a1a', pants: '#1a1a1a', skin: '#d7a15c' }
    }
];

class GunbloodDuel {
    constructor(canvas, ui) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.ui = ui;

        // Dimensions
        this.width = 1000;
        this.height = 560;
        this.canvas.width = this.width;
        this.canvas.height = this.height;

        // Duel State
        this.state = 'MENU'; // MENU, COUNTDOWN, FOUL, FIRE, SHOOTING, DUEL_OVER, BONUS, VICTORY
        this.currentLevel = 0;
        this.score = 0;
        this.playerHP = 100;
        this.enemyHP = 100;
        this.playerBullets = 6;
        this.enemyBullets = 6;
        
        // Timing
        this.countdownTimer = 3;
        this.countdownTimestamp = 0;
        this.fireTimestamp = 0;
        this.playerReactionTime = null;
        this.enemyShootTimeout = null;

        // Holster Chamber Position (Bottom Left)
        this.chamber = {
            x: 100,
            y: 470,
            radius: 54,
            isHovered: false
        };

        // Mouse coordinates relative to canvas
        this.mouse = { x: -100, y: -100, isInside: false };

        // Particles & Physics Objects
        this.particles = [];
        this.flyingHats = [];
        this.flyingGuns = [];
        this.bloodStains = [];
        this.floatingTexts = [];
        this.tumbleweeds = [];

        // Bonus Round Targets
        this.bonusTargets = [];
        this.bonusTimeLeft = 15;
        this.bonusScore = 0;

        // Setup Environmental Assets
        this.initEnvironment();
        this.bindEvents();

        // Animation Loop
        this.lastFrameTime = performance.now();
        requestAnimationFrame((t) => this.loop(t));
    }

    initEnvironment() {
        this.tumbleweeds = [
            { x: -50, y: 440, vx: 2.2, vy: 0, rot: 0, size: 28 },
            { x: 400, y: 450, vx: 1.8, vy: 0, rot: 0, size: 22 }
        ];
    }

    bindEvents() {
        this.canvas.addEventListener('mousemove', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            const scaleX = this.canvas.width / rect.width;
            const scaleY = this.canvas.height / rect.height;
            this.mouse.x = (e.clientX - rect.left) * scaleX;
            this.mouse.y = (e.clientY - rect.top) * scaleY;
            this.mouse.isInside = true;

            this.checkChamberHover();
        });

        this.canvas.addEventListener('mouseleave', () => {
            this.mouse.isInside = false;
            this.chamber.isHovered = false;
            if (this.state === 'COUNTDOWN') {
                this.triggerFoul('¡FALTA! Quitaste el cursor de la recámara antes de tiempo.');
            }
        });

        this.canvas.addEventListener('mousedown', (e) => {
            if (e.button === 0) { // Left click
                this.handlePlayerShot();
            }
        });
    }

    checkChamberHover() {
        const dx = this.mouse.x - this.chamber.x;
        const dy = this.mouse.y - this.chamber.y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        const wasHovered = this.chamber.isHovered;
        this.chamber.isHovered = dist <= this.chamber.radius;

        if (!wasHovered && this.chamber.isHovered) {
            window.soundEngine.playCock();
            if (this.state === 'READY_WAIT' || this.state === 'FOUL') {
                this.startCountdown();
            }
        } else if (wasHovered && !this.chamber.isHovered) {
            if (this.state === 'COUNTDOWN') {
                this.triggerFoul('¡FALTA! Mantén el cursor en el tambor hasta el ¡FUEGO!');
            }
        }
    }

    startNewGame() {
        this.currentLevel = 0;
        this.score = 0;
        this.startLevel(this.currentLevel);
    }

    startLevel(levelIdx) {
        if (levelIdx >= OUTLAWS.length) {
            this.state = 'VICTORY';
            window.soundEngine.playVictory();
            this.ui.updateHUD(this);
            return;
        }

        // Check if bonus round (after level 3 and level 6)
        if (levelIdx === 3 || levelIdx === 6) {
            if (!this.bonusPlayed) {
                this.bonusPlayed = true;
                this.startBonusRound();
                return;
            }
        }
        this.bonusPlayed = false;

        this.currentLevel = levelIdx;
        this.currentEnemy = OUTLAWS[this.currentLevel];
        this.playerHP = 100;
        this.enemyHP = this.currentEnemy.hp;
        this.playerBullets = 6;
        this.enemyBullets = 6;
        this.playerReactionTime = null;
        this.flyingHats = [];
        this.flyingGuns = [];
        this.particles = [];
        this.floatingTexts = [];

        this.state = 'READY_WAIT';
        this.ui.showReadyPrompt(`Nivel ${this.currentLevel + 1}: ${this.currentEnemy.name}`, this.currentEnemy.quote);
        this.ui.updateHUD(this);
    }

    startCountdown() {
        this.state = 'COUNTDOWN';
        this.countdownTimer = 3;
        this.countdownTimestamp = performance.now();
        window.soundEngine.playHeartbeat();
        this.ui.hidePrompt();

        const tick = () => {
            if (this.state !== 'COUNTDOWN') return;
            if (this.countdownTimer > 1) {
                this.countdownTimer--;
                window.soundEngine.playHeartbeat();
                setTimeout(tick, 900);
            } else {
                this.triggerFire();
            }
        };
        setTimeout(tick, 900);
    }

    triggerFoul(msg) {
        this.state = 'FOUL';
        window.soundEngine.playFoul();
        this.ui.showFoulMessage(msg);
        this.addFloatingText('¡FALTA!', this.chamber.x, this.chamber.y - 40, '#ff4757');
    }

    triggerFire() {
        this.state = 'SHOOTING';
        this.fireTimestamp = performance.now();
        window.soundEngine.playBell();
        this.addFloatingText('¡¡FUEGO!!', this.width / 2, 180, '#ff4757', 48);

        // Schedule Enemy Shot based on AI reaction time + variance
        const enemy = this.currentEnemy;
        const variance = (Math.random() * 80 - 40);
        const enemyDelay = Math.max(120, enemy.reactionTime + variance);

        this.enemyShootTimeout = setTimeout(() => {
            if (this.state === 'SHOOTING' && this.enemyHP > 0 && this.playerHP > 0) {
                this.handleEnemyShot();
            }
        }, enemyDelay);
    }

    handlePlayerShot() {
        window.soundEngine.init();

        if (this.state === 'BONUS') {
            this.handleBonusShot();
            return;
        }

        if (this.state === 'COUNTDOWN') {
            this.triggerFoul('¡DISPARO ANTES DE TIEMPO! Tramposo.');
            return;
        }

        if (this.state !== 'SHOOTING') return;

        if (this.playerBullets <= 0) {
            window.soundEngine.playDryFire();
            this.addFloatingText('*CLICK* ¡Sin balas!', this.mouse.x, this.mouse.y, '#ffd32a');
            return;
        }

        this.playerBullets--;
        window.soundEngine.playGunshot(false);
        this.createMuzzleFlash(220, 390);

        // Calculate reaction time on first shot
        if (this.playerReactionTime === null) {
            this.playerReactionTime = ((performance.now() - this.fireTimestamp) / 1000).toFixed(3);
        }

        // Check Hit on Enemy (Enemy position: X: 780, Y: 260)
        const hitResult = this.checkEnemyHit(this.mouse.x, this.mouse.y);
        if (hitResult) {
            this.enemyHP = Math.max(0, this.enemyHP - hitResult.damage);
            window.soundEngine.playHitFlesh(hitResult.isHeadshot);

            // Blood & particle effects
            this.createBloodSplatter(this.mouse.x, this.mouse.y, hitResult.isHeadshot ? 35 : 18);
            this.addFloatingText(hitResult.text, this.mouse.x, this.mouse.y - 20, hitResult.color, hitResult.fontSize);

            // Headshot dislodges hat
            if (hitResult.isHeadshot) {
                this.launchHat(780, 270);
                this.score += 500;
            } else if (hitResult.zone === 'ARM') {
                this.launchGun(730, 370);
                this.score += 250;
                // Disarming delays or cancels enemy shot
                clearTimeout(this.enemyShootTimeout);
            } else {
                this.score += 150;
            }

            this.ui.updateHUD(this);

            if (this.enemyHP <= 0) {
                this.endDuel(true);
            }
        } else {
            // Missed shot -> Bullet impact on wooden backdrop
            this.createBulletPuff(this.mouse.x, this.mouse.y);
        }
    }

    checkEnemyHit(x, y) {
        // Enemy center is at X: 780, Base ground Y: 460
        // Head / Hat Box: X: [750, 810], Y: [230, 290]
        if (x >= 750 && x <= 810 && y >= 230 && y <= 290) {
            return { zone: 'HEAD', damage: 150, isHeadshot: true, text: '💥 ¡HEADSHOT! (150)', color: '#ff3838', fontSize: 24 };
        }
        // Chest / Heart: X: [745, 815], Y: [290, 370]
        if (x >= 745 && x <= 815 && y > 290 && y <= 370) {
            return { zone: 'CHEST', damage: 55, isHeadshot: false, text: '🎯 ¡PECHO! (55)', color: '#ff9f1a', fontSize: 20 };
        }
        // Gun Arm: X: [710, 750], Y: [340, 410]
        if (x >= 710 && x <= 750 && y >= 340 && y <= 410) {
            return { zone: 'ARM', damage: 35, isHeadshot: false, text: '🦾 ¡DESARMADO! (35)', color: '#00d2d3', fontSize: 18 };
        }
        // Stomach & Legs: X: [750, 810], Y: [370, 470]
        if (x >= 750 && x <= 810 && y > 370 && y <= 470) {
            return { zone: 'LEGS', damage: 30, isHeadshot: false, text: '🦵 Piernas (30)', color: '#c56cf0', fontSize: 16 };
        }
        return null;
    }

    handleEnemyShot() {
        if (this.enemyBullets <= 0 || this.enemyHP <= 0) return;
        this.enemyBullets--;

        window.soundEngine.playGunshot(true);
        this.createMuzzleFlash(730, 385);

        // Determine if enemy hits player based on accuracy
        const hits = Math.random() <= this.currentEnemy.accuracy;
        if (hits) {
            const isHead = Math.random() < 0.25;
            const damage = isHead ? 65 : (30 + Math.floor(Math.random() * 20));
            this.playerHP = Math.max(0, this.playerHP - damage);
            window.soundEngine.playHitFlesh(isHead);
            this.createBloodSplatter(220, isHead ? 280 : 340, 25);
            this.addFloatingText(isHead ? '¡CRÍTICO ENEMIGO!' : '¡HERIDO!', 220, 270, '#ff3838', 22);

            if (isHead) {
                this.launchHat(220, 260, -1);
            }

            this.ui.updateHUD(this);

            if (this.playerHP <= 0) {
                this.endDuel(false);
            }
        } else {
            // Enemy missed
            this.createBulletPuff(140 + Math.random() * 160, 260 + Math.random() * 180);
        }

        // Enemy can fire consecutive shots if both alive
        if (this.playerHP > 0 && this.enemyHP > 0 && this.enemyBullets > 0) {
            const nextShotDelay = 600 + Math.random() * 400;
            this.enemyShootTimeout = setTimeout(() => this.handleEnemyShot(), nextShotDelay);
        }
    }

    endDuel(playerWon) {
        clearTimeout(this.enemyShootTimeout);
        this.state = 'DUEL_OVER';

        if (playerWon) {
            window.soundEngine.playVictory();
            const timeBonus = Math.max(0, Math.floor((1.0 - parseFloat(this.playerReactionTime || 1.0)) * 1000));
            const hpBonus = this.playerHP * 5;
            this.score += timeBonus + hpBonus;
            
            setTimeout(() => {
                this.ui.showVictoryModal({
                    winner: true,
                    enemyName: this.currentEnemy.name,
                    bounty: this.currentEnemy.bounty,
                    reactionTime: this.playerReactionTime,
                    timeBonus,
                    hpBonus,
                    totalScore: this.score,
                    nextLevel: () => this.startLevel(this.currentLevel + 1)
                });
            }, 1200);
        } else {
            setTimeout(() => {
                this.ui.showGameOverModal({
                    enemyName: this.currentEnemy.name,
                    quote: this.currentEnemy.quote,
                    score: this.score,
                    retry: () => this.startLevel(this.currentLevel)
                });
            }, 1200);
        }
    }

    // BONUS ROUND (Target practice with bottles & dynamite)
    startBonusRound() {
        this.state = 'BONUS';
        this.bonusTargets = [];
        this.bonusTimeLeft = 15;
        this.bonusScore = 0;
        this.playerBullets = 6;
        this.ui.showBonusNotification();

        // Spawn targets periodically
        this.bonusInterval = setInterval(() => {
            if (this.state !== 'BONUS') {
                clearInterval(this.bonusInterval);
                return;
            }
            this.spawnBonusTarget();
        }, 1100);

        this.bonusTimerInterval = setInterval(() => {
            if (this.state !== 'BONUS') {
                clearInterval(this.bonusTimerInterval);
                return;
            }
            this.bonusTimeLeft--;
            this.ui.updateBonusTimer(this.bonusTimeLeft);
            if (this.bonusTimeLeft <= 0) {
                clearInterval(this.bonusTimerInterval);
                this.endBonusRound();
            }
        }, 1000);
    }

    spawnBonusTarget() {
        const fromLeft = Math.random() > 0.5;
        const isDynamite = Math.random() > 0.75;
        this.bonusTargets.push({
            id: Math.random(),
            isDynamite,
            x: fromLeft ? 40 : this.width - 40,
            y: 440,
            vx: (fromLeft ? 1 : -1) * (4 + Math.random() * 3),
            vy: -(11 + Math.random() * 4),
            gravity: 0.38,
            rot: 0,
            rotSpeed: (Math.random() - 0.5) * 0.2,
            radius: isDynamite ? 24 : 18,
            hit: false
        });
    }

    handleBonusShot() {
        window.soundEngine.playGunshot(false);
        this.playerBullets--;
        if (this.playerBullets <= 0) {
            setTimeout(() => {
                this.playerBullets = 6;
                window.soundEngine.playCock();
            }, 400);
        }

        // Check if hit any flying target
        for (let i = this.bonusTargets.length - 1; i >= 0; i--) {
            const t = this.bonusTargets[i];
            if (t.hit) continue;
            const dx = this.mouse.x - t.x;
            const dy = this.mouse.y - t.y;
            if (Math.sqrt(dx * dx + dy * dy) <= t.radius + 12) {
                t.hit = true;
                if (t.isDynamite) {
                    window.soundEngine.playGunshot(false);
                    this.createExplosion(t.x, t.y);
                    this.bonusScore += 250;
                    this.addFloatingText('💥 ¡DYNAMITE! +250', t.x, t.y, '#ff4757', 22);
                } else {
                    window.soundEngine.playRicochet();
                    this.createGlassShatter(t.x, t.y);
                    this.bonusScore += 100;
                    this.addFloatingText('🍾 ¡BOTELLA! +100', t.x, t.y, '#2ed573', 20);
                }
                break;
            }
        }
    }

    endBonusRound() {
        this.state = 'BONUS_OVER';
        this.score += this.bonusScore;
        this.ui.showBonusSummary(this.bonusScore, () => {
            this.startLevel(this.currentLevel + 1);
        });
    }

    // Physics & Visual Effects
    launchHat(x, y, dir = 1) {
        this.flyingHats.push({
            x,
            y,
            vx: dir * (3 + Math.random() * 4),
            vy: -(7 + Math.random() * 5),
            gravity: 0.35,
            rot: 0,
            vRot: dir * (0.15 + Math.random() * 0.2)
        });
    }

    launchGun(x, y, dir = 1) {
        this.flyingGuns.push({
            x,
            y,
            vx: dir * (2 + Math.random() * 3),
            vy: -(6 + Math.random() * 4),
            gravity: 0.4,
            rot: 0,
            vRot: dir * 0.3
        });
    }

    createBloodSplatter(x, y, count = 20) {
        for (let i = 0; i < count; i++) {
            const angle = Math.random() * Math.PI * 2;
            const speed = 2 + Math.random() * 6;
            this.particles.push({
                x,
                y,
                vx: Math.cos(angle) * speed,
                vy: Math.sin(angle) * speed - 1,
                gravity: 0.25,
                color: Math.random() > 0.4 ? '#b71540' : '#eb2f06',
                size: 3 + Math.random() * 4,
                life: 1.0,
                decay: 0.015 + Math.random() * 0.02
            });
        }
    }

    createMuzzleFlash(x, y) {
        this.particles.push({
            type: 'flash',
            x,
            y,
            radius: 40,
            life: 1.0,
            decay: 0.2
        });
        // Smoke puff
        for (let i = 0; i < 6; i++) {
            this.particles.push({
                x: x + (Math.random() * 20 - 10),
                y: y + (Math.random() * 10 - 5),
                vx: (Math.random() - 0.5) * 1.5,
                vy: -1.2 - Math.random() * 1.5,
                gravity: -0.02,
                color: 'rgba(200, 200, 200, 0.6)',
                size: 8 + Math.random() * 12,
                life: 1.0,
                decay: 0.03
            });
        }
    }

    createBulletPuff(x, y) {
        for (let i = 0; i < 8; i++) {
            this.particles.push({
                x,
                y,
                vx: (Math.random() - 0.5) * 4,
                vy: (Math.random() - 0.5) * 4,
                gravity: 0.1,
                color: '#d2b48c',
                size: 3 + Math.random() * 3,
                life: 1.0,
                decay: 0.04
            });
        }
    }

    createGlassShatter(x, y) {
        for (let i = 0; i < 15; i++) {
            this.particles.push({
                x,
                y,
                vx: (Math.random() - 0.5) * 8,
                vy: -3 - Math.random() * 5,
                gravity: 0.35,
                color: '#a4b0be',
                size: 2 + Math.random() * 3,
                life: 1.0,
                decay: 0.03
            });
        }
    }

    createExplosion(x, y) {
        for (let i = 0; i < 25; i++) {
            this.particles.push({
                x,
                y,
                vx: (Math.random() - 0.5) * 10,
                vy: (Math.random() - 0.5) * 10,
                gravity: 0.2,
                color: Math.random() > 0.5 ? '#e55039' : '#f6b93b',
                size: 5 + Math.random() * 8,
                life: 1.0,
                decay: 0.03
            });
        }
    }

    addFloatingText(text, x, y, color = '#fff', fontSize = 20) {
        this.floatingTexts.push({
            text,
            x,
            y,
            color,
            fontSize,
            life: 1.0,
            decay: 0.018
        });
    }

    // MAIN RENDER & UPDATE LOOP
    loop(timestamp) {
        const dt = (timestamp - this.lastFrameTime) / 1000;
        this.lastFrameTime = timestamp;

        this.update(dt);
        this.render();

        requestAnimationFrame((t) => this.loop(t));
    }

    update(dt) {
        // Update Tumbleweeds
        this.tumbleweeds.forEach(tw => {
            tw.x += tw.vx;
            tw.rot += 0.05;
            if (tw.x > this.width + 100) tw.x = -80;
        });

        // Update Flying Hats
        this.flyingHats.forEach(h => {
            h.x += h.vx;
            h.y += h.vy;
            h.vy += h.gravity;
            h.rot += h.vRot;
        });

        // Update Flying Guns
        this.flyingGuns.forEach(g => {
            g.x += g.vx;
            g.y += g.vy;
            g.vy += g.gravity;
            g.rot += g.vRot;
        });

        // Update Particles
        for (let i = this.particles.length - 1; i >= 0; i--) {
            const p = this.particles[i];
            p.x += p.vx || 0;
            p.y += p.vy || 0;
            if (p.gravity) p.vy += p.gravity;
            p.life -= p.decay;
            if (p.life <= 0) this.particles.splice(i, 1);
        }

        // Update Floating Texts
        for (let i = this.floatingTexts.length - 1; i >= 0; i--) {
            const ft = this.floatingTexts[i];
            ft.y -= 1.2;
            ft.life -= ft.decay;
            if (ft.life <= 0) this.floatingTexts.splice(i, 1);
        }

        // Update Bonus Targets
        if (this.state === 'BONUS') {
            for (let i = this.bonusTargets.length - 1; i >= 0; i--) {
                const t = this.bonusTargets[i];
                t.x += t.vx;
                t.y += t.vy;
                t.vy += t.gravity;
                t.rot += t.rotSpeed;
                if (t.y > this.height + 60) {
                    this.bonusTargets.splice(i, 1);
                }
            }
        }
    }

    render() {
        const ctx = this.ctx;
        ctx.clearRect(0, 0, this.width, this.height);

        // 1. Sky & Western Sunset Gradient
        const skyGrad = ctx.createLinearGradient(0, 0, 0, 380);
        skyGrad.addColorStop(0, '#e58e26');
        skyGrad.addColorStop(0.5, '#f6b93b');
        skyGrad.addColorStop(1, '#fad390');
        ctx.fillStyle = skyGrad;
        ctx.fillRect(0, 0, this.width, 380);

        // 2. Distant Desert Mesas / Mountains
        ctx.fillStyle = '#b7791f';
        ctx.beginPath();
        ctx.moveTo(0, 360);
        ctx.lineTo(80, 290);
        ctx.lineTo(220, 290);
        ctx.lineTo(320, 360);
        ctx.lineTo(540, 300);
        ctx.lineTo(680, 300);
        ctx.lineTo(790, 360);
        ctx.lineTo(950, 270);
        ctx.lineTo(1000, 360);
        ctx.lineTo(1000, 420);
        ctx.lineTo(0, 420);
        ctx.fill();

        // 3. Desert Ground
        const groundGrad = ctx.createLinearGradient(0, 360, 0, this.height);
        groundGrad.addColorStop(0, '#c77f3d');
        groundGrad.addColorStop(1, '#8c4819');
        ctx.fillStyle = groundGrad;
        ctx.fillRect(0, 360, this.width, this.height - 360);

        // Ground grit texture lines
        ctx.strokeStyle = 'rgba(0,0,0,0.08)';
        ctx.lineWidth = 2;
        for (let i = 370; i < this.height; i += 15) {
            ctx.beginPath();
            ctx.moveTo(0, i);
            ctx.lineTo(this.width, i);
            ctx.stroke();
        }

        // 4. Western Saloon Facade (Background Left/Right)
        this.renderSaloonBuilding(ctx);

        // 5. Tumbleweeds
        this.tumbleweeds.forEach(tw => this.renderTumbleweed(ctx, tw));

        // 6. Duelists (Player & Enemy)
        if (this.state !== 'BONUS') {
            this.renderPlayer(ctx);
            if (this.currentEnemy) {
                this.renderEnemy(ctx, this.currentEnemy);
            }
        } else {
            this.renderBonusTargets(ctx);
        }

        // 7. Flying Hats & Guns
        this.flyingHats.forEach(h => this.renderHat(ctx, h.x, h.y, h.rot, '#212121'));
        this.flyingGuns.forEach(g => this.renderRevolver(ctx, g.x, g.y, g.rot));

        // 8. Particles (Blood, Smoke, Flashes)
        this.renderParticles(ctx);

        // 9. Floating Combat Texts
        this.floatingTexts.forEach(ft => {
            ctx.save();
            ctx.globalAlpha = Math.max(0, ft.life);
            ctx.fillStyle = ft.color;
            ctx.font = `900 ${ft.fontSize}px 'Outfit', sans-serif`;
            ctx.textAlign = 'center';
            ctx.shadowColor = '#000';
            ctx.shadowBlur = 8;
            ctx.fillText(ft.text, ft.x, ft.y);
            ctx.restore();
        });

        // 10. Chamber Holster HUD (Bottom Left)
        if (this.state !== 'BONUS') {
            this.renderChamber(ctx);
        }

        // 11. Crosshair / Aim Pointer
        if (this.mouse.isInside && (this.state === 'SHOOTING' || this.state === 'BONUS')) {
            this.renderCrosshair(ctx, this.mouse.x, this.mouse.y);
        }
    }

    renderSaloonBuilding(ctx) {
        // Saloon facade on far right background
        ctx.fillStyle = '#4a2810';
        ctx.fillRect(840, 240, 160, 150);
        ctx.fillStyle = '#2c180b';
        ctx.fillRect(870, 290, 45, 100); // Doorway
        ctx.fillRect(940, 270, 40, 45); // Window

        // Water tower on left
        ctx.fillStyle = '#5c3a21';
        ctx.fillRect(40, 250, 60, 70);
        ctx.fillRect(45, 320, 8, 70);
        ctx.fillRect(85, 320, 8, 70);
    }

    renderTumbleweed(ctx, tw) {
        ctx.save();
        ctx.translate(tw.x, tw.y);
        ctx.rotate(tw.rot);
        ctx.strokeStyle = '#8d6e63';
        ctx.lineWidth = 2;
        ctx.beginPath();
        for (let a = 0; a < Math.PI * 2; a += Math.PI / 4) {
            ctx.arc(0, 0, tw.size, a, a + 0.8);
        }
        ctx.stroke();
        ctx.restore();
    }

    renderPlayer(ctx) {
        const x = 220;
        const y = 460;
        const isDead = this.playerHP <= 0;

        ctx.save();
        ctx.translate(x, y);
        if (isDead) {
            ctx.rotate(-Math.PI / 2.5); // Fallen on ground
        }

        // Shadow
        ctx.fillStyle = 'rgba(0,0,0,0.25)';
        ctx.beginPath();
        ctx.ellipse(0, 0, 32, 10, 0, 0, Math.PI * 2);
        ctx.fill();

        // Legs / Pants
        ctx.fillStyle = '#2c3e50';
        ctx.fillRect(-14, -80, 12, 80);
        ctx.fillRect(2, -80, 12, 80);

        // Boots
        ctx.fillStyle = '#1e272e';
        ctx.fillRect(-18, -12, 16, 12);
        ctx.fillRect(2, -12, 16, 12);

        // Torso / White Shirt + Brown Duster Coat
        ctx.fillStyle = '#d2dae2';
        ctx.fillRect(-16, -145, 32, 65);
        ctx.fillStyle = '#795548'; // Duster Coat
        ctx.fillRect(-20, -145, 10, 85);
        ctx.fillRect(10, -145, 10, 85);

        // Sheriff Gold Star
        ctx.fillStyle = '#ffd32a';
        ctx.beginPath();
        ctx.arc(-5, -125, 4, 0, Math.PI * 2);
        ctx.fill();

        // Gun Arm & Holster
        ctx.fillStyle = '#d2dae2';
        if (this.state === 'SHOOTING' || this.state === 'DUEL_OVER') {
            // Aiming forward
            ctx.fillRect(10, -135, 40, 10);
            this.renderRevolver(ctx, 50, -130, 0);
        } else {
            // Resting by holster
            ctx.fillRect(10, -135, 10, 45);
        }

        // Head
        ctx.fillStyle = '#ffcc80';
        ctx.beginPath();
        ctx.arc(0, -165, 16, 0, Math.PI * 2);
        ctx.fill();

        // Sheriff Hat (if not knocked off)
        const hatFlying = this.flyingHats.some(h => h.vx < 0);
        if (!hatFlying) {
            this.renderHat(ctx, 0, -178, 0, '#ecf0f1');
        }

        ctx.restore();
    }

    renderEnemy(ctx, enemy) {
        const x = 780;
        const y = 460;
        const isDead = this.enemyHP <= 0;

        ctx.save();
        ctx.translate(x, y);
        if (isDead) {
            ctx.rotate(Math.PI / 2.3); // Fallen backward
        }

        // Shadow
        ctx.fillStyle = 'rgba(0,0,0,0.25)';
        ctx.beginPath();
        ctx.ellipse(0, 0, 34, 10, 0, 0, Math.PI * 2);
        ctx.fill();

        // Legs
        ctx.fillStyle = enemy.colors.pants;
        ctx.fillRect(-14, -80, 12, 80);
        ctx.fillRect(2, -80, 12, 80);

        // Boots
        ctx.fillStyle = '#1e272e';
        ctx.fillRect(-14, -12, 16, 12);
        ctx.fillRect(2, -12, 16, 12);

        // Torso / Shirt + Vest
        ctx.fillStyle = enemy.colors.shirt;
        ctx.fillRect(-16, -145, 32, 65);
        ctx.fillStyle = enemy.colors.vest;
        ctx.fillRect(-18, -145, 10, 60);
        ctx.fillRect(8, -145, 10, 60);

        // Gun Arm (Left relative to enemy facing)
        ctx.fillStyle = enemy.colors.shirt;
        const gunFlying = this.flyingGuns.length > 0;
        if ((this.state === 'SHOOTING' || this.state === 'DUEL_OVER') && !gunFlying && !isDead) {
            // Aiming at player
            ctx.fillRect(-50, -135, 40, 10);
            this.renderRevolver(ctx, -50, -130, Math.PI);
        } else {
            ctx.fillRect(-18, -135, 10, 45);
        }

        // Head
        ctx.fillStyle = enemy.colors.skin;
        ctx.beginPath();
        ctx.arc(0, -165, 16, 0, Math.PI * 2);
        ctx.fill();

        // Enemy Hat (if not flying)
        const hatFlying = this.flyingHats.some(h => h.vx > 0);
        if (!hatFlying) {
            this.renderHat(ctx, 0, -178, 0, enemy.colors.hat);
        }

        ctx.restore();
    }

    renderHat(ctx, x, y, rot, color) {
        ctx.save();
        ctx.translate(x, y);
        ctx.rotate(rot);
        ctx.fillStyle = color;
        // Brim
        ctx.beginPath();
        ctx.ellipse(0, 0, 26, 6, 0, 0, Math.PI * 2);
        ctx.fill();
        // Crown
        ctx.fillRect(-12, -14, 24, 14);
        ctx.restore();
    }

    renderRevolver(ctx, x, y, rot) {
        ctx.save();
        ctx.translate(x, y);
        ctx.rotate(rot);
        ctx.fillStyle = '#2f3542';
        ctx.fillRect(0, -4, 18, 5); // Barrel
        ctx.fillRect(-4, -6, 8, 8); // Cylinder
        ctx.fillStyle = '#795548';
        ctx.fillRect(-6, 0, 6, 10); // Wooden Grip
        ctx.restore();
    }

    renderBonusTargets(ctx) {
        this.bonusTargets.forEach(t => {
            if (t.hit) return;
            ctx.save();
            ctx.translate(t.x, t.y);
            ctx.rotate(t.rot);
            if (t.isDynamite) {
                // Red dynamite stick with sparkling fuse
                ctx.fillStyle = '#ff4757';
                ctx.fillRect(-10, -22, 20, 44);
                ctx.fillStyle = '#ffa502';
                ctx.fillRect(-3, -28, 6, 6); // Fuse spark
            } else {
                // Glass whiskey bottle
                ctx.fillStyle = '#2ed573';
                ctx.fillRect(-8, -15, 16, 30);
                ctx.fillRect(-4, -24, 8, 10);
            }
            ctx.restore();
        });
    }

    renderParticles(ctx) {
        this.particles.forEach(p => {
            ctx.save();
            ctx.globalAlpha = Math.max(0, p.life);
            if (p.type === 'flash') {
                const grad = ctx.createRadialGradient(p.x, p.y, 2, p.x, p.y, p.radius);
                grad.addColorStop(0, '#ffffff');
                grad.addColorStop(0.4, '#ffd32a');
                grad.addColorStop(1, 'transparent');
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(p.x, p.y, p.radius, 0, Math.PI * 2);
                ctx.fill();
            } else {
                ctx.fillStyle = p.color;
                ctx.beginPath();
                ctx.arc(p.x, p.y, p.size, 0, Math.PI * 2);
                ctx.fill();
            }
            ctx.restore();
        });
    }

    renderChamber(ctx) {
        const { x, y, radius, isHovered } = this.chamber;

        ctx.save();
        // Pulse ring when hovering
        if (isHovered) {
            ctx.strokeStyle = 'rgba(46, 213, 115, 0.7)';
            ctx.lineWidth = 4;
            ctx.beginPath();
            ctx.arc(x, y, radius + 6, 0, Math.PI * 2);
            ctx.stroke();
        } else {
            ctx.strokeStyle = 'rgba(255, 71, 87, 0.6)';
            ctx.lineWidth = 2;
            ctx.setLineDash([6, 6]);
            ctx.beginPath();
            ctx.arc(x, y, radius + 4, 0, Math.PI * 2);
            ctx.stroke();
            ctx.setLineDash([]);
        }

        // Metal Chamber Body
        const grad = ctx.createRadialGradient(x, y, 5, x, y, radius);
        grad.addColorStop(0, '#57606f');
        grad.addColorStop(0.8, '#2f3542');
        grad.addColorStop(1, '#1e272e');
        ctx.fillStyle = grad;
        ctx.beginPath();
        ctx.arc(x, y, radius, 0, Math.PI * 2);
        ctx.fill();

        // 6 Revolver Chamber Holes
        for (let i = 0; i < 6; i++) {
            const angle = (i * Math.PI / 3);
            const hx = x + Math.cos(angle) * 30;
            const hy = y + Math.sin(angle) * 30;

            ctx.fillStyle = (i < this.playerBullets) ? '#eccc68' : '#111'; // Loaded Brass or Empty
            ctx.beginPath();
            ctx.arc(hx, hy, 9, 0, Math.PI * 2);
            ctx.fill();
            ctx.strokeStyle = '#2f3542';
            ctx.lineWidth = 1.5;
            ctx.stroke();
        }

        // Center Ratchet Pin
        ctx.fillStyle = '#a4b0be';
        ctx.beginPath();
        ctx.arc(x, y, 10, 0, Math.PI * 2);
        ctx.fill();

        // Holster Text label
        ctx.fillStyle = isHovered ? '#2ed573' : '#ff4757';
        ctx.font = "700 11px 'Outfit', sans-serif";
        ctx.textAlign = 'center';
        ctx.fillText(isHovered ? 'DESENFUNDA AL DISPARAR' : 'COLOCA EL CURSOR AQUÍ', x, y + radius + 22);

        ctx.restore();
    }

    renderCrosshair(ctx, x, y) {
        ctx.save();
        ctx.strokeStyle = '#ff4757';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(x, y, 14, 0, Math.PI * 2);
        ctx.moveTo(x - 20, y);
        ctx.lineTo(x + 20, y);
        ctx.moveTo(x, y - 20);
        ctx.lineTo(x, y + 20);
        ctx.stroke();

        ctx.fillStyle = '#ff4757';
        ctx.beginPath();
        ctx.arc(x, y, 2, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
    }
}

window.GunbloodDuel = GunbloodDuel;
