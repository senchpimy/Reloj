#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include <Arduino.h>

// Declaraciones a futuro
class GraphCoordGen;
class Data;
class GUI;
class Font;
class BigFont;
// class SmallFont; // Sigue comentada

class Font {
public:
  // El puntero apuntará a datos en PROGMEM
  const unsigned char * data;
  int largo_total;
  int altura;
  int ancho;

  void draw_char(char w, int x, int y);
  // Se usa const char* para evitar warnings
  void draw_string(const char* s, int x, int y);
};

class BigFont : public Font {
public:
  BigFont();
};

/*
class SmallFont : public Font {
public:
  SmallFont();
  void draw_string(const char* s, int x, int y);
  void draw_char(char w, int x, int y);
};
*/

class GraphCoordGen {
private:
  float coord_min, paso, val_min;
public:
  GraphCoordGen(float val_max, float val_min, float coord_max, float coord_min);
  int form_pen(float y);
};

class Data {
public:
  const char* nombre;
  float max_v, min_v;
  float* lista;
  char* string_float_repr;
  boolean scaled;
  
  Data(const char* nombre, float* lista);
  void find_max_min();
  void plot();
  void draw_labels();
  void draw_graph();
};

class GUI {
private:
  int numberCoins;
public:
  void init(int coins);
  void draw_graph(Data* d);
  void progress(int number);
  void draw_image_from_stream(Stream* stream, size_t totalSize);
  void test_pattern();
};

#endif // GUI_UTILS_H
