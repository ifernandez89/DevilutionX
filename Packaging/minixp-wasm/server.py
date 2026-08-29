#!/usr/bin/env python3
"""
server.py - Servidor HTTP local optimizado para WebAssembly (WASM) y v86
Soporta peticiones HTTP Range para streaming de imágenes ISO y cabeceras COOP/COEP.
"""

import os
import sys
import http.server
import socketserver

PORT = 8080

class WasmHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Cabeceras requeridas para SharedArrayBuffer y WebAssembly multihilo
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Accept-Ranges", "bytes")
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

def run():
    dir_path = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dir_path)
    
    handler = WasmHTTPRequestHandler
    socketserver.TCPServer.allow_reuse_address = True
    
    with socketserver.TCPServer(("", PORT), handler) as httpd:
        print(f"\n========================================================")
        print(f"  Mini Windows XP - Entorno WebAssembly")
        print(f"  Servidor iniciado en: http://localhost:{PORT}")
        print(f"  Presiona Ctrl+C para detener el servidor")
        print(f"========================================================\n")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServidor detenido.")

if __name__ == "__main__":
    run()
