import json
from datetime import datetime, timedelta
import matplotlib.pyplot as plt

# Leer datos desde db.json
with open("db.json", "r") as f:
    data = json.load(f)

# Extraer la fecha más antigua de los datos
oldest_date = datetime.strptime(data["date"], "%Y-%m-%d")

# Suponiendo que todas las listas tienen la misma longitud
num_points = len(data["eth"])
# Generar la lista de fechas a partir de la fecha más antigua
dates = [oldest_date + timedelta(days=i) for i in range(num_points)]

# Configuración de la gráfica
plt.figure(figsize=(10, 6))
#for coin in ["eth", "doge", "xmr"]:
for coin in ["doge"]:
    plt.plot(dates, data[coin], marker='o', label=coin)

plt.xlabel("Fecha")
plt.ylabel("Valor")
plt.title("Evolución de las monedas en los últimos días")
plt.legend()
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()
