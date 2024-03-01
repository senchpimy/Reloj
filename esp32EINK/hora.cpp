#include <Arduino.h>
#include <TM1637Display.h>
#include "hora.h"

#define CLK 23
#define DIO 22

Reloj::Reloj(int hora, int minutos) : display(CLK, DIO) {
    //display.setBrightness(0x0f);
    display.setBrightness(5);
    char input[10];
    f = true;
    if (hora >= 0 && hora <= 23 && minutos >= 0 && minutos <= 59) {
      this -> hora = hora;
      this -> minutos = minutos;
    } else {
      this -> hora = 0;
      this -> minutos = 0;
    }
  }

  void Reloj::aumentarMinuto() {
    minutos += 1;
    hora += minutos / 60;
    minutos %= 60;
    hora %= 24; // Asegurarse de que la hora esté en formato de 24 horas
  }

  void Reloj::reducirMinuto() {
    if (minutos > 0) {
        minutos -= 1;
    } else {
        minutos = 59; 
        if (hora > 0) {
            hora -= 1;
        } else {
            hora = 23;
        }
    }
}

  int Reloj::obtenerHora() {
    return (hora * 100) + minutos;
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
