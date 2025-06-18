import json
import requests
from datetime import date, timedelta, datetime
import re
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import signal
import sys
import os

headers = {'User-Agent': 'curl'}
site = "https://rate.sx"
coins = ["eth", "doge", "xmr"]
lock = threading.Lock()
FILE = ""
DELAY = 10  # Mayor delay entre requests
DB_FILE = "db.json"
data = {}  # Datos globales en memoria

def modiffy_str(new_data):
    with lock:
        global FILE
        FILE = new_data

def get_str():
    with lock:
        # Si FILE está vacío, retorna un JSON por defecto "{}"
        return FILE if FILE.strip() else json.dumps({})

modiffy_str("")

def save_to_json():
    """Guarda los datos actuales en el archivo JSON."""
    try:
        data["time"] = datetime.now().strftime("%H:%M")
        with open(DB_FILE, "w") as f:
            json.dump(data, f, indent=4)
        modiffy_str(json.dumps(data))
        print("\nDatos guardados en db.json correctamente.")
    except Exception as e:
        print(f"\nError al guardar JSON: {str(e)}")

def signal_handler(sig, frame):
    """Maneja la señal de interrupción (Ctrl+C), guardando los datos antes de salir."""
    print("\nInterrupción detectada. Guardando datos antes de salir...")
    save_to_json()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def get_value(coin: str, target_date: date):
    try:
        previous_date = target_date - timedelta(days=1)
        url = f"{site}/{coin}@{previous_date}..{target_date}"
        response = requests.get(url, headers=headers)
        response.raise_for_status()
        
        avg_pattern = re.search(r'avg:\x1b\[0m \$(\d+\.?\d*)\x1b\[2m', response.text)
        return float(avg_pattern.group(1)) if avg_pattern else 0.0
    except Exception as e:
        print(f"Error en {coin} para {target_date}: {str(e)}")
        return 0.0

def init():
    """Inicializa la base de datos con los últimos 30 días de valores."""
    global data
    print("Inicializando últimos 30 días...")
    data = {coin: [] for coin in coins}
    data["date"] = date.today().strftime("%Y-%m-%d")
    
    for days_back in range(30):
        target_date = date.today() - timedelta(days=days_back)
        daily_data = {}
        
        for coin in coins:
            daily_data[coin] = get_value(coin, target_date)
            print(f"Obteniendo valor para {coin} para el día {target_date}")
            time.sleep(DELAY)  # Delay entre requests
            
        for coin in coins:
            data[coin].insert(0, daily_data[coin])  # Insertar al inicio
        
        data["date"] = target_date.strftime("%Y-%m-%d")
        save_to_json()
        print(f"Día {target_date} registrado")
    
    save_to_json()

def actualizar_diario():
    """Actualiza los datos diariamente."""
    global data
    while True:
        try:
            if not os.path.exists(DB_FILE):
                print("No se encontró db.json, inicializando...")
                init()
                continue

            with open(DB_FILE, "r") as f:
                data = json.load(f)
            
            ultima_fecha = datetime.strptime(data["date"], "%Y-%m-%d").date()
            hoy = date.today()
            
            if ultima_fecha >= hoy:
                time.sleep(3600*24)  # Esperar 1 día si ya está actualizado
                continue
                
            print(f"Actualizando nuevo día: {hoy}")
            nuevos_datos = {}
            
            for coin in coins:
                nuevos_datos[coin] = get_value(coin, hoy)
                print(f"Obteniendo valor de {coin}")
                time.sleep(DELAY)
            
            for coin in coins:
                data[coin].insert(0, nuevos_datos[coin])
                if len(data[coin]) > 30:
                    data[coin].pop()  # Eliminar el más antiguo
            
            data["date"] = hoy.strftime("%Y-%m-%d")
            save_to_json()
            
            time.sleep(86400 - DELAY * len(coins))  # Esperar 24 horas exactas
            
        except Exception as e:
            print(f"Error en actualización: {str(e)}")
            time.sleep(3600)

class Server(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        respuesta = json.loads(get_str())
        respuesta["time"] = datetime.now().strftime("%H:%M")
        self.wfile.write(json.dumps(respuesta).encode())

if __name__ == "__main__":
    if not os.path.exists(DB_FILE):
        init()
    else:
        with open(DB_FILE, "r") as f:
            data = json.load(f)  # Cargar datos iniciales
        # Actualizamos FILE con el contenido leído
        modiffy_str(json.dumps(data))

    # Iniciar el hilo para actualizar datos diariamente
    update_thread = threading.Thread(target=actualizar_diario)
    update_thread.daemon = True
    update_thread.start()

    # Iniciar el servidor HTTP en un hilo separado
    server_thread = threading.Thread(target=HTTPServer(("0.0.0.0", 1234), Server).serve_forever)
    server_thread.daemon = True
    server_thread.start()
    
    print("Servidor corriendo en el puerto 8080. Presiona Ctrl+C para salir.")
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nCerrando el programa...")
        save_to_json()
        sys.exit(0)
