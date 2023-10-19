#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace std;
#include <nlohmann/json.hpp>
using json = nlohmann::json;
const int DATE_STRING_SIZE=11;

float get_val_of_date(char * price, int price_count){
  printf("1\n");
  char *important ;//= nullptr; // Initialize the pointer to nullptr
  int number_newlines=6;
  printf("2\n");
  for (int i = price_count;i>0;i--){
    if (price[i]=='\n'){
      number_newlines--;
    }
    if (number_newlines==0){
      important=&price[i];
      break;
    }
  }
  printf("3\n");

  for (int i = 0;i<200;i++){
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
  printf("4\n");

  return atof(important);
}

class fecha
{
  public:
    int year;
    int month;
    int day;
    int dates_ptr;
    char* dates;

 fecha(char* str){
  int ind = 0;
  while(true){if (str[ind]=='T'){break;}ind++;}
  str[ind]='\0';

  char* year=str;
  ind = 0;
  while(true){if (str[ind]=='-'){ break; }ind++;}
  year[ind]='\0';
  char* month = &str[ind+1];

  ind = 0;
  while(true){if (month[ind]=='-'){break;}ind++;}
  month[ind]='\0';
  char* day = &month[ind+1];

  this->day   = atoi(day);
  this->month = atoi(month);
  this->year  = atoi(year);
  this->dates_ptr=0;
  this->dates = (char *) malloc(sizeof(char)*3500);
 }

 void gen_dates(){
   char* buffer = new char[DATE_STRING_SIZE];
  for (int i=1; i<32; i++){
      sprintf(buffer,"%i-%02i-%02i",this->year,this->month,this->day);
      for (int j=0;j<DATE_STRING_SIZE;j++){
          this->dates[dates_ptr+j]=buffer[j];
      }
      dates_ptr+=DATE_STRING_SIZE;
      this->reduce_one();
  }

//  for (int k=0;k<dates_ptr;k++){
//    if (this->dates[k]=='\0'){
//      printf("\n");
//    }else{
//      printf("%c",this->dates[k]);
//    }
//  }
 }

 void reduce_one() {
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

};

class Prices
{
public:
  float* precios;
  Prices();
  void gen_precios(char* ,char* , FILE* , char*, int&);
  void print_values();
};

Prices::Prices()
{
  this->precios=(float *) malloc(sizeof(float)*30);
}

void Prices::gen_precios(char* name, char* dates, FILE* data, char* price, int& price_count){
  char* web = "/bin/curl http://rate.sx/";
  char* whole=new char[50];
  for (int i = 0; i<31;i++){
    price_count=0;
    sprintf(whole, "%s%s@%s..%s",web,name,&dates[(i+1)*DATE_STRING_SIZE],&dates[i*DATE_STRING_SIZE]);
    data = popen(whole, "r");
    while ((price[price_count] = fgetc(data)) != EOF) {
      price_count++;
    }
    this->precios[i]=get_val_of_date(price, price_count);
  }
}
void Prices::print_values(){
  printf("CRYPTO:");
  for (int i = 0; i < 30; i++) {
    printf("%f, ",this->precios[i]);
  }
  printf("\n\n");
}

int main (int argc, char *argv[])
{
  FILE *data;
//  FILE *time;
  int price_count = 0;
  int time_count = 0;
  char ch;
  char price[4000];
  //char time_d[300];
  string time_d = "{\"$id\":\"1\",\"currentDateTime\":\"2023-10-02T14:58Z\",\"utcOffset\":\"00:00:00\",\"isDayLightSavingsTime\":false,\"dayOfTheWeek\":\"Monday\",\"timeZoneName\":\"UTC\",\"currentFileTime\":133407323331869728,\"ordinalDate\":\"2023-275\",\"serviceResponse\":null}";
  json cont= json::parse(time_d);
 // if (time_d[0]!='{'){
 //   printf("Stringu %s",time_d);
 //   char time_d[] = "{\"$id\":\"1\",\"currentDateTime\":\"2023-10-02T14:58Z\",\"utcOffset\":\"00:00:00\",\"isDayLightSavingsTime\":false,\"dayOfTheWeek\":\"Monday\",\"timeZoneName\":\"UTC\",\"currentFileTime\":133407323331869728,\"ordinalDate\":\"2023-275\",\"serviceResponse\":null};";
 //   //return 0;
 // }
  //std::cout<< cont["currentDateTime"]<<"\n";
  fecha f((char *) cont.value("currentDateTime","2023-10-02T14:58Z").c_str());
 f.gen_dates();
  Prices p;
//  Prices p1;
//  Prices p2;
  cout<<price_count;
  p.gen_precios("eth", f.dates,data,price, price_count);
//  p1.gen_precios("xmr", f.dates,data,price, price_count);
//  p2.gen_precios("doge", f.dates,data,price, price_count);
//  pclose(data);
  //printf("LALALA: %s",important);
  ///printf("Size of string: %i \n",time_count);
  ///printf("%s",time_d);
  printf("Size of string: %i \n",price_count);
  float eth[]={1682.500000, 1675.500000, 1664.199951, 1628.500000, 1598.500000, 1588.599976, 1581.199951, 1591.900024, 1593.099976, 1593.099976, 1601.699951, 1631.900024, 1642.300049, 1639.400024, 1628.000000, 1638.099976, 1627.400024, 1624.699951, 1598.500000, 1587.599976, 1585.400024, 1623.199951, 1634.199951, 1637.300049, 1634.900024, 1630.099976, 1628.199951, 1632.699951, 1635.199951, 1633.699951};
  float xmr []={147.320007, 146.330002, 145.630005, 146.500000, 145.259995, 144.289993, 144.250000, 143.070007, 143.820007, 145.740005, 146.970001, 147.350006, 147.289993, 146.479996, 144.699997, 145.740005, 147.610001, 144.850006, 143.110001, 141.229996, 141.149994, 142.820007, 143.279999, 143.080002, 142.320007, 139.380005, 139.919998, 142.029999, 140.399994, 140.520004};
  float doge[]={0.062352, 0.062069, 0.061790, 0.060970, 0.060636, 0.060714, 0.060868, 0.061460, 0.061598, 0.061542, 0.062031, 0.062409, 0.062514, 0.061968, 0.061981, 0.062320, 0.062033, 0.061655, 0.061105, 0.061243, 0.060733, 0.061960, 0.063527, 0.063335, 0.063335, 0.063757, 0.063732, 0.063178, 0.063383, 0.063549};
  //printf("%s",price);
  return 0;
}
