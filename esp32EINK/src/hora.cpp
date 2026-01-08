#include <Arduino.h>
#include <TM1637Display.h>
#include "hora.h"

#define CLK 23
#define DIO 22

Reloj::Reloj(int hora, int minutos) : display(CLK, DIO) {
    display.setBrightness(5);
    f = true;
    sync(hora, minutos);
}

void Reloj::sync(int h, int m) {
    this->syncHora = h;
    this->syncMinutos = m;
    this->syncMillis = millis();
    this->hora = h;
    this->minutos = m;
}

void Reloj::aumentarMinuto() {
    // Para botones manuales: adelantamos el tiempo de sincronización
    syncMinutos++;
    if (syncMinutos >= 60) {
        syncMinutos = 0;
        syncHora = (syncHora + 1) % 24;
    }
}

void Reloj::reducirMinuto() {
    syncMinutos--;
    if (syncMinutos < 0) {
        syncMinutos = 59;
        syncHora = (syncHora + 23) % 24;
    }
}

int Reloj::obtenerHora() {
    // Calculamos el tiempo total transcurrido en segundos
    unsigned long elapsedSecs = (millis() - syncMillis) / 1000;
    
    int totalMinutes = (syncHora * 60) + syncMinutos + (elapsedSecs / 60);
    int h = (totalMinutes / 60) % 24;
    int m = totalMinutes % 60;
    
    return (h * 100) + m;
}
  void Reloj::setHora(int hora, int min) {
    this -> hora = hora;
    this -> minutos = min;
  }
  void Reloj::update_value() {
    if (Serial.available() > 0) {
      Serial.readBytes(input, 10);
      Serial.print("\nValor Actualizado");
      Serial.println(input);
      for (int i = 0; i < 2; i++) {
        hrs = hrs * 10;
        hrs += ((int) input[i]) - 48;
      }
      for (int i = 2; i < 4; i++) {
        min = min * 10;
        min += ((int) input[i]) - 48;
      }
      this -> setHora(hrs, min);
      this->show();
    }

  }
  void Reloj::show() {
    display.showNumberDecEx(this -> obtenerHora(), 0b11100000, false, 4, 0);
  }
  void Reloj::apagar(){
   display.clear();
  }

//auto r = Reloj(16, 02);

/*
 if (millis() - lastMillis >= 60*1000UL) //cada minuto
  {
   lastMillis = millis();
   r.aumentarMinuto();
  }
    if (f){
  display.showNumberDecEx(r.obtenerHora(), 0b11100000, false, 4, 0);  
  }else{
     display.showNumberDecEx(r.obtenerHora(), 0b00000000, false, 4, 0);  
     }
  f=!f;*/
