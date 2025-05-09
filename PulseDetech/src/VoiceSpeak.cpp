#include "VoiceSpeak.hpp"

/// @brief Judge the normal or not and speak
/// @param SerialChannel 
/// @param BPM 
/// @param Temp 
/// @param Oxigen 
void Text::Speak(int BPM , float Temp , float Oxigen){
    int isNormalBPM;
    int isNormalTemp;
    int isNormalOxi;
    isNormalOxi = 0;
    isNormalBPM = 0;
    isNormalOxi = 0;
    if(BPM < 60){
        isNormalBPM = -1;
    }
    if(BPM > 160){
        isNormalBPM = 1;
        Text::BPMChanel(isNormalBPM , BPM);
    }
    if(Temp < 35.5f){
        isNormalTemp = -1;
    }
    if(Temp > 37.5f){
        isNormalTemp = 1;
        Text::TempChanel(isNormalTemp , Temp);
    }
    if(Oxigen < 80.0f){
        isNormalOxi = -1;
        Text::OxigenChanel(isNormalOxi , Oxigen);
    }

    if(isNormalBPM == 0 && isNormalTemp == 0 && isNormalOxi == 0){
        PSerial.println(Text::LoadText(3 , 0));
    }

}

/// @brief Inoput the value and state to speak thorugh serial
/// @param state 
/// @param value 
void Text::BPMChanel(int state , int value){
    String text;
    switch(state){
        case 0:
            text = Text::LoadText(3 , 0);
            break;
        case 1:
            text = Text::LoadText(0 , value);
            break;
        case -1:
            break;
        default:
            break;
        }
    PSerial.println(text);
}

void Text::TempChanel(int state , float value){
    String text;
    switch(state){
        case 0:
            text = Text::LoadText(3 , 0);
            break;
        case 1:
            text = Text::LoadText(1 , value);
            break;
        case -1:
            break;
        default:
            break;
        }
    PSerial.println(text);

}


void Text::OxigenChanel(int state , float value){
    String text;
    switch(state){
        case 0:
            text = Text::LoadText(3 , 0);
            break;
        case 1:
            text = Text::LoadText(2 , value);
            break;
        case -1:
            break;
        default:
            break;
        }
    PSerial.println(text);
}
