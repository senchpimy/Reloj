#include <HTTPClient.h>
#include <time.h>
#include "hora.h"
#include <ArduinoJson.h>

#pragma once

class Prices
{
public:
    StaticJsonDocument<2000> doc;
    bool update( HTTPClient* client, const char* url);

    void gen_precios(char* name, float prices[]);
    int set_time(Reloj** r);
    void print_values();
};
