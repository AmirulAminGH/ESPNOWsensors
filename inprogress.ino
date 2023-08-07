//-------------LIBRARY DEFINITION BEGIN--------------
#include "max6675.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ESP32Time.h>
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

const int SDCARD=1;
const int TEMP1=2;
const int TEMP2=3;
//-------PIN DEFINITION END-------------------
//-------GLOBAL VAR INIT BEGIN----------------
float temperature1=0.00;
float temperature2=0.00;

String namefile="";
//-------GLOBAL VAR INIT END----------------
//--------LIBRARY INIT BEGIN-----------------
MAX6675 thermocouple1(CLK1, TEMP1, MISO1);
MAX6675 thermocouple2(CLK1, TEMP2, MISO1);
File myFile;
ESP32Time rtc(0); 

//--------LIBRARY INIT END------------------
//---------SETUP BEGIN---------------------------------------------------------------------------------------
void setup(){
  Serial.begin(115200);
  SPI.begin(7,8,9,-1);
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
  //while(digitalRead(SW5)==HIGH){int i=1;}  //----------RTC init & config
  //rtc.setTime(0, 23, 14, 7, 8, 2023);       //---------ONLY REDO AFTER TOTAL POWER LOSS
  // RTC=rtc.getTime("%A, %B %d %Y %H:%M:%S");
  

     if(!SD.begin(1)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    String RTC= rtc.getTime("%A,%B%d%Y-%H%M%S");
    namefile="/SensorData/"+RTC+".csv";
    Serial.println(namefile);
    const char * filename=namefile.c_str();
   
    

    
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
     createDir(SD, "/SensorData");
     writeFile(SD,filename, "Date,Time,Sensor1,Sensor2\n");

  }
//---------SETUP END------------------------------------------------------------------------------------------
//---------LOOP BEGIN-----------------------------------------------------------------------------------------
void loop(){
   Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
   const char * filename=namefile.c_str();
   String dateTime= rtc.getTime("%B%d%Y,%H:%M:%S,");
   //String DAQdata= temperature1 + "," temperature2 + "\n"
   String dataStream=dateTime;
   const char * streamData= dataStream.c_str();
   char data1[10];
   char data2[10];
   char comma[]=",";
   char eol[]="\n";
   dtostrf(temperature1, 6, 2, data1);
   dtostrf(temperature2, 6, 2, data2);
   char datainbound[100];
   strcpy(datainbound,streamData);
   strcat(datainbound,data1);
   strcat(datainbound,comma);
   strcat(datainbound,data2);
   strcat(datainbound,eol);

   SPI.begin(7,8,9,-1);
   SD.begin(1);
   appendFile(SD,filename, datainbound);
   SD.end();
   SPI.end();

  MAX6675 thermocouple1(CLK1, TEMP1, MISO1);
  MAX6675 thermocouple2(CLK1, TEMP2, MISO1);
    
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
    temperature1=thermocouple1.readCelsius();
    temperature2=thermocouple2.readCelsius();
   Serial.print("C1 = "); 
   Serial.println(temperature1);
   Serial.print("C2 = "); 
   Serial.println(temperature2);
   delay(250);
  }
//---------LOOP END--------------------------------------------------------------------------------------------
//---------------FUNCTIONS DEFINITION BEGIN---------
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
  void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        rgb(RGBR,50);
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
        rgb(RGBG,50);
    } else {
        Serial.println("Append failed");
        rgb(RGBR,50);
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}
//---------------FUNCTIONS DEFINITION BEGIN---------
