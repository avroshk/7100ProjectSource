//
//  myFeatures.cpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/3/15.
//
//

#include "myFeatures.hpp"

void myFeatures::resetFeatures() {
    sumOfFftBins = 0; rms =0; instantaneousFluxPrev = instantaneousFlux; instantaneousFlux = 0; instantaneousFluxLP = 0; instantaneousFluxLogPrev = instantaneousFluxLog; instantaneousFluxLog = 0;
    
    instantaneousRollOffPrev = instantaneousRollOff; instantaneousRollOff = 0; instantaneousRollOffLP = 0;
    instantaneousPitch = 0; instantaneousSC = 0; instantaneousSS = 0; instantaneousSD = 0;
    harmonicsSum = 0; chromaSum = 0, rms = 0; instantaneousPCF = 0; instantaneousPCC = 0;
    instantaneousSCR = 0; instantaneousSF = 0;

}

myFeatures::myFeatures(int sRate, int bufSize) {
    
    resetFeatures();
    
    bufferSize = bufSize;
    sampleRate = sRate;

    
    //Initialize FFT
    //fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
    
    fftData.resize(bufferSize);
    fftDataPrev.resize(bufferSize);
    
    fftSize = fft->getBinSize();
    
    //Inititalize Midi Pitches vector for Pitch Chroma
    
    referencePitch = 440; //hard-coded
    curFreq = 0;
    int length = fftSize;
    int height = 2;
    pitchChroma.resize(12);
    finalPitchChroma.resize(12);
    middlePitchChroma.resize(12);
    
    midiBins = new float*[height];
    midiBins[0] = new float[length];
    midiBins[1] = new float[length];
    
    float tempCalc = sampleRate/(bufferSize);
    
    for (unsigned int k = 0; k < fftSize; k++) {
        
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

int myFeatures::getFftSize() {
    return fftSize;
}

vector<float> myFeatures::getFftData() {
    return fftData;
}

float myFeatures::getSpectralFlux(float alpha) {
    
    instantaneousFluxLP = (1-alpha)*instantaneousFlux + alpha*instantaneousFlux;
    
    return instantaneousFluxLP;
}

float myFeatures::getSpectralFluxLog(float alpha) {
    
    instantaneousFluxLog = (1-alpha)*instantaneousFluxLog + alpha*instantaneousFluxLogPrev;
    
    return instantaneousFluxLog;
}

float myFeatures::getSpectralRollOff(float alpha) {
    
    instantaneousRollOffLP = (1-alpha)*instantaneousRollOff + alpha * instantaneousRollOffPrev;
    
    return instantaneousRollOff;
}

float myFeatures::getSpectralCentroid() {
    return instantaneousSC;
}

float myFeatures::getSpectralSpread() {
    return instantaneousSS;
}

float myFeatures::getSpectralDecrease() {
    return instantaneousSD;
}

float myFeatures::getSpectralFlatness() {
    return instantaneousSF;
}

float myFeatures::getSpectralCrest() {
    return instantaneousSCR;
}

vector<float> myFeatures::getPitchChroma() {
    soundMutex.lock();
    finalPitchChroma = middlePitchChroma;
    soundMutex.unlock();
    
    return finalPitchChroma;
    
}

float myFeatures::getPitch() {
    return instantaneousPitch;
}

float myFeatures::getPitchChromaFlatness() {
    return instantaneousPCF;
}

float myFeatures::getPitchChromaCrestFactor() {
    return instantaneousPCC;
}

bool myFeatures::spectralFluxLevelCrossingRateChanged() {
    if (LCRFlux > LCRFluxThreshold) {
        LCRFlux = 0;
        return true;
    }
    return false;
}

void myFeatures::extractFeatures(float* input, int nChannels) {
    resetFeatures();
    
    signal = input;
    
    
//    normalizeInputAudio();
    calcRms();
        if (isSilenceDetected()) {
        calcFft();
        sumFftBins();
        calcSpectralFlux(2);
        calcSpectralFluxLog();
        calcSpectralRollOff(0.85);
        calcSpectralCentroid();
        calcSpectralSpread();
        calcSpectralDecrease(); // keep no dependency
        calcSpectralFlatness();
        calcSpectralCrest();
        calcPitchChroma();
        calcPitchChromaFlatness();
        
        fftDataPrev = fftData;
    }
    
}

void myFeatures::normalizeInputAudio() {
    float maxAmp = 0;
    for (int i=0; i<bufferSize; i++) {
        if (maxAmp < signal[i]) {
            maxAmp = signal[i];
        }
    }
    if (maxAmp > 0) {
        for (int i=0; i<bufferSize; i++) {
            signal[i] /=maxAmp;
        }
    }
}

void myFeatures::sumFftBins() {
    for(int i = 0; i < fftSize; i++) {
        sumOfFftBins = sumOfFftBins + fftData[i];
    }
}

void myFeatures::calcRms() {
    for(int i = 0; i < bufferSize; i++) {
        rms = rms + signal[i]*signal[i];
    }
    rms = sqrt(rms/bufferSize);
}

bool myFeatures::isSilenceDetected() {
    if (rms < 0.000001) {
        return false; //return false if Silence is detected
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
}

void myFeatures::calcSpectralFluxLog() {
    for(int i = 0; i < fftSize; i++) {
        try {
            instantaneousFluxLog +=  abs(log2f(fftData[i]/fftDataPrev[i]));
        } catch (logic_error e) {
            //continue looping
        }
        
    }
    instantaneousFluxLog = instantaneousFluxLog/fftSize;
    
}

void myFeatures::calcSpectralFlux(float degree) {
    //boundary conditions
    if (degree < 0.25) {
        degree = 0.25;
    }
    else if (degree > 3) {
        degree = 3;
    }

    for(int i = 0; i < fftSize; i++) {
        instantaneousFlux +=  pow(abs(fftData[i]-fftDataPrev[i]), degree);
    }
    
    instantaneousFlux = pow(instantaneousFlux, (float)1.0/degree)/fftSize;
    
    //Normalize
//    instantaneousFlux /= maxAmpl; //not needed

    if (instantaneousFlux > instantaneousFluxThreshold) {
        LCRFlux++;
        cout<<LCRFlux<<" ";
    }
}

void myFeatures::calcSpectralRollOff(float rollOffPerc) {

    float threshold = rollOffPerc*sumOfFftBins;
    int i; float cumSum = 0;
    for (i=0; i<fftSize; i++) {
        cumSum +=fftData[i];
        if (cumSum > threshold) {
            break;
        }
    }
    
//    instantaneousRollOff = (float) i/fftSize;
    instantaneousRollOff = (float) i*sampleRate/(bufferSize);
    
}

void myFeatures::calcSpectralCentroid() {
    float sumSC=0, curSumSC=0;
    for(int i = 0; i < fftSize; i++) {
        curSumSC = pow(fftData[i],2);
        
        instantaneousSC += curSumSC*i;
        
        sumSC += curSumSC;
    }
    
    instantaneousSC /= sumSC;
    
    //Normalize
//    instantaneousSC /= fftSize;
    //instantaneousSC

}

void myFeatures::calcSpectralSpread() {
    float sumSS=0, curSumSS=0;
    for(int i = 0; i < fftSize; i++) {
        curSumSS = pow(fftData[i],2);
        
        instantaneousSS += pow(i-instantaneousSC, 2)*curSumSS;
        
        sumSS += curSumSS;
    }
    
    instantaneousSS /= sumSS;
    
    //Normalize
    
}

void myFeatures::calcSpectralDecrease() {
    
    for(int i = 0; i < fftSize; i++) {
        instantaneousSD += (fftData[i] - fftData[0])/(i+1);
    }
    
    instantaneousSD /= sumOfFftBins;
    
    //Normalize
    
}


void myFeatures::calcSpectralFlatness() {
    
    for(int i = 0; i < fftSize; i++) {
        instantaneousSF += (log(fftData[i]));
    }
    
    instantaneousSF /= fftSize;
    
    instantaneousSF = exp(instantaneousSF);
    
    instantaneousSF /= (sumOfFftBins/fftSize);
    
    //Normalize
    
}

void myFeatures::calcSpectralCrest() {
    
    for(int i = 0; i < fftSize; i++) {
        if (instantaneousSCR < fftData[i]) {
            instantaneousSCR = fftData[i];
        }
    }
    
    instantaneousSCR /= sumOfFftBins;
    
    //Normalize
    
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
    
    //Max pitch chroma
    float maxPitchChroma = 0;
    for (int i =0; i<pitchChroma.size(); i++) {
        if (maxPitchChroma < pitchChroma[i]) {
            maxPitchChroma = pitchChroma[i];
        }
    }
    
    //Calculate pitch chroma crest factor
    instantaneousPCC = calcPitchChromaCrestFactor(maxPitchChroma);
    
    //Normalize
    for (int m=0; m<12; m++) {
        pitchChroma[m] /= chromaSum;
    }
    
    soundMutex.lock();
    middlePitchChroma = pitchChroma;
    soundMutex.unlock();
}

void myFeatures::calcPitchChromaFlatness() {
    for (int m=0; m<12; m++) {
        instantaneousPCF += log(pitchChroma[m]);
    }
    
    instantaneousPCF /= 12;
    
    instantaneousPCF = exp(instantaneousPCF);
    
    instantaneousPCF /= (chromaSum/12);
}

float myFeatures::calcPitchChromaCrestFactor(float maxPitchChroma) {
    return (maxPitchChroma/chromaSum);
}

vector<float> myFeatures::getNormalizedFeatureSet() {
    vector<float> setOfFeatures;
    setOfFeatures.resize(numFeatures);
//    for (int i=0; i<numFeatures; i++) {
//        setOfFeatures[i] =
//    }
    setOfFeatures[0] = instantaneousFlux;
    setOfFeatures[1] = instantaneousRollOff;
    setOfFeatures[2] = instantaneousSC;
    setOfFeatures[3] = instantaneousSS;
    setOfFeatures[4] = instantaneousSD;
    setOfFeatures[5] = instantaneousPitch;
    //debug this - something wrong
   
    return setOfFeatures;
}

