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
    myFeatures(int,int,int); //Constructor
    myFeatures(); //Constructor
    
    void extractFeatures(vector<float> &); //processes FFT data, time domain signal, and calculates features //calls other private functions to calculate each features
    
    int getNumOfFeatures();
    float getSpectralFlux();
    float getSpectralRollOff();
    vector<float> getPitchChroma();
    
private:
    int numFeatures = 6;
    
    ofxFft* fft;
    
    int fftSize, sampleRate, bufferSize, overlapMultiple;
    vector <float> signal, fftData, fftDataPrev, pitchChroma;
    
    float instantaneousFlux, instantaneousFluxPrev, alphaFlux;
    float instantaneousRollOff, instantaneousRollOffPrev, alphaRollOff;
    float instantaneousPitch;
    float sumOfFftBins,rms,maxBinLoc,MaxBinValue;
    
    float referencePitch, curFreq, chromaSum, harmonicsSum;
    float** midiBins; //for Pitch Chroma
    
    bool calcRms();
    void calcFft();
    void findMaxBin();
    void sumFftBins();
    void calcSpectralFlux();
    void calcSpectralRollOff();
    void calcPitchChroma();
    
    //Normalize features
    //Expose features
};


#endif /* myFeatures_hpp */
