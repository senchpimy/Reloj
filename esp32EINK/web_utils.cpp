#include "stdio.h"
#include "web_utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

const int DATE_STRING_SIZE=11;
//const char date_server[] = "worldclockapi.com";
//const char date_path[] = "/api/json/utc/now";
//char *date_server = "http://worldclockapi.com/api/json/utc/now";
//const char* date_server = "http://worldtimeapi.org/api/timezone/America/Mexico_City";
char *date_server = "http://www.whattimeisit.com/";



float get_val_of_date(char * price, int price_count){
  char *important = nullptr; // Initialize the pointer to nullptr
  int number_newlines=6;
  Serial.println("A1");
  for (int i = price_count;i>0;i--){
    if (price[i]=='\n'){
      number_newlines--;
    }
    if (number_newlines==0){
      important=&price[i];
      break;
    }
  }
Serial.println("A2");
  for (int i = 0;i<200;i++){
   // Serial.println(i);
    if (important[i]=='a' && important[i+1]=='v'&& important[i+2]=='g'){
      important=&important[i+10];
      for (int j =0; j<20;j++){
        if (important[j]==' '){
          important[j]='\0';
          break;
        }
      }
      break;
    }
  }
Serial.println("A3");
  return atof(important);
}

Fecha::Fecha(HTTPClient* http){
    http->begin(date_server);
        int httpResponseCode = http->GET();
          if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String content = http->getString();
                
        int start = content.indexOf("(UTC)");
        int mid = content.indexOf("</td>",start);
        int d = content.indexOf('>',mid);
        char * c_str=(char *) content.c_str();
        c_str=&c_str[d+35]; //Magic numbers cause I dont want to parse html
        c_str[8]='\0'; //Magic numbers cause I dont want to parse html
        Serial.printf("String date:%s:\n", c_str);
        original_date=c_str;

      }
      else {
        Serial.print("Impossible to get date; Error code: ");
        Serial.println(httpResponseCode);
        original_date="10/18/23";
      }
      http->end();
}

void Fecha::fill_data_v2(){ //"currentDateTime" from the api
  int ind = 0;
  char* month=original_date;
  while(true){if (original_date[ind]=='/'){ break; }ind++;}
  month[ind]='\0';
  char* day = &original_date[ind+1];
  ind = 0;
  while(true){if (day[ind]=='/'){break;}ind++;}
  day[ind]='\0';
  char* year= &day[ind+1];
  this->day   = atoi(day);
  this->month = atoi(month);
  this->year  = atoi(year)+2000;
  this->dates_ptr=0;
  this->dates = (char *) malloc(sizeof(char)*(DATE_STRING_SIZE*33));
  Serial.printf("AÑO: %i; MES: %i; DIA: %i\n", this->year, this->month, this->day);
 }

/*void deprecated_get(HTTPClient* http){ //Drepecated
    StaticJsonDocument<4000> json;
    http->begin(date_server);
         
    int httpResponseCode = http->GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        char * payload = (char *)http->getString().c_str();
        DeserializationError error=deserializeJson(json,payload);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          original_date="2023-10-18T12:37Z";
          return;
        }
        const char * original_d=json["currentDateTime"]; // Subject to hcange
        original_date = (char *) malloc(sizeof(char)*35); // subject to change
        strcpy(original_date, original_d);
        Serial.printf("%s",original_date);
       
        //serializeJsonPretty(json, Serial);
      }
      else {
        Serial.print("Impossible to get date; Error code: ");
        Serial.println(httpResponseCode);
        original_date="2023-10-18T12:37Z";
      }
//      delete json;
      //free(original_date); // Unesesary free?
      // Free resources
      http->end();
  }*/

 void Fecha::fill_data(){ //"currentDateTime" from the api
  int ind = 0;
  while(true){if (original_date[ind]=='T'){break;}ind++;}
  original_date[ind]='\0';
  char* year=original_date;
  ind = 0;
  while(true){if (original_date[ind]=='-'){ break; }ind++;}
  year[ind]='\0';
  char* month = &original_date[ind+1];
  ind = 0;
  while(true){if (month[ind]=='-'){break;}ind++;}
  month[ind]='\0';
  char* day = &month[ind+1];

  this->day   = atoi(day);
  this->month = atoi(month);
  this->year  = atoi(year);
  this->dates_ptr=0;
  this->dates = (char *) malloc(sizeof(char)*(DATE_STRING_SIZE*33));
 }

 void Fecha::gen_dates(){
   char* buffer = new char[DATE_STRING_SIZE];
  for (int i=1; i<33; i++){ // DIAS = 33
      sprintf(buffer,"%i-%02i-%02i",this->year,this->month,this->day);
      for (int j=0;j<DATE_STRING_SIZE;j++){
          this->dates[dates_ptr+j]=buffer[j];
      }
      dates_ptr+=DATE_STRING_SIZE;
      this->reduce_one();
  }
 }

 void Fecha::reduce_one() {
    if (this->day > 1) {
        this->day -= 1;
    } else {
        if (this->month > 1) {
            this->month -= 1;
            switch (this->month) {
                case 4: case 6: case 9: case 11:
                    this->day = 30;
                    break;
                case 2:
                    if ((this->year % 4 == 0 && this->year % 100 != 0) || (this->year % 400 == 0)) {
                        this->day = 29;
                    } else {
                        this->day = 28;
                    }
                    break;
                default:
                    this->day = 31;
                    break;
            }
        } else {
            this->year -= 1;
            this->month = 12;
            this->day = 31;
        }
    }
}

