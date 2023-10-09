#include "stdio.h"
#include "web_utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
const int DATE_STRING_SIZE=11;
//const char date_server[] = "worldclockapi.com";
//const char date_path[] = "/api/json/utc/now";
char *date_server = "http://worldclockapi.com/api/json/utc/now";


//const char* date_server = "http://worldtimeapi.org/api/timezone/America/Mexico_City";


float get_val_of_date(char * price, int price_count){
  Serial.printf("1\n");
  char *important = nullptr; // Initialize the pointer to nullptr
  int number_newlines=6;
  Serial.printf("2\n");
  for (int i = price_count;i>0;i--){
    Serial.println("AAA");
    if (price[i]=='\n'){
      Serial.println("boorado");
      number_newlines--;
    }
    if (number_newlines==0){
      important=&price[i];
      break;
    }
  }
  Serial.printf("3:%s:\n",important);

  for (int i = 0;i<200;i++){
   // Serial.println(i);
    if (important[i]=='a' && important[i+1]=='v'&& important[i+2]=='g'){
      important=&important[i+10];
      for (int j =0; j<20;j++){
       Serial.println("encontrado");
        if (important[j]==' '){
          important[j]='\0';
          break;
        }
      }
      break;
    }
  }
  Serial.printf("4\n");

  return atof(important);
}

Fecha::Fecha(HTTPClient* http){
    StaticJsonDocument<4000> json;
    http->setUserAgent("curl");
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
          return;
        }
        const char * original_d=json["currentDateTime"]; // Subject to hcange
        original_date = (char *) malloc(sizeof(char)*35); // subject to change
        strcpy(original_date, original_d);
        Serial.printf("%s",original_date);
        //serializeJsonPretty(json, Serial);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
//      delete json;
      free(original_date);
      // Free resources
      http->end();
  }

 //void Fecha::fill_data(char* str){ //"currentDateTime" from the api
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

Prices::Prices(){
  this->precios=(float *) malloc(sizeof(float)*30);
}

void Prices::gen_precios(char* name, char* dates, char* price, int& price_count, HTTPClient* http){

  char web []= "http://rate.sx/"; //TODO
  char *whole=new char[40];
  for (int i = 0; i<31;i++){
    price_count=0;
    sprintf(whole, "%s%s@%s..%s",web,name,&dates[(i+1)*DATE_STRING_SIZE],&dates[i*DATE_STRING_SIZE]);
    //data = popen(whole, "r");
    Serial.printf("STRING:%s\n",whole);
    http->begin(whole);
    int httpResponseCode = http->GET();
    if (httpResponseCode>0){
      
    }else{
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      //return;
    }

    //Serial.printf("%i\n",http->getSize());
    this->precios[i]=get_val_of_date((char *)http->getString().c_str(), http->getSize());
    Serial.printf("RECOLLECION TERMINADA: %s\n", http->getString());
    http->end();
  }
  
  
}

void Prices::print_values(){
  Serial.printf("CRYPTO:");
  for (int i = 0; i < 30; i++) {
    Serial.printf("%f, ",this->precios[i]);
  }
  Serial.printf("\n\n");
}
