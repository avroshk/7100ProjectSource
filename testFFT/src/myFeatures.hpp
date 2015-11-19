//
//  myFeatures.hpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/3/15.
//
//

#ifndef myFeatures_hpp
#define myFeatures_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxFft.h"

class myFeatures {
public:
    myFeatures(int,int); //Constructor
    myFeatures(); //Constructor
    ~myFeatures(); //Destructor -- not implemented yet
    
//    enum class Feature {
//        SpectralFlux,
//        SPECTRALROLLOFF,
//        SPECTRALCENTROID,
//        SPECTRALSPREAD,
//        SPECTRALDECREASE,
//        PITCH,
//        SPECTRALFLATNESS,
//        PITCHCHROMAFLATNESS};
    
    void extractFeatures(float*,int); //processes FFT data, time domain signal, and calculates features //calls other private functions to calculate each features
    
    int getNumOfFeatures();
    int getFftSize();
    vector<float> getFftData();
    float getSpectralFlux(float);
    float getSpectralFluxLog(float);
    float getSpectralRollOff(float);
    float getSpectralCentroid();
    float getSpectralSpread();
    float getSpectralDecrease();
    float getSpectralFlatness();
    float getSpectralCrest();
    vector<float> getPitchChroma();
    float getPitch();
    float getPitchChromaFlatness();
    float getPitchChromaCrestFactor();
    bool spectralFluxLevelCrossingRateChanged();

    vector<float> getNormalizedFeatureSet();
    
    int LCRFluxThreshold = 20; //default
    float instantaneousFluxThreshold=0.5; //default
    
private:
    int numFeatures = 10;
    
    ofxFft* fft;
    ofMutex soundMutex;
    
    int fftSize, sampleRate, bufferSize;
    float* signal;
    vector <float> fftData, fftDataPrev, pitchChroma, finalPitchChroma, middlePitchChroma;
    
    float instantaneousFlux, instantaneousFluxLog, instantaneousFluxLP, instantaneousFluxPrev=0, instantaneousFluxLogPrev=0;
    float instantaneousRollOff, instantaneousRollOffPrev = 0, instantaneousRollOffLP;
    float instantaneousSC, instantaneousSS, instantaneousSD, instantaneousSF, instantaneousPCF;
    float instantaneousPitch, instantaneousSCR, instantaneousPCC;
    float sumOfFftBins,rms;
    
    int LCRFlux = 0; //global counter
    
    float alphaFlux,alphaRollOff;
    
    float referencePitch, curFreq, chromaSum, harmonicsSum;
    float** midiBins; //for Pitch Chroma
    
    void resetFeatures();
    void calcRms();
    bool isSilenceDetected();
    void normalizeInputAudio();
    void calcFft();
    void findMaxBin();
    void sumFftBins();
    void calcSpectralFlux(float);
    void calcSpectralFluxLog();
    void calcSpectralRollOff(float);
    void calcSpectralCentroid();
    void calcSpectralSpread();
    void calcSpectralDecrease();
    void calcSpectralFlatness();
    void calcSpectralCrest();
    void calcPitchChroma();
    void calcPitchChromaFlatness();
    float calcPitchChromaCrestFactor(float); //called from calcPitchChroma()
    
    //Normalize features
    //Expose features
};


#endif /* myFeatures_hpp */
