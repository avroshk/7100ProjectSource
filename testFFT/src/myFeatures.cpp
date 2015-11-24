//
//  myFeatures.cpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/3/15.
//
//

#include "myFeatures.hpp"

void myFeatures::resetFeatures() {
    sumOfFftBins = 0; sumOfNormFftBins = 0;
    rms = 0;
    instantaneousFluxPrev = instantaneousFlux;
    instantaneousFlux = 0;
    instantaneousFluxLP = 0;
    instantaneousFluxLogPrev = instantaneousFluxLog;
    instantaneousFluxLog = 0;
    
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
    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HANN, OF_FFT_BASIC);
    
    fftSize = fft->getBinSize();
    
    fftData.resize(fftSize);
    fftDataPrev.resize(fftSize);
    normalizedInput.resize(bufferSize);
    normalizedFft.resize(fftSize);
    
    //Inititalize Midi Pitches vector for Pitch Chroma
    
    referencePitch = 440; //hard-coded
    curFreq = 0;
    int length = fftSize;
    int height = 3;
    pitchChroma.resize(12);
    finalPitchChroma.resize(12);
    middlePitchChroma.resize(12);
    
    midis.resize(fftSize);
    midiWeights.resize(fftSize);
    midiSkipper.resize(fftSize);
   
    midiBins = new float*[height];
    midiBins[0] = new float[length];
    midiBins[1] = new float[length];
    midiBins[2] = new float[length];
    int curAvgCount = 1;
    
    float tempCalc = (float)sampleRate*2/(float)(fftSize);
    
    for (int k = 0; k < fftSize; k++) {
        
        curFreq = (k+1)*tempCalc;
        
        if(curFreq == 0)
            midiBins[0][k] = 0;
        else
            midiBins[0][k] = round(69+12*log2f(curFreq/referencePitch));
        
        midiBins[1][k] = 1;
        midiWeights[k] = midiBins[1][k];
        midiBins[2][k] = 0;
        midiSkipper[k] = midiBins[2][k];
        
        if (k>0) {
            if(midiBins[0][k-1] == midiBins[0][k]) {
                curAvgCount++;
            }
            else {
                if(curAvgCount>1) {
                    for(int i =0;i<curAvgCount; i++) {
                        midiBins[1][k-i-1] = (float)1/curAvgCount;
                        midiWeights[k-i-1] = midiBins[1][k-i-1];
                        if (i!=0) {
                            midiBins[2][k-i-1] = -1;
                            midiSkipper[k-i-1] = midiBins[2][k-i-1];
                        }
                    }
                    curAvgCount = 1;
                }
                
            }
        }
        midis[k] = midiBins[0][k];
    }
}

int myFeatures::getNumOfFeatures() {
    return numFeatures;
}

vector<float> myFeatures::getNormalizedInputSignal() {
    return normalizedInput;
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
    
    calcRms();
    normalizeInputAudio(); //Not all features us normalized input
        if (!isSilenceDetected()) {
        calcFft();
        calcNormFft();
        sumFftBins();
        sumNormFftBins();
        calcSpectralFlux(2);
//        calcSpectralFluxLog();
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
    float maxValue = 0;

    for(int i = 0; i < bufferSize; i++) {
        if(abs(signal[i]) > maxValue) {
            maxValue = abs(signal[i]);
        }
    }
    
    for(int i = 0; i < bufferSize; i++) {
        normalizedInput[i] = signal[i]/maxValue;
    }
}

//void myFeatures::normalizeFft() {
//    float maxValue = 0;
//    
//    for(int i = 0; i < fftSize; i++) {
//        if(abs(fftData[i]) > maxValue) {
//            maxValue = abs(fftData[i]);
//        }
//    }
//    
//    for(int i = 0; i < fftSize; i++) {
//        normalizedFft[i] = fftData[i]/maxValue;
//    }
//}

void myFeatures::sumFftBins() {
    for(int i = 0; i < fftSize; i++) {
        sumOfFftBins = sumOfFftBins + fftData[i];
    }
}

void myFeatures::sumNormFftBins() {
    for(int i = 0; i < fftSize; i++) {
        sumOfNormFftBins = sumOfNormFftBins + normalizedFft[i];
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
        return true; //return false if Silence is detected
    }
    return false;
}

void myFeatures::calcFft() {
    
    fft->setSignal(signal);
    
    float* curFft = fft->getAmplitude();
    //    memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
    copy(curFft, curFft + fft->getBinSize(), fftData.begin());
    
    for(int i = 0; i < fftSize; i++) {
        fftData[i] /=2;
    }
    
//    normalizeFft();

}

void myFeatures::calcNormFft() {
    
    fft->setSignal(normalizedInput);
    
    float* curFft = fft->getAmplitude();
    //    memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
    copy(curFft, curFft + fft->getBinSize(), normalizedFft.begin());
    
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
    
    instantaneousRollOff = (float) i/fftSize;
//    instantaneousRollOff = (float) (i+1)*sampleRate/(bufferSize); //correctin i to i+1
    
}

void myFeatures::calcSpectralCentroid() {
    float sumSC=0, curSumSC=0;
    for(int i = 0; i < fftSize; i++) {
        curSumSC = pow(normalizedFft[i],2);
        
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
        curSumSS = pow(normalizedFft[i],2);
        
        instantaneousSS += pow(i-instantaneousSC, 2)*curSumSS;
        
        sumSS += curSumSS;
    }
    
    instantaneousSS /= sumSS;
    
    //Normalize
    
}

void myFeatures::calcSpectralDecrease() {
    
    for(int i = 0; i < fftSize; i++) {
        instantaneousSD += (normalizedFft[i] - normalizedFft[0])/(i+1);
    }
    
    instantaneousSD /= sumOfNormFftBins;
    
    //Normalize
    
}


void myFeatures::calcSpectralFlatness() {
    
    for(int i = 0; i < fftSize; i++) {
        instantaneousSF += (log(normalizedFft[i]));
    }
    
    instantaneousSF /= fftSize;
    
    instantaneousSF = exp(instantaneousSF);
    
    instantaneousSF /= (sumOfNormFftBins/fftSize);
    
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
    
    for (int i=0; i<12; i++) {
        pitchChroma[i] = 0.0;
    }
    
    float maxBinValue = 0;
    int maxBinLoc = 0;

    int lowestMidiPitch = 60; // C4 = 261.6Hz

    for (int j=0; j<12; j++) {
        int numOctaves = 4; //Check only four octaves
        for (int k=0; k<fftSize; k++) {
            if(int(midiBins[0][k]) >= lowestMidiPitch && numOctaves > 0)
            {
                if ((int(midiBins[0][k]) - lowestMidiPitch) % 12 == 0) {
//                    if (midiBins[1][k] != -1) {
//                        harmonicsSum += midiBins[1][k]*midiBins[1][k];
//                        numOctaves--;
//                    }
                    harmonicsSum += pow(midiBins[1][k]*fftData[k],2);
                    if (midiBins[2][k] != -1) {
                        numOctaves--;
                    }
                }
            }
        }
        pitchChroma[j] = harmonicsSum;
        harmonicsSum = 0;
        lowestMidiPitch++;
        
        chromaSum = chromaSum + pitchChroma[j];
    }
    
    for (int i=0; i<fftSize; i++) {
        if (maxBinValue < fftData[i]) {
            maxBinValue = fftData[i];
            maxBinLoc = i;
        }
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

