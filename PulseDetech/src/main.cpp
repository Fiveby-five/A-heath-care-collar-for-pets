#include <Arduino.h>
#include "GatherData.hpp"
#include "Com.hpp"
#include "VoiceSpeak.hpp"
#include "Bus.hpp"
#define Button1 1
#define Button2 2

Alg BPM;
BusOperation BusOs;
Data<uint , float , float , char* , char*> DataWireless;
Send<uint , float , float , char* , char*> H_send;
Receive<uint , float , float , char* , char*> S_receive;

SemaphoreHandle_t MUTEX;


void setup() {
  Serial.begin(115200);
  BPM.init();
  pinMode(Button1, INPUT);
  pinMode(Button2, INPUT);
  MUTEX = xSemaphoreCreateMutex();    // Use Mutex to ensure there will be only one task to access the data
  xTaskCreate(VoiceSpeak, "VoiceSpeak", 2048, NULL, 1, NULL);
  xTaskCreate(Com, "Com", 2048, NULL, 1, NULL);
  xTaskCreate(BPMDetect, "BPMDetect", 20000, NULL, 3, NULL);
  xTaskCreate(Bus , "Bus", 8192, NULL, 2, NULL);

}

void loop() {

}


void BPMDetect(void *pvParameters) {
  for (;;) {
    bool flag = 0;
    BPM.loadToBuffer(&flag);
    if(flag == 1){
        double Rate = 60*BPM.FFTAnalsys();
        BusOs.getChannel<double>(0).SetData(Rate);
        BPM.clearBuffer();
    }
    vTaskDelay(5);
  }
}

void Com(void *pvParameters) {
  while(1){
    delay(1);
  }
}

void VoiceSpeak(void *pvParameters) {
  while(1){
    delay(1);
  }
}

void Bus(void *pvParameters) {
  while(1){
    if(BusOs.Timer(static_cast<uint32_t>(20000) , 0) == true){
      Serial.print("BPM:");
      Serial.println(BusOs.getChannel<double>(0).GetData());
      BusOs.Timer(static_cast<uint32_t>(10250) , 0 , 1);
    }
    vTaskDelay(1);
  }
}

