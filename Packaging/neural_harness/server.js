const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 8080;
const ROOT_DIR = path.resolve(__dirname, '..'); // Points to Packaging/

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.ico': 'image/x-icon',
    '.svg': 'image/svg+xml',
    '.wgsl': 'text/plain; charset=utf-8',
    '.wasm': 'application/wasm',
    '.onnx': 'application/octet-stream',
    '.mpq': 'application/octet-stream',
    '.raw': 'application/octet-stream',
    '.sv': 'application/octet-stream',
    '.hsv': 'application/octet-stream',
    '.ini': 'text/plain; charset=utf-8',
};

const server = http.createServer((req, res) => {
    let reqUrl = req.url.split('?')[0];

    // Default route: Serve Diablo Web Game (Packaging/emscripten/index.html)
    let filePath;
    if (reqUrl === '/' || reqUrl === '/index.html') {
        filePath = path.join(ROOT_DIR, 'emscripten', 'index.html');
    } else if (reqUrl.startsWith('/harness')) {
        filePath = path.join(ROOT_DIR, 'neural_harness', reqUrl.replace('/harness', '') || 'index.html');
    } else if (reqUrl.startsWith('/minixp')) {
        filePath = path.join(ROOT_DIR, 'minixp-wasm', reqUrl.replace('/minixp', '') || 'index.html');
    } else {
        // Try emscripten directory first, then ROOT_DIR
        const emscriptenPath = path.join(ROOT_DIR, 'emscripten', reqUrl);
        if (fs.existsSync(emscriptenPath) && fs.statSync(emscriptenPath).isFile()) {
            filePath = emscriptenPath;
        } else {
            filePath = path.join(ROOT_DIR, reqUrl);
        }
    }

    // Security headers for WebAssembly & SharedArrayBuffer / WebGPU
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0');
    res.setHeader('Pragma', 'no-cache');
    res.setHeader('Expires', '0');

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('404 Not Found: ' + reqUrl);
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, {
            'Content-Type': contentType,
            'Content-Length': stats.size,
            'Cache-Control': 'no-cache, no-store, must-revalidate',
        });

        const readStream = fs.createReadStream(filePath);
        readStream.pipe(res);
    });
});

server.listen(PORT, '127.0.0.1', () => {
    console.log(`[Diablo Nightmare Server] Running at http://localhost:${PORT}/`);
    console.log(`  • Main Game:   http://localhost:${PORT}/`);
    console.log(`  • Lab Harness: http://localhost:${PORT}/harness/index.html`);
    console.log(`  • Tiny Core:   http://localhost:${PORT}/minixp/index.html`);
});
