// DevilutionX Nightmare Edition - Phase 6 Optimized Loader 
// Generated: dom. 18/01/2026 12:05:02,29 
 
var Module = typeof Module !== 'undefined' ? Module : {}; 
 
// Optimized WebAssembly loader 
Module['locateFile'] = function(path, prefix) { 
  if (path.endsWith('.wasm')) return 'devilutionx.wasm'; 
  if (path.endsWith('.data')) return 'devilutionx.data'; 
  return prefix + path; 
}; 
 
// Initialize WebAssembly 
Module['onRuntimeInitialized'] = function() { 
  console.log('DevilutionX Nightmare Edition - Phase 6 Optimized - Ready!'); 
  if (typeof Module.setStatus === 'function') { 
    Module.setStatus('Phase 6 Optimization Complete - Ready to Play!'); 
  } 
}; 
 
// Error handling 
Module['onAbort'] = function(what) { 
  console.error('DevilutionX aborted:', what); 
  if (typeof Module.setStatus === 'function') { 
    Module.setStatus('Error loading game: ' + what); 
  } 
}; 
 
// Load the WebAssembly module 
(function() { 
  var script = document.createElement('script'); 
  script.src = 'https://cdn.jsdelivr.net/npm/emscripten@3.1.45/emscripten.js'; 
  script.onload = function() { 
    console.log('Emscripten runtime loaded'); 
  }; 
  script.onerror = function() { 
    console.error('Failed to load Emscripten runtime'); 
  }; 
  document.head.appendChild(script); 
})(); 
