#include "DEV_Config.h"

#include "EPD.h"

#include "GUI_Paint.h"

#include <WiFi.h>
#include <HTTPClient.h>
HTTPClient http;
#include "web_utils.h"
#include "arduino_secrets.h"
#include <ArduinoJson.h>
#include "font.h"
#include "hora.h"

UBYTE * BlackImage;
const int ALTO = 448;
const int DIAS = 30;
const int ANCHO = 600;
const int NUM_L_W = 11;
const int NUM_L_H = 8;
const int DESFACE = 25;
const int WIDTH = ALTO / (NUM_L_W + 1);
const int SCALE_F = 10000;
const int HEIGHT = ANCHO / (NUM_L_H);

int charToValue(char w);
void prepare_display();
void draw_grid();
void show_buffer();
int round_to(float number);

class GraphCoordGen {
  private: float coord_min;
  float paso;
  float val_min;

  public: GraphCoordGen(float val_max, float val_min, float coord_max, float coord_min) {
    float dif_uni = val_max - val_min;
    float dif_coord = coord_max - coord_min;
    this -> coord_min = coord_min;
    this -> paso = dif_coord / dif_uni;
    this -> val_min = val_min;
    //   Serial.printf("val min= %f; dif_uni=%f; dif_coord=%f; paso=%f\n",val_min,dif_uni,dif_coord, paso);
  }

  int form_pen(float y) {
    float dis_y = y - this -> val_min;
    float pasos_tot = dis_y * this -> paso;
    return (int)
      (this -> coord_min + pasos_tot);
  }
};

class Font {
  public: const unsigned char * data;
  int largo_total;
  int altura;
  int ancho;

