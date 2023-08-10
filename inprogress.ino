//-------------LIBRARY DEFINITION BEGIN--------------
#include "max6675.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ESP32Time.h>
#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0x34, 0x85, 0x18, 0x89, 0x2C, 0xC4};
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
int recordState=0;
unsigned long previousTask1Time = 0;
const unsigned long task1Interval = 250;
unsigned long previousTask2Time = 0;
const unsigned long task2Interval = 3000;

unsigned long buttonPressStartTime1 = 0;
bool taskExecuted1 = false;
unsigned long buttonPressStartTime2 = 0;
bool taskExecuted2 = false;
unsigned long buttonPressStartTime3 = 0;
bool taskExecuted3 = false;
unsigned long buttonPressStartTime4 = 0;
bool taskExecuted4 = false;
unsigned long buttonPressStartTime5 = 0;
bool taskExecuted5 = false;

int serialState=1;
int serialFlip=0;

int idleState=1;

typedef struct struct_message {int id; float x;float y;} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

int networkStat=0;//0-offline//1-onlinesuccess//2-online fail
int runmode=0;//0-online//1-offline

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
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {Serial.println("Error initializing ESP-NOW");return;}
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){Serial.println("Failed to add peer");return;}
  
  pinMode(RGBR,OUTPUT);
  pinMode(RGBG,OUTPUT);
  pinMode(RGBB,OUTPUT);
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);
  pinMode(SW3,INPUT_PULLUP);
  pinMode(SW4,INPUT_PULLUP);
  pinMode(SW5,INPUT);  
  rgb(RGBR,50);
  rgb(RGBG,50);
  rgb(RGBB,50);
  powerup();
  //while(digitalRead(SW5)==HIGH){int i=1;}  //----------RTC init & config
  //rtc.setTime(0, 41, 17, 8, 8, 2023);       //---------ONLY REDO AFTER TOTAL POWER LOSS
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
     //writeFile(SD,filename, "Date,Time,Sensor1,Sensor2\n");

  }
//---------SETUP END------------------------------------------------------------------------------------------
//---------LOOP BEGIN-----------------------------------------------------------------------------------------
void loop(){
    if (serialFlip==1){
      serialFlip=0;
      if(serialState==1){Serial.begin(115200);rgb(RGBB,500);rgb(RGBB,500);}
      if(serialState==0){Serial.end();rgb(RGBR,500);rgb(RGBR,50);}
      }
    
    unsigned long currentTime = millis();
    if (currentTime - previousTask1Time >= task1Interval) {
      previousTask1Time = currentTime;
       if (recordState==1){
        if(runmode==0){
        myData.id = 1;
        myData.x = temperature1;
        myData.y = temperature2;
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        if (result == ESP_OK) {Serial.println("Sent with success");}
        else {Serial.println("Error sending the data");}
        }
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
        temperature1=thermocouple1.readCelsius();
        temperature2=thermocouple2.readCelsius();
        Serial.print("C1 = "); 
        Serial.println(temperature1);
        Serial.print("C2 = "); 
        Serial.println(temperature2);
      }
  }
   unsigned long currentTime2 = millis();
  if (currentTime2 - previousTask2Time >= task2Interval) {
    previousTask2Time = currentTime2;
    if (idleState==1){
     digitalWrite(RGBG,HIGH);
     digitalWrite(RGBR,HIGH);
     digitalWrite(RGBB,HIGH);
     delay(50);
     digitalWrite(RGBG,LOW);
     digitalWrite(RGBR,LOW);
     digitalWrite(RGBB,LOW);
    }
  }

    
    if (digitalRead(SW1) == LOW) {
      if (!taskExecuted1) {
      if (millis() - buttonPressStartTime1 >= 1000) {
        executeTask1();
        taskExecuted1= true;
      }}} 
      else {buttonPressStartTime1 = millis();taskExecuted1 = false; 
      }
      
if (digitalRead(SW2) == LOW) {
      if (!taskExecuted4) {
      if (millis() - buttonPressStartTime4 >= 1000) {
        executeTask4();
        taskExecuted4= true;
      }}} 
      else {buttonPressStartTime4 = millis();taskExecuted4 = false; }
//----
      
      if (digitalRead(SW3)==LOW){
    rgb(RGBB,50);
    beep(4000,50);
    }
//-----   
    if (digitalRead(SW4) == LOW) {
      if (!taskExecuted3) {
      if (millis() - buttonPressStartTime3 >= 1000) {
        executeTask3();
        taskExecuted3= true;
      }}} 
      else {buttonPressStartTime3 = millis();taskExecuted3 = false; 
      }
    if (digitalRead(SW5) == HIGH) {
      if (!taskExecuted2) {
      if (millis() - buttonPressStartTime2 >= 1000) {
        executeTask2();
        taskExecuted2 = true;
      }}} 
      else {buttonPressStartTime2 = millis();taskExecuted2 = false; 
      }

  }
