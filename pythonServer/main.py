import json
import requests
from datetime import date, timedelta, datetime
import re
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import time
import signal
import sys
import hashlib
from spotify_client import SpotifyClient
from image_processor import process_image
from database import Database

headers = {'User-Agent': 'curl'}
site = "https://rate.sx"
coins = ["eth", "doge", "xmr"]
lock = threading.Lock()
spotify_lock = threading.Lock()
DELAY = 10

db = Database()
data_cache = {}

spotify_client = None
current_spotify_state = {"is_playing": False}
current_image_data = None
current_image_hash = ""
last_spotify_poll = 0
SPOTIFY_POLL_INTERVAL = 3


def get_spotify_data():
    global current_spotify_state, current_image_data, current_image_hash, last_spotify_poll

    with spotify_lock:
        now = time.time()
        if now - last_spotify_poll < SPOTIFY_POLL_INTERVAL:
            return current_spotify_state

        last_spotify_poll = now
        try:
            if spotify_client:
                info = spotify_client.get_playback_info()

                # Check if cover changed
                new_cover_url = info.get("cover_url")
                old_cover_url = current_spotify_state.get("cover_url")

                if new_cover_url and new_cover_url != old_cover_url:
                    print(f"New cover detected: {new_cover_url}")
                    img_data, dom_color = process_image(new_cover_url)
                    if img_data:
                        current_image_data = img_data
                        current_image_hash = hashlib.md5(
                            new_cover_url.encode()).hexdigest()[:8]
                        current_spotify_state["dominant_color"] = dom_color
                        print(f"Image processed. Hash: {
                            current_image_hash}, Color: {dom_color}")

                info["image_hash"] = current_image_hash
                info["dominant_color"] = current_spotify_state.get(
                    "dominant_color", 0)
                current_spotify_state = info
        except Exception as e:
            print(f"Error getting Spotify data: {e}")

        return current_spotify_state

# --- Funciones de Datos ---


def get_value(coin: str, target_date: date):
    try:
        url = f"{site}/{coin}@{target_date.strftime('%Y-%m-%d')}"
        response = requests.get(url, headers=headers,
                                timeout=10)  # Timeout añadido
        response.raise_for_status()
        avg_pattern = re.search(
            r'avg:\x1b\[0m \$(\d+\.?\d*)\x1b\[2m', response.text)
        return float(avg_pattern.group(1)) if avg_pattern else 0.0
    except Exception as e:
        print(f"Error en {coin} para {target_date}: {str(e)}")
        return 0.0


def update_cache():
    """Actualiza la caché en memoria desde la base de datos."""
    global data_cache
    with lock:
        print("Actualizando caché desde DB...")
        temp_data = {}

        start_date = db.get_oldest_date_in_limit(30)
        if not start_date:
            start_date = (date.today() - timedelta(days=29)
                          ).strftime("%Y-%m-%d")

        temp_data["date"] = start_date

        for coin in coins:
            prices = db.get_last_30_days(coin)
            while len(prices) < 30:
                prices.insert(0, 0.0)
            temp_data[coin] = prices

        data_cache = temp_data
        print("Caché actualizada.")


def init():
    """
    Rellena huecos en la base de datos asegurando siempre los últimos 30 días.
    """
    print("Verificando integridad de la base de datos...")

    today = date.today()
    window_start = today - timedelta(days=29)

    latest_str = db.get_latest_date()
    start_fetch_date = window_start

    if latest_str:
        latest = datetime.strptime(latest_str, "%Y-%m-%d").date()
        if latest >= window_start:
            start_fetch_date = latest + timedelta(days=1)
        else:
            print(f"Datos demasiado antiguos (último: {
                  latest}). Reiniciando ventana de 30 días.")
            start_fetch_date = window_start

    if start_fetch_date > today:
        print("Datos al día.")
    else:
        curr = start_fetch_date
        print(f"Actualizando datos desde {curr} hasta {today}...")

        while curr <= today:
            print(f"Procesando: {curr}")
            for coin in coins:
                val = get_value(coin, curr)
                db.insert_price(coin, curr.strftime("%Y-%m-%d"), val)
                print(f"  - {coin}: {val}")
                time.sleep(DELAY)
            curr += timedelta(days=1)

    conn = db.get_connection()
    cursor = conn.cursor()
    cutoff_date = (today - timedelta(days=30)).strftime("%Y-%m-%d")
    cursor.execute("DELETE FROM prices WHERE date <= ?", (cutoff_date,))
    deleted = cursor.rowcount
    conn.commit()
    conn.close()
    if deleted > 0:
        print(f"Limpieza: Se eliminaron {deleted} registros antiguos.")

    print("Sincronización completada.")
    update_cache()