  void draw_char(char w, int x, int y) {
    int letra = charToValue(w);
    if (letra == -1) {
      Serial.println("Nothing to do");
      return;
    }
    int ini = ancho * letra;
    int fin = ancho * (letra + 1);
    for (int row = 0; row < altura; row++) {
      for (int col = ini; col < fin; col++) {
        int bitPosition = (row * largo_total) + col;

        int byteIndex = bitPosition / 8;

        int bitIndex = bitPosition % 8;

        bool bitValue = (data[byteIndex] >> (7 - bitIndex)) & 1;
        if (!bitValue) {
          //    Serial.printf("Leyendo valor x= %i, y= %i \n", col, row);
          int X = (col - ini) + x;
          int Y = row + y;

          int X0 = ANCHO - Y - 1;
          int Y0 = X;
          Paint_DrawPoint(X0, Y0, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
        }
      }
    }
  }
  void draw_string(char * s, int x, int y) {
    int index = 1;
    while ( * s != '\0') {
      draw_char( * s, x + (index * ancho), y);
      s++;
      index++;
    }
  }
};

class BigFont: public Font {
  public: BigFont() {
    data = fuente_grande;
    altura = 60;
    largo_total = 1520;
    ancho = 40;
  }
};

class SmallFont: public Font { // For some reason the code for the big font it didnt work with the small font even tho
  // I did exact same method to get the bitmap, I had to map it to 2 bytes per pixel
  public: SmallFont() {
    data = fuente_small;
    altura = 28;
    largo_total = 684;
    ancho = 18;
  }
  void draw_string(char * s, int x, int y) { // Rewrite the exact same function because for some reason it doesnt work with the one
    // of the parent class so whats even the point of OOP?
    int index = 1;
    while ( * s != '\0') {
      draw_char( * s, x + (index * ancho), y);
      s++;
      index++;
    }
  }
  void draw_char(char w, int x, int y) {
    int letra = charToValue(w);
    if (letra == -1) {
      Serial.println("Nothing to do");
      return;
    }
    int ini = ancho * letra;
    int fin = ancho * (letra + 1);
    for (int row = 0; row < altura; row++) {
      for (int col = ini; col < fin; col++) {
        int bitPosition = (row * largo_total) + col;
        /* I couldnt get this part to work with the small font
         int byteIndex = bitPosition / 8;
         int bitIndex = bitPosition % 8;
        bool bitValue = (data[byteIndex] >> (7 - bitIndex)) & 1;*/
        if (!data[bitPosition]) {
          int X = (col - ini) + x;
          int Y = row + y;

          int X0 = ANCHO - Y - 1;
          int Y0 = X;
          Paint_DrawPoint(X0, Y0, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
        }
      }
    }
  }
};

class Transform {
  public: int x0;
  int y0;
  int x1;
  int y1;
  void tt(int X0, int Y0, int X1, int Y1) {
    this -> x0 = ANCHO - Y0 - 1;
    this -> y0 = X0;
    this -> x1 = ANCHO - Y1 - 1;
    this -> y1 = X1;

  }
  void mostrar_valores() {
    Serial.printf("Pintando en x0=%i y0=%i; X=600, Y=448\n", this -> x0, this -> y0);
    Serial.printf("Pintando en x1=%i y1=%i; X=600, Y=448\n", this -> x1, this -> y1);
  }
};

auto bf = BigFont();
auto sf = SmallFont();
auto t = new Transform(); //Transform

class Data {
  public: char * nombre;
  float max_v;
  float min_v;
  float * lista;
  char * string_float_repr;
  boolean scaled;
  Data(char * nombre, float * lista) {
    //  lista=(double*)malloc(sizeof(double)*NUM_DIAS);
    this -> nombre = nombre;
    this -> lista = lista;
    scaled = false;
    string_float_repr=(char *) malloc(sizeof(char)*10);
  }
  void find_max_min() {
    float max = lista[0];
    float min = lista[0];
    for (int i = 1; i < DIAS; i++) {
      //     Serial.printf("valor %i=%f\n",i,lista[i]);
      if (lista[i] < min) {
        min = lista[i];
      }
      if (lista[i] > max) {
        max = lista[i];
      }
    }
    max_v = max;
    min_v = min;
    if (1.0 > max_v) {
      scaled = true;
      Serial.println("ACTUALIZANDO VALORES");
      for (int i = 0; i < DIAS; i++)
        lista[i] = lista[i] * SCALE_F;

      max_v = max_v * SCALE_F;
      min_v = min_v * SCALE_F;
    }
  }
  void plot() {
    bf.draw_string(nombre, 30, 0); //Titulo
    find_max_min();
    draw_graph();
    draw_labels();
    
  }
  void draw_labels(){
    int c;
    float d_min = min_v;
    float d_max = max_v;
    float tol_val= (d_max>1000.0)? 20.0:2.0;
    float t_min =round_to(d_min-tol_val);
    float t_max =round_to(d_max+tol_val);
    float step = (t_max-t_min)/(NUM_L_H-1);
    for (int i = 0; i<NUM_L_H-1;i++){
      float val = t_max-(step*((float)i+1));
      if (scaled){
        sprintf(string_float_repr,"%2f",val/SCALE_F);
      }else{
          sprintf(string_float_repr,"%2f",val);
      }
      sf.draw_string(string_float_repr,-11,((HEIGHT*(i+1))-sf.altura));
    }

  }
  void draw_graph() {
    int paso = (int)(580 - 75) / (DIAS + 7);
    auto obj = GraphCoordGen(this -> max_v, this -> min_v, 75, 520);
    int x0 = 46;
    int y0 = obj.form_pen(this -> lista[0]);
    int x1 = 46 + paso;
    int y1 = obj.form_pen(this -> lista[1]);
    int holder;
    for (int i = 2; i < DIAS; i++) {
      t -> tt(x0 + DESFACE, y0, x1 + DESFACE, y1);
      //t->mostrar_valores();
      Paint_DrawLine(t -> x0, t -> y0, t -> x1, t -> y1, EPD_5IN65F_RED, DOT_PIXEL_3X3, LINE_STYLE_SOLID);
      x0 = x1;
      y0 = y1;
      y1 = obj.form_pen(this -> lista[i]);
      x1 = 46 + (paso * i);
    }
  }
};


class GUI {
  public: void init() {
    prepare_display();
    Paint_DrawRectangle(0, 0, 600, 448, EPD_5IN65F_WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL); //Fix for weird bug in my display, comment if necesary
  }
  void draw_graph(Data * d) {
    draw_grid();
    d -> plot();
    show_buffer();
  }
};

auto gui = new GUI();

void no_setup() {
  //get_val_of_date("This is a char*",233);
  Serial.begin(115200);
  //float eth[]={1682.50, 1675.500000, 1664.199951, 1628.500000, 1598.500000, 1588.599976, 1581.199951, 1591.900024, 1593.099976, 1593.099976, 1601.699951, 1631.900024, 1642.300049, 1639.400024, 1628.000000, 1638.099976, 1627.400024, 1624.699951, 1598.500000, 1587.599976, 1585.400024, 1623.199951, 1634.199951, 1637.300049, 1634.900024, 1630.099976, 1628.199951, 1632.699951, 1635.199951, 1633.699951};
  //float xmr[] = {147.320007,146.330002,145.630005,146.500000,145.259995,144.289993,144.250000,143.070007,143.820007,145.740005,146.970001,147.350006,147.289993,146.479996,144.699997,145.740005,147.610001,144.850006,143.110001,141.229996,141.149994,142.820007,143.279999,143.080002,142.320007,139.380005,139.919998,142.029999,140.399994,140.520004  };
  //float doge[]={0.062352, 0.062069, 0.061790, 0.060970, 0.060636, 0.060714, 0.060868, 0.061460, 0.061598, 0.061542, 0.062031, 0.062409, 0.062514, 0.061968, 0.061981, 0.062320, 0.062033, 0.061655, 0.061105, 0.061243, 0.060733, 0.061960, 0.063527, 0.063335, 0.063335, 0.063757, 0.063732, 0.063178, 0.063383, 0.063549};
  //sf.draw_string("HOLA", 100,100);
  //auto data = Data("doge", doge);
  //gui -> init();
  /*connect();
  char* holder =(char *) malloc(sizeof(char)*2700);//
  
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
  Reloj r(21,0);
  r.show();
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
void setup(){
  Serial.begin(115200);
  Serial.println("\nListo");
  r.show();
  Serial.println("\nListo");
}

void loop() {
  r.update_value();
  delay(500);
  }

void show_buffer() {
  EPD_5IN65F_Display(BlackImage);
  DEV_Delay_ms(500);
}

void prepare_display() {
  DEV_Module_Init();
  Serial.print("e-Paper Init and Clear...\r\n");
  EPD_5IN65F_Init();
  EPD_5IN65F_Clear(EPD_5IN65F_WHITE);
  DEV_Delay_ms(100);
  UDOUBLE Imagesize = ((EPD_5IN65F_WIDTH % 2 == 0) ? (EPD_5IN65F_WIDTH / 2) : (EPD_5IN65F_WIDTH / 2 + 1)) * EPD_5IN65F_HEIGHT;
  Imagesize = Imagesize * 3 / 4 + 1000;
  Serial.printf("Imagesize %d\r\n", Imagesize);
  if ((BlackImage = (UBYTE * ) malloc(Imagesize)) == NULL) {
    Serial.print("Failed to apply for black memory...\r\n");
    while (1);
  }
  Paint_NewImage(BlackImage, EPD_5IN65F_WIDTH, EPD_5IN65F_HEIGHT, 0, EPD_5IN65F_WHITE);
  Paint_SetScale(7);
}

void draw_grid() {
  int c;
  for (int i = 0; i < NUM_L_W; i++) { // Vertical
    c = ((i + 1) * WIDTH) + DESFACE;
    t -> tt(c, 75, c, 525);
    Paint_DrawLine(t -> x0, t -> y0, t -> x1, t -> y1, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
  }
  for (int i = 0; i < (NUM_L_H - 1); i++) { // Horizontal
    c = (i + 1) * HEIGHT;
    t -> tt(35, c, 431, c);
    Paint_DrawLine(t -> x0, t -> y0, t -> x1, t -> y1, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
  }
}

int charToValue(char ch) {
  if (isalpha(ch)) {
    return ((tolower(ch) - 'a'));

  } else if (isdigit(ch)) {
    return (ch - '0' + 26);
  } else if (ch == '.') {
    return 36;
  } else if (ch == '$') {
    return 37;
  } else {
    return -1; // Default case for other characters
  }
}
int round_to(float number){
    return (int)(number / 5) * 5;
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
