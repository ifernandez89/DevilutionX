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

		// Collect existing files in IDBFS directory
		let existingFiles = [];
		try {
			existingFiles = FS.readdir('/libsdl/diasurgical/devilution') || [];
		} catch (e) {}

		// Track occupied files in lowercase
		const occupiedFiles = new Set(existingFiles.map(f => f.toLowerCase()));

		// Check if retail diabdat.mpq is present or being uploaded in this batch
		const hasDiabdat = existingFiles.some(f => f.toLowerCase() === 'diabdat.mpq') ||
		                   validFiles.some(f => f.name.toLowerCase() === 'diabdat.mpq');
		const hasHellfireMpq = existingFiles.some(f => f.toLowerCase() === 'hellfire.mpq') ||
		                       validFiles.some(f => f.name.toLowerCase() === 'hellfire.mpq');
		const hasHsvFiles = validFiles.some(f => f.name.toLowerCase().endsWith('.hsv'));
		const hasRetailSaves = validFiles.some(f => {
			const n = f.name.toLowerCase();
			return (n.startsWith('single_') || n.startsWith('multi_')) && (n.endsWith('.sv') || n.endsWith('.hsv') || n.endsWith('.dsv'));
		});

		// Auto-configure Hellfire in diablo.ini if .hsv files are uploaded
		if (hasHsvFiles) {
			try {
				const iniPath = '/libsdl/diasurgical/devilution/diablo.ini';
				let iniText = '';
				try { iniText = new TextDecoder().decode(FS.readFile(iniPath)); } catch (e) {}
				if (!iniText || iniText.trim().length === 0) {
					iniText = "[Game]\nTest Barbarian=1\nTest Bard=1\nRun in Town=1\nCow Quest=1\nTheo Quest=1\n";
				}
				// StartUpGameMode enum: Hellfire=1, Diablo=2
				iniText = setIniKey(iniText, 'GameMode', 'Game', '1');
				iniText = setIniKey(iniText, 'Mods', 'Hellfire', '1');
				iniText = setIniKey(iniText, 'Mods', 'clock', '1');
				FS.writeFile(iniPath, iniText);
				try { FS.writeFile('/diablo.ini', iniText); } catch (e) {}
				console.log('[File Manager] Auto-configurado diablo.ini a modo Hellfire por carga de archivo .hsv');
			} catch (err) {
				console.warn('[File Manager] No se pudo auto-configurar diablo.ini a Hellfire:', err);
			}
		}

		function getNextFreeSlot(prefix, ext) {
			for (let i = 0; i < 99; i++) {
				const candidate = (prefix + i + '.' + ext).toLowerCase();
				if (!occupiedFiles.has(candidate)) {
					occupiedFiles.add(candidate);
					return i;
				}
			}
			return 98;
		}

		let processed = 0;
		let errors = [];
		let installedSummaries = [];

		validFiles.forEach(file => {
			const reader = new FileReader();
			reader.onload = function(e) {
				try {
					const data = new Uint8Array(e.target.result);
					const lowerName = file.name.toLowerCase();
					let destFilename = lowerName;

					if (lowerName.endsWith('.mpq')) {
						destFilename = lowerName; // Strictly lowercase for Unix/Emscripten case sensitivity
					} else if (lowerName.endsWith('.ini')) {
						destFilename = 'diablo.ini';
					} else {
						// Save game file (.sv, .hsv, .dsv)
						let ext = 'sv';
						if (lowerName.endsWith('.hsv')) {
							ext = 'hsv';
						} // .dsv and .sv both normalize to .sv

						// Determine prefix
						let prefix = 'single_';
						if (lowerName.startsWith('multi_')) prefix = 'multi_';
						else if (lowerName.startsWith('share_')) prefix = 'share_';
						else if (lowerName.startsWith('spawn_')) prefix = 'spawn_';
						else if (!hasDiabdat && !lowerName.startsWith('single_')) prefix = 'spawn_';

						// Match slot number: e.g. single_0.sv, single_1 (1).sv, etc.
						const standardMatch = lowerName.match(/^(?:single|spawn|multi|share)_(\d+)\.(?:sv|hsv|dsv)$/);
						const copyMatch = lowerName.match(/^(?:single|spawn|multi|share)_(\d+)\s*\(\d+\)\.(?:sv|hsv|dsv)$/);

						let chosenSlot = null;

						if (standardMatch) {
							const requestedSlot = parseInt(standardMatch[1], 10);
							const candidate = (prefix + requestedSlot + '.' + ext).toLowerCase();

							if (occupiedFiles.has(candidate)) {
								const confirmOverwrite = confirm(
									`Ya existe una partida guardada en la casilla ${requestedSlot} (${prefix}${requestedSlot}.${ext}).\n\n` +
									`• Pulsa ACEPTAR para SOBRESCRIBIR la casilla ${requestedSlot}.\n` +
									`• Pulsa CANCELAR para GUARDARLA EN UNA CASILLA LIBRE sin perder tu partida actual.`
								);
								if (confirmOverwrite) {
									chosenSlot = requestedSlot;
								} else {
									chosenSlot = getNextFreeSlot(prefix, ext);
								}
							} else {
								chosenSlot = requestedSlot;
								occupiedFiles.add(candidate);
							}
						} else if (copyMatch) {
							// Downloaded duplicate copy like "single_0 (1).sv": auto-assign to free slot
							chosenSlot = getNextFreeSlot(prefix, ext);
						} else {
							// Arbitrary name (e.g. "guerrero.sv", "save.dsv"): auto-assign to free slot
							chosenSlot = getNextFreeSlot(prefix, ext);
						}

						destFilename = `${prefix}${chosenSlot}.${ext}`;
					}

					const destPath = '/libsdl/diasurgical/devilution/' + destFilename;

					// Remove any conflicting uppercase or mixed-case variant from IDBFS directory
					try {
						const currentFiles = FS.readdir('/libsdl/diasurgical/devilution');
						currentFiles.forEach(f => {
							if (f.toLowerCase() === destFilename.toLowerCase() && f !== destFilename) {
								try { FS.unlink('/libsdl/diasurgical/devilution/' + f); } catch (err) {}
							}
						});
					} catch (err) {}

					// Write normalized file to IDBFS persistent directory
					FS.writeFile(destPath, data);

					// Mirror to virtual RAM root
					try { FS.writeFile('/' + destFilename, data); } catch (err) {}
					if (lowerName === 'diabdat.mpq') {
						try { FS.writeFile('/DIABDAT.MPQ', data); } catch (err) {}
					}

					// If running in spawn mode, also mirror single_ to spawn_ as an extra compatibility fallback
					if (!hasDiabdat && destFilename.startsWith('single_')) {
						const spawnFilename = 'spawn_' + destFilename.substring('single_'.length);
						try {
							FS.writeFile('/libsdl/diasurgical/devilution/' + spawnFilename, data);
							try { FS.writeFile('/' + spawnFilename, data); } catch (e) {}
							console.log('[File Manager] Clonado de compatibilidad para modo spawn:', spawnFilename);
						} catch (e) {}
					}

					// Clear deleted spawn flag if user uploaded a new spawn.mpq
					if (lowerName === 'spawn.mpq') {
						try { localStorage.removeItem('devilutionx_deleted_spawn'); } catch (err) {}
					}

					installedSummaries.push(file.name + (file.name !== destFilename ? ' ➔ ' + destFilename : ''));
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
					window.syncInProgress = true;
					// Persist atomic sync to IndexedDB
					FS.syncfs(false, function(err) {
						window.syncInProgress = false;
						if (err) {
							console.error('[File Manager] Error sincronizando a IndexedDB:', err);
							alert('Error guardando archivos en el almacenamiento del navegador.');
						} else {
							let warnings = [];
							if (hasHsvFiles && !hasHellfireMpq) {
								warnings.push('⚠️ AVISO HELLFIRE: Has subido partidas Hellfire (.hsv), pero no se detectaron los archivos de Hellfire (hellfire.mpq, hfmonk.mpq, etc.). Recuerda subirlos para que el juego pueda cargar tus personajes de Hellfire.');
							}
							if (hasRetailSaves && !hasDiabdat) {
								warnings.push('⚠️ AVISO PARTIDA PC: Has subido partidas de la versión comercial de PC (single_*). Sin DIABDAT.MPQ en el navegador, el juego corre en modo Demo/Shareware y podría no desencriptar los personajes de PC.');
							}

							let msg = '';
							if (errors.length > 0) {
								msg = 'Se cargaron ' + (validFiles.length - errors.length) + ' archivo(s). Hubo errores con: ' + errors.join(', ');
							} else {
								msg = '¡' + validFiles.length + ' archivo(s) instalados y normalizados con éxito!\n\n' +
								      installedSummaries.map(s => '• ' + s).join('\n');
							}
							if (warnings.length > 0) {
								msg += '\n\n' + warnings.join('\n\n');
							}
							msg += '\n\nRecargando el juego...';
							alert(msg);
							setTimeout(() => location.reload(), 300);
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

	function setIniKey(iniText, section, key, value) {
		let lines = (iniText || '').split(/\r?\n/);
		let targetSection = '[' + section.toLowerCase() + ']';
		let currentSection = '';
		let sectionFound = false;
		let keyFound = false;
		let newLines = [];

		for (let i = 0; i < lines.length; i++) {
			let line = lines[i];
			let trimmed = line.trim();
			if (trimmed.startsWith('[') && trimmed.endsWith(']')) {
				if (currentSection === targetSection && !keyFound) {
					newLines.push(key + '=' + value);
					keyFound = true;
				}
				currentSection = trimmed.toLowerCase();
				if (currentSection === targetSection) {
					sectionFound = true;
				}
			} else if (currentSection === targetSection && trimmed.length > 0 && !trimmed.startsWith(';') && !trimmed.startsWith('#')) {
				let eqIdx = line.indexOf('=');
				if (eqIdx !== -1) {
					let k = line.substring(0, eqIdx).trim();
					if (k.toLowerCase() === key.toLowerCase()) {
						line = key + '=' + value;
						keyFound = true;
					}
				}
			}
			newLines.push(line);
		}

		if (currentSection === targetSection && !keyFound) {
			newLines.push(key + '=' + value);
			keyFound = true;
		}

		if (!sectionFound) {
			if (newLines.length > 0 && newLines[newLines.length - 1].trim().length > 0) {
				newLines.push('');
			}
			newLines.push('[' + section + ']');
			newLines.push(key + '=' + value);
		}

		return newLines.join('\n');
	}

	function getCurrentGameMode() {
		// StartUpGameMode enum: Ask=0, Hellfire=1, Diablo=2
		try {
			const iniPath = '/libsdl/diasurgical/devilution/diablo.ini';
			const ini = new TextDecoder().decode(FS.readFile(iniPath));
			const gmMatch = ini.match(/\[GameMode\][\s\S]*?^Game\s*=\s*(\d+)/im);
			if (gmMatch && gmMatch[1]) {
				return gmMatch[1] === '1' ? 'hellfire' : 'diablo';
			}
			const modMatch = ini.match(/\[Mods\][\s\S]*?^Hellfire\s*=\s*(\d+)/im);
			if (modMatch && modMatch[1] === '1') {
				return 'hellfire';
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

		let displayName = filename;
		if (isSaveFile) {
			const m = filename.toLowerCase().match(/^(single|spawn|multi|share)_(\d+)\.(sv|hsv)$/);
			if (m) {
				const modeLabel = (m[1] === 'single' ? 'Un Jugador' : (m[1] === 'spawn' ? 'Shareware' : 'Multijugador'));
				const expLabel = (m[3] === 'hsv' ? '🔥 Hellfire' : '⚔️ Diablo');
				displayName = `${filename} (${modeLabel} • Casilla ${m[2]} • ${expLabel})`;
			}
		}
		nameSpan.textContent = displayName;

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

			const req1 = indexedDB.deleteDatabase('/libsdl/diasurgical');
			const req2 = indexedDB.deleteDatabase('/libsdl');

			let doneCount = 0;
			function checkDone() {
				doneCount++;
				if (doneCount >= 2) {
					alert('¡Almacenamiento e IndexedDB restablecidos de fábrica al 100%! Recargando...');
					location.reload();
				}
			}

			req1.onsuccess = checkDone;
			req1.onerror = checkDone;
			req2.onsuccess = checkDone;
			req2.onerror = checkDone;
			req1.onblocked = function() {
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
				currentIni = "[Game]\nTest Barbarian=1\nTest Bard=1\nRun in Town=1\nCow Quest=1\nTheo Quest=1\n";
			}

			// Configure canonical C++ DevilutionX options
			// StartUpGameMode enum: Hellfire=1, Diablo=2
			currentIni = setIniKey(currentIni, 'GameMode', 'Game', mode === 'Hellfire' ? '1' : '2');
			currentIni = setIniKey(currentIni, 'Mods', 'Hellfire', mode === 'Hellfire' ? '1' : '0');
			currentIni = setIniKey(currentIni, 'Mods', 'clock', '1');

			FS.writeFile(iniPath, currentIni);
			try { FS.writeFile('/diablo.ini', currentIni); } catch (e) {}

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

				const defaultIni = "[Game]\nTest Barbarian=1\nTest Bard=1\nRun in Town=1\nCow Quest=1\nTheo Quest=1\n\n[Mods]\nclock=1\n";
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
