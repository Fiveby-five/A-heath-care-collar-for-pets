#include <Arduino.h>
#include "GatherData.hpp"
#include "Com.hpp"
#include "VoiceSpeak.hpp"
#include "Bus.hpp"
#define LEDBlue 13
#define LEDRed 12
#define LEDGreen 11


Alg BPM;
BusOperation BusOs;
Application App;

Data<float , float , float , uint8_t , uint8_t> DataWireless;
Send<float , float , float , uint8_t , uint8_t> H_send;
Receive<float , float , float , uint8_t , uint8_t> S_receive;
/*The structure of the data<BPM , SPO2 , Temperature , Command , Command2>
Command protocol: Command = 0x00 -------Standing by
Command = 0x01 -------Refresh the data
Command = 0x02 -------Send data
*/

TaskHandle_t ComHandle;
TaskHandle_t BPMDetectHandle;
TaskHandle_t SPO2Handle;

SemaphoreHandle_t MUTEX;

bool comState;
uint8_t *macS;
uint8_t *macH;


void setup() {
  Serial.begin(115200);
  BPM.init();
  MUTEX = xSemaphoreCreateMutex();    // Use Mutex to ensure there will be only one task to access the data
  H_send.Init(macH);
  BusOs.getChannel<uint8_t>(1).SetData(0); // Channel 1 for switch RX or TX
  BusOs.getChannel<float>(0).SetData(0); // Channel 0 for pass the BPM
  xTaskCreate(Com, "Com", 2048, NULL, 4, &ComHandle);
  xTaskCreate(BPMDetect, "BPMDetect", 20000, NULL, 3, &BPMDetectHandle);
  xTaskCreate(Bus , "Bus", 8192, NULL, 5, NULL);
  xTaskCreate(SPO2 , "SPO2", 8192, NULL, 3, &SPO2Handle);

}

void loop() {

}


void BPMDetect(void *pvParameters) {
  for (;;) {
    bool flag = 0;
    BPM.loadToBuffer(&flag);
    if(flag == 1){
       float Rate = 60*BPM.FFTAnalsys();
        BusOs.getChannel<float>(0).SetData(Rate);// BPM data
        BPM.clearBuffer();
    }
    vTaskDelay(5);
  }
}


void SPO2(void *pvParameters) {
  for (;;) {
    bool valid = App.SpO2(2);
    delay(5);
  }
}

void Com(void *pvParameters) {
  bool mode;
  while(1){
    delay(1);

  }
}


/// @brief Bus task to mannage the whole system. Base on the multi threads rimer. And driven by communication
/// @param pvParameters 
void Bus(void *pvParameters) {
  while(1){
    if(BusOs.Timer(static_cast<uint32_t>(20000) , 0) == true){
      Serial.print("BPM:");
      Serial.println(BusOs.getChannel<float>(0).GetData());
      BusOs.Timer(static_cast<uint32_t>(10250) , 0 , 1);
    }

    uint8_t command = S_receive.DataRx.data4;
    switch (command){
      case 0:
        vTaskPrioritySet(BPMDetectHandle , 1);
        vTaskPrioritySet(SPO2Handle , 1);
        break;
      case 1:
        vTaskPrioritySet(BPMDetectHandle , 4);
        vTaskPrioritySet(SPO2Handle , 4);
        BusOs.Timer(static_cast<uint32_t>(20000) , 1);
        break;
      case 2:
        DataWireless.data1 = BusOs.getChannel<float>(0).GetData();
        DataWireless.data2 = BusOs.getChannel<float>(2).GetData();
        H_send.SendData(DataWireless);
      default:
        break;
    }
    if (BusOs.Timer(static_cast<uint32_t>(20000) , 1) == true){
      vTaskPrioritySet(BPMDetectHandle , 1);
      vTaskPrioritySet(SPO2Handle , 1);
      BusOs.Timer(static_cast<uint32_t>(10000) , 1 , 1);
    }
    vTaskDelay(2);
  }
}



