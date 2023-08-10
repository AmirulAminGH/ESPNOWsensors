/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp32/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <esp_now.h>
#include <WiFi.h>
const int RGBR=11;
const int RGBG=13;
const int RGBB=12;
const int buzzer=21;
const int SW1=14;
const int SW2=15;
const int SW3=16;
const int SW4=17;
const int SW5=18;
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  float x;
  float y;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2, board3};

// callback function that will be executed when data is received
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
  //Serial.println();
  float board1X = boardsStruct[0].x;
  float board1Y = boardsStruct[0].y;
  //Serial.print("tempA1-1: ");
  Serial.println(board1X);
  //Serial.print("tempA1-2: ");
  Serial.println(board1Y); 
  digitalWrite(RGBG,HIGH);
  delay(50);
  digitalWrite(RGBG,LOW);
  
}
 
void setup() {
  pinMode(RGBG,OUTPUT);
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  // Acess the variables for each board
  /*int board1X = boardsStruct[0].x;
  int board1Y = boardsStruct[0].y;
  int board2X = boardsStruct[1].x;
  int board2Y = boardsStruct[1].y;
  int board3X = boardsStruct[2].x;
  int board3Y = boardsStruct[2].y;*/
//  float board1X = boardsStruct[0].x;
//  float board1Y = boardsStruct[0].y;
//  Serial.print("temp1: ");
//  Serial.println(board1X);
//  Serial.print("temp2: ");
//  Serial.println(board1Y); 
}
