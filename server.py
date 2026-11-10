"""Local web server that connects the NC A&T GUI to the team's C++ StudentManager.

Only Python standard library and a C++17 compiler are required. The existing
backend source files stay unchanged. Run from the root of the GitHub project:
    py server.py            # Windows
    python3 server.py       # macOS / Linux
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import threading
import urllib.parse
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WEB = ROOT / "frontend"
DATA = ROOT / "data" / "records.json"
HOST = "127.0.0.1"
PORT = 8765
MAX_REQUEST = 65536


def build_bridge():
    sources = sorted((ROOT / "backend").glob("*.cpp"))
    if not sources or not (ROOT / "backend" / "StudentManager.h").exists():
        raise RuntimeError(
            "Missing the team's C++ backend. Copy this frontend package into the "
            "GitHub repository root, alongside its 'backend' folder."
        )
    exe = ROOT / ("backend_bridge.exe" if os.name == "nt" else "backend_bridge")
    dependencies = [ROOT / "backend_bridge.cpp", *sources, *((ROOT / "backend").glob("*.h"))]
    if exe.exists() and exe.stat().st_mtime >= max(p.stat().st_mtime for p in dependencies):
        return exe

    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        raise RuntimeError(
            "No C++ compiler found. Install g++ (MinGW-w64) or clang++ and "
            "add it to your PATH, then run 'py server.py' again."
        )
    command = [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", str(ROOT / "backend_bridge.cpp")]
    command += [str(source) for source in sources]
    command += ["-o", str(exe)]
    print("Compiling the C++ backend bridge...", flush=True)
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError("C++ compilation failed:\n" + result.stdout + result.stderr)
    print("C++ bridge compiled successfully.", flush=True)
    return exe


class Backend:
    def __init__(self, executable):
        self.process = subprocess.Popen(
            [str(executable)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL, text=True, encoding="utf-8", bufsize=1, cwd=ROOT
        )
        self.lock = threading.RLock()
        self.history = []
        if DATA.exists():
            content = json.loads(DATA.read_text(encoding="utf-8"))
            if not isinstance(content, list):
                raise RuntimeError("data/records.json must contain a JSON list.")
            for item in content:
                if not isinstance(item, list) or not all(isinstance(p, str) for p in item):
                    raise RuntimeError("Invalid saved command in data/records.json.")
                response = self._send(item)
                if not response.get("ok"):
                    raise RuntimeError("Could not restore saved data: " + response.get("error", "unknown error"))
                self.history.append(item)
            print(f"Restored {len(self.history)} saved operations.", flush=True)

    def _send(self, fields):
        if self.process.poll() is not None:
            raise RuntimeError("The C++ backend closed unexpectedly.")
        encoded = "\t".join(urllib.parse.quote(str(field), safe="") for field in fields)
        self.process.stdin.write(encoded + "\n")
        self.process.stdin.flush()
        raw = self.process.stdout.readline()
        if not raw:
            raise RuntimeError("The C++ backend did not respond.")
        return json.loads(raw)

    def request(self, fields, save=False):
        with self.lock:
            response = self._send(fields)
            if save and response.get("ok"):
                self.history.append(fields)
                DATA.parent.mkdir(parents=True, exist_ok=True)
                temporary = DATA.with_suffix(".tmp")
                temporary.write_text(json.dumps(self.history, ensure_ascii=False, indent=2), encoding="utf-8")
                temporary.replace(DATA)
            return response


backend = None


def require_text(payload, key):
    text = payload.get(key, "")
    if not isinstance(text, str) or not text.strip() or len(text) > 300:
        raise ValueError(f"Enter a valid {key} (1–300 characters).")
    return text.strip()


class Handler(BaseHTTPRequestHandler):
    def respond(self, data, status=200):
        output = json.dumps(data, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(output)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(output)

    def do_GET(self):
        parsed = urllib.parse.urlsplit(self.path)
        path = parsed.path
        try:
            if path == "/api/health":
                return self.respond(backend.request(["PING"]))
            if path == "/api/courses":
                return self.respond(backend.request(["LIST_COURSES"]))
            if path == "/api/students":
                return self.respond(backend.request(["LIST_STUDENTS"]))
            if path == "/api/student":
                params = urllib.parse.parse_qs(parsed.query)
                return self.respond(backend.request(["GET_STUDENT", params.get("id", [""])[0]]))

            files = {
                "/": ("index.html", "text/html; charset=utf-8"),
                "/index.html": ("index.html", "text/html; charset=utf-8"),
                "/style.css": ("style.css", "text/css; charset=utf-8"),
                "/app.js": ("app.js", "text/javascript; charset=utf-8"),
            }
            if path not in files:
                return self.respond({"ok": False, "error": "Not found."}, 404)
            filename, mime = files[path]
            content = (WEB / filename).read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", mime)
            self.send_header("Content-Length", str(len(content)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.end_headers()
            self.wfile.write(content)
        except Exception as exc:
            self.respond({"ok": False, "error": str(exc)}, 500)

    def do_POST(self):
        routes = {
            "/api/students": ("ADD_STUDENT", ["type", "firstName", "lastName", "id", "phone", "address"]),
            "/api/register-course": ("REGISTER_COURSE", ["id", "courseCode"]),
            "/api/grade": ("ASSIGN_GRADE", ["id", "courseCode", "grade"]),
            "/api/project": ("REGISTER_PROJECT", ["id", "title"]),
            "/api/dissertation": ("REGISTER_DISSERTATION", ["id", "title"]),
        }
        path = urllib.parse.urlsplit(self.path).path
        if path not in routes:
            return self.respond({"ok": False, "error": "Unknown endpoint."}, 404)
        try:
            size = int(self.headers.get("Content-Length", "0"))
            if size <= 0 or size > MAX_REQUEST:
                return self.respond({"ok": False, "error": "Invalid request size."}, 400)
            payload = json.loads(self.rfile.read(size).decode("utf-8"))
            if not isinstance(payload, dict):
                raise ValueError("Expected a JSON object.")
            command, keys = routes[path]
            fields = [command] + [require_text(payload, key) for key in keys]
            result = backend.request(fields, save=True)
            return self.respond(result, 200 if result.get("ok") else 400)
        except (ValueError, UnicodeError, json.JSONDecodeError) as exc:
            self.respond({"ok": False, "error": str(exc)}, 400)
        except Exception as exc:
            self.respond({"ok": False, "error": str(exc)}, 500)

    def log_message(self, fmt, *args):
        if self.path.startswith("/api/"):
            return
        super().log_message(fmt, *args)


def main():
    global backend
    executable = build_bridge()
    backend = Backend(executable)
    server = ThreadingHTTPServer((HOST, PORT), Handler)
    url = f"http://{HOST}:{PORT}"
    print("\nNC A&T University Management Portal")
    print("Open in your browser: " + url)
    print("Press Ctrl+C to stop. Student data saves to data/records.json.\n", flush=True)
    if "--no-browser" not in sys.argv:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
    finally:
        server.server_close()
        backend.process.terminate()
        try:
            backend.process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            backend.process.kill()


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("\nERROR:", exc, file=sys.stderr)
        sys.exit(1)
