/**
 * server.js - Servidor HTTP local con soporte de HTTP Range (206 Partial Content) y COOP/COEP
 * para emulación WebAssembly (v86 / Mini Windows XP)
 */

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 8000;
const PUBLIC_DIR = __dirname;

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.wasm': 'application/wasm',
    '.iso': 'application/x-iso9660-image',
    '.bin': 'application/octet-stream',
    '.json': 'application/json; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon'
};

const server = http.createServer((req, res) => {
    // Cabeceras requeridas para WebAssembly con SharedArrayBuffer
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Accept-Ranges', 'bytes');
    res.setHeader('Cache-Control', 'no-cache');

    let reqUrl = decodeURIComponent(req.url.split('?')[0]);
    if (reqUrl === '/') reqUrl = '/index.html';

    const filePath = path.join(PUBLIC_DIR, reqUrl);

    // Evitar directory traversal
    if (!filePath.startsWith(PUBLIC_DIR)) {
        res.writeHead(403);
        res.end('Acceso denegado');
        return;
    }

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('404 Archivo no encontrado');
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';
        const totalSize = stats.size;
        const range = req.headers.range;

        if (range) {
            // Manejar peticiones HTTP Range (206 Partial Content) para carga fluida de la ISO
            const parts = range.replace(/bytes=/, '').split('-');
            const start = parseInt(parts[0], 10);
            const end = parts[1] ? parseInt(parts[1], 10) : totalSize - 1;

            if (start >= totalSize || start > end) {
                res.writeHead(416, { 'Content-Range': `bytes */${totalSize}` });
                res.end();
                return;
            }

            const chunkSize = (end - start) + 1;
            const fileStream = fs.createReadStream(filePath, { start, end });

            res.writeHead(206, {
                'Content-Range': `bytes ${start}-${end}/${totalSize}`,
                'Content-Length': chunkSize,
                'Content-Type': contentType
            });

            fileStream.pipe(res);
        } else {
            res.writeHead(200, {
                'Content-Length': totalSize,
                'Content-Type': contentType
            });
            fs.createReadStream(filePath).pipe(res);
        }
    });
});

server.listen(PORT, () => {
    console.log('========================================================');
    console.log('  Mini Windows XP - Servidor WebAssembly v86');
    console.log(`  Disponible en: http://localhost:${PORT}`);
    console.log('  Presiona Ctrl+C para detener el servidor');
    console.log('========================================================');
});
