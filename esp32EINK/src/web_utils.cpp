#include "stdio.h"
#include "web_utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

const int DATE_STRING_SIZE = 11;

#include <WiFiClientSecure.h>

bool SpotifyHandler::getStatus(HTTPClient * http, const char* serverUrl, SpotifyStatus &status) {
    String url = String(serverUrl) + "/spotify";
    http->begin(url);
    int code = http->GET();
    if (code == 200) {
        String payload = http->getString();
        deserializeJson(doc, payload);
        status.is_playing = doc["is_playing"];
        if (status.is_playing) {
            status.title = doc["title"].as<String>();
            status.artist = doc["artist"].as<String>();
            status.image_hash = doc["image_hash"].as<String>();
            status.dominant_color = doc["dominant_color"] | 0;
        }
        http->end();
        return true;
    }
    http->end();
    return false;
}

bool SpotifyHandler::processImageStream(HTTPClient * http, const char* serverUrl, std::function<void(Stream*, size_t)> processor) {
    String url = String(serverUrl) + "/spotify/image";
    Serial.print("Iniciando stream desde: "); Serial.println(url);
    
    http->setTimeout(15000); 
    http->begin(url);
    
    int code = http->GET();
    bool success = false;

    if (code == 200) {
        int len = http->getSize();
        if (len > 0) {
            WiFiClient * stream = http->getStreamPtr();
            processor(stream, len);
            success = true;
        } else {
            Serial.println("Error: Content-Length inválido.");
        }
    } else {
        Serial.print("Error descarga HTTP: "); Serial.println(code);
    }
    
    http->end();
    return success;
}


bool Prices::update(HTTPClient * http, const char* serverUrl) {
  WiFiClient cli;
  for (int tries = 0; tries < 10; tries++) {
    Serial.print("Connecting to: "); Serial.println(serverUrl);
    http->begin(cli, (char *)serverUrl);
    int httpResponseCode = http->GET();
    Serial.println(httpResponseCode);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http->getString();
      Serial.println(payload);
      deserializeJson(doc, payload);
      return true;
    }
    delay(1000 * 10);
  }
  return false;
}

void Prices::gen_precios(char* name, float prices[]) {
  JsonArray arr = doc[name].as<JsonArray>();
  int i = 0;
  for (JsonVariant v : arr) {
    float val = v.as<float>();
    prices[i] = val;
    i++;
  }

}

int Prices::set_time(Reloj** r) {
  String str = doc["time"].as<String>();
  Serial.print("HORA OBTENIDA: ");
  Serial.println(str);
  //sscanf(str.c_str(), "%i:%i", &hora, &minuto); //for some reason some times it doesnt parse correctly
  char * ptr =(char *) str.c_str();
  ptr[2] = '\0';
  int hora = atoi(ptr);
  int minuto = atoi(ptr+3);
  Serial.print("INTS OBTENIDOS DE LA HORA: ");
  Serial.print(hora);
  Serial.print(",");
  Serial.println(minuto);
  
  if (hora == 0 && minuto == 0){
    Serial.println("Parse failed");
    return 1;
  }
  
  (*r)->sync(hora, minuto);
  (*r)->show();
  return 0;
}

void Prices::print_values() {
  Serial.printf("CRYPTO:");
  for (int i = 0; i < 30; i++) {
  }
  Serial.printf("\n\n");
}
