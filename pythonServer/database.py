import sqlite3
from datetime import datetime, timedelta

DB_NAME = "crypto_data.db"

class Database:
    def __init__(self, db_name=DB_NAME):
        self.db_name = db_name
        self.init_db()

    def get_connection(self):
        return sqlite3.connect(self.db_name)

    def init_db(self):
        """Crea la tabla si no existe."""
        conn = self.get_connection()
        cursor = conn.cursor()
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS prices (
                date TEXT NOT NULL,
                coin TEXT NOT NULL,
                price REAL NOT NULL,
                PRIMARY KEY (date, coin)
            )
        ''')
        conn.commit()
        conn.close()

    def insert_price(self, coin, date_str, price):
        """Inserta o actualiza un precio."""
        conn = self.get_connection()
        cursor = conn.cursor()
        cursor.execute('''
            INSERT OR REPLACE INTO prices (date, coin, price)
            VALUES (?, ?, ?)
        ''', (date_str, coin, price))
        conn.commit()
        conn.close()

    def get_last_30_days(self, coin):
        """Obtiene los últimos 30 precios ordenados por fecha."""
        conn = self.get_connection()
        cursor = conn.cursor()
        # Obtenemos los últimos 30 registros
        cursor.execute('''
            SELECT price FROM prices 
            WHERE coin = ? 
            ORDER BY date DESC 
            LIMIT 30
        ''', (coin,))
        rows = cursor.fetchall()
        conn.close()
        # SQL devuelve del más nuevo al más viejo, invertimos para que sea cronológico
        return [row[0] for row in rows][::-1]

    def get_latest_date(self):
        """Devuelve la fecha más reciente registrada en la DB."""
        conn = self.get_connection()
        cursor = conn.cursor()
        cursor.execute('SELECT MAX(date) FROM prices')
        result = cursor.fetchone()
        conn.close()
        return result[0] if result and result[0] else None

    def get_oldest_date_in_limit(self, limit=30):
        """Devuelve la fecha más antigua dentro del límite de días (para saber start_date)."""
        conn = self.get_connection()
        cursor = conn.cursor()
        # Asumimos que todas las monedas tienen las mismas fechas, miramos una cualquiera (eth)
        cursor.execute('''
            SELECT date FROM prices 
            WHERE coin = 'eth' 
            ORDER BY date DESC 
            LIMIT ?
        ''', (limit,))
        rows = cursor.fetchall()
        conn.close()
        if rows:
            return rows[-1][0] # La última de la lista (la más antigua de los 30)
        return None
