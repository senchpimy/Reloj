#include <HTTPClient.h>

float get_val_of_date(char* price, int price_count);
#pragma once
class Fecha
{
  public:
    int year;
    int month;
    int day;
    int dates_ptr;
    char* dates;
    char* original_date;

    Fecha(HTTPClient* http);

    void fill_data();

    void gen_dates();

    void reduce_one();
        
    void print_dates();
};

class Prices
{
public:
    float* precios;

    Prices();

    void gen_precios(char* name, char* dates, char* price, int& price_count,HTTPClient* http);

    void print_values();
};
