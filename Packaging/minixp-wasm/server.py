#!/usr/bin/env python3
"""
server.py - Servidor HTTP local optimizado para WebAssembly (WASM) y v86
Soporta peticiones HTTP Range (206 Partial Content) para streaming de imágenes ISO y cabeceras COOP/COEP.
"""

import os
import sys
import re
import http.server
import socketserver

PORT = 8085

class WasmRangeHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def end_headers(self):
        # Cabeceras requeridas para SharedArrayBuffer y streaming en navegadores modernos
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Accept-Ranges", "bytes")
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()

    def guess_type(self, path):
        if path.endswith(".wasm"):
            return "application/wasm"
        if path.endswith(".js"):
            return "application/javascript"
        if path.endswith(".iso"):
            return "application/x-iso9660-image"
        if path.endswith(".bin"):
            return "application/octet-stream"
        return super().guess_type(path)

    def do_GET(self):
        range_header = self.headers.get("Range")
        if not range_header:
            return super().do_GET()

        path = self.translate_path(self.path)
        if not os.path.isfile(path):
            return super().do_GET()

        file_size = os.path.getsize(path)
        range_match = re.match(r"bytes=(\d+)-(\d*)", range_header)
        if not range_match:
            return super().do_GET()

        start = int(range_match.group(1))
        end = int(range_match.group(2)) if range_match.group(2) else file_size - 1

        if start >= file_size or start > end:
            self.send_response(416)
            self.send_header("Content-Range", f"bytes */{file_size}")
            self.end_headers()
            return

        length = end - start + 1

        self.send_response(206)
        self.send_header("Content-Type", self.guess_type(path))
        self.send_header("Content-Range", f"bytes {start}-{end}/{file_size}")
        self.send_header("Content-Length", str(length))
        self.end_headers()

        with open(path, "rb") as f:
            f.seek(start)
            bytes_to_send = length
            chunk_size = 64 * 1024
            while bytes_to_send > 0:
                chunk = f.read(min(bytes_to_send, chunk_size))
                if not chunk:
                    break
                self.wfile.write(chunk)
                bytes_to_send -= len(chunk)

    def do_POST(self):
        if self.path == "/api/telemetry":
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8', errors='ignore')
            log_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "telemetry.log")
            with open(log_path, "a", encoding="utf-8") as f:
                f.write(post_data + "\n")
            print(f"[TELEMETRY] {post_data}")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(b'{"status":"ok"}')
            return
        self.send_response(404)
        self.end_headers()

class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True

def run(port=PORT):
    dir_path = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dir_path)
    
    handler = WasmRangeHTTPRequestHandler
    
    with ThreadedTCPServer(("", port), handler) as httpd:
        print(f"\n========================================================")
        print(f"  Retro Virtual PC & WASM Lab (v86)")
        print(f"  Servidor iniciado en: http://localhost:{port}")
        print(f"  Soporte HTTP Range (206) + Multihilo Activado")
        print(f"  Presiona Ctrl+C para detener el servidor")

        print(f"========================================================\n")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServidor detenido.")
        except KeyboardInterrupt:
            print("\nServidor detenido.")

if __name__ == "__main__":
    p = int(sys.argv[1]) if len(sys.argv) > 1 else PORT
    run(p)
