#ifndef _COM_HPP_
#define _COM_HPP_


#include <esp_now.h>
#include <WiFi.h>

#endif



template<typename T1 , typename T2 , typename T3 , typename T4 , typename T5>
struct Data{
    T1 data1;
    T2 data2;
    T3 data3;
    T4 data4;
    T5 data5;
};

template<typename T1 , typename T2 , typename T3 , typename T4 , typename T5>
class Send{
    private:
        uint8_t Mac[6];
    public:
    using DataStc = Data<T1 , T2 , T3 , T4 , T5>;

        void GetMac(uint8_t *mac){
            WiFi.macAddress(mac);
            for(int i = 0; i < 6; i++){
                Serial.print(mac[i], HEX);
                Serial.print("  ");
            }
        }


        /// @brief Get the MAC address of your target device, and initialize ESP-NOW
        /// @param mac 
        void Init(uint8_t *mac){
            WiFi.mode(WIFI_STA);
            if (esp_now_init() != ESP_OK) {
                Serial.println("Error initializing ESP-NOW");
                return;
            }
            esp_now_peer_info_t peer;
            memcpy(peer.peer_addr, mac, 6);
            peer.channel = 0;
            peer.ifidx = WIFI_IF_STA;
            esp_now_add_peer(&peer);
            Serial.println("ESP NOW Initialized");
            memcpy(Mac, mac, 6);
        }


        void SendData(const DataStc &data){
            esp_now_send(Mac, (uint8_t *)&data, sizeof(DataStc))
        }
};

/// @brief When the data is received, it will be stored in the DataRx struct
/// @tparam T1 
/// @tparam T2 
/// @tparam T3 
/// @tparam T4 
/// @tparam T5 
template<typename T1 , typename T2 , typename T3 , typename T4 , typename T5>
class Receive{
    public:
    using DataStc = Data<T1 , T2 , T3 , T4 , T5>;
    static DataStc DataRx;

    void Init(uint8_t *TargetMac){
        WiFi.mode(WIFI_AP);
        if (esp_now_init() != ESP_OK) {
            Serial.println("Initialzing failed");
            return;
        }

        esp_now_register_recv_cb(OnDataReceive);
        esp_now_peer_info_t peer;
        memcpy(peer.peer_addr, TargetMac, 6);
        peer.channel = 0;
        peer.ifidx = WIFI_IF_AP;
        esp_now_add_peer(&peer);
        Serial.println("Initialized complete");
    }

    
    static void OnDataReceive(const uint8_t *mac, esp_now_recv_info_t *rxInfo, const uint8_t *data, int len) {
        if (len != sizeof(DataStc)) return;
        memcpy(&DataRx, data, sizeof(DataStc));
    }


};
