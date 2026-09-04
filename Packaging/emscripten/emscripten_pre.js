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
          }
          if (modified) {
            FS.writeFile(iniPath, currentIni);
            FS.syncfs(false, function() {});
          }

          // Mirror all MPQs from IndexedDB into root virtual filesystem with casing variants
          try {
            var files = FS.readdir('/libsdl/diasurgical/devilution');
            files.forEach(function(fname) {
              if (fname === '.' || fname === '..') return;
              var lower = fname.toLowerCase();
              if (lower.endsWith('.mpq')) {
                var content = FS.readFile('/libsdl/diasurgical/devilution/' + fname);
                if (lower === 'diabdat.mpq') {
                  try { FS.writeFile('/libsdl/diasurgical/devilution/DIABDAT.MPQ', content); } catch(e) {}
                  try { FS.writeFile('/libsdl/diasurgical/devilution/diabdat.mpq', content); } catch(e) {}
                  try { FS.writeFile('/DIABDAT.MPQ', content); } catch(e) {}
                  try { FS.writeFile('/diabdat.mpq', content); } catch(e) {}
                } else if (lower === 'spawn.mpq') {
                  try { FS.writeFile('/libsdl/diasurgical/devilution/spawn.mpq', content); } catch(e) {}
                  try { FS.writeFile('/libsdl/diasurgical/devilution/SPAWN.MPQ', content); } catch(e) {}
                  try { FS.writeFile('/spawn.mpq', content); } catch(e) {}
                  try { FS.writeFile('/SPAWN.MPQ', content); } catch(e) {}
                } else if (lower === 'hellfire.mpq') {
                  try { FS.writeFile('/libsdl/diasurgical/devilution/hellfire.mpq', content); } catch(e) {}
                  try { FS.writeFile('/libsdl/diasurgical/devilution/HELLFIRE.MPQ', content); } catch(e) {}
                  try { FS.writeFile('/hellfire.mpq', content); } catch(e) {}
                  try { FS.writeFile('/HELLFIRE.MPQ', content); } catch(e) {}
                } else {
                  try { FS.writeFile('/' + fname, content); } catch(e) {}
                }
              }
            });
          } catch(e) {
            console.warn('Error mirroring MPQ files from IDBFS:', e);
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

// Load MPQ files from the server directory
Module['preRun'].push(function() {
  // List of MPQ files to try loading (in priority order)
  var mpqFiles = [
    'spawn.mpq',
  ];

  // Create a promise-based loading system
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
          console.log('Loading ' + filename + ' into virtual filesystem...');
          var u8 = new Uint8Array(data);
          FS.writeFile('/' + filename, u8);
          try { FS.writeFile('/' + filename.toUpperCase(), u8); } catch(e) {}
          try {
            mkdirSafe('/libsdl/diasurgical/devilution');
            FS.writeFile('/libsdl/diasurgical/devilution/' + filename, u8);
            FS.writeFile('/libsdl/diasurgical/devilution/' + filename.toUpperCase(), u8);
          } catch(e) {}
          console.log('Successfully loaded ' + filename);
          resolve();
        })
        .catch(function() {
          // File doesn't exist on server, skip silently (may exist in IndexedDB)
          resolve();
        });
    });
  });

  // Wait for all MPQ files to load before continuing
  Module.addRunDependency('loadMPQs');
  Promise.all(loadPromises).then(function() {
    Module.removeRunDependency('loadMPQs');
  });
});

// Track if a sync is in progress to prevent overlapping operations
var syncInProgress = false;

// Expose function to manually save to IndexedDB
Module['saveToIndexedDB'] = function() {
  if (syncInProgress) {
    return;
  }

  syncInProgress = true;
  FS.syncfs(false, function(err) {
    syncInProgress = false;
    if (err) {
      console.error('Error persisting saves to IndexedDB:', err);
    }
  });
};

// Auto-sync to IndexedDB every 30 seconds as a fallback
Module['postRun'] = Module['postRun'] || [];
Module['postRun'].push(function() {
  setInterval(function() {
    if (!syncInProgress) {
      syncInProgress = true;
      FS.syncfs(false, function(err) {
        syncInProgress = false;
        if (err) {
          console.error('Auto-sync error:', err);
        }
      });
    }
  }, 30000);

  // Sync when the page is about to close
  window.addEventListener('beforeunload', function() {
    if (!syncInProgress) {
      FS.syncfs(false, function(err) {
        if (err) console.error('Error syncing on page unload:', err);
      });
    }
  });
});
