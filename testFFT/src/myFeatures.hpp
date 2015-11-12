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
    
//    enum class Feature {
//        SPECTRALFLUX,
//        SPECTRALROLLOFF,
//        SPECTRALCENTROID,
//        SPECTRALSPREAD,
//        SPECTRALDECREASE,
//        PITCH,
//        SPECTRALFLATNESS,
//        PITCHCHROMAFLATNESS};
    
    void extractFeatures(float*,int); //processes FFT data, time domain signal, and calculates features //calls other private functions to calculate each features
    
    int* getNumOfFeatures();
    int* getFftSize();
    vector<float> getFftData();
    float* getSpectralFlux();
    float* getSpectralRollOff();
    float* getSpectralCentroid();
    float* getSpectralSpread();
    float* getSpectralDecrease();
    float* getSpectralFlatness();
    float* getSpectralCrest();
    vector<float> getPitchChroma();
    float* getPitch();
    float* getPitchChromaFlatness();
    float* getPitchChromaCrestFactor();
    bool spectralFluxLevelCrossingRateChanged();

    vector<float> getNormalizedFeatureSet();
    
    void setAlphaFlux(float);
    void setAlphaRollOff(float);
    
private:
    int numFeatures = 10;
    
    ofxFft* fft;
    ofMutex soundMutex;
    
    int fftSize, sampleRate, bufferSize;
    float* signal;
    vector <float> fftData, fftDataPrev, pitchChroma, finalPitchChroma, middlePitchChroma;
    
    float instantaneousFlux, instantaneousFluxPrev;
    float instantaneousRollOff, instantaneousRollOffPrev;
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
    void calcFft();
    void findMaxBin();
    void sumFftBins();
    void calcSpectralFlux();
    void calcSpectralRollOff();
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
