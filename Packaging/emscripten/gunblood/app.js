/**
 * Gunblood Web UI Controller
 */

class GunbloodUI {
    constructor() {
        this.game = null;

        // Elements
        this.canvas = document.getElementById('duelCanvas');
        this.modal = document.getElementById('duelModal');
        this.modalContent = document.getElementById('modalContent');
        this.promptBanner = document.getElementById('promptBanner');
        
        // HUD Elements
        this.playerHPFill = document.getElementById('playerHPFill');
        this.enemyHPFill = document.getElementById('enemyHPFill');
        this.enemyNameHUD = document.getElementById('enemyNameHUD');
        this.scoreDisplay = document.getElementById('scoreDisplay');
        this.bountyDisplay = document.getElementById('bountyDisplay');

        this.init();
    }

    init() {
        this.game = new window.GunbloodDuel(this.canvas, this);
        this.showTitleScreen();
    }

    showTitleScreen() {
        this.modal.classList.remove('hidden');
        this.promptBanner.style.display = 'none';

        this.modalContent.innerHTML = `
            <div class="wanted-poster">
                <div class="wanted-title">WANTED</div>
                <div class="wanted-name" style="font-size: 1.8rem; color: #b71540;">DEAD OR ALIVE</div>
                <p style="margin: 1rem 0; font-size: 0.95rem; line-height: 1.5;">
                    Demuestra que eres el pistolero más rápido del salvaje oeste.<br>
                    Coloca tu cursor en el tambor de tu revólver y dispara al escuchar el <b>¡¡FUEGO!!</b>
                </p>
                <button class="btn-start-duel" id="btnStartGame">🔥 INICIAR DUELOS</button>
            </div>
        `;

        document.getElementById('btnStartGame').addEventListener('click', () => {
            window.soundEngine.init();
            this.modal.classList.add('hidden');
            this.game.startNewGame();
        });
    }

    showReadyPrompt(levelTitle, enemyQuote) {
        this.modal.classList.remove('hidden');
        this.promptBanner.style.display = 'none';

        const enemy = this.game.currentEnemy;
        this.modalContent.innerHTML = `
            <div class="wanted-poster">
                <div class="wanted-title">WANTED</div>
                <div class="wanted-name">${enemy.name}</div>
                <div style="font-size: 0.85rem; color: #795548; font-weight: 700; text-transform: uppercase;">${enemy.title}</div>
                <div class="wanted-bounty">RECOMPENSA: ${enemy.bounty}</div>
                <div class="wanted-quote">"${enemy.quote}"</div>
                <div style="margin-top: 1.25rem;">
                    <button class="btn-start-duel" id="btnBeginDuel">⚔️ ENFRENTAR</button>
                </div>
            </div>
        `;

        document.getElementById('btnBeginDuel').addEventListener('click', () => {
            window.soundEngine.init();
            this.modal.classList.add('hidden');
            this.promptBanner.style.display = 'block';
            this.promptBanner.innerText = '👇 COLOCA EL CURSOR EN EL TAMBOR DE BALAS';
        });
    }

    showFoulMessage(msg) {
        this.promptBanner.style.display = 'block';
        this.promptBanner.innerText = `⚠️ ${msg}`;
    }

    hidePrompt() {
        this.promptBanner.style.display = 'none';
    }

    updateHUD(game) {
        const pPct = Math.max(0, game.playerHP);
        const ePct = Math.max(0, (game.enemyHP / (game.currentEnemy ? game.currentEnemy.hp : 100)) * 100);

        this.playerHPFill.style.width = `${pPct}%`;
        this.enemyHPFill.style.width = `${ePct}%`;

        if (pPct < 35) this.playerHPFill.classList.add('danger');
        else this.playerHPFill.classList.remove('danger');

        if (ePct < 35) this.enemyHPFill.classList.add('danger');
        else this.enemyHPFill.classList.remove('danger');

        if (game.currentEnemy) {
            this.enemyNameHUD.innerText = game.currentEnemy.name;
            this.bountyDisplay.innerText = `RECOMPENSA: ${game.currentEnemy.bounty}`;
        }
        this.scoreDisplay.innerText = `$${game.score}`;
    }

