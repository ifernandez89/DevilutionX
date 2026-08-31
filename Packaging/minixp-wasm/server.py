#!/usr/bin/env python3
"""
server.py
Servidor HTTP local con soporte completo para HTTP Range Requests (Status 206 Partial Content),
necesario para que el emulador WebAssembly v86 lea la imagen ISO por bloques bajo demanda.
"""

import os
import sys
import re
from http.server import SimpleHTTPRequestHandler, HTTPServer


class RangeHTTPRequestHandler(SimpleHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def end_headers(self):
        # Cabeceras requeridas para SharedArrayBuffer y streaming en navegadores modernos
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Accept-Ranges", "bytes")
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()

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


def run_server(port=8000, directory=None):
    if directory:
        os.chdir(directory)
    
    server_address = ("0.0.0.0", port)
    httpd = HTTPServer(server_address, RangeHTTPRequestHandler)
    print("=" * 65)
    print(f"[*] Servidor Mini XP v86 WASM activo")
    print(f"[*] Accede localmente en: http://localhost:{port}")
    print("=" * 65)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[*] Servidor detenido.")
        httpd.server_close()


if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    run_server(port)
