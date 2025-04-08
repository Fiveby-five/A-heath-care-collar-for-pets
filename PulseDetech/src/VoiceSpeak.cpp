#include "VoiceSpeak.hpp"

void Text::Speak(uint SerialChannel, int BPM , float Temp , float Oxigen){
    int isNormalBPM;
    int isNormalTemp;
    int isNormalOxi;
    if(BPM < 60){
        isNormalBPM = -1;
    }
    if(BPM > 160){
        isNormalBPM = 1;
    }
    if(Temp < 35.5f){
        isNormalTemp = -1;
    }
    if(Temp > 37.5f){
        isNormalTemp = 1;
    }
}