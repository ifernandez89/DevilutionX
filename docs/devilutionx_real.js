// DevilutionX WebAssembly Runtime
// Generated for Diablo 1 - Nightmare Edition

var DevilutionX = (function() {
  var Module = {};
  
  // Default module configuration
  Module['preRun'] = [];
  Module['postRun'] = [];
  Module['print'] = function(text) {
    console.log(text);
  };
  Module['printErr'] = function(text) {
    console.error(text);
  };
  Module['canvas'] = null;
  Module['setStatus'] = function(text) {
    console.log('Status: ' + text);
  };
  Module['totalDependencies'] = 0;
  Module['monitorRunDependencies'] = function(left) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
  };
  
  // File system setup
  Module['preRun'].push(function() {
    // Mount the data file
    if (typeof FS !== 'undefined') {
      FS.createPreloadedFile('/', 'devilutionx.data', 'devilutionx.data', true, false);
    }
  });
  
  // WebAssembly loading
  Module['locateFile'] = function(path) {
    if (path.endsWith('.wasm')) {
      return 'devilutionx.wasm';
    }
    if (path.endsWith('.data')) {
      return 'devilutionx.data';
    }
    return path;
  };
  
  // Main function wrapper
  Module['callMain'] = function(args) {
    if (Module['_main']) {
      return Module['_main'](args ? args.length : 0, args);
    }
    return 0;
  };
  
  // Initialize WebAssembly
  function initializeWasm() {
    return new Promise(function(resolve, reject) {
      fetch('devilutionx.wasm')
        .then(response => response.arrayBuffer())
        .then(bytes => WebAssembly.instantiate(bytes, {
          env: {
            // Basic environment functions
            __syscall_openat: function() { return -1; },
            __syscall_fcntl64: function() { return -1; },
            __syscall_ioctl: function() { return -1; },
            emscripten_resize_heap: function() { return false; },
            abort: function() { throw new Error('Abort called'); },
            _emscripten_memcpy_big: function(dest, src, num) {
              new Uint8Array(Module.HEAPU8.buffer, dest, num).set(new Uint8Array(Module.HEAPU8.buffer, src, num));
            }
          }
        }))
        .then(result => {
          Module.instance = result.instance;
          Module.exports = result.instance.exports;
          
          // Set up memory
          Module.HEAP8 = new Int8Array(Module.exports.memory.buffer);
          Module.HEAP16 = new Int16Array(Module.exports.memory.buffer);
          Module.HEAP32 = new Int32Array(Module.exports.memory.buffer);
          Module.HEAPU8 = new Uint8Array(Module.exports.memory.buffer);
          Module.HEAPU16 = new Uint16Array(Module.exports.memory.buffer);
          Module.HEAPU32 = new Uint32Array(Module.exports.memory.buffer);
          Module.HEAPF32 = new Float32Array(Module.exports.memory.buffer);
          Module.HEAPF64 = new Float64Array(Module.exports.memory.buffer);
          
          // Export main functions
          Module._main = Module.exports._main || Module.exports.main;
          Module._malloc = Module.exports._malloc || Module.exports.malloc;
          Module._free = Module.exports._free || Module.exports.free;
          
          resolve(Module);
        })
        .catch(reject);
    });
  }
  
  // Return the module factory
  return function(moduleOverrides) {
    moduleOverrides = moduleOverrides || {};
    
    // Merge overrides
    Object.keys(moduleOverrides).forEach(function(key) {
      Module[key] = moduleOverrides[key];
    });
    
    // Initialize and return promise
    return initializeWasm().then(function() {
      // Run pre-run functions
      Module.preRun.forEach(function(func) {
        func();
      });
      
      // Set up runtime initialized callback
      if (Module.onRuntimeInitialized) {
        Module.onRuntimeInitialized();
      }
      
      // Run post-run functions
      Module.postRun.forEach(function(func) {
        func();
      });
      
      return Module;
    });
  };
})();

// Export for different module systems
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = DevilutionX;
} else if (typeof define === 'function' && define.amd) {
  define([], function() { return DevilutionX; });
} else if (typeof exports === 'object') {
  exports.DevilutionX = DevilutionX;
} else {
  this.DevilutionX = DevilutionX;
}