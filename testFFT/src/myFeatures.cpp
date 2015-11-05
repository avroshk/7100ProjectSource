//
//  myFeatures.cpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/3/15.
//
//

#include "myFeatures.hpp"

myFeatures::myFeatures() {
    sumOfFftBins = 0; rms =0; instantaneousFlux = 0; instantaneousFluxPrev = 0;
    instantaneousRollOff =0;
    instantaneousRollOffPrev = 0;
    harmonicsSum = 0; chromaSum = 10;
    
    //expose functions to set alpha values
    alphaFlux = 0.5;
    alphaRollOff = 0.5;
}

myFeatures::myFeatures(int sRate, int bufSize, int numBins) {
    
    bufferSize = bufSize;
    sampleRate = sRate;

    
    //Initialize FFT
    //fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
    
    fftData.resize(bufferSize);
    signal.resize(bufferSize);
    fftDataPrev.resize(bufferSize);
    
    fftSize = fft->getBinSize();
    
    //Inititalize Midi Pitches vector for Pitch Chroma
    
    referencePitch = 440; //hard-coded
    curFreq = 0;
    int length = numBins;
    int height = 2;
    pitchChroma.resize(12);
    
    midiBins = new float*[height];
    midiBins[0] = new float[length];
    midiBins[1] = new float[length];
    
    float tempCalc = sampleRate/(bufferSize);
    
    for (unsigned int k = 0; k < numBins; k++) {
        
        curFreq = k*tempCalc;
        
        if(curFreq == 0)
            midiBins[0][k] = 0;
        else
            midiBins[0][k] = round(69+12*log2f(curFreq/referencePitch));
    }
    
}

int myFeatures::getNumOfFeatures() {
    return numFeatures;
}

float myFeatures::getSpectralFlux() {
    return instantaneousFlux;
}

float myFeatures::getSpectralRollOff() {
    return instantaneousRollOff;
}

vector<float> myFeatures::getPitchChroma() {
    return pitchChroma;
}

void myFeatures::extractFeatures(vector<float>& input) {
    
    myFeatures(); //call constructor to reset values
    signal = input;
    
    if (calcRms()) {
        calcFft();
        sumFftBins();
        calcSpectralFlux();
        calcSpectralRollOff();
        calcPitchChroma();
    }
}

void myFeatures::sumFftBins() {
    for(int i = 0; i < fftSize; i++) {
        sumOfFftBins = sumOfFftBins + fftData[i];
    }
}

bool myFeatures::calcRms() {
    for(int i = 0; i < signal.size(); i++) {
        rms = rms + signal[i]*signal[i];
    }
    rms = sqrt(rms/signal.size());
    
    if (rms == 0) {
        return false; //return flase if Silence is detected
    }
    return true;
}

void myFeatures::calcFft() {
    
    float maxValue = 0;
    
    for(int i = 0; i < bufferSize; i++) {
        if(abs(signal[i]) > maxValue) {
            maxValue = abs(signal[i]);
        }
    }
    
    for(int i = 0; i < bufferSize; i++) {
        signal[i] /= maxValue;
    }
    
    fft->setSignal(signal);
    
    float* curFft = fft->getAmplitude();
    //    memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
    copy(curFft, curFft + fft->getBinSize(), fftData.begin());
    
    
    maxValue = 0;
    for(int i = 0; i < fftSize; i++) {
        if(abs(fftData[i]) > maxValue) {
            maxValue = abs(fftData[i]);
        }
    }
    for(int i = 0; i < fftSize; i++) {
        fftData[i] /= maxValue;
    }
    
    fftDataPrev = fftData;
}

void myFeatures::calcSpectralFlux() {
    for(int i = 0; i < fftSize; i++) {
        instantaneousFlux = ((fftDataPrev[i]-fftData[i])*(fftDataPrev[i]-fftData[i])) + instantaneousFlux;
    }
    instantaneousFlux = 2*instantaneousFlux/fftSize;
    
    instantaneousFlux = (1-alphaFlux)*instantaneousFlux + alphaFlux*instantaneousFluxPrev;
    
    instantaneousFluxPrev = instantaneousFlux;

}

void myFeatures::calcSpectralRollOff() {
    instantaneousRollOff = 2*sumOfFftBins/fftSize;
    
    instantaneousRollOff = (1-alphaRollOff)*instantaneousRollOff + alphaRollOff *instantaneousRollOffPrev;
    
    instantaneousRollOffPrev = instantaneousRollOff;

}

void myFeatures::calcPitchChroma() {
    
    float maxBinValue = 0;
    int maxBinLoc = 0;
    float prevMidiPitch = -1;
    int curAvgCount = 1;
    float tempSum = 0;
    
    
    for (unsigned int k = 0; k < fftSize; k++){
        
        midiBins[1][k] = fftData[k];
        
        if (k>0) {
            if(midiBins[0][k-1] == midiBins[0][k]) {
                curAvgCount++;
            }
            else {
                if(curAvgCount>1) {
                    for(int i =0;i<curAvgCount; i++){
                        tempSum += midiBins[1][k-i-1];
                        midiBins[1][k-i-1] = -1;
                    }
                    midiBins[1][k-1] = tempSum/curAvgCount;
                    curAvgCount = 1;
                    tempSum = 0;
                }
                
            }
        }
        
        if(maxBinValue < fftData[k]){
            maxBinValue = fftData[k];
            maxBinLoc = k;
        }
    }

    int lowestMidiPitch = 21;
    
    
    
    for (int j=0; j<12; j++) {
        for (int k=0; k<fftSize; k++) {
            if ((int(midiBins[0][k]) - lowestMidiPitch) % 12 == 0) {
                if (midiBins[1][k] != -1) {
                    harmonicsSum += midiBins[1][k];
                }
            }
        }
        pitchChroma[j] = harmonicsSum;
        harmonicsSum = 0;
        lowestMidiPitch++;
        
        chromaSum = chromaSum + pitchChroma[j];
    }
    
    instantaneousPitch =  maxBinLoc*sampleRate/(bufferSize);
    
    for (int m=0; m<12; m++) {
        pitchChroma[m] /= chromaSum;
    }
}

