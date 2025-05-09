#pragma once
#include <Arduino.h>
#include <memory>
#include <array>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define BUS_SIZE 16
#define GETMILLISECOND millis()

/// @brief Base class for polymorphic memory management in Bus channels
class BusMemBase{
    public:
        virtual ~BusMemBase() = default;
};

template <typename T>
class BusMem : public BusMemBase
{
    private:
        std::unique_ptr<T> DataPTR;
        std::unique_ptr<uint8_t> FlagPTR;

    public:
        void SetData(const T& Data){
            if(!DataPTR){
                DataPTR = std::make_unique<T>();
            }
            *DataPTR = Data;
        }

        void SetValid(uint8_t Flag){
            if(!FlagPTR){
                FlagPTR = std::make_unique<uint8_t>();
            }
            *FlagPTR = Flag;
        }

        T GetData(){
            if(!DataPTR){
                Serial.println("BusMem: Data is not set");
                return T{};
            }
            return *DataPTR;
        }

        uint8_t GetValid(){
            if(!FlagPTR){
                Serial.println("BusMem: Flag is not set");
                return 0;
            }
            return *FlagPTR;
        }
};


/// @brief Central bus management class for channel and timer operations
/// @details Provides thread-safe access to shared resources using a mutex
class BusOperation
{
    private:
        std::array<std::unique_ptr<BusMemBase> , BUS_SIZE> channel;
        std::array<std::unique_ptr<uint> , BUS_SIZE> timer;
        SemaphoreHandle_t mutex;
        
    public:
    /// @brief Constructor initializes mutex and resource arrays
        BusOperation(){
            mutex = xSemaphoreCreateMutex();
            if(!mutex){
                Serial.println("BusOperation: Mutex is not created");
            }
        }

        /// @brief Destructor releases mutex and all resources
        ~BusOperation(){
            vSemaphoreDelete(mutex);
            for(auto& t : timer) if(t) t.reset();
            for(auto& c : channel) if(c) c.reset();
        }




        /// @brief In order to manage Busmem, select the channel and do setting or get data.
        /// @brief ETC. Bus.getChannel<type>(index).set(data)
        /// @tparam T 
        /// @param select channel
        /// @return 
        template <typename T>
        BusMem<T>& getChannel(uint index){
            if(xSemaphoreTake(mutex , portMAX_DELAY) !=  pdTRUE){
                Serial.println("BusOperation: Failed to take mutex");
                static BusMem<T> defaultChannel;
                return defaultChannel;
            }
            if(index >= BUS_SIZE){
                Serial.println("BusOperation: Channel is out of range");
                static BusMem<T> defaultChannel;
                return defaultChannel;

            }
            if(!channel[index]){
                channel[index] = std::make_unique<BusMem<T>>();
            }
            BusMem<T>* Ptr = static_cast<BusMem<T>*>(channel[index].get());
            xSemaphoreGive(mutex);
            return *Ptr;
        }


        /// @brief 
        /// @tparam T 
        /// @param index Remmember to release the channel when you don't need it
        template <typename T>
        void ReleaseChannel(uint index){
            if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
                return;
            }
            if(index >= BUS_SIZE){
                Serial.println("BusOperation: Channel is out of range");
                return;
            }
            channel[index].reset();
            xSemaphoreGive(mutex);
        }


    /// @brief Timer functionality for periodic operations
    /// @tparam T Time value type (uint32_t recommended)
    /// @param dtms Delay time in milliseconds
    /// @param index Timer channel index
    /// @param reset Whether to reset timer after time out (default: false)
    /// @return True if timer has expired
    template <typename T>
    bool Timer(T dtms, uint index, bool reset = false) {

        if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
            Serial.println("BusOperation: Failed to take mutex in Timer()");
            return false;
        }

        if (index >= BUS_SIZE) {
            Serial.println("BusOperation: Timer index out of range");
            return false;
        }

        if (!timer[index]) {
            timer[index] = std::make_unique<uint>();
            if (!timer[index]) {
                xSemaphoreGive(mutex);
                Serial.println("BusOperation: Failed to allocate timer memory");
                return false;
            }
            *timer[index] = 0;
        }//Initialization

        if (*timer[index] == 0) {
            *timer[index] = GETMILLISECOND + static_cast<uint>(dtms);
        }

        bool expired = (GETMILLISECOND >= *timer[index]);
        if (expired && reset) {
            *timer[index] = 0; // Reset timer if requested
        }

        xSemaphoreGive(mutex);

        return expired;
    }
};



