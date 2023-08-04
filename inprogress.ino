//-------------LIBRARY DEFINITION BEGIN--------------
#include "max6675.h"
#include <SPI.h>
#include <SD.h>
//-------------LIBRARY DEFINITION END------------------

//-------PIN DEFINITION BEGIN-------------------

const int CLK1 = 7;
const int MISO1 = 8;
const int MOSI1 = 9;

const int RGBR=11;
const int RGBG=13;
const int RGBB=12;
const int buzzer=21;
const int SW1=14;
const int SW2=15;
const int SW3=16;
const int SW4=17;
const int SW5=18;

const int sdcard=1;

const int temp1=2;
const int temp2=3;
//-------PIN DEFINITION END-------------------

//--------LIBRARY INIT BEGIN-----------------
MAX6675 thermocouple1(CLK1, temp1, MISO1);
MAX6675 thermocouple2(CLK1, temp2, MISO1);
File myFile;
//--------LIBRARY INIT END------------------
void setup(){
  Serial.begin(9600);
  pinMode(RGBR,OUTPUT);
  pinMode(RGBG,OUTPUT);
  pinMode(RGBB,OUTPUT);
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);
  pinMode(SW3,INPUT_PULLUP);
  pinMode(SW4,INPUT_PULLUP);
  pinMode(SW5,INPUT_PULLUP);
  
  rgb(RGBR,500);
  rgb(RGBG,500);
  rgb(RGBB,500);

  }
void loop(){
//  rgb(RGBR,1000);
//  rgb(RGBG,1000);
//  rgb(RGBB,1000);
  //beep(3000,25);
  //rgb(RGBG,50);
  //beep(3000,25);
  if (digitalRead(SW1)==LOW){
    rgb(RGBR,50);
    beep(3000,25);
    }
     if (digitalRead(SW2)==LOW){
    rgb(RGBG,50);
    beep(3000,25);
    }
      if (digitalRead(SW3)==LOW){
    rgb(RGBB,50);
    beep(3000,25);
    }
      if (digitalRead(SW4)==LOW){
    rgb(RGBR,50);
    beep(3000,25);
    }
      if (digitalRead(SW5)==LOW){
    rgb(RGBB,50);
    beep(3000,25);
    }
   Serial.print("C1 = "); 
   Serial.println(thermocouple1.readCelsius());
   Serial.print("C2 = "); 
   Serial.println(thermocouple2.readCelsius());
  delay(1000);

 
  }

void rgb(int pin, int ms){
  digitalWrite(pin,HIGH);
  delay(ms);
  digitalWrite(pin,LOW);
  delay(ms);
  }
 void beep(int freq ,int ms){
   tone(buzzer, freq); // Send 1KHz sound signal...
  delay(ms);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(ms);
  }
