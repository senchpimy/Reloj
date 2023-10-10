#include <WiFi.h>
#include <HTTPClient.h>
HTTPClient http;
#include "web_utils.h"
#include "arduino_secrets.h"
#include <ArduinoJson.h>
#include "hora.h"
#include "gui_utils.h"


auto gui = new GUI();
void loop(){}
void setup() {
  //get_val_of_date("This is a char*",233);
  Serial.begin(115200);
  //float eth[]={1682.50, 1675.500000, 1664.199951, 1628.500000, 1598.500000, 1588.599976, 1581.199951, 1591.900024, 1593.099976, 1593.099976, 1601.699951, 1631.900024, 1642.300049, 1639.400024, 1628.000000, 1638.099976, 1627.400024, 1624.699951, 1598.500000, 1587.599976, 1585.400024, 1623.199951, 1634.199951, 1637.300049, 1634.900024, 1630.099976, 1628.199951, 1632.699951, 1635.199951, 1633.699951};
  //float xmr[] = {147.320007,146.330002,145.630005,146.500000,145.259995,144.289993,144.250000,143.070007,143.820007,145.740005,146.970001,147.350006,147.289993,146.479996,144.699997,145.740005,147.610001,144.850006,143.110001,141.229996,141.149994,142.820007,143.279999,143.080002,142.320007,139.380005,139.919998,142.029999,140.399994,140.520004  };
  float doge[]={0.062352, 0.062069, 0.061790, 0.060970, 0.060636, 0.060714, 0.060868, 0.061460, 0.061598, 0.061542, 0.062031, 0.062409, 0.062514, 0.061968, 0.061981, 0.062320, 0.062033, 0.061655, 0.061105, 0.061243, 0.060733, 0.061960, 0.063527, 0.063335, 0.063335, 0.063757, 0.063732, 0.063178, 0.063383, 0.063549};
  //sf.draw_string("HOLA", 100,100);
  auto data = Data("doge", doge);
  gui -> init(3);
  gui->progress(0);
  
  //gui->draw_graph(&data);
  /*connect();
  char* holder =(char *) malloc(sizeof(char)*2000);//
  
  int price_count = 200;
  auto f = Fecha(&http);
  f.fill_data();
  f.gen_dates();
  f.print_dates();
  auto p = Prices();
  char * name="eth";
  p.gen_precios(name,f.dates,holder,price_count,&http);
  p.print_values();*/
//  f.print_dates();
 // Reloj r(21,0);
//  r.show();
  //gui -> draw_graph( & data);
  /*int price_count = 0;
  int time_count = 0;
  char ch;
  StaticJsonDocument<4000> doc;
    char json[] =
      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
  DeserializationError error = deserializeJson(doc, json);*/
  
  //free(BlackImage);
  //BlackImage = NULL;
}
auto r = Reloj(21,24);
/*
void setup(){
  Serial.begin(115200);
  Serial.println("\nListo");
  r.show();
  Serial.println("\nListo");
}

void loop() {
  r.update_value();
  delay(500);
  }*/

inline void connect(){
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
  }
  WiFi.waitForConnectResult();
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(1,1,1,1));
}
