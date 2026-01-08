#include <HTTPClient.h>
#include <time.h>
#include "hora.h"
#include <ArduinoJson.h>

#pragma once

#include <functional>

struct SpotifyStatus {
    bool is_playing;
    String title;
    String artist;
    String image_hash;
    int dominant_color;
};

class SpotifyHandler {
public:
    JsonDocument doc; 
    bool getStatus(HTTPClient * http, const char* serverUrl, SpotifyStatus &status);
    bool processImageStream(HTTPClient * http, const char* serverUrl, std::function<void(Stream*, size_t)> processor);
};

class Prices
{
public:
    StaticJsonDocument<2000> doc;
    bool update( HTTPClient* client, const char* url);

    void gen_precios(char* name, float prices[]);
    int set_time(Reloj** r);
    void print_values();
};
