#include <Arduino.h>
#include "Bus.hpp"
#include "Com.hpp"
#include "VoiceSpeak.hpp"

#define LinerMotor 6
#define Button1 0
#define Button2 1
#define SPEAKER_TX 4
#define SPEAKER_RX 5

Data<float , float , float , uint8_t , uint8_t> DataWireless;
Send<float , float , float , uint8_t , uint8_t> H_send;
Receive<float , float , float , uint8_t , uint8_t> S_receive;

Text Speak;
BusOperation BusOS;

TaskHandle_t H_Com;
TaskHandle_t H_Vib;


SemaphoreHandle_t MUTEX;

uint8_t *macS;
uint8_t *macH;

void setup() { 
    Serial.begin(115200);
    Serial1.begin(38400);
    pinMode(Button1 , INPUT_PULLUP);
    pinMode(Button2 , INPUT_PULLUP);
    ledcAttach(LinerMotor , 10 , 100);
    BusOS.getChannel<float>(0).SetData(0.0);//BPM
    BusOS.getChannel<float>(1).SetData(0.0);//SPO2
    BusOS.getChannel<float>(2).SetData(0.0);//Temperature
    BusOS.getChannel<uint8_t>(3).SetData(0);//Command
    BusOS.getChannel<uint8_t>(4).SetData(0);//Button state
    BusOS.getChannel<uint8_t>(5).SetData(0);//TX or RX
    H_send.Init(macS);
    xTaskCreate(Com , "Com" , 8192 , NULL , 4 , &H_Com);
    xTaskCreate(Detect , "Detect" , 8192 , NULL , 5 , NULL);
    xTaskCreate(Vibrate , "Vibrate" , 8192 , NULL , 3 , &H_Vib);
}


void loop() { 

}

void Com(void *pvParameters) {
    while(1){
        if(BusOS.getChannel<uint8_t>(4).GetData() == 1){
            DataWireless.data4 = 1;
            H_send.SendData(DataWireless);
            delay(100);
            DataWireless.data4 = 2;
            H_send.SendData(DataWireless);
            BusOS.getChannel<uint8_t>(4).SetData(0);

        }else{
            BusOS.getChannel<float>(0).SetData(S_receive.DataRx.data1);
            BusOS.getChannel<float>(1).SetData(S_receive.DataRx.data2);
            BusOS.getChannel<float>(2).SetData(S_receive.DataRx.data3);
        }

    }

}

void Detect(void *pvParameters) {
    while(1){
        if(digitalRead(Button1) == 0){
            BusOS.getChannel<uint8_t>(4).SetData(1);
            vTaskPrioritySet(H_Com , 5);
            delay(120);
            vTaskPrioritySet(H_Com , 1);
            delay(500);
            BusOS.getChannel<uint8_t>(3).SetData(1);
        }else{
            delay(100);
        }
        if(digitalRead(Button2) == 0){
            BusOS.getChannel<uint8_t>(3).SetData(1);
        }
        

    }


}

void Vibrate(void *pvParameters) {
    int BPM;
    float SPO2;
    float Temperature;
    while (1){
        if(BusOS.getChannel<uint8_t>(3).GetData() == 1){
            ledcWrite(LinerMotor , 1023);
            delay(300);
            ledcWrite(LinerMotor , 0);
            BPM = BusOS.getChannel<float>(0).GetData()/1;
            SPO2 = BusOS.getChannel<float>(1).GetData();
            Temperature = BusOS.getChannel<float>(2).GetData();
            Speak.Speak(BPM , SPO2 , Temperature);
            delay(100);
            BusOS.getChannel<uint8_t>(3).SetData(0);
        }else{
            delay(100);
        }
        
        
    }
}