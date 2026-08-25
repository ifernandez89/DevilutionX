// File Manager functionality
(function() {
	const modal = document.getElementById('fileManagerModal');
	const fileManagerBtn = document.getElementById('fileManagerBtn');
	const closeModalBtn = document.getElementById('closeModal');
	const dropZone = document.getElementById('dropZone');
	const fileInput = document.getElementById('fileInput');
	const browseBtn = document.getElementById('browseBtn');
	const resetSettingsBtn = document.getElementById('resetSettingsBtn');
	const mpqFilesList = document.getElementById('mpqFilesList');
	const saveFilesList = document.getElementById('saveFilesList');

	// Open/close modal
	fileManagerBtn.addEventListener('click', () => {
		modal.classList.add('show');
		refreshFileList();
	});

	closeModalBtn.addEventListener('click', () => {
		modal.classList.remove('show');
	});

	modal.addEventListener('click', (e) => {
		if (e.target === modal) {
			modal.classList.remove('show');
		}
	});

	// Browse button
	browseBtn.addEventListener('click', () => {
		fileInput.click();
	});

	// Drag and drop
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

	fileInput.addEventListener('change', (e) => {
		handleFiles(e.target.files);
	});

	// Handle file upload (MPQ data and .sv/.dsv/.ini save files)
	function handleFiles(files) {
		if (!files || files.length === 0) return;

		// Wait for Module and FS to be ready
		if (typeof Module === 'undefined' || typeof FS === 'undefined') {
			alert('Game is still loading. Please wait and try again.');
			return;
		}

		const validFiles = Array.from(files).filter(f => {
			const name = f.name.toLowerCase();
			return name.endsWith('.mpq') || name.endsWith('.sv') || name.endsWith('.dsv') || name.endsWith('.ini');
		});

		if (validFiles.length === 0) {
			alert('Por favor selecciona archivos válidos (.mpq, .sv, .dsv, .ini).');
			return;
		}

		let processed = 0;
		validFiles.forEach(file => {
			const reader = new FileReader();
			reader.onload = function(e) {
				try {
					const data = new Uint8Array(e.target.result);
					const path = '/libsdl/diasurgical/devilution/' + file.name;

					// Create directory if it doesn't exist
					try {
						FS.mkdir('/libsdl/diasurgical/devilution');
					} catch (e) {
						// Directory might already exist, ignore
					}

					// Write file to IDBFS-backed directory
					FS.writeFile(path, data);
					console.log('Uploaded:', file.name, '(' + formatBytes(file.size) + ')');

					processed++;
					if (processed === validFiles.length) {
						// Sync to IndexedDB
						FS.syncfs(false, function(err) {
							if (err) {
								console.error('Error syncing files:', err);
								alert('Error guardando archivos. Revisa la consola.');
							} else {
								alert('¡Archivos cargados con éxito! Recargando el juego...');
								setTimeout(() => location.reload(), 500);
							}
						});
					}
				} catch (err) {
					console.error('Error writing file:', err);
					alert('Error al subir el archivo: ' + file.name);
				}
			};
			reader.readAsArrayBuffer(file);
		});
	}

	// Refresh file list
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
				return;
			}

			const files = FS.readdir('/libsdl/diasurgical/devilution');
			const mpqFiles = files.filter(f => f.toLowerCase().endsWith('.mpq') && f !== '.' && f !== '..');
			const saveFiles = files.filter(f => {
				const name = f.toLowerCase();
				return (name.endsWith('.sv') || name.endsWith('.dsv') || name.endsWith('.ini')) && f !== '.' && f !== '..';
			});

			// Render MPQ list
			if (mpqFilesList) {
				if (mpqFiles.length === 0) {
					mpqFilesList.innerHTML = '<p class="info-text">No se encontraron archivos MPQ.</p>';
				} else {
					mpqFilesList.innerHTML = '';
					mpqFiles.forEach(filename => {
						renderFileRow(mpqFilesList, filename, false);
					});
				}
			}

			// Render Saves list
			if (saveFilesList) {
				if (saveFiles.length === 0) {
					saveFilesList.innerHTML = '<p class="info-text">No hay partidas guardadas (.sv / .dsv).</p>';
				} else {
					saveFilesList.innerHTML = '';
					saveFiles.forEach(filename => {
						renderFileRow(saveFilesList, filename, true);
					});
				}
			}
		} catch (err) {
			console.error('Error reading files:', err);
			if (mpqFilesList) mpqFilesList.innerHTML = '<p class="info-text">Error al leer los archivos.</p>';
		}
	}

	function renderFileRow(container, filename, isSaveFile) {
		const path = '/libsdl/diasurgical/devilution/' + filename;
		const stat = FS.stat(path);

		const item = document.createElement('div');
		item.className = 'file-item';

		const nameSpan = document.createElement('span');
		nameSpan.className = 'file-item-name';
		nameSpan.textContent = filename;

		const sizeSpan = document.createElement('span');
		sizeSpan.className = 'file-item-size';
		sizeSpan.textContent = formatBytes(stat.size);

		const actionsDiv = document.createElement('div');
		actionsDiv.style.display = 'flex';
		actionsDiv.style.alignItems = 'center';

		if (isSaveFile) {
			const exportBtn = document.createElement('button');
			exportBtn.className = 'btn btn-export';
			exportBtn.textContent = '💾 Descargar';
			exportBtn.title = 'Descargar copia de seguridad a tu PC';
			exportBtn.addEventListener('click', () => exportFile(filename));
			actionsDiv.appendChild(exportBtn);
		}

		const deleteBtn = document.createElement('button');
		deleteBtn.className = 'btn btn-delete';
		deleteBtn.textContent = 'Eliminar';
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
		} catch (err) {
			console.error('Error exporting file:', err);
			alert('Error exportando partida: ' + filename);
		}
	}

	// Delete file
	window.deleteFile = function(filename) {
		if (!confirm('¿Eliminar ' + filename + '? Esto recargará el juego.')) {
			return;
		}

		try {
			const path = '/libsdl/diasurgical/devilution/' + filename;
			FS.unlink(path);

			// Sync deletion to IndexedDB
			FS.syncfs(false, function(err) {
				if (err) {
					console.error('Error syncing deletion:', err);
					alert('Error eliminando archivo.');
				} else {
					alert('¡Archivo eliminado! Recargando juego...');
					setTimeout(() => location.reload(), 500);
				}
			});
		} catch (err) {
			console.error('Error deleting file:', err);
			alert('Error eliminando archivo: ' + filename);
		}
	};

	// Reset settings
	resetSettingsBtn.addEventListener('click', () => {
		if (!confirm('¿Restablecer la configuración? Esto eliminará diablo.ini pero conservará tus partidas guardadas. El juego se recargará.')) {
			return;
		}

		try {
			const iniPath = '/libsdl/diasurgical/devilution/diablo.ini';

			try {
				FS.stat(iniPath);
				FS.unlink(iniPath);
			} catch (e) {
				// File doesn't exist
			}

			FS.syncfs(false, function(err) {
				if (err) {
					console.error('Error syncing settings reset:', err);
					alert('Error al restablecer la configuración.');
				} else {
					alert('¡Configuración restablecida! Recargando juego...');
					setTimeout(() => location.reload(), 500);
				}
			});
		} catch (err) {
			console.error('Error resetting settings:', err);
			alert('Error al restablecer la configuración.');
		}
	});

	// Helper function
	function formatBytes(bytes) {
		if (bytes === 0) return '0 Bytes';
		const k = 1024;
		const sizes = ['Bytes', 'KB', 'MB', 'GB'];
		const i = Math.floor(Math.log(bytes) / Math.log(k));
		return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
	}
})();
