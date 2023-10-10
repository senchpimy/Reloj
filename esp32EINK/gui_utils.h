#ifndef MY_HEADER_H
#define MY_HEADER_H

#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "font.h"

class Font;
class BigFont;
class SmallFont;
class Transform;
class GraphCoordGen;
class Data;
class GUI;

class Font {
public:
  const unsigned char * data;
  int largo_total;
  int altura;
  int ancho;

  void draw_char(char w, int x, int y);
  void draw_string(char * s, int x, int y);
};

class BigFont : public Font {
public:
  BigFont();
};

class SmallFont : public Font {
public:
  SmallFont();
  void draw_string(char * s, int x, int y);
  void draw_char(char w, int x, int y);

};

class Transform {
public:
  int x0;
  int y0;
  int x1;
  int y1;
  void tt(int X0, int Y0, int X1, int Y1);
  void mostrar_valores();
};

class GraphCoordGen {
  private: float coord_min;
  float paso;
  float val_min;
public:
  GraphCoordGen(float val_max, float val_min, float coord_max, float coord_min);
  int form_pen(float y);
};

class Data {
public:
  char * nombre;
  float max_v;
  float min_v;
  float * lista;
  char * string_float_repr;
  boolean scaled;
  Data(char * nombre, float * lista);
  void find_max_min();
  void plot();
  void draw_labels();
  void draw_graph();
};

class GUI {
 private:
  int numberCoins;
public:
  void init(int);
  void draw_graph(Data * d);
  void progress(int number);
};

#endif // MY_HEADER_H
