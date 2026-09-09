// File Manager & Storage System for DevilutionX WebAssembly
(function() {
	const modal = document.getElementById('fileManagerModal');
	const fileManagerBtn = document.getElementById('fileManagerBtn');
	const closeModalBtn = document.getElementById('closeModal');
	const dropZone = document.getElementById('dropZone');
	const fileInput = document.getElementById('fileInput');
	const browseBtn = document.getElementById('browseBtn');
	const resetSettingsBtn = document.getElementById('resetSettingsBtn');
	const purgeMpqsBtn = document.getElementById('purgeMpqsBtn');
	const factoryResetBtn = document.getElementById('factoryResetBtn');
	const mpqFilesList = document.getElementById('mpqFilesList');
	const saveFilesList = document.getElementById('saveFilesList');
	const hellfireStatusEl = document.getElementById('hellfireStatus');
	const gameModeDiabloBtn = document.getElementById('gameModeDiabloBtn');
	const gameModeHellfireBtn = document.getElementById('gameModeHellfireBtn');

	// Open/close modal
	if (fileManagerBtn) {
		fileManagerBtn.addEventListener('click', () => {
			if (modal) modal.classList.add('show');
			refreshFileList();
		});
	}

	if (closeModalBtn) {
		closeModalBtn.addEventListener('click', () => {
			if (modal) modal.classList.remove('show');
		});
	}

	if (modal) {
		modal.addEventListener('click', (e) => {
			if (e.target === modal) {
				modal.classList.remove('show');
			}
		});
	}

	// Browse button
	if (browseBtn && fileInput) {
		browseBtn.addEventListener('click', () => {
			fileInput.click();
		});
	}

	// Drag and drop zone
	if (dropZone && fileInput) {
		dropZone.addEventListener('click', () => {
			fileInput.click();
		});

		dropZone.addEventListener('dragover', (e) => {
			e.preventDefault();
			dropZone.classList.add('dragover');
		});

		dropZone.addEventListener('dragleave', () => {
			dropZone.classList.remove('dragover');
		});

		dropZone.addEventListener('drop', (e) => {
			e.preventDefault();
			dropZone.classList.remove('dragover');
			handleFiles(e.dataTransfer.files);
		});
	}

	if (fileInput) {
		fileInput.addEventListener('change', (e) => {
			handleFiles(e.target.files);
		});
	}

	// Make handleFiles globally accessible for window-level drag & drop
	window.handleDevilutionFiles = handleFiles;

	// Handle file upload (MPQ data and .sv/.hsv/.ini save files)
	function handleFiles(files) {
		if (!files || files.length === 0) return;

		// Wait for Module and FS to be ready
		if (typeof Module === 'undefined' || typeof FS === 'undefined') {
			alert('El motor de juego aún se está cargando. Por favor espera unos segundos y reintenta.');
			return;
		}

		const validFiles = Array.from(files).filter(f => {
			const name = f.name.toLowerCase();
			return name.endsWith('.mpq') || name.endsWith('.sv') || name.endsWith('.hsv') || name.endsWith('.dsv') || name.endsWith('.ini');
		});

		if (validFiles.length === 0) {
			alert('Por favor selecciona archivos válidos (.mpq, .sv, .hsv, .dsv, .ini).');
			return;
		}

		// Ensure persistent target directory exists
		try { FS.mkdir('/libsdl'); } catch (e) {}
		try { FS.mkdir('/libsdl/diasurgical'); } catch (e) {}
		try { FS.mkdir('/libsdl/diasurgical/devilution'); } catch (e) {}

		let processed = 0;
		let errors = [];

		validFiles.forEach(file => {
			const reader = new FileReader();
			reader.onload = function(e) {
				try {
					const data = new Uint8Array(e.target.result);
					const lowerName = file.name.toLowerCase();
					// Normalize MPQs strictly to lowercase for Unix/Emscripten case sensitivity
					const destFilename = lowerName.endsWith('.mpq') ? lowerName : file.name;
					const destPath = '/libsdl/diasurgical/devilution/' + destFilename;

					// Remove any conflicting uppercase or mixed-case variant from IDBFS directory
					try {
						const existing = FS.readdir('/libsdl/diasurgical/devilution');
						existing.forEach(f => {
							if (f.toLowerCase() === lowerName && f !== destFilename) {
								try { FS.unlink('/libsdl/diasurgical/devilution/' + f); } catch (err) {}
							}
						});
					} catch (err) {}

					// Write single normalized file to IDBFS persistent directory
					FS.writeFile(destPath, data);

					// Mirror to virtual RAM root
					try { FS.writeFile('/' + destFilename, data); } catch (err) {}
					if (lowerName === 'diabdat.mpq') {
						try { FS.writeFile('/DIABDAT.MPQ', data); } catch (err) {}
					}

					// Clear deleted spawn flag if user uploaded a new spawn.mpq
					if (lowerName === 'spawn.mpq') {
						try { localStorage.removeItem('devilutionx_deleted_spawn'); } catch (err) {}
					}

					console.log('[File Manager] Instalado con éxito:', destFilename, '(' + formatBytes(file.size) + ')');
				} catch (err) {
					console.error('[File Manager] Error al escribir ' + file.name + ':', err);
					errors.push(file.name);
				}

				processed++;
				if (processed === validFiles.length) {
					if (typeof showToast === 'function') {
						showToast('Guardando ' + validFiles.length + ' archivo(s) en IndexedDB...');
					}
					// Persist atomic sync to IndexedDB
					FS.syncfs(false, function(err) {
						if (err) {
							console.error('[File Manager] Error sincronizando a IndexedDB:', err);
							alert('Error guardando archivos en el almacenamiento del navegador.');
						} else {
							if (errors.length > 0) {
								alert('Se cargaron ' + (validFiles.length - errors.length) + ' archivo(s). Hubo errores con: ' + errors.join(', '));
							} else {
								alert('¡' + validFiles.length + ' archivo(s) instalados con éxito! Recargando el juego...');
							}
							setTimeout(() => location.reload(), 400);
						}
					});
				}
			};
			reader.readAsArrayBuffer(file);
		});
	}

	// Refresh file list & system status
	function refreshFileList() {
		if (typeof Module === 'undefined' || typeof FS === 'undefined') {
			if (mpqFilesList) mpqFilesList.innerHTML = '<p class="info-text">El juego se está cargando...</p>';
			if (saveFilesList) saveFilesList.innerHTML = '<p class="info-text">El juego se está cargando...</p>';
			return;
		}

		try {
			// Check if devilution directory exists
			try {
				FS.stat('/libsdl/diasurgical/devilution');
			} catch (e) {
				if (mpqFilesList) mpqFilesList.innerHTML = '<p class="info-text">No se encontraron archivos MPQ.</p>';
				if (saveFilesList) saveFilesList.innerHTML = '<p class="info-text">No se encontraron partidas guardadas.</p>';
				updateHellfireStatus([]);
				updateGameModeUI();
				return;
			}

			const files = FS.readdir('/libsdl/diasurgical/devilution');
			const mpqFiles = files.filter(f => f.toLowerCase().endsWith('.mpq') && f !== '.' && f !== '..');
			const saveFiles = files.filter(f => {
				const name = f.toLowerCase();
				return (name.endsWith('.sv') || name.endsWith('.hsv') || name.endsWith('.dsv')) && f !== '.' && f !== '..';
			});

			// Render MPQ list
			if (mpqFilesList) {
				if (mpqFiles.length === 0) {
					mpqFilesList.innerHTML = '<p class="info-text" style="color: #b0a080;">No se encontraron archivos MPQ en el almacenamiento local.</p>';
				} else {
					mpqFilesList.innerHTML = '';
					// Sort alphabetically
					mpqFiles.sort().forEach(filename => {
						renderFileRow(mpqFilesList, filename, false);
					});
				}
			}

			// Render Saves list
			if (saveFilesList) {
				if (saveFiles.length === 0) {
					saveFilesList.innerHTML = '<p class="info-text" style="color: #b0a080;">No hay partidas guardadas (.sv / .hsv).</p>';
				} else {
					saveFilesList.innerHTML = '';
					saveFiles.sort().forEach(filename => {
						renderFileRow(saveFilesList, filename, true);
					});
				}
			}

			// Update Hellfire Status checklist
			updateHellfireStatus(mpqFiles);

			// Update active Game Mode buttons
			updateGameModeUI();

		} catch (err) {
			console.error('[File Manager] Error al leer archivos:', err);
			if (mpqFilesList) mpqFilesList.innerHTML = '<p class="info-text">Error al leer los archivos.</p>';
		}
	}

	function updateHellfireStatus(mpqFiles) {
		if (!hellfireStatusEl) return;
		const lowerFiles = (mpqFiles || []).map(f => f.toLowerCase());

		const required = [
			{ id: 'hellfire.mpq', name: 'hellfire.mpq (Núcleo Hellfire)' },
			{ id: 'hfmonk.mpq', name: 'hfmonk.mpq (Gráficos Monje)' },
			{ id: 'hfmusic.mpq', name: 'hfmusic.mpq (Música)' },
			{ id: 'hfvoice.mpq', name: 'hfvoice.mpq (Voces y Efectos)' }
		];
		const optional = [
			{ id: 'hfbard.mpq', name: 'hfbard.mpq (Bardo)' },
			{ id: 'hfbarb.mpq', name: 'hfbarb.mpq (Bárbaro)' }
		];

		let missingRequired = [];
		let installedCount = 0;

		let html = '<div style="display:flex; flex-direction:column; gap:4px; font-size:12px;">';

		required.forEach(item => {
			const hasIt = lowerFiles.includes(item.id);
			if (hasIt) {
				installedCount++;
				html += `<div style="color:#86efac;">✔ <strong>${item.id}</strong> instalado</div>`;
			} else {
				missingRequired.push(item.id);
				html += `<div style="color:#f87171;">✖ <strong>${item.id}</strong> ausente</div>`;
			}
		});

		optional.forEach(item => {
			const hasIt = lowerFiles.includes(item.id);
			if (hasIt) {
				html += `<div style="color:#93c5fd;">✔ <strong>${item.id}</strong> (Opcional)</div>`;
			}
		});

		if (missingRequired.length === 0) {
			html = `<div style="margin-bottom:8px; color:#4ade80; font-weight:bold;">⚔️ ¡Hellfire está COMPLETO y listo para iniciar!</div>` + html;
		} else {
			html = `<div style="margin-bottom:8px; color:#fbbf24; font-weight:bold;">⚠️ Hellfire incompleto (${installedCount}/4 requeridos instalados). Faltan: ${missingRequired.join(', ')}</div>` + html;
		}
		html += '</div>';

		hellfireStatusEl.innerHTML = html;
	}

	function getCurrentGameMode() {
		try {
			const iniPath = '/libsdl/diasurgical/devilution/diablo.ini';
			const ini = new TextDecoder().decode(FS.readFile(iniPath));
			const match = ini.match(/Game Mode\s*=\s*(\w+)/i);
			if (match && match[1]) {
				return match[1].toLowerCase();
			}
		} catch (e) {}
		return 'diablo';
	}

	function updateGameModeUI() {
		const currentMode = getCurrentGameMode();
		if (gameModeDiabloBtn && gameModeHellfireBtn) {
			if (currentMode === 'hellfire') {
				gameModeHellfireBtn.style.background = 'linear-gradient(180deg, #8b181b 0%, #4a0d0e 100%)';
				gameModeHellfireBtn.style.borderColor = '#e5c158';
				gameModeHellfireBtn.style.color = '#fff';
				gameModeDiabloBtn.style.background = '#1c1814';
				gameModeDiabloBtn.style.borderColor = '#3d3222';
				gameModeDiabloBtn.style.color = '#a89878';
			} else {
				gameModeDiabloBtn.style.background = 'linear-gradient(180deg, #3d3222 0%, #201a14 100%)';
				gameModeDiabloBtn.style.borderColor = '#d4af37';
				gameModeDiabloBtn.style.color = '#fff';
				gameModeHellfireBtn.style.background = '#1c1814';
				gameModeHellfireBtn.style.borderColor = '#3d3222';
				gameModeHellfireBtn.style.color = '#a89878';
			}
		}
	}

	function renderFileRow(container, filename, isSaveFile) {
		const path = '/libsdl/diasurgical/devilution/' + filename;
		let size = 0;
		try {
			const stat = FS.stat(path);
			size = stat.size;
		} catch (e) {}

		const item = document.createElement('div');
		item.className = 'file-item';

		const nameSpan = document.createElement('span');
		nameSpan.className = 'file-item-name';
		nameSpan.textContent = filename;

		const sizeSpan = document.createElement('span');
		sizeSpan.className = 'file-item-size';
		sizeSpan.textContent = formatBytes(size);

		const actionsDiv = document.createElement('div');
		actionsDiv.style.display = 'flex';
		actionsDiv.style.alignItems = 'center';

		const exportBtn = document.createElement('button');
		exportBtn.className = 'btn btn-export';
		exportBtn.textContent = '💾 Descargar';
		exportBtn.title = 'Descargar copia de este archivo a tu PC';
		exportBtn.addEventListener('click', () => exportFile(filename));
		actionsDiv.appendChild(exportBtn);

		const deleteBtn = document.createElement('button');
		deleteBtn.className = 'btn btn-delete';
		deleteBtn.textContent = '🗑️ Eliminar';
		deleteBtn.addEventListener('click', () => window.deleteFile(filename));
		actionsDiv.appendChild(deleteBtn);

		item.appendChild(nameSpan);
		item.appendChild(sizeSpan);
		item.appendChild(actionsDiv);
		container.appendChild(item);
	}

	// Export / Download file to browser
	function exportFile(filename) {
		try {
			const path = '/libsdl/diasurgical/devilution/' + filename;
			const data = FS.readFile(path);
			const blob = new Blob([data], { type: 'application/octet-stream' });
			const url = URL.createObjectURL(blob);
			const a = document.createElement('a');
			a.href = url;
			a.download = filename;
			document.body.appendChild(a);
			a.click();
			document.body.removeChild(a);
			URL.revokeObjectURL(url);
			if (typeof showToast === 'function') {
				showToast('¡Descargando ' + filename + '!');
			}
		} catch (err) {
			console.error('[File Manager] Error exportando archivo:', err);
			alert('Error exportando archivo: ' + filename);
		}
	}

	// Robust delete: unlinks all casing variants from /libsdl/diasurgical/devilution/ and root /
	window.deleteFile = function(filename) {
		if (!confirm('¿Eliminar ' + filename + ' del almacenamiento del navegador? Esto recargará el juego.')) {
			return;
		}

		try {
			const lower = filename.toLowerCase();

			// 1. Delete all casing variants from persistent IDBFS directory
			try {
				const files = FS.readdir('/libsdl/diasurgical/devilution');
				files.forEach(f => {
					if (f.toLowerCase() === lower) {
						try { FS.unlink('/libsdl/diasurgical/devilution/' + f); } catch (e) {}
					}
				});
			} catch (err) {}

			// 2. Delete all casing variants from virtual RAM root
			try {
				const rootFiles = FS.readdir('/');
				rootFiles.forEach(f => {
					if (f.toLowerCase() === lower) {
						try { FS.unlink('/' + f); } catch (e) {}
					}
				});
			} catch (err) {}

			// 3. Mark deletion flag for spawn.mpq so server preRun does not resurrect it
			if (lower === 'spawn.mpq') {
				try { localStorage.setItem('devilutionx_deleted_spawn', 'true'); } catch (e) {}
			}

			// 4. Sync deletion to IndexedDB and reload
			FS.syncfs(false, function(err) {
				if (err) {
					console.error('[File Manager] Error sincronizando eliminación:', err);
					alert('Error al persistir la eliminación en IndexedDB.');
				} else {
					alert('¡Archivo ' + filename + ' eliminado permanentemente! Recargando...');
					setTimeout(() => location.reload(), 300);
				}
			});
		} catch (err) {
			console.error('[File Manager] Error eliminando archivo:', err);
			alert('Error eliminando archivo: ' + filename);
		}
	};

	// Purge all MPQs (keeps saves intact)
	window.purgeAllMpqs = function() {
		if (!confirm('⚠️ ¿ELIMINAR TODOS LOS ARCHIVOS MPQ? Tus partidas guardadas (.sv / .hsv) se conservarán intactas. El juego se recargará en limpio.')) {
			return;
		}

		try {
			// Unlink all .mpq in IDBFS directory
			try {
				const files = FS.readdir('/libsdl/diasurgical/devilution');
				files.forEach(f => {
					if (f.toLowerCase().endsWith('.mpq')) {
						try { FS.unlink('/libsdl/diasurgical/devilution/' + f); } catch (e) {}
					}
				});
			} catch (err) {}

			// Unlink all .mpq in virtual RAM root
			try {
				const rootFiles = FS.readdir('/');
				rootFiles.forEach(f => {
					if (f.toLowerCase().endsWith('.mpq')) {
						try { FS.unlink('/' + f); } catch (e) {}
					}
				});
			} catch (err) {}

			try { localStorage.setItem('devilutionx_deleted_spawn', 'true'); } catch (e) {}

			FS.syncfs(false, function(err) {
				alert('¡Todos los archivos MPQ han sido eliminados de IndexedDB! Recargando...');
				setTimeout(() => location.reload(), 300);
			});
		} catch (err) {
			console.error('[File Manager] Error purgando MPQs:', err);
			alert('Error al purgar MPQs: ' + err.message);
		}
	};

	if (purgeMpqsBtn) {
		purgeMpqsBtn.addEventListener('click', window.purgeAllMpqs);
	}

	// Factory reset: completely purges IndexedDB database and clears cache
	window.factoryResetAll = function() {
		if (!confirm('🚨 ATENCIÓN: ¿Restablecimiento total de fábrica? Esto BORRARÁ TODOS LOS DATOS: MPQs, configuraciones y partidas guardadas de IndexedDB para solucionar cualquier corrupción o datos antiguos.')) {
			return;
		}

		try {
			localStorage.clear();
			sessionStorage.clear();

			const req = indexedDB.deleteDatabase('/libsdl');
			req.onsuccess = function() {
				alert('¡Almacenamiento e IndexedDB restablecidos de fábrica al 100%! Recargando...');
				location.reload();
			};
			req.onerror = function() {
				// Fallback: manually unlink everything
				try {
					const files = FS.readdir('/libsdl/diasurgical/devilution');
					files.forEach(f => {
						if (f !== '.' && f !== '..') {
							try { FS.unlink('/libsdl/diasurgical/devilution/' + f); } catch (e) {}
						}
					});
					FS.syncfs(false, function() {
						alert('¡Datos restablecidos! Recargando...');
						location.reload();
					});
				} catch (e) {
					location.reload();
				}
			};
			req.onblocked = function() {
				alert('Por favor cierra otras pestañas abiertas de DevilutionX para finalizar el reseteo.');
				location.reload();
			};
		} catch (e) {
			console.error('Error en factory reset:', e);
			alert('Error al resetear: ' + e.message);
			location.reload();
		}
	};

	if (factoryResetBtn) {
		factoryResetBtn.addEventListener('click', window.factoryResetAll);
	}

	// Game Mode switcher (Diablo 1 vs Hellfire)
	window.setGameMode = function(mode) {
		// mode: 'Diablo' or 'Hellfire'
		try {
			const iniPath = '/libsdl/diasurgical/devilution/diablo.ini';
			let currentIni = '';
			try {
				currentIni = new TextDecoder().decode(FS.readFile(iniPath));
			} catch (e) {}

			if (!currentIni || currentIni.trim().length === 0) {
				currentIni = "[Game]\nTest Barbarian=1\nTest Bard=1\nRun in Town=1\nCow Quest=1\nTheo Quest=1\nGame Mode=" + mode + "\n";
			} else {
				if (currentIni.indexOf('Game Mode=') !== -1) {
					currentIni = currentIni.replace(/Game Mode\s*=\s*\w+/i, 'Game Mode=' + mode);
				} else if (currentIni.indexOf('[Game]') !== -1) {
					currentIni = currentIni.replace('[Game]', "[Game]\nGame Mode=" + mode);
				} else {
					currentIni += "\n[Game]\nGame Mode=" + mode + "\n";
				}
			}

			FS.writeFile(iniPath, currentIni);
			FS.syncfs(false, function(err) {
				alert('¡Modo de juego configurado a ' + (mode === 'Hellfire' ? '🔥 Hellfire' : '⚔️ Diablo 1') + '! Recargando...');
				setTimeout(() => location.reload(), 300);
			});
		} catch (err) {
			console.error('[File Manager] Error configurando modo de juego:', err);
			alert('Error al cambiar el modo de juego.');
		}
	};

	if (gameModeDiabloBtn) {
		gameModeDiabloBtn.addEventListener('click', () => window.setGameMode('Diablo'));
	}
	if (gameModeHellfireBtn) {
		gameModeHellfireBtn.addEventListener('click', () => window.setGameMode('Hellfire'));
	}

	// Reset settings (diablo.ini)
	if (resetSettingsBtn) {
		resetSettingsBtn.addEventListener('click', () => {
			if (!confirm('¿Restablecer la configuración gráfica? Esto reescribirá diablo.ini con valores por defecto pero conservará todos tus MPQs y partidas guardadas. El juego se recargará.')) {
				return;
			}

			try {
				const iniPath1 = '/libsdl/diasurgical/devilution/diablo.ini';
				const iniPath2 = '/libsdl/diasurgical/diablo.ini';

				const defaultIni = "[Game]\nTest Barbarian=1\nTest Bard=1\nRun in Town=1\nCow Quest=1\nTheo Quest=1\n";
				try { FS.writeFile(iniPath1, defaultIni); } catch (e) {}
				try { FS.writeFile(iniPath2, defaultIni); } catch (e) {}
				try { FS.unlink('/diablo.ini'); } catch (e) {}

				FS.syncfs(false, function(err) {
					if (err) {
						console.error('[File Manager] Error sincronizando reset de settings:', err);
					}
					alert('¡Configuración restablecida con Bárbaro y Bardo activos! Recargando juego...');
					setTimeout(() => location.reload(), 300);
				});
			} catch (err) {
				console.error('[File Manager] Error resetting settings:', err);
				alert('Error al restablecer la configuración.');
			}
		});
	}

	// Helper function
	function formatBytes(bytes) {
		if (bytes === 0) return '0 Bytes';
		const k = 1024;
		const sizes = ['Bytes', 'KB', 'MB', 'GB'];
		const i = Math.floor(Math.log(bytes) / Math.log(k));
		return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
	}
})();
