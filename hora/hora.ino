//#include <Arduino.h>
#include <TM1637Display.h>

#define CLK A0
#define DIO 9

class Reloj {
private:
    int hora;
    int minutos;

public:
    Reloj(int hora, int minutos) {
        if (hora >= 0 && hora <= 23 && minutos >= 0 && minutos <= 59) {
            this->hora = hora;
            this->minutos = minutos;
        } else {
            this->hora = 0;
            this->minutos = 0;
        }
    }

    void aumentarMinuto() {
            minutos += 1;
            hora += minutos / 60;
            minutos %= 60;
            hora %= 24; // Asegurarse de que la hora esté en formato de 24 horas
        
    }

    int obtenerHora() {
        return (hora*100)+minutos;
    }
   void setHora(int hora, int min){
    this->hora=hora;
    this->minutos=min;
    }
};

TM1637Display display(CLK, DIO);
auto r = Reloj(16,02);
unsigned long lastMillis;
char input[10];
int hrs=0;
int min=0;

void setup()
{
  display.setBrightness(0x0f);
  Serial.begin(9600);
}
  bool f = true;
  int g = 0;
  int byte_i = 0; // for incoming serial data
void loop()
{
  if (Serial.available() > 0) {
    Serial.readBytes(input,10);
    for (int i = 0; i<2; i++){
      hrs=hrs*10;
      hrs+=((int)input[i])-48;
    }
    for (int i = 2; i<4; i++){
      min=min*10;
      min+=((int)input[i])-48;
    }
    r.setHora(hrs,min);
  }

 if (millis() - lastMillis >= 60*1000UL) 
  {
   lastMillis = millis();
   r.aumentarMinuto();
  }
    if (f){
  display.showNumberDecEx(r.obtenerHora(), 0b11100000, false, 4, 0);  
  }else{
     display.showNumberDecEx(r.obtenerHora(), 0b00000000, false, 4, 0);  
     }
  f=!f;
  delay(500);
}
