// Pre-load MPQ files from the server directory into Emscripten virtual filesystem
Module['preRun'] = Module['preRun'] || [];

// Mount IDBFS for persistent save files
Module['preRun'].push(function() {
  console.log('Setting up IDBFS for persistent saves...');

  // SDL uses //libsdl/ as the base path for Emscripten
  // Save files are in //libsdl/diasurgical/devilution/
  // Config files (diablo.ini) would be in //libsdl/diasurgical/
  try {
    // Helper function to create directory if it doesn't exist
    function mkdirSafe(path) {
      try {
        // Check if path exists
        var stat = FS.stat(path);
        // If it exists and is a directory, we're good
        if (FS.isDir(stat.mode)) {
          return;
        }
        // If it exists but is not a directory, this is an error
        console.error('Path exists but is not a directory: ' + path);
        return;
      } catch (e) {
        // Path doesn't exist, try to create it
        try {
          FS.mkdir(path);
        } catch (mkdirErr) {
          // Only throw if it's not an "already exists" error
          if (mkdirErr.errno !== 20 && mkdirErr.errno !== 17) {
            throw mkdirErr;
          }
        }
      }
    }

    // Create SDL directory hierarchy if needed
    mkdirSafe('/libsdl');
    mkdirSafe('/libsdl/diasurgical');

    // Mount the diasurgical directory as IDBFS to persist saves AND settings
    FS.mount(IDBFS, {}, '/libsdl/diasurgical');
    console.log('IDBFS mounted successfully at /libsdl/diasurgical');

    // Sync from IndexedDB to memory (load existing saves)
    Module.addRunDependency('syncfs');
    FS.syncfs(true, function(err) {
      if (err) {
        console.error('Error loading saves from IndexedDB:', err);
      } else {
        console.log('Existing saves loaded from IndexedDB');
        try {
          // Remove any stray /diablo.ini in root to prevent portable mode override
          try { FS.unlink('/diablo.ini'); } catch(e) {}

          mkdirSafe('/libsdl/diasurgical/devilution');
          var iniPath = '/libsdl/diasurgical/devilution/diablo.ini';
          var currentIni = '';
          try {
            currentIni = new TextDecoder().decode(FS.readFile(iniPath));
          } catch(e) {}

          var modified = false;
          if (!currentIni || currentIni.trim().length === 0) {
            currentIni = "[Game]\nTest Barbarian=1\nTest Bard=1\nRun in Town=1\nCow Quest=1\nTheo Quest=1\n";
            modified = true;
          } else {
            if (currentIni.indexOf('Test Barbarian=1') === -1) {
              if (currentIni.indexOf('Test Barbarian') !== -1) {
                currentIni = currentIni.replace(/Test Barbarian\s*=\s*\d+/i, 'Test Barbarian=1');
              } else if (currentIni.indexOf('[Game]') !== -1) {
                currentIni = currentIni.replace('[Game]', "[Game]\nTest Barbarian=1");
              } else {
                currentIni += "\n[Game]\nTest Barbarian=1\n";
              }
              modified = true;
            }
            if (currentIni.indexOf('Test Bard=1') === -1) {
              if (currentIni.indexOf('Test Bard') !== -1) {
                currentIni = currentIni.replace(/Test Bard\s*=\s*\d+/i, 'Test Bard=1');
              } else if (currentIni.indexOf('[Game]') !== -1) {
                currentIni = currentIni.replace('[Game]', "[Game]\nTest Bard=1");
              } else {
                currentIni += "\n[Game]\nTest Bard=1\n";
              }
              modified = true;
            }
            if (currentIni.indexOf('Run in Town=1') === -1) {
              if (currentIni.indexOf('Run in Town') !== -1) {
                currentIni = currentIni.replace(/Run in Town\s*=\s*\d+/i, 'Run in Town=1');
              } else if (currentIni.indexOf('[Game]') !== -1) {
                currentIni = currentIni.replace('[Game]', "[Game]\nRun in Town=1");
              } else {
                currentIni += "\n[Game]\nRun in Town=1\n";
              }
              modified = true;
            }
            if (currentIni.indexOf('clock=1') === -1) {
              if (currentIni.indexOf('clock') !== -1) {
                currentIni = currentIni.replace(/clock\s*=\s*\d+/i, 'clock=1');
              } else if (currentIni.indexOf('[Mods]') !== -1) {
                currentIni = currentIni.replace('[Mods]', "[Mods]\nclock=1");
              } else {
                currentIni += "\n[Mods]\nclock=1\n";
              }
              modified = true;
            }
          }

          // Migrate corrupted string values to proper integer enum values
          // Previous versions wrote Game=Hellfire/Game=Diablo instead of Game=1/Game=2
          if (currentIni.indexOf('Game=Hellfire') !== -1) {
            currentIni = currentIni.replace(/Game=Hellfire/g, 'Game=1');
            modified = true;
          }
          if (currentIni.indexOf('Game=Diablo') !== -1) {
            currentIni = currentIni.replace(/Game=Diablo/g, 'Game=2');
            modified = true;
          }
          // Remove legacy Game Mode keys that the C++ parser doesn't understand
          if (currentIni.indexOf('Game Mode=') !== -1) {
            currentIni = currentIni.replace(/^Game Mode=.*$/gm, '');
            modified = true;
          }

          // If Hellfire MPQs or .hsv saves exist, and no explicit Diablo mode was chosen, configure Hellfire mode
          // StartUpGameMode enum: Ask=0, Hellfire=1, Diablo=2
          try {
            var devFiles = FS.readdir('/libsdl/diasurgical/devilution') || [];
            var hasHf = devFiles.some(function(f) {
              var low = f.toLowerCase();
              return low === 'hellfire.mpq' || low.endsWith('.hsv');
            });
            if (hasHf && currentIni.indexOf('Game=2') === -1) {
              if (currentIni.indexOf('[GameMode]') === -1) {
                currentIni += "\n[GameMode]\nGame=1\n";
                modified = true;
              } else if (currentIni.indexOf('Game=1') === -1) {
                currentIni = currentIni.replace('[GameMode]', "[GameMode]\nGame=1");
                modified = true;
              }
              if (currentIni.indexOf('[Mods]') === -1) {
                currentIni += "\n[Mods]\nHellfire=1\nclock=1\n";
                modified = true;
              } else {
                if (currentIni.indexOf('Hellfire=1') === -1) {
                  currentIni = currentIni.replace('[Mods]', "[Mods]\nHellfire=1");
                  modified = true;
                }
                if (currentIni.indexOf('clock=') === -1) {
                  currentIni = currentIni.replace('[Mods]', "[Mods]\nclock=1");
                  modified = true;
                }
              }
            }
          } catch(e) {}

          if (modified) {
            FS.writeFile(iniPath, currentIni);
            try { FS.writeFile('/diablo.ini', currentIni); } catch(e) {}
            FS.syncfs(false, function() {});
          }

          // Deduplicate and normalize all MPQs in IndexedDB to clean lowercase
          try {
            var files = FS.readdir('/libsdl/diasurgical/devilution');
            var needSync = false;
            var lowerMap = {};
            files.forEach(function(fname) {
              if (fname === '.' || fname === '..') return;
              var lower = fname.toLowerCase();
              if (lower.endsWith('.mpq')) {
                if (!lowerMap[lower]) {
                  lowerMap[lower] = [];
                }
                lowerMap[lower].push(fname);
              }
            });

            for (var lower in lowerMap) {
              var list = lowerMap[lower];
              var targetPath = '/libsdl/diasurgical/devilution/' + lower;
              if (list.indexOf(lower) === -1) {
                // Only non-lowercase exists, rename the first to lowercase
                var sourcePath = '/libsdl/diasurgical/devilution/' + list[0];
                var data = FS.readFile(sourcePath);
                FS.writeFile(targetPath, data);
                needSync = true;
              }
              // Delete all non-lowercase duplicate files
              list.forEach(function(fname) {
                if (fname !== lower) {
                  try { FS.unlink('/libsdl/diasurgical/devilution/' + fname); } catch(e) {}
                  needSync = true;
                }
              });

              // Mirror lowercase into root virtual memory filesystem
              try {
                var content = FS.readFile(targetPath);
                FS.writeFile('/' + lower, content);
                if (lower === 'diabdat.mpq') {
                  try { FS.writeFile('/DIABDAT.MPQ', content); } catch(e) {}
                }
              } catch(e) {}
            }

            // Auto-heal and normalize all save files (.sv, .hsv, .dsv) in IDBFS to clean lowercase slots
            try {
              var allFiles = FS.readdir('/libsdl/diasurgical/devilution');
              var saveRegex = /^(single|spawn|multi|share)_(\d+)\.(sv|hsv)$/;
              var standardOccupied = {};
              var irregularSaves = [];

              allFiles.forEach(function(fname) {
                if (fname === '.' || fname === '..') return;
                var lower = fname.toLowerCase();
                if (lower.endsWith('.sv') || lower.endsWith('.hsv') || lower.endsWith('.dsv')) {
                  var cleanExtLower = lower;
                  if (cleanExtLower.endsWith('.dsv')) {
                    cleanExtLower = cleanExtLower.substring(0, cleanExtLower.length - 4) + '.sv';
                  }

                  // STASH / ALIJO COMPARTIDO: Preservar y sincronizar sin alterar como personaje
                  if (cleanExtLower.indexOf('stash') === 0) {
                    var canonicalStash = cleanExtLower.endsWith('.hsv') ? 'stash.hsv' : 'stash.sv';
                    var spawnStash = cleanExtLower.endsWith('.hsv') ? 'stash_spawn.hsv' : 'stash_spawn.sv';
                    try {
                      var stashData = FS.readFile('/libsdl/diasurgical/devilution/' + fname);
                      // Asegurar versión canónica y spawn en IDBFS
                      FS.writeFile('/libsdl/diasurgical/devilution/' + canonicalStash, stashData);
                      FS.writeFile('/libsdl/diasurgical/devilution/' + spawnStash, stashData);
                      // Espejear en raíz RAM virtual
                      try { FS.writeFile('/' + canonicalStash, stashData); } catch(e) {}
                      try { FS.writeFile('/' + spawnStash, stashData); } catch(e) {}
                      // Eliminar nombre con mayúsculas o no estándar si aplica
                      if (fname !== canonicalStash && fname !== spawnStash) {
                        try { FS.unlink('/libsdl/diasurgical/devilution/' + fname); } catch(e) {}
                        needSync = true;
                      }
                      console.log('[IDBFS] Alijo compartido protegido y sincronizado:', canonicalStash, '/', spawnStash);
                    } catch(e) {
                      console.warn('[IDBFS] Error sincronizando alijo compartido:', fname, e);
                    }
                    return; // No clasificar como partida irregular ni renombrar a single_X.sv
                  }

                  var m = cleanExtLower.match(saveRegex);
                  if (m && fname === cleanExtLower) {
                    standardOccupied[cleanExtLower] = true;
                    // Mirror valid save into root virtual memory
                    try {
                      var sc = FS.readFile('/libsdl/diasurgical/devilution/' + fname);
                      FS.writeFile('/' + fname, sc);
                    } catch(e) {}
                  } else {
                    irregularSaves.push({ original: fname, normalizedLower: cleanExtLower });
                  }
                }
              });

              if (irregularSaves.length > 0) {
                irregularSaves.forEach(function(item) {
                  var targetFilename = '';
                  var m = item.normalizedLower.match(saveRegex);
                  if (m && !standardOccupied[item.normalizedLower]) {
                    targetFilename = item.normalizedLower;
                  } else {
                    var prefix = 'single_';
                    if (item.normalizedLower.indexOf('spawn_') === 0) prefix = 'spawn_';
                    else if (item.normalizedLower.indexOf('multi_') === 0) prefix = 'multi_';
                    else if (item.normalizedLower.indexOf('share_') === 0) prefix = 'share_';

                    var ext = item.normalizedLower.endsWith('.hsv') ? '.hsv' : '.sv';
                    for (var s = 0; s < 99; s++) {
                      var candidate = prefix + s + ext;
                      if (!standardOccupied[candidate]) {
                        targetFilename = candidate;
                        break;
                      }
                    }
                  }

                  if (targetFilename) {
                    try {
                      var oldPath = '/libsdl/diasurgical/devilution/' + item.original;
                      var newPath = '/libsdl/diasurgical/devilution/' + targetFilename;
                      var saveContent = FS.readFile(oldPath);
                      FS.writeFile(newPath, saveContent);
                      if (item.original !== targetFilename) {
                        try { FS.unlink(oldPath); } catch(e) {}
                      }
                      standardOccupied[targetFilename] = true;
                      needSync = true;
                      console.log('[IDBFS] Partida reparada/normalizada:', item.original, '->', targetFilename);
                      try { FS.writeFile('/' + targetFilename, saveContent); } catch(e) {}
                    } catch(err) {
                      console.warn('[IDBFS] Error reparando partida ' + item.original + ':', err);
                    }
                  }
                });
              }
            } catch(e) {
              console.warn('Error auto-healing save files from IDBFS:', e);
            }

            if (needSync) {
              FS.syncfs(false, function() {});
            }
          } catch(e) {
            console.warn('Error normalizing MPQ files from IDBFS:', e);
          }
        } catch(e) {
          console.error('Error during IDBFS post-sync:', e);
        }
      }
      Module.removeRunDependency('syncfs');
    });
  } catch (e) {
    console.error('Error setting up IDBFS:', e);
  }
});

