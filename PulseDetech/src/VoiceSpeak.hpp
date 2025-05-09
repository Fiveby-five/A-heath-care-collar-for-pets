#pragma once
#include <Arduino.h>
#define OVER_HEARTBEAT 0
#define OVER_TEMPERATURE 1
#define LOW_OXIGEN 2
#define NORMAL 0
#define PSerial Serial1



class Text{
    public:
    std::array<String, 3> Normal={
        "I am feeling good great. Thank you for caring me master",
        "I am in a perfect state and ready for next adventure",
        "Everything is fine and the day is shinny"
    };

    std::array<String , 2> OverHeartBeat={
        "My heart beat emergeed, master I need to take a rest",
        "I need take a rest, master"
    };
    
    std::array<String , 2> OverTemperature={
        "My temperature is too high, master I think I need treatment",
        "I am feeling hot, and not feeling well, master"
    };

    std::array<String , 2> LowOxigen={
        "Master, I am feeling dizzy",
        "I am breathing out , master"
    };

    String Temp = "My temperature is ";
    String Heart = "My heart beat is ";
    String Oxigen = "My oxigen ratio is ";


    /// @brief This function will load the text according to the state and value, 0 = OverBPM, 1 = Overtemperature, 2 = LowOxigen , default = Normal
    /// @tparam T 
    /// @param state 
    /// @param value 
    /// @return 
    template <typename T>
    String LoadText(uint state , T value){
        String v = String(value);
        switch (state){
            case 0:
                return OverHeartBeat[random(0,1)] + " " + Temp + v
            case 1:
                return OverTemperature[random(0,1)] + " " + Heart + v;
            case 2:
                return LowOxigen[random(0,1)] + " " + Oxigen + v;
            case 3:
                return Normal[random(0,2)];
            default:
                return Normal[random(0,2)];
        }
    }

    void Speak(int BPM , float Temp , float Oxigen);
    void BPMChanel(int state , int value);
    void TempChanel(int state , float value);
    void OxigenChanel(int state , float value);

};