import http.server
import socketserver
import webbrowser
import os
import sys

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        super().end_headers()

def main():
    os.chdir(DIRECTORY)
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        url = f"http://localhost:{PORT}/index.html"
        print(f"==========================================================")
        print(f" PC VIVAZ / EDUCATIONAL COMPUTER 2000 - EMULATOR SERVER")
        print(f" Servidor iniciado en: {url}")
        print(f" Presiona Ctrl+C para detener el servidor")
        print(f"==========================================================")
        try:
            webbrowser.open(url)
        except Exception as e:
            print(f"No se pudo abrir el navegador automáticamente: {e}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServidor detenido.")

if __name__ == '__main__':
    main()
