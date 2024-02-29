import json
import requests
from datetime import date,timedelta,datetime
import re
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import signal
import sys
import os

headers = {'User-Agent':'curl'}
site = "https://rate.sx"
coins = ["eth", "doge","xmr"]
values = {}
lock = threading.Lock()

def modiffy_str(str):
    with lock:
        global FILE
        FILE = str

def get_str():
    with lock:
        return FILE

modiffy_str("")

def signal_handler(sig, frame):
    print('\nProgram is being terminated...')
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def get_value(coin:str, d1:str,d2:str)->float:
    str = f"{site}/{coin}@{d2}..{d1}"
    print(str)
    r = requests.get(str,headers=headers)
    #TODO HANDLE error in request
    avg_pattern = re.search(r'avg:\x1b\[0m \$(\d+(\.\d+)?)\x1b\[2m',r.text)
    #TODO HANDLE avg_pattern being None
    res = avg_pattern.group(1)
    return float(res)

def init():
    for coin in coins:
        arr=[]
        for iter in range(30):
            td = date.today()-timedelta(days=iter)
            d1 = td.strftime("%Y-%m-%d")
            db = date.today()-timedelta(days=iter+1)
            d2 = db.strftime("%Y-%m-%d")
            res = get_value(coin,d1,d2)
            arr.append(res)
            time.sleep(2)
        values[coin]=arr[::-1]
    t = date.today().strftime("%Y-%m-%d")
    values["date"]=t
    values["time"]=datetime.now().strftime("%H:%M")
    file = json.dumps(values)
    with open("db.json","w") as f:
        f.write(file)
    print("Finalizing CREATING db")
    modiffy_str(file)
			
#init()
def check_db():
    print("Cheking DB...")
    if os.path.isfile("db.json"):
        print("DB FOUND")
        f = open("db.json","r").read() #CHECK If it is up to date
        modiffy_str(f)
    else:
        print("Creating New DB")
        init()
    conj = json.loads(get_str())
    current_db = datetime.strptime(conj["date"],"%Y-%m-%d")
    while True:
        hoy = datetime.now() 
        res = hoy-current_db
        if res.days>1:
            for i in range(res.days):
                for coin in coins:
                    conj[coin].pop()
                    td = date.today()-timedelta(days=i)
                    d1 = td.strftime("%Y-%m-%d")
                    db = date.today()-timedelta(days=i+1)
                    d2 = db.strftime("%Y-%m-%d")
                    res = get_value(coin,d1,d2) 
                    conj[coin].insert(0,res)
            
hostname="localhost"
port = 8080
class Server(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type","text/json")
        self.end_headers()
        f = get_str()
        dic = json.loads(f)
        dic["time"]=datetime.now().strftime("%H:%M")
        text = json.dumps(dic)
        print(f"Sending {text}")
        modiffy_str(text)
        self.wfile.write(bytes(text,"utf-8"))


if __name__=="__main__":
    threading.Thread(target=check_db,daemon=True).start()
    webServer = HTTPServer((hostname,port),Server)
    try:
        print("Server Started")
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server STOPPED")
