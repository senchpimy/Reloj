#include <WiFi.h>
#include <HTTPClient.h>
HTTPClient http;
#include "web_utils.h"
#include "arduino_secrets.h"
#include "hora.h"
#include "gui_utils.h"
#include <time.h>

auto gui = new GUI();
long lastMillis;
auto r = new Reloj(12,0);
bool UpdateDatosSig=false;
void loop(){}
void setup() {
  Serial.begin(115200);
  TaskHandle_t Task1;
  xTaskCreatePinnedToCore(
     relojLoop, /* Function to implement the task */
     "Reloj", /* Name of the task */
     10000,  /* Stack size in words */
     NULL,  /* Task input parameter */
     tskIDLE_PRIORITY,  /* Priority of the task */
     &Task1,  /* Task handle. */
     0); /* Core where the task should run */
  //connect();

  //http.setUserAgent("curl");
  //auto f = Fecha(&http);
  //char* holder =(char *) malloc(sizeof(char)*2300);
  //int price_count = 200; 
  //f.fill_data_v2(); //Obtiene la fecha actual

 
  //f.gen_dates(); // Crea un array en el formato con las fechas de los ultimos 30 dias
  float eth[]={1682.50, 1675.500000, 1664.199951, 1628.500000, 1598.500000, 1588.599976, 1581.199951, 1591.900024, 1593.099976, 1593.099976, 1601.699951, 1631.900024, 1642.300049, 1639.400024, 1628.000000, 1638.099976, 1627.400024, 1624.699951, 1598.500000, 1587.599976, 1585.400024, 1623.199951, 1634.199951, 1637.300049, 1634.900024, 1630.099976, 1628.199951, 1632.699951, 1635.199951, 1633.699951};
  float xmr[] = {147.320007,146.330002,145.630005,146.500000,145.259995,144.289993,144.250000,143.070007,143.820007,145.740005,146.970001,147.350006,147.289993,146.479996,144.699997,145.740005,147.610001,144.850006,143.110001,141.229996,141.149994,142.820007,143.279999,143.080002,142.320007,139.380005,139.919998,142.029999,140.399994,140.520004  };
  float doge[]={0.062352, 0.062069, 0.061790, 0.060970, 0.060636, 0.060714, 0.060868, 0.061460, 0.061598, 0.061542, 0.062031, 0.062409, 0.062514, 0.061968, 0.061981, 0.062320, 0.062033, 0.061655, 0.061105, 0.061243, 0.060733, 0.061960, 0.063527, 0.063335, 0.063335, 0.063757, 0.063732, 0.063178, 0.063383, 0.063549};
  Data * datos = (Data*) malloc(sizeof(Data)*3);
 /* datos[0] = Data("doge", doge); // Asignar los datos recogidos
  datos[1] = Data("eth", eth);
  datos[2] = Data("xmr", xmr);*/

  gui -> init(3);

  //gui->progress(0);
  auto p = Prices();
  //p.gen_precios_v2("doge",CM_API_KEY,&http);
  datos[0] = Data("doge", doge);

  //gui->progress(1);
  //auto p1 = Prices();
  //p1.gen_precios_v2("eth",CM_API_KEY,&http);
  datos[1] = Data("eth", eth);

  //gui->progress(2);
  //auto p2 = Prices();
  //p2.gen_precios_v2("xmr",CM_API_KEY,&http);
  datos[2] = Data("xmr", xmr);
  
      //Limpiar pantalla
  Paint_Clear(EPD_5IN65F_WHITE);
   int i =0;
   int hora=0;
  while(1){
    if (hora!=(r->obtenerHora()/100))
      {
        hora = r->obtenerHora()/100;
        Paint_Clear(EPD_5IN65F_WHITE);
        gui->draw_graph(&datos[i]);  
        i++;
        if (i==3){i=0;}
    }
  }
  

  //free(BlackImage);
  //BlackImage = NULL;
}

void relojLoop( void * parameters) {
  Serial.print("RUNNIN ON ANOTHER CORE");
  Serial.println(xPortGetCoreID());
  int buttonState = 0; 
  bool parpa=true;
  bool parpadear=true;
  bool updateDatos=true;
  
  while(true) {
    if (r->obtenerHora()==2358 && updateDatos){
      //Obtener nuevos datos
      UpdateDatosSig=true;
      updateDatos=false;
    }
    if (r->obtenerHora()==2359){
      updateDatos=true;
    }
    
    if (parpadear){
      if (parpa){
        r->apagar();
        parpa=false;
      }
      else{
        r->show();
        parpa=true;
      }
    }
  if (millis() - lastMillis >= 60*1000UL) //cada minuto
     {
      lastMillis=millis();
      r->aumentarMinuto();
      r->show();
    }
    if (digitalRead(2)){
      r->aumentarMinuto();
      parpadear=false;
    }
    if (digitalRead(4)){
      r->reducirMinuto();
      parpadear=false;
    }
    delay(200);
  }
  
}

inline void connect(){
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
  }
  WiFi.waitForConnectResult();
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(1,1,1,1));
}