    showVictoryModal(data) {
        this.modal.classList.remove('hidden');
        this.promptBanner.style.display = 'none';

        this.modalContent.innerHTML = `
            <div class="wanted-poster" style="border-color: #2ed573;">
                <div class="wanted-title" style="color: #2ed573; border-color: #2ed573;">¡VICTORIA!</div>
                <div class="wanted-name">Has abatido a ${data.enemyName}</div>
                <div style="margin: 1rem 0; font-size: 0.95rem; line-height: 1.8; text-align: left; background: rgba(0,0,0,0.05); padding: 1rem; border-radius: 8px;">
                    ⚡ <b>Tiempo de Reacción:</b> <span style="color: #ff4757; font-weight: 800;">${data.reactionTime}s</span><br>
                    ⏱️ <b>Bonus de Velocidad:</b> +$${data.timeBonus}<br>
                    ❤️ <b>Bonus de Salud:</b> +$${data.hpBonus}<br>
                    💰 <b>Recompensa Cobrada:</b> ${data.bounty}<br>
                    ⭐ <b>Puntuación Total:</b> <span style="color: #b7791f; font-weight: 900;">$${data.totalScore}</span>
                </div>
                <button class="btn-start-duel" id="btnNextDuel">SIGUIENTE FORAJIDO ▶</button>
            </div>
        `;

        document.getElementById('btnNextDuel').addEventListener('click', () => {
            this.modal.classList.add('hidden');
            data.nextLevel();
        });
    }

    showGameOverModal(data) {
        this.modal.classList.remove('hidden');
        this.promptBanner.style.display = 'none';

        this.modalContent.innerHTML = `
            <div class="wanted-poster" style="border-color: #ff4757;">
                <div class="wanted-title" style="color: #ff4757; border-color: #ff4757;">CAÍDO EN DUELO</div>
                <div class="wanted-name">${data.enemyName} fue más rápido</div>
                <div class="wanted-quote" style="margin: 1rem 0;">"${data.quote}"</div>
                <div style="margin-bottom: 1.25rem; font-size: 1.1rem; font-weight: 800;">
                    Puntuación Final: <span style="color: #b7791f;">$${data.score}</span>
                </div>
                <button class="btn-start-duel" id="btnRetryDuel">🔄 REINTENTAR DUELO</button>
            </div>
        `;

        document.getElementById('btnRetryDuel').addEventListener('click', () => {
            this.modal.classList.add('hidden');
            data.retry();
        });
    }

    showBonusNotification() {
        this.modal.classList.remove('hidden');
        this.modalContent.innerHTML = `
            <div class="wanted-poster" style="border-color: #00d2d3;">
                <div class="wanted-title" style="color: #00d2d3; border-color: #00d2d3;">🎯 RONDA BONUS</div>
                <div class="wanted-name">Práctica de Puntería</div>
                <p style="margin: 1rem 0; font-size: 0.95rem; line-height: 1.5;">
                    ¡Dispara a las botellas de whisky y cartuchos de dinamita que vuelan por el aire para ganar dinero extra!
                </p>
                <button class="btn-start-duel" id="btnStartBonus" style="background: linear-gradient(135deg, #00d2d3, #0984e3);">🎯 ¡A DISPARAR!</button>
            </div>
        `;

        document.getElementById('btnStartBonus').addEventListener('click', () => {
            this.modal.classList.add('hidden');
        });
    }

    updateBonusTimer(seconds) {
        this.promptBanner.style.display = 'block';
        this.promptBanner.innerText = `🎯 TIEMPO RESTANTE: ${seconds}s`;
    }

    showBonusSummary(bonusScore, onContinue) {
        this.modal.classList.remove('hidden');
        this.promptBanner.style.display = 'none';

        this.modalContent.innerHTML = `
            <div class="wanted-poster" style="border-color: #2ed573;">
                <div class="wanted-title" style="color: #2ed573; border-color: #2ed573;">¡TIEMPO AGOTADO!</div>
                <div class="wanted-name">Puntos Bonus Ganados</div>
                <div class="wanted-bounty" style="color: #2ed573; margin: 1rem 0;">+$${bonusScore}</div>
                <button class="btn-start-duel" id="btnContinueAfterBonus">CONTINUAR DUELOS ▶</button>
            </div>
        `;

        document.getElementById('btnContinueAfterBonus').addEventListener('click', () => {
            this.modal.classList.add('hidden');
            onContinue();
        });
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.ui = new GunbloodUI();
});
