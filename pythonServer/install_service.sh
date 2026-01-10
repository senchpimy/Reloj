#!/bin/bash

if [ "$EUID" -ne 0 ]; then
  echo "Por favor, ejecuta con sudo para instalar el servicio."
  exit 1
fi

SERVICE_FILE="reloj.service"
SYSTEMD_PATH="/etc/systemd/system/$SERVICE_FILE"

echo "Instalando el servicio $SERVICE_FILE..."

cp "$SERVICE_FILE" "$SYSTEMD_PATH"

systemctl daemon-reload

systemctl enable "$SERVICE_FILE"
systemctl restart "$SERVICE_FILE"

echo "Instalación completada."
echo "Puedes ver los logs con: sudo journalctl -u $SERVICE_FILE -f"
echo "Recuerda que si es la primera vez, deberás ver los logs para autorizar Spotify."