void Fecha::print_dates(){
  for (int i =0; i<32;i++){
    Serial.printf("%s:%i\n",&dates[(i)*DATE_STRING_SIZE],i);    
  }
}

#include <WiFiClientSecure.h>

Prices::Prices(){
  this->precios=(float *) malloc(sizeof(float)*30);
}
void Prices::gen_precios_v2(char* name, char* api_key,  HTTPClient* http){
  //char web []= "https://min-api.cryptocompare.com/data/v2/histoday?fsym=BTC&tsym=USD&limit=30";
 /* char web []= "https://min-api.cryptocompare.com/data/v2/histoday?fsym=";//BTC&tsym=USD&limit=30";
  char whole[150];

    sprintf(whole, "GET %s%s&tsym=USD&limit=3 HTTP/1.1\r\n",web,name);
    Serial.printf("STRING:%s\n",whole);
     WiFiClientSecure client;
     
  if (!client.connect(whole, 443)) {
    Serial.println("Connection HTTPS failed");
    return;
  }
    client.print(String(whole) +"Connection: close\r\n\r\n");
    Serial.println("Request sent");
    while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }*/
   WiFiClientSecure client;
    const char* host = "min-api.cryptocompare.com";
  const int httpsPort = 443;

  // Compose the request
  String request = String("GET /data/v2/histoday?fsym=") + name + "&tsym=USD&limit=3 HTTP/1.1\r\n"+
"Host: " + host + "\r\n"+
"Connection: close\r\n\r\n";

  // Connect to the server
  client.setInsecure();
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection to HTTPS server failed");
    return;
  }

  // Send the request
  client.print(request);
  Serial.println("Request sent");

  // Read and print the response
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }

  // Close the connection
  client.stop();
     return;


   /*  
     client.println("GET ");
    Serial.printf("STRING:%s\n",whole);
    http->begin("Google.com");
    
    Serial.println("YOU ARE HERE");
    int httpResponseCode = http->GET();
    Serial.println("Point 1");
    if (httpResponseCode>0){
      Serial.println("SUCCES CODE");
      }else{
      }//Handle errors
    Serial.println("Point 2");
    String json = http->getString();
        StaticJsonDocument<12000> doc;
     DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  for (int i = 0; i<30;i++){
    JsonObject index = doc["Data"]["Data"][i];
    float val = index["open"].as<float>();
    this->precios[i]=val;
    Serial.printf("SAVED VAL #%i\n",i);
  }
   http->end();
  delete whole;*/
}

void Prices::gen_precios(char* name, char* dates,  HTTPClient* http){

  char web []= "http://rate.sx/";
  char *whole=new char[40];
  for (int i = 0; i<31;i++){
    sprintf(whole, "%s%s@%s..%s",web,name,&dates[(i+1)*DATE_STRING_SIZE],&dates[i*DATE_STRING_SIZE]);
    Serial.printf("STRING:%s\n",whole);
    http->begin(whole);
    int httpResponseCode = http->GET();
    if (httpResponseCode>0){
      
    }else{
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      //return;
    }
    while (http->getSize()<1000){
      Serial.println("Error getting data");
      Serial.println(http->getString());
      http->end();
      http->begin(whole);
      http->GET();
      delay(3000); 
    }
    Serial.printf("Parsing Input #%i\n",i);
    Serial.printf("SIZE: %i\n",http->getSize());
    this->precios[i]=get_val_of_date((char *)http->getString().c_str(), http->getSize());
    Serial.printf("RECOLLECION TERMINADA; ELEMENTO %i; CRYPTO:%s\n", i, name);
    http->end();
    delay(3000); //Avoid getting marked as ddos
  }
  delete whole;
}

void Prices::print_values(){
  Serial.printf("CRYPTO:");
  for (int i = 0; i < 30; i++) {
    Serial.printf("%f, ",this->precios[i]);
  }
  Serial.printf("\n\n");
}
