// ===== LIBRERÍAS =====
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "web_utils.h"
#include "hora.h"
#include "gui_utils.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Preferences.h>
#include <ArduinoJson.h>

HTTPClient http;
Preferences preferences;

char ssid[64];
char pass[64];
char serverUrl[128];

#define CONFIG_PIN 0
#define SERVICE_UUID                  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SCHEMA_CHARACTERISTIC_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8" 
#define DATA_CHARACTERISTIC_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a9"
BLEServer* pServer = NULL;
bool deviceConnected = false;

auto gui = new GUI();
long lastMillis;
auto r = new Reloj(12, 0);
auto p = Prices();
bool parpadear = true;
inline void update_data();
float eth[] = {1682.50, 1675.500000, 1664.199951, 1628.500000, 1598.500000, 1588.599976, 1581.199951, 1591.900024, 1593.099976, 1593.099976, 1601.699951, 1631.900024, 1642.300049, 1639.400024, 1628.000000, 1638.099976, 1627.400024, 1624.699951, 1598.500000, 1587.599976, 1585.400024, 1623.199951, 1634.199951, 1637.300049, 1634.900024, 1630.099976, 1628.199951, 1632.699951, 1635.199951, 1633.699951};
float xmr[] = {147.320007, 146.330002, 145.630005, 146.500000, 145.259995, 144.289993, 144.250000, 143.070007, 143.820007, 145.740005, 146.970001, 147.350006, 147.289993, 146.479996, 144.699997, 145.740005, 147.610001, 144.850006, 143.110001, 141.229996, 141.149994, 142.820007, 143.279999, 143.080002, 142.320007, 139.380005, 139.919998, 142.029999, 140.399994, 140.520004  };
float doge[] = {0.062352, 0.062069, 0.061790, 0.060970, 0.060636, 0.060714, 0.060868, 0.061460, 0.061598, 0.061542, 0.062031, 0.062409, 0.062514, 0.061968, 0.061981, 0.062320, 0.062033, 0.061655, 0.061105, 0.061243, 0.060733, 0.061960, 0.063527, 0.063335, 0.063335, 0.063757, 0.063732, 0.063178, 0.063383, 0.063549};
Data * datos;


class DataCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.println("*********");
            Serial.print("Nuevos datos recibidos: ");
            Serial.println(value.c_str());

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, value);

            if (error) {
                Serial.print("deserializeJson() falló: ");
                Serial.println(error.c_str());
                return;
            }
            const char* newSsid = doc["ssid"];
            const char* newPass = doc["pass"];
            const char* newServer = doc["server"];
            
            preferences.begin("wifi-creds", false);
            preferences.putString("ssid", newSsid);
            preferences.putString("pass", newPass);
            preferences.putString("server", newServer);
            preferences.end();
            
            Serial.println("Credenciales guardadas. Reiniciando...");
            delay(1000);
            ESP.restart();
        }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; Serial.println("Dispositivo conectado"); }
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; Serial.println("Dispositivo desconectado"); pServer->getAdvertising()->start(); }
};


void setupBLE() {
    Serial.println("Iniciando modo configuración por BLE...");
    
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char deviceName[32];
    sprintf(deviceName, "Configurador_CryptoReloj_%02X%02X", mac[4], mac[5]);
    
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pSchemaCharacteristic = pService->createCharacteristic(SCHEMA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
    
    const char* schemaJson = R"json([
        { "key": "ssid", "label": "Nombre de Red (SSID)", "type": "string", "required": true },
        { "key": "pass", "label": "Contraseña WiFi", "type": "password", "required": false },
        { "key": "server", "label": "URL del Servidor", "type": "string", "required": true }
    ])json";

    pSchemaCharacteristic->setValue(schemaJson);
    Serial.print("Esquema de configuración publicado: ");
    Serial.println(schemaJson);

    BLECharacteristic *pDataCharacteristic = pService->createCharacteristic(DATA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
    pDataCharacteristic->setCallbacks(new DataCharacteristicCallbacks());

    pService->start();
    BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();
    Serial.println("Esperando conexión de la app...");
}

bool loadCredentials() {
    preferences.begin("wifi-creds", true);
    String savedSsid = preferences.getString("ssid", "");
    String savedPass = preferences.getString("pass", "");
    String savedServer = preferences.getString("server", "");
    preferences.end();
    if (savedSsid.length() > 0 && savedServer.length() > 0) {
        strcpy(ssid, savedSsid.c_str());
        strcpy(pass, savedPass.c_str());
        strcpy(serverUrl, savedServer.c_str());
        Serial.println("Credenciales cargadas desde memoria.");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Servidor: ");
        Serial.println(serverUrl);
        Serial.print("Contraseña: ");
        Serial.println(pass);
        return true;
    }
    Serial.println("No se encontraron credenciales válidas.");
    return false;
}

void connectToWiFi() {
    Serial.print("Conectando a: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado a WiFi!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFallo de conexión. Borrando credenciales y reiniciando...");
      delay(2000);
      preferences.begin("wifi-creds", false);
      preferences.clear();
      preferences.end();
      ESP.restart();
    }
}

void loop() {}

void setup() {
  Serial.begin(115200);
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  
  if (digitalRead(CONFIG_PIN) == LOW) {
    Serial.println("Botón presionado. Forzando modo configuración.");
    preferences.begin("wifi-creds", false);
    preferences.clear();
    preferences.end();
    delay(100);
  }

  if (!loadCredentials()) {
    setupBLE();
    return;
  }

  connectToWiFi();

  bool task = p.update(&http, serverUrl);

  datos = (Data*) malloc(sizeof(Data) * 3);
  if (task) {
    update_data();
    int result = p.set_time(&r);
    if (result == 0)
      parpadear = false;
  }

  TaskHandle_t Task1;
  xTaskCreatePinnedToCore(
    relojLoop, "Reloj", 10000, NULL, tskIDLE_PRIORITY, &Task1, 0
  );

  gui -> init(1);
  int i = 0;
  int hora = 0;
  while (1) {
    if (hora != (r->obtenerHora() / 100)) {
      hora = r->obtenerHora() / 100;
      gui->draw_graph(&datos[i]);
      i++;
      if (i == 3) {
        i = 0;
      }
    }
    delay(500);
  }
}

void relojLoop( void * parameters) {
  Serial.print("Tarea Reloj corriendo en el core: ");
  Serial.println(xPortGetCoreID());
  bool parpa = true;
  bool updateDatos = true;

  while (true) {
    if (updateDatos) {
      bool result = p.update(&http, serverUrl); 
      if (result) {
        update_data();
        p.set_time(&r);
      }else {
        //TODO manejar error de actualización
        Serial.println("Error al actualizar los datos.");
      }
      updateDatos = false;
    }
    if (r->obtenerHora() == 2359) {
      updateDatos = true;
    }

    if (parpadear) {
      if (parpa) { r->apagar(); parpa = false; }
      else { r->show(); parpa = true; }
    } else {
      r->show();
    }

    if (millis() - lastMillis >= 60 * 1000UL) {
      lastMillis = millis();
      r->aumentarMinuto();
    }
    if (digitalRead(2)) { r->aumentarMinuto(); parpadear = false; }
    if (digitalRead(4)) { r->reducirMinuto(); parpadear = false; }
    
    delay(200);
  }
}

inline void update_data() {
  p.gen_precios((char *)"doge", doge);
  p.gen_precios((char *)"eth", eth);
  p.gen_precios((char *)"xmr", xmr);
  datos[0] = Data("doge", doge);
  datos[2] = Data("eth", eth);
  datos[1] = Data("xmr", xmr);
}