def actualizar_diario():
    """
    Hilo en segundo plano para actualizar los datos diariamente.
    """
    while True:
        try:
            now = datetime.now()
            next_run = (now + timedelta(days=1)).replace(hour=0,
                                                         minute=1, second=0, microsecond=0)
            sleep_duration = (next_run - now).total_seconds()

            print(f"\nPróxima actualización programada para {
                  next_run.strftime('%Y-%m-%d %H:%M:%S')}")
            time.sleep(sleep_duration)

            print(f"\n[{datetime.now()}] Ejecutando actualización diaria...")

            latest_str = db.get_latest_date()
            if latest_str == date.today().strftime("%Y-%m-%d"):
                print("Ya existen datos de hoy.")
                continue

            today = date.today()
            for coin in coins:
                val = get_value(coin, today)
                db.insert_price(coin, today.strftime("%Y-%m-%d"), val)
                print(f"  - {coin}: {val}")
                time.sleep(DELAY)

            conn = db.get_connection()
            cursor = conn.cursor()
            cutoff_date = (today - timedelta(days=30)).strftime("%Y-%m-%d")
            cursor.execute(
                "DELETE FROM prices WHERE date <= ?", (cutoff_date,))
            conn.commit()
            conn.close()

            update_cache()

        except Exception as e:
            print(f"Error en actualización diaria: {e}")
            time.sleep(3600)


class Server(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/spotify":
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            info = get_spotify_data()
            self.wfile.write(json.dumps(info).encode())
            return

        if self.path == "/spotify/image":
            if current_image_data:
                self.send_response(200)
                self.send_header("Content-type", "application/octet-stream")
                self.send_header("Content-Length",
                                 str(len(current_image_data)))
                self.end_headers()
                self.wfile.write(current_image_data)
            else:
                self.send_response(404)
                self.end_headers()
            return

        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()

        with lock:
            response = data_cache.copy()

        response["time"] = datetime.now().strftime("%H:%M")
        self.wfile.write(json.dumps(response).encode())

    def do_POST(self):
        if not spotify_client:
            self.send_response(503)
            self.end_headers()
            self.wfile.write(b'{"error": "Spotify client not available"}')
            return

        response_data = {"status": "success"}
        try:
            if self.path == "/spotify/play":
                content_length = int(self.headers.get('Content-Length', 0))
                body = self.rfile.read(content_length)
                data = json.loads(body) if body else {}
                context_uri = data.get("context_uri")
                spotify_client.play(context_uri)

            elif self.path == "/spotify/pause":
                spotify_client.pause()

            elif self.path == "/spotify/next":
                spotify_client.next_track()

            elif self.path == "/spotify/previous":
                spotify_client.previous_track()

            elif self.path == "/spotify/volume":
                content_length = int(self.headers.get('Content-Length', 0))
                body = self.rfile.read(content_length)
                data = json.loads(body) if body else {}
                volume = data.get("volume")
                if volume is not None:
                    spotify_client.set_volume(int(volume))
                else:
                    self.send_response(400)
                    self.end_headers()
                    return

            elif self.path == "/spotify/shuffle":
                content_length = int(self.headers.get('Content-Length', 0))
                body = self.rfile.read(content_length)
                data = json.loads(body) if body else {}
                state = data.get("state")
                if isinstance(state, bool):
                    spotify_client.toggle_shuffle(state)
                else:
                    self.send_response(400)
                    self.end_headers()
                    return

            elif self.path == "/spotify/repeat":
                content_length = int(self.headers.get('Content-Length', 0))
                body = self.rfile.read(content_length)
                data = json.loads(body) if body else {}
                state = data.get("state")
                if state in ['track', 'context', 'off']:
                    spotify_client.set_repeat(state)
                else:
                    self.send_response(400)
                    self.end_headers()
                    return

            elif self.path == "/spotify/seek":
                content_length = int(self.headers.get('Content-Length', 0))
                body = self.rfile.read(content_length)
                data = json.loads(body) if body else {}
                position_ms = data.get("position_ms")
                if isinstance(position_ms, int) and position_ms >= 0:
                    spotify_client.seek(position_ms)
                else:
                    self.send_response(400)
                    self.end_headers()
                    return

            else:
                self.send_response(404)
                self.end_headers()
                return

            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps(response_data).encode())

        except Exception as e:
            print(f"Error handling POST {self.path}: {e}")
            self.send_response(500)
            self.end_headers()


if __name__ == "__main__":
    try:
        spotify_client = SpotifyClient()
        print("Spotify Client initialized.")
    except Exception as e:
        print(f"Warning: Could not initialize Spotify Client: {e}")

    threading.Thread(target=init, daemon=True).start()

    update_thread = threading.Thread(target=actualizar_diario, daemon=True)
    update_thread.start()

    server = ThreadingHTTPServer(("0.0.0.0", 1234), Server)

    if spotify_client:
        print("Verificando cuenta de Spotify...")
        get_spotify_data()

    print("Servidor corriendo en el puerto 1234. Presiona Ctrl+C para salir.")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nCerrando el servidor...")
        server.shutdown()
        sys.exit(0)
