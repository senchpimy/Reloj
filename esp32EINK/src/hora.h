#ifndef RELOJ_H
#define RELOJ_H

#include <TM1637Display.h>

class Reloj {
private:
  int hora;
  int minutos;
  unsigned long syncMillis;
  int syncHora;
  int syncMinutos;
  int hrs = 0;
  int min = 0;
  TM1637Display display;
  bool f;
  char input[10];

public:
  Reloj(int hora, int minutos);
  void sync(int h, int m);
  void aumentarMinuto();
  void reducirMinuto();
  int obtenerHora();
  void setHora(int hora, int min);
  void update_value();
  void show();
  void apagar();
};

#endif // RELOJ_H
