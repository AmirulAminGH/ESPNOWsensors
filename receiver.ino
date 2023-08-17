//-------------LIBRARY DEFINITION BEGIN--------------
#include "max6675.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ESP32Time.h>
#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress1[] = {0x34, 0x85, 0x18, 0x89, 0x2F, 0x90};//34:85:18:89:2F:90
uint8_t broadcastAddress2[] = {0x34, 0x85, 0x18, 0x89, 0x2C, 0xC4};//34:85:18:89:2C:C4
uint8_t broadcastAddress3[] = {0x70, 0x04, 0x1D, 0xA8, 0xB9, 0x28};//70:04:1D:A8:B9:28
uint8_t broadcastAddress4[] = {0x70, 0x04, 0x1D, 0xA8, 0xB9, 0x30};//70:04:1D:A8:B9:30
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

const int BATEN=10;
const int BATTERY=4;

const int SDCARD=1;
const int TEMP1=2;
const int TEMP2=3;
//-------PIN DEFINITION END-------------------
// Structure example to receive data
// Must match the sender structure
String namefile="";
float board1X=0.00;
float board1Y=0.00;
float board2X=0.00;
float board2Y=0.00;
float board3X=0.00;
float board3Y=0.00;
float board4X=0.00;
float board4Y=0.00;

typedef struct struct_command {int id;int cmd;} struct_command;
struct_command myCommand;

typedef struct struct_message {int id;float x;float y;}struct_message;
struct_message myData;
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;
struct_message boardsStruct[4] = {board1, board2, board3,board4};

File myFile;
ESP32Time rtc(0); 

esp_now_peer_info_t peerInfo;

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
int oneTime=0;
int idleState=1;
int networkStat=0;//0-offline//1-onlinesuccess//2-online fail
int runmode=0;//0-online//1-offline

int remoteState=0;
void setup() {
  Serial.begin(115200);
  rtc.setTime(0, 0, 15, 17, 8, 2023);
  SPI.begin(7,8,9,-1);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {Serial.println("Error initializing ESP-NOW");return;}
  esp_now_register_recv_cb(OnDataRecv);
  //esp_now_register_send_cb(OnDataSent);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){Serial.println("Failed to add peer");return;}
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){Serial.println("Failed to add peer");return;}
  memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){Serial.println("Failed to add peer");return;}
  memcpy(peerInfo.peer_addr, broadcastAddress4, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){Serial.println("Failed to add peer");return;}


  pinMode(RGBR,OUTPUT);
  pinMode(RGBG,OUTPUT);
  pinMode(RGBB,OUTPUT);
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);
  pinMode(SW3,INPUT_PULLUP);
  pinMode(SW4,INPUT_PULLUP);
  pinMode(SW5,INPUT);
  pinMode(BATEN,OUTPUT);
  //digitalWrite(BATEN,HIGH); 
  rgb(RGBR,50);
  rgb(RGBG,50);
  rgb(RGBB,50);
  powerup();

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
 
