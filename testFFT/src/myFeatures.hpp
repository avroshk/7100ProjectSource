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
    
    void extractFeatures(float*,int); //processes FFT data, time domain signal, and calculates features //calls other private functions to calculate each features
    
    int getNumOfFeatures();
    int getFftSize();
    vector<float> getFftData();
    float getSpectralFlux();
    float getSpectralRollOff();
    float getSpectralCentroid();
    float getSpectralSpread();
    float getSpectralDecrease();
    vector<float> getPitchChroma();
    float getPitch();
    
    vector<float> getNormalizedFeatureSet();
   
    
    float alphaFlux,alphaRollOff;
    
private:
    int numFeatures = 6;
    
    ofxFft* fft;
    ofMutex soundMutex;
    
    int fftSize, sampleRate, bufferSize;
    float* signal;
    vector <float> fftData, fftDataPrev, pitchChroma, finalPitchChroma, middlePitchChroma;
    
    float instantaneousFlux, instantaneousFluxPrev;
    float instantaneousRollOff, instantaneousRollOffPrev;
    float instantaneousSC, instantaneousSS, instantaneousSD;
    float instantaneousPitch;
    float sumOfFftBins,rms;
    
    float referencePitch, curFreq, chromaSum, harmonicsSum;
    float** midiBins; //for Pitch Chroma
    
    void resetFeatures();
    bool calcRms();
    void calcFft();
    void findMaxBin();
    void sumFftBins();
    void calcSpectralFlux();
    void calcSpectralRollOff();
    void calcSpectralCentroid();
    void calcSpectralSpread();
    void calcSpectralDecrease();
    void calcPitchChroma();
    
    //Normalize features
    //Expose features
};


#endif /* myFeatures_hpp */