//---------LOOP END--------------------------------------------------------------------------------------------
//---------------FUNCTIONS DEFINITION BEGIN---------
void onlineTone(){
  int tempo=100;
  digitalWrite(RGBB,HIGH);
  beep(2306,tempo);beep(3456,tempo);beep(0,tempo);beep(2306,tempo);beep(3456,tempo);
  digitalWrite(RGBB,LOW);
  }
void offlineTone(){
  int tempo=100;
  digitalWrite(RGBR,HIGH);
  beep(3456,tempo);beep(2306,tempo);beep(0,tempo);beep(3456,tempo);beep(2306,tempo);
  digitalWrite(RGBR,LOW);
  }
void executeTask4(){
  runmode=(runmode+1)%2;
  if (runmode==0){onlineTone();}
  if (runmode==1){offlineTone();}
  }
void networkStatus(){
  if (networkStat==1){rgb(RGBB,50);}
  if (networkStat==2){rgb(RGBR,50);}
  }
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == ESP_NOW_SEND_SUCCESS) {
    networkStat=1;
} else {
    networkStat=2;
}
}
void executeTask1(){
   recordState = (recordState + 1) % 2;
     if (recordState==1){
    idleState=0;
    String RTC= rtc.getTime("%A,%B%d%Y-%H%M%S");
    namefile="/SensorData/"+RTC+".csv";
    const char * filename=namefile.c_str();
    writeFile(SD,filename, "Date,Time,Sensor1,Sensor2\n");
      }
      if (recordState==0){
         idleState=1;
        }
    rgb(RGBR,50);
    beep(4000,25);
  }
void executeTask2() {
    rgb(RGBB,50);
    powerdown();
    delay(3000);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_18,1);
     Serial.println("Going to sleep now");
      esp_deep_sleep_start();
}
void executeTask3(){
    serialFlip=1;
    serialState=(serialState+1)%2;
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
 void powerup(){
  int tempo=50;
 // beep(2637,50);beep(2637,50);beep(0,50);beep(2637,50);
 // beep(0,50);beep(2093,50);beep(2637,50);beep(0,50);
 // beep(3136,50);beep(0,50);beep(0,50);beep(0,50);
 // beep(1568,50);beep(0,50);beep(0,50);beep(0,50);
beep(2093,tempo);beep(2349,tempo);beep(2637,tempo);
beep(2794,tempo);beep(3136,tempo);beep(3520,tempo);beep(3951,tempo);beep(0,tempo);
}
 void powerdown(){
  int tempo=50;
//  beep(2637,50);beep(2637,50);beep(0,50);beep(2637,50);
//  beep(0,50);beep(2093,50);beep(2637,50);beep(0,50);
//  beep(3136,50);beep(0,50);beep(0,50);beep(0,50);
//  beep(1568,50);beep(0,50);beep(0,50);beep(0,50);
beep(3951,tempo);beep(3520,tempo);beep(3136,tempo);beep(2794,tempo);
beep(2637,tempo);beep(2349,tempo);beep(2093,tempo);
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
        if(runmode==0){networkStatus();}
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
        rgb(RGBG,50);
        if(runmode==0){networkStatus();}
    } else {
        Serial.println("Append failed");
        rgb(RGBR,50);
        if(runmode==0){networkStatus();}
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
