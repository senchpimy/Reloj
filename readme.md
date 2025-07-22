# Reloj - Monitor y Visualizador de Precios Cripto con ESP32 y Servidor HTTP

![Badge](https://img.shields.io/badge/License-MIT-blue)
![Badge](https://img.shields.io/badge/PlatformIO-Supported-orange)
![Badge](https://img.shields.io/badge/Python-3.11+-blue)

**Reloj** es un sistema completo para monitorear y visualizar precios de criptomonedas que combina:

- Un **servidor Python** que descarga, guarda y actualiza periódicamente datos históricos y diarios de precios para criptomonedas como ETH, DOGE y XMR, usando checkpointing para garantizar continuidad y confiabilidad, y que expone estos datos vía HTTP en formato JSON para su consulta.

- Un **firmware para ESP32** con pantalla E-Ink que se conecta a la red WiFi (configurable vía BLE), descarga los datos del servidor y los muestra en tiempo real junto con la hora local. Incluye manejo de multitarea para actualización y visualización fluida, y permite configuración inalámbrica sencilla mediante Bluetooth.

---

## 🚀 Características

- **📈 Monitor de precios**: Obtiene datos históricos y actualizados de criptomonedas.
- **⏱ Checkpointing robusto**: Guarda y recupera datos para evitar pérdidas.
- **🔄 Actualización automática diaria**: Mantiene datos siempre frescos.
- **🌐 Servidor HTTP con API JSON**: Facilita la consulta remota de datos.
- **📶 Firmware ESP32 con pantalla E-Ink**: Visualiza datos y hora local.
- **🔐 Configuración vía BLE**: Permite ajustar WiFi y servidor fácilmente.
- **🧵 Multitarea y hilos**: Para ejecución fluida y eficiente.

---

## ⚙️ Requisitos

- **Python 3.11+** para el servidor.
- **PlatformIO** para compilar y flashear el firmware ESP32.
- Dependencias Python: `requests`

---

## 📄 Licencia

Este proyecto está bajo la **Licencia MIT**. Consulta el archivo [LICENSE](./LICENSE) para más información.

---

## ✨ Créditos

Desarrollado por [senchpimy](https://github.com/senchpimy)