// Load MPQ files from the server directory only if no user MPQs exist and not deleted
Module['preRun'].push(function() {
  var userDeletedSpawn = false;
  try {
    userDeletedSpawn = (localStorage.getItem('devilutionx_deleted_spawn') === 'true');
  } catch(e) {}

  if (userDeletedSpawn) {
    console.log('User previously removed spawn.mpq, skipping server download.');
    return;
  }

  // Check if any MPQ already exists in IndexedDB directory
  var hasLocalMpq = false;
  try {
    var files = FS.readdir('/libsdl/diasurgical/devilution');
    files.forEach(function(f) {
      if (f.toLowerCase().endsWith('.mpq') && f !== '.' && f !== '..') {
        hasLocalMpq = true;
      }
    });
  } catch(e) {}

  if (hasLocalMpq) {
    console.log('Local MPQ archive found in IDBFS, skipping server fallback.');
    return;
  }

  var mpqFiles = ['spawn.mpq'];
  var loadPromises = mpqFiles.map(function(filename) {
    return new Promise(function(resolve) {
      fetch(filename)
        .then(function(response) {
          if (response.ok) {
            return response.arrayBuffer();
          }
          throw new Error('File not found');
        })
        .then(function(data) {
          console.log('Loading ' + filename + ' into virtual filesystem (RAM only)...');
          var u8 = new Uint8Array(data);
          FS.writeFile('/' + filename.toLowerCase(), u8);
          try { FS.writeFile('/' + filename.toUpperCase(), u8); } catch(e) {}
          resolve();
        })
        .catch(function() {
          resolve();
        });
    });
  });

  Module.addRunDependency('loadMPQs');
  Promise.all(loadPromises).then(function() {
    Module.removeRunDependency('loadMPQs');
  });
});

