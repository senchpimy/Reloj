#ifndef RELOJ_H
#define RELOJ_H

#include <TM1637Display.h>

class Reloj {
private:
  int hora;
  int minutos;
  int hrs = 0;
  int min = 0;
  TM1637Display display;
  unsigned long lastMillis;
  bool f;
  char input[10];

public:
  Reloj(int hora, int minutos);
  void aumentarMinuto();
  int obtenerHora();
  void setHora(int hora, int min);
  void update_value();
  void show();
  void apagar();
};

#endif // RELOJ_H
