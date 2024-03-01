#include "stdio.h"
#include "web_utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

const int DATE_STRING_SIZE = 11;

#include <WiFiClientSecure.h>


bool Prices::update(HTTPClient * http) {
  WiFiClient cli;
  for (int tries = 0; tries < 10; tries++) {
    http->begin(cli, (char *)SERVER);
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

void Prices::set_time(Reloj** r) {
  int hora, minuto;
  String str = doc["time"].as<String>();
  sscanf(str.c_str(), "%i:%i", &hora, &minuto);
  *r = new Reloj(hora, minuto);
  (*r)->show();
}

void Prices::print_values() {
  Serial.printf("CRYPTO:");
  for (int i = 0; i < 30; i++) {
  }
  Serial.printf("\n\n");
}
