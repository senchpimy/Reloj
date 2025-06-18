
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD2.h>
#include <GxEPD2_7C.h>
#include <epd7c/GxEPD2_565c.h>
#include <pgmspace.h>

#include "font.h" 
#include "gui_utils.h"

GxEPD2_7C<GxEPD2_565c, 32> display(GxEPD2_565c(15, 27,26,25));

const int ALTO = 448;
const int DIAS = 30;
const int ANCHO = 600;
const int NUM_L_W = 11;
const int NUM_L_H = 8;
const int DESFACE = 25;
const int WIDTH = ALTO / (NUM_L_W + 1);
const int SCALE_F = 10000;
const int HEIGHT = ANCHO / (NUM_L_H);
const int DESFACE_GRAFICA_X = 10; // ** NUEVA CONSTANTE PARA DESPLAZAR TODO **

int charToValue(char w);
void draw_grid();
int round_to(float number);


GraphCoordGen::GraphCoordGen(float val_max, float val_min, float coord_max, float coord_min) {
    float dif_uni = val_max - val_min;
    float dif_coord = coord_max - coord_min;
    this->coord_min = coord_min;
    this->paso = dif_coord / dif_uni;
    this->val_min = val_min;
}

int GraphCoordGen::form_pen(float y) {
    float dis_y = y - this->val_min;
    float pasos_tot = dis_y * this->paso;
    return (int)(this->coord_min + pasos_tot);
}

void Font::draw_char(char w, int x, int y) {
    int letra = charToValue(w);
    if (letra == -1) return;
    
    int ini = ancho * letra;
    int fin = ancho * (letra + 1);
    for (int row = 0; row < altura; row++) {
        for (int col = ini; col < fin; col++) {
            int bitPosition = (row * largo_total) + col;
            int byteIndex = bitPosition / 8;
            int bitIndex = bitPosition % 8;

            unsigned char byte_from_flash = pgm_read_byte(&data[byteIndex]);
            bool bitValue = (byte_from_flash >> (7 - bitIndex)) & 1;

            if (!bitValue) {
                display.drawPixel((col - ini) + x, row + y, GxEPD_BLACK);
            }
        }
    }
}

void Font::draw_string(const char* s, int x, int y) {
    int index = 1;
    while (*s != '\0') {
        draw_char(*s, x + (index * ancho), y);
        s++;
        index++;
    }
}

BigFont::BigFont() {
    data = fuente_grande;
    altura = 60;
    largo_total = 1520;
    ancho = 40;
}

auto bf = BigFont();

void draw_grid() {
    int c;
    const int GROSOR_LINEA = 2;

    for (int i = 0; i < NUM_L_W; i++) { // Vertical
        c = ((i + 1) * WIDTH) + DESFACE;
        display.fillRect(c + DESFACE_GRAFICA_X, 75, GROSOR_LINEA, 525 - 75, GxEPD_BLACK);
    }
    for (int i = 0; i < (NUM_L_H - 1); i++) { // Horizontal
        c = (i + 1) * HEIGHT;
        display.fillRect(35 + DESFACE_GRAFICA_X, c, 431 - 35, GROSOR_LINEA, GxEPD_BLACK);
    }
}

int charToValue(char ch) {
    if (isalpha(ch)) { return ((tolower(ch) - 'a')); }
    else if (isdigit(ch)) { return (ch - '0' + 26); }
    else if (ch == '.') { return 36; }
    else if (ch == '$') { return 37; }
    else { return -1; }
}
int round_to(float number){ return (int)(number / 5) * 5; }

Data::Data(const char * nombre, float * lista) {
    this->nombre = nombre; this->lista = lista;
    if (strcmp(nombre,"doge")==0){ for (int i = 0; i<DIAS; i++){ this->lista[i]=this->lista[i]*1000; } }
    scaled = false; string_float_repr=(char *) malloc(sizeof(char)*10);
}

void Data::find_max_min() {
    float max = lista[0]; float min = lista[0];
    for (int i = 1; i < DIAS; i++) { if (lista[i] < min) min = lista[i]; if (lista[i] > max) max = lista[i]; }
    max_v = max; min_v = min;
    if (1.0 > max_v) { scaled = true; for (int i = 0; i < DIAS; i++) lista[i] = lista[i] * SCALE_F; max_v = max_v * SCALE_F; min_v = min_v * SCALE_F; }
}

void Data::plot() {
    bf.draw_string(nombre, 30 + DESFACE_GRAFICA_X, 0);

    find_max_min();
    draw_graph();
    draw_labels();
}
      

void Data::draw_labels() {
    float d_min = min_v, d_max = max_v, tol_val = (d_max > 1000.0) ? 20.0 : 2.0;
    float t_min = round_to(d_min - tol_val), t_max = round_to(d_max + tol_val);
    float step = (t_max - t_min) / (NUM_L_H - 1);
    for (int i = 0; i < NUM_L_H - 1; i++) {
        float val = t_max - (step * ((float)i + 1));
        
        if (scaled) { sprintf(string_float_repr, "%.3f", val / SCALE_F); }
        else { sprintf(string_float_repr, "%.3f", val); }
        
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(0 + DESFACE_GRAFICA_X, (HEIGHT * (i + 1)));
        display.println(string_float_repr);
    }
}

void Data::draw_graph() {
    const int GROSOR_GRAFICA = 3;
    int paso = (int)(580 - 75) / (DIAS + 7);
    auto obj = GraphCoordGen(this->max_v, this->min_v, 75, 520);
    int x0 = 46, y0 = obj.form_pen(this->lista[0]);
    int x1 = 46 + paso, y1 = obj.form_pen(this->lista[1]);
    for (int i = 2; i < DIAS; i++) {
        for (int j = 0; j < GROSOR_GRAFICA; j++) {
            display.drawLine(x0 + DESFACE + DESFACE_GRAFICA_X + j, y0, x1 + DESFACE + DESFACE_GRAFICA_X + j, y1, GxEPD_RED);
        }
        x0 = x1; y0 = y1;
        y1 = obj.form_pen(this->lista[i]);
        x1 = 46 + (paso * i);
    }
}

enum DrawMode { NONE, PROGRESS, GRAPH };
DrawMode currentDrawMode = NONE;
Data* currentData = nullptr;
int currentProgressNumber = 0;
int totalProgressCoins = 0;

void drawCallback(const void* params) {
    if (currentDrawMode == PROGRESS) {
        bf.draw_string("RETRIEVING", -20, 200);
        bf.draw_string("DATA", 90, 260);
        
        int x = (360 / totalProgressCoins) * currentProgressNumber;
        display.fillRect(40, 325, x, 50, GxEPD_RED);
        display.drawRect(40, 325, 360, 50, GxEPD_BLUE);

    } else if (currentDrawMode == GRAPH && currentData != nullptr) {
        draw_grid();
        currentData->plot();
    }
}

void GUI::init(int coins) {
    this->numberCoins = coins;
    SPI.begin(13, -1, 14, 15);
    display.init(115200);
    display.setRotation(3);
    Serial.println("GUI init con GxEPD2 finalizado.");
}

void GUI::progress(int number) {
    currentDrawMode = PROGRESS;
    totalProgressCoins = this->numberCoins;
    currentProgressNumber = number;
    
    display.fillScreen(GxEPD_WHITE);
    display.drawPaged(&drawCallback, 0); 
}

void GUI::draw_graph(Data* d) {
    currentDrawMode = GRAPH;
    currentData = d;
    
    display.fillScreen(GxEPD_WHITE);
    display.drawPaged(&drawCallback, 0);
}
