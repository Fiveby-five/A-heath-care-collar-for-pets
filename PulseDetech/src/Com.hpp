#ifndef _COM_HPP_
#define _COM_HPP_


#include <esp_now.h>
#include <WiFi.h>

#endif

class Sender{
    public:
        void Init(){
            WiFi.mode(WIFI_STA);
            if (esp_now_init() != ESP_OK) {
                Serial.println("Error initializing ESP-NOW");
                return;
            }
        }

    private:
};