// Track if a sync is in progress to prevent overlapping operations
var syncInProgress = false;
window.syncInProgress = false;

// Expose function to manually save to IndexedDB
Module['saveToIndexedDB'] = function() {
  if (syncInProgress || window.syncInProgress) {
    return;
  }

  syncInProgress = true;
  window.syncInProgress = true;
  FS.syncfs(false, function(err) {
    syncInProgress = false;
    window.syncInProgress = false;
    if (err) {
      console.error('Error persisting saves to IndexedDB:', err);
    }
  });
};

// Auto-sync to IndexedDB every 45 seconds as a fallback
Module['postRun'] = Module['postRun'] || [];
Module['postRun'].push(function() {
  setInterval(function() {
    if (!syncInProgress && !window.syncInProgress) {
      syncInProgress = true;
      window.syncInProgress = true;
      var t0 = performance.now();
      console.log('[IDBFS] Auto-sync periódico iniciado...');
      FS.syncfs(false, function(err) {
        syncInProgress = false;
        window.syncInProgress = false;
        var elapsed = Math.round(performance.now() - t0);
        if (err) {
          console.error('[IDBFS] Error en auto-sync (' + elapsed + 'ms):', err);
        } else {
          console.log('[IDBFS] Auto-sync completado con éxito (' + elapsed + 'ms).');
        }
      });
    }
  }, 45000);

  // Sync when the page is about to close
  window.addEventListener('beforeunload', function() {
    if (!syncInProgress && !window.syncInProgress) {
      FS.syncfs(false, function(err) {
        if (err) console.error('[IDBFS] Error en sincronización de cierre:', err);
      });
    }
  });
});
