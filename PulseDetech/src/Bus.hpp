#pragma once
#include <Arduino.h>
#include <memory>
#include <array>

#define BUS_SIZE 16
#define GETMILLISECOND millis()


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


/// @brief Bus Operation
class BusOperation
{
    private:
        std::array<std::unique_ptr<BusMemBase> , BUS_SIZE> channel;
        std::array<std::unique_ptr<uint> , BUS_SIZE> timer;
        
    public:
        /// @brief In order to manage Busmem, select the channel and do setting or get data.
        /// @brief ETC. Bus.getChannel<type>(index).set(data)
        /// @tparam T 
        /// @param select channel
        /// @return 
        template <typename T>
        BusMem<T>& getChannel(uint index){
            if(index >= BUS_SIZE){
                Serial.println("BusOperation: Channel is out of range");
                static BusMem<T> defaultChannel;
                return defaultChannel;

            }
            if(!channel[index]){
                channel[index] = std::make_unique<BusMem<T>>();
            }
            return *static_cast<BusMem<T>*>(channel[index].get());
        }


        /// @brief 
        /// @tparam T 
        /// @param index Remmember to release the channel when you don't need it
        template <typename T>
        void ReleaseChannel(uint index){
            if(index >= BUS_SIZE){
                Serial.println("BusOperation: Channel is out of range");
                return;
            }
            channel[index].reset();
        }


        /// @brief 
        /// @tparam T 
        /// @param dtms How long you want to delay
        /// @param index Timer channel
        /// @param reset Mannual reset. Default is false which is no reset
        /// @return 
        template <typename T>
        bool Timer(T dtms , uint index , bool reset = 0){
            if(index >= BUS_SIZE){
                Serial.println("BusOperation: Timer channel is out of range");
                return false;
            }
            if(!timer[index]){
                timer[index] = std::make_unique<uint>();
                *timer[index] = 0;
            }
            if(*timer[index] == 0){
                *timer[index] = GETMILLISECOND + static_cast<uint>(dtms);
            }
            if(GETMILLISECOND >= *timer[index]){
                *timer[index] = *timer[index]*(reset == 1 ? 0 : 1);
                return true;
            }else{
                return false;
            }
        }


};