void loop() {
  
      unsigned long currentTime = millis();
      if (currentTime - previousTask1Time >= task1Interval) {
      previousTask1Time = currentTime;
      if(recordState==1){

      Serial.print("Board1X:");Serial.println(board1X);
      Serial.print("Board1Y:");Serial.println(board1Y);
      Serial.print("Board2X:");Serial.println(board2X);
      Serial.print("Board2Y:");Serial.println(board2Y);
      Serial.print("Board3X:");Serial.println(board3X);
      Serial.print("Board3Y:");Serial.println(board3Y);
      Serial.print("Board4X:");Serial.println(board4X);
      Serial.print("Board4Y:");Serial.println(board4Y);

        Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
        const char * filename=namefile.c_str();
        String dateTime= rtc.getTime("%B%d%Y,%H:%M:%S,");
        String dataStream=dateTime;
        const char * streamData= dataStream.c_str();
        char data1[10];
        char data2[10];
        char data3[10];
        char data4[10];
        char data5[10];
        char data6[10];
        char data7[10];
        char data8[10];
        char comma[]=",";
        char eol[]="\n";
        dtostrf(board1X, 6, 2, data1);
        dtostrf(board1Y, 6, 2, data2);
        dtostrf(board2X, 6, 2, data3);
        dtostrf(board2Y, 6, 2, data4);
        dtostrf(board3X, 6, 2, data5);
        dtostrf(board3Y, 6, 2, data6);
        dtostrf(board4X, 6, 2, data7);
        dtostrf(board4Y, 6, 2, data8);
        char datainbound[100];
        strcpy(datainbound,streamData);
        strcat(datainbound,data1);
        strcat(datainbound,comma);
        strcat(datainbound,data2);
        strcat(datainbound,comma);
        strcat(datainbound,data3);
        strcat(datainbound,comma);
        strcat(datainbound,data4);
        strcat(datainbound,comma);
        strcat(datainbound,data5);
        strcat(datainbound,comma);
        strcat(datainbound,data6);
        strcat(datainbound,comma);
        strcat(datainbound,data7);
        strcat(datainbound,comma);
        strcat(datainbound,data8);
        strcat(datainbound,eol);

        SPI.begin(7,8,9,-1);
        SD.begin(1);
        if(oneTime==1){
        appendFile(SD,filename,"Date,Time,Sensor1A,Sensor1B,Sensor2A,Sensor2B,Sensor3A,Sensor3B,Sensor4A,Sensor4B\n");  
          }
        oneTime=0;
        appendFile(SD,filename, datainbound);
        SD.end();
        SPI.end();
    }}
  
      if (digitalRead(SW1) == LOW) {
      if (!taskExecuted1) {
      if (millis() - buttonPressStartTime1 >= 1000) {
        executeTask1();
        delay(250);
        recordState=(recordState+1)%2;
        oneTime=recordState;
        taskExecuted1= true;
      }}} 
      else {buttonPressStartTime1 = millis();taskExecuted1 = false; 
      }
      
      if (digitalRead(SW2) == LOW) {
      if (!taskExecuted2) {
      if (millis() - buttonPressStartTime2 >= 1000) {
        executeTask2();
        taskExecuted2= true;
      }}} 
      else {buttonPressStartTime2 = millis();taskExecuted2 = false; }

      if (digitalRead(SW3) == LOW) {
      if (!taskExecuted3) {
      if (millis() - buttonPressStartTime3 >= 1000) {
        remoteState=(remoteState+1)%2;
        executeTask3();
        taskExecuted3= true;
      }}} 
      else {buttonPressStartTime3 = millis();taskExecuted3 = false; }

      if (digitalRead(SW4) == LOW) {
      if (!taskExecuted4) {
      if (millis() - buttonPressStartTime4 >= 1000) {
        executeTask4();
        taskExecuted4= true;
      }}} 
      else {buttonPressStartTime4 = millis();taskExecuted4 = false; }

      if (digitalRead(SW5) == HIGH) {
      if (!taskExecuted5) {
      if (millis() - buttonPressStartTime5 >= 1000) {
        executeTask5();
        taskExecuted5= true;
      }}} 
      else {buttonPressStartTime5 = millis();taskExecuted5 = false; }


}
void executeTask5(){
      rgb(RGBB,50);
    powerdown();
    delay(3000);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_18,1);
     Serial.println("Going to sleep now");
      esp_deep_sleep_start();
  }
void executeTask4(){
    myCommand.id = 40; //change base on board
    myCommand.cmd = rtc.getEpoch();
    esp_now_send(broadcastAddress1, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress2, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress3, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress4, (uint8_t *) &myCommand, sizeof(myCommand));
    rgb(RGBG,50);
    beep(4000,25);
  }
void executeTask3(){
    if(remoteState==1){myCommand.id = 20;}
    if(remoteState==0){myCommand.id = 30;}
    myCommand.cmd = rtc.getEpoch();
    esp_now_send(broadcastAddress1, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress2, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress3, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress4, (uint8_t *) &myCommand, sizeof(myCommand));
    rgb(RGBG,50);
    beep(4000,25);
  }
void executeTask2(){
    myCommand.id = 10; //change base on board
    myCommand.cmd = rtc.getEpoch();
    esp_now_send(broadcastAddress1, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress2, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress3, (uint8_t *) &myCommand, sizeof(myCommand));
    esp_now_send(broadcastAddress4, (uint8_t *) &myCommand, sizeof(myCommand));
    rgb(RGBG,50);
    beep(4000,25);
  }
void executeTask1(){
     
     if (recordState==1){
    idleState=0;
    String RTC= rtc.getTime("%A,%B%d%Y-%H%M%S");
    namefile="/SensorData/"+RTC+".csv";
    const char * filename=namefile.c_str();
    writeFile(SD,filename,"");
    
      }
      if (recordState==0){
         idleState=1;
        }
    rgb(RGBR,50);
    beep(4000,25);
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
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  //Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
   mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  //Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  boardsStruct[myData.id-1].y = myData.y;
  //Serial.printf("x value: %g \n", boardsStruct[myData.id-1].x);
  //Serial.printf("y value: %g \n", boardsStruct[myData.id-1].y);
  board1X = boardsStruct[0].x;
  board1Y = boardsStruct[0].y;
  board2X = boardsStruct[1].x;
  board2Y = boardsStruct[1].y;
  board3X = boardsStruct[2].x;
  board3Y = boardsStruct[2].y;
  board4X = boardsStruct[3].x;
  board4Y = boardsStruct[3].y;
  
  
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
//---------------FUNCTIONS DEFINITION END---------
