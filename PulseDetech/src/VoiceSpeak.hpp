#pragma once
#include <Arduino.h>
#define OVER_HEARTBEAT 0
#define OVER_TEMPERATURE 1
#define LOW_OXIGEN 2
#define NORMAL 0



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

    template <typename T>
    String *LoadText(uint state , T value){
        String v = String(value);
        switch (state){
            case 0:
                String OVBPM[3] = {OverHeartBeat[random(0,1)] ,Temp , v};
                return OVBPM;
            case 1:
                String OVT[3] = {OverTemperature[random(0,1)] ,Heart , v};
                return OVT;
            case 2:
                String LOX[3] = {LowOxigen[random(0,1)] , Oxigen , v};
                return LOX;
            default:
                String HEALTH = Normal[random(0,2)];
                return HEALTH;
        }
    }

    void Speak(uint SerialChannel , int BPM , float Temp , float Oxigen);
};