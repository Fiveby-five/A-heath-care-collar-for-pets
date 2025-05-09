#pragma once

#include <arduinoFFT.h>
#include <Wire.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <numeric>
#include "Bus.hpp"

#define SDA 4
#define SCL 5
#define SampleAmount 512
#define samplingFrequency 27

MAX30105 mSensor;
double vReal[SampleAmount];
double vImag[SampleAmount];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SampleAmount, samplingFrequency);


class GatherData {
    protected:
        uint32_t Red;
        uint32_t IR;
        uint32_t Green;
        uint32_t BufferR[SampleAmount];
        uint32_t BufferIR[SampleAmount];

    public:
        uint16_t BPM;
        uint16_t SpO2;

        void init(){
            Wire.begin(SDA, SCL);
            mSensor.begin();
            mSensor.setup();
            mSensor.enableDIETEMPRDY();
            mSensor.setPulseAmplitudeIR(20);
            mSensor.setPulseAmplitudeRed(1); //Turn Red LED to low to indicate sensor is running
            mSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
            mSensor.setSampleRate(50);
            mSensor.enableDIETEMPRDY();
        }

        void loadToBuffer(bool * flag) {
            *flag = false;
            int count = 0;
            while (count < SampleAmount) {
                if (mSensor.check()) {
                    IR = mSensor.getIR();
                    BufferIR[count] = IR;
                    Serial.println(IR);
                    count++;
                }
            }
            *flag = true;
        }

        void clearBuffer(){
            for(int i = 0; i < SampleAmount; i++){
                BufferR[i] = 0;
                BufferIR[i] = 0;
            }
        }

        float getTemp(){
             return mSensor.readTemperature();
        }

        void cleanI2C(){
            if(Wire.available() > 1){
                Wire.flush();
            }
        }



};

class Alg : public GatherData {
    protected:

    public:
        /// @brief This function should be placed after the loadToBuffer() function. It will return the main frequency HZ.
        /// @return 
        double FFTAnalsys(){
            double out;
            for(int i = 0; i < SampleAmount; i++){
                vReal[i] = static_cast<double>(BufferIR[i]) / 100.0;
            }
            memset(vImag, 0, sizeof(vImag));
            FFT.windowing(FFTWindow::Hamming , FFTDirection::Forward);
            FFT.compute(FFTDirection::Forward);
            FFT.complexToMagnitude();
            Serial.println("Done");
            double *RawFreq = new double[85];
            double *RawMag = new double[85];
            for(int i = 15; i < 99; i++){
                RawFreq[i-15] = (double)i * (samplingFrequency) / SampleAmount;
                RawMag[i-15] = vReal[i];
            }
            double *maxMag = std::max_element(RawMag, RawMag + 85);
            int indexOfMax = maxMag - RawMag;
            double MedFreq[3] = {RawFreq[indexOfMax - 1], RawFreq[indexOfMax], RawFreq[indexOfMax + 1]};
            double MedMag[3] = {RawMag[indexOfMax - 1], RawMag[indexOfMax], RawMag[indexOfMax + 1]};
            delete[] RawMag;
            delete[] RawFreq;
            double MainFrequency = WeightAva(MedFreq, RawFreq, 1.0, indexOfMax, 3);
            
            return MainFrequency;
        }



        /// @brief Calculate the weighted average of the array.
        /// @tparam T 
        /// @param arr Array's X position
        /// @param TargetArr Array's Y position
        /// @param Dconst Control the decay of the distance. Set to 0 there will be no weight.
        /// @param maxIndex 
        /// @param size 
        /// @return 
        template <typename T>
        T WeightAva(T * arr, T * TargetArr , double Dconst , int maxIndex , int size){
            T value = 0;
            T totalWeight = 0;
            for(int i = 0; i < size; i++){
                T D = abs(arr[maxIndex] - arr[i]);
                T weight = 1.0 / (Dconst * D + 1.0);
                value += weight * TargetArr[i];
                totalWeight += weight;
            }
            return totalWeight == 0 ? 0 : value / totalWeight;
        }



        uint32_t AutoBrightness(uint mode){//0 = Red, 1 = IR, 2 = Green , 3 = ShutAll
            switch(mode){
                case 0:
                    uint illuminance;
                    mSensor.setPulseAmplitudeRed(1);
                    while(mSensor.getRed() < 20000){
                        mSensor.setPulseAmplitudeRed(illuminance++);
                    }
                    return illuminance;
                    break;
                case 1:
                    uint illuminance;
                    mSensor.setPulseAmplitudeIR(1);
                    while(mSensor.getIR() < 20000){
                        mSensor.setPulseAmplitudeIR(illuminance++);
                    }
                    return illuminance;
                    break;
                case 2:
                    uint illuminance;
                    mSensor.setPulseAmplitudeGreen(1);
                    while(mSensor.getGreen() < 20000){
                        mSensor.setPulseAmplitudeGreen(illuminance++);
                    }
                    return illuminance;
                    break;
                case 3:
                    mSensor.setPulseAmplitudeRed(0);
                    mSensor.setPulseAmplitudeIR(0);
                    mSensor.setPulseAmplitudeGreen(0);
                    return 0;
                    break;
                default:
                    return 0;
                    break;
            }
        }


        template <typename T>
        double SPO2Alg(uint16_t Samples){
            AutoBrightness(3);
            AutoBrightness(1);
            AutoBrightness(0);
            std::unique_ptr<uint32_t[]> SampleIR(new uint32_t[Samples]);
            std::unique_ptr<uint32_t[]> SampleRed(new uint32_t[Samples]);
            int e = 0 , i = 0;
            while(i < Samples){
                if(mSensor.check()){
                    SampleIR[i] = mSensor.getIR();
                    SampleRed[i] = mSensor.getRed();
                    i++;
                    e = 0;
                }else{
                    e++;
                }
                if(e > 5*Samples){
                    return 0;
                }
            }
            double R;
            double a2 = std::accumulate(SampleIR, SampleIR + Samples, T(0))/Samples;
            double a1 = std::accumulate(SampleRed, SampleRed + Samples, T(0))/Samples;
            double V1;
            double V2;
            for(int i = 0; i < Samples; i++){
                V1 = (SampleRed[i] - a1)*(SampleIR[i] - a2)* SampleRed[i] / Samples;
                V2 = pow((SampleIR[i] - a2), 2.0) * SampleIR[i] / Samples;
            }
            R = V1/V2;
            return 109.3-11.2*R;
        }


};


class Application : public Alg{
    protected:

    public:
    BusOperation BusOs;


    /// @brief 
    /// @param MemChannelIndex 
    /// @return 
    bool BPM(int MemChannelIndex){
        bool flag = false;
        loadToBuffer(&flag);\
        AutoBrightness(3);
        AutoBrightness(1);
        if(flag){
            double MainFrequency = FFTAnalsys();
            double bpm = 60.0*MainFrequency;
            BusOs.getChannel<double>(MemChannelIndex).SetData(bpm);
            BusOs.getChannel<double>(MemChannelIndex).SetValid(1);
            return true;
        }else{
            BusOs.getChannel<double>(MemChannelIndex).SetValid(0);
            return false;
        }
    }

    /// @brief mesuring temperature and save it into the channel you set. data type is double.
    /// @param MemChannelIndex 
    /// @return 
    bool Temperature(int MemChannelIndex){
        double temperature = mSensor.readTemperature();
        BusOs.getChannel<double>(MemChannelIndex).SetData(temperature);
        BusOs.getChannel<double>(MemChannelIndex).SetValid(1);
        return true;
    }

    /// @brief Save the SPO2 value into the channel you set, data type is double.
    /// @param MemChannelIndex 
    /// @return 
    bool SpO2(int MemChannelIndex){
        double SpO2 = SPO2Alg<double>(100);
        if(SpO2 == 0){
            BusOs.getChannel<double>(MemChannelIndex).SetValid(0);
            Serial.println("SpO2 Calculation Error");
            return false;
        }
        BusOs.getChannel<double>(MemChannelIndex).SetData(SpO2);
        BusOs.getChannel<double>(MemChannelIndex).SetValid(1);
        return true;
    }


};

