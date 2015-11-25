//
//  myFeatures.cpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/3/15.
//
//

#include "myFeatures.hpp"

void myFeatures::resetFeatures() {
    rms = 0;
    sumOfFftBins = 0;
    sumOfNormFftBins = 0;
    
    //Set previous flux
    instantaneousFluxPrev = instantaneousFlux;
    instantaneousFlux = 0;
    instantaneousFluxLP = 0;
    
    //Set previous flux
    instantaneousFluxLogPrev = instantaneousFluxLog;
    instantaneousFluxLog = 0;
    
    instantaneousRollOffPrev = instantaneousRollOff;
    instantaneousRollOff = 0;
    instantaneousRollOffLP = 0;
    
    instantaneousPitch = 0;
    instantaneousSC = 0;
    instantaneousSS = 0;
    instantaneousSD = 0;
    
    instantaneousPCF = 0;
    instantaneousPCC = 0;
    instantaneousSCR = 0;
    instantaneousSF = 0;
    
    harmonicsSum = 0;
    chromaSum = 0;
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
    
    pitchChroma.resize(12);
    middlePitchChroma.resize(12);
    finalPitchChroma.resize(12);
    
    
    //Inititalize weighting for Pitch Chroma
    initPitchChromaWeightFilter();
   
}

void myFeatures::extractFeatures(float* input, int nChannels) {
    resetFeatures();
    
    signal = input;
    
    calcRms();
    
    if (!isSilenceDetected()) {
        normalizeInputAudioBlock(); //Not all features use normalized input
        calcFft();
        calcNormFft(); //Calculate fft using normalized input
        sumFftBins();
        sumNormFftBins();
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

//----------- get methods start >>>>>>>>>>>>>

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

vector<float> myFeatures::getNormalizedFftData() {
    return normalizedFft;
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

//<<<<<<<<<<< get methods end -------------

void myFeatures::normalizeInputAudioBlock() {
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

void myFeatures::normalizeFft() {
    float maxValue = 0;
    
    for(int i = 0; i < fftSize; i++) {
        if(abs(fftData[i]) > maxValue) {
            maxValue = abs(fftData[i]);
        }
    }
    
    for(int i = 0; i < fftSize; i++) {
        normalizedFft[i] = fftData[i]/maxValue;
    }
}

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
            instantaneousFluxLog = 0;
            return;
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
    
    instantaneousRollOff = (float) i/fftSize; //Normalized output
}

void myFeatures::calcSpectralCentroid() {
    float sumSC=0, curSumSC=0;
    
    for(int i = 0; i < fftSize; i++) {
        curSumSC = pow(normalizedFft[i],2);
        instantaneousSC += curSumSC*i;
        sumSC += curSumSC;
    }
    try {
        instantaneousSC /= sumSC;
    } catch (logic_error e) {
        instantaneousSC = 0;
        return;
    }

    //Normalize
}

void myFeatures::calcSpectralSpread() {
    float sumSS=0, curSumSS=0;
    for(int i = 0; i < fftSize; i++) {
        curSumSS = pow(normalizedFft[i],2);
        
        instantaneousSS += pow(i-instantaneousSC, 2)*curSumSS;
        
        sumSS += curSumSS;
    }
    
    try {
        instantaneousSS /= sumSS;
    } catch (logic_error e) {
        instantaneousSS = 0;
        return;
    }

    //Normalize
    
}

void myFeatures::calcSpectralDecrease() {
    
    for(int i = 0; i < fftSize; i++) {
        instantaneousSD += (normalizedFft[i] - normalizedFft[0])/(i+1);
    }
    
    
    try {
        instantaneousSD /= sumOfNormFftBins;
    } catch (logic_error e) {
        instantaneousSD = 0;
        return;
    }
    
    //Normalize
    
}


void myFeatures::calcSpectralFlatness() {
    
    for(int i = 0; i < fftSize; i++) {
        instantaneousSF += (log(normalizedFft[i]));
    }
    
    instantaneousSF /= fftSize;
    instantaneousSF = exp(instantaneousSF);
    
    try {
        instantaneousSF /= (sumOfNormFftBins/fftSize);
    } catch (logic_error e) {
        instantaneousSF = 0;
        return;
    }
    
    //Normalize
    
}

void myFeatures::calcSpectralCrest() {
    
    for(int i = 0; i < fftSize; i++) {
        if (instantaneousSCR < fftData[i]) {
            instantaneousSCR = fftData[i];
        }
    }
    
    try {
        instantaneousSCR /= sumOfFftBins;
    } catch (logic_error e) {
        instantaneousSCR = 0;
        return;
    }
    
    //Normalize
}

void myFeatures::resetPitchChroma() {
    //Reset pitch chroma values to 0
    for (int i=0; i<12; i++) {
        pitchChroma[i] = 0.0;
    }
}

void myFeatures::calcPitchChroma() {
    
    resetPitchChroma();
    
    float maxBinValue = 0;
    int maxBinLoc = 0;

    for (int j=0; j<12; j++) {
       
        for (int k=0; k<fftSize; k++) {
            harmonicsSum += pow(filterWeightsForPitchChroma[j][k]*fftData[k],2);
        }
        pitchChroma[j] = harmonicsSum;
        harmonicsSum = 0;
        
        chromaSum += pitchChroma[j];
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
        try {
            pitchChroma[m] /= chromaSum;
        } catch (logic_error e) {
            resetPitchChroma();
            soundMutex.lock();
            middlePitchChroma = pitchChroma;
            soundMutex.unlock();
            return;
        }
    }

    soundMutex.lock();
    middlePitchChroma = pitchChroma;
    soundMutex.unlock();
}

void myFeatures::initPitchChromaWeightFilter() {
    
    filterWeightsForPitchChroma.resize(12,vector<float>(fftSize));
    
    
    referencePitch = 440; //hard-coded
    curFreq = 0;
    
    matchedBinsCount = 1;
    prevBinMidiPitch = 0;
    currentBinMidiPitch = 0;
    numOctaves =4;
    chromaMidiPitch = 60; // C4 = 261.6Hz
    numNotesInOctave = 12;
    
    binFreqMultiple = (float)sampleRate*0.5/(float)(fftSize);
    
    //Iterate through all the bins of the spectrum
    for (int k = 0; k < fftSize; k++) {
        
        //1. calculate the midi pitch value of the bin
        //2. rounding of the pitch to integer gives quantizes the midi pitches in the -50 cents to +50 cents range
        curFreq = (k+1)*binFreqMultiple;
        currentBinMidiPitch = round(69+12*log2f(curFreq/referencePitch));
        
        //reset the chroma midi pitch to the lowest value at the start from on every iteration
        chromaMidiPitch = 60;
        
        //Start from the second bin since we keep track of midi pitch of previous bin
        if (k>0) {
            
            //Iterate through all the midi pitches in an octave starting from the lowest
            for (int j=0; j<numNotesInOctave; j++) {
                
                //check if the current bin's midi value lies between the defined range of octaves
                if(currentBinMidiPitch >= chromaMidiPitch && currentBinMidiPitch <= (numOctaves*numNotesInOctave)+chromaMidiPitch) {
                    
                    //do the following only for octaves of the midi pitch we are currently dealing with
                    if ((currentBinMidiPitch - chromaMidiPitch) % 12 == 0) {
                        
                        //keep track of a counter of the current midi pitch is same as previous
                        //we are keeping track of the span of a midi pitch value across bins
                        if(prevBinMidiPitch == currentBinMidiPitch) {
                            matchedBinsCount++;
                        }
                        else {
                            
                            //previous matches were found if count is greater than 1
                            if(matchedBinsCount>1) {
                                
                                for(int i =0;i<matchedBinsCount; i++) {
                                    
                                    //since matches were found for the previous pitch in the chroma we decrease the index j by 1 in a cyclic manner.
                                    int index = -1;
                                    if (j == 0) {
                                        index = 11;
                                    } else {
                                        index = j-1;
                                    }
                                    
                                    //since previous matches were found iterate through those bins and distribute the weights
                                    filterWeightsForPitchChroma[index][k-i-1] = (float)1/matchedBinsCount;
                                }
                                matchedBinsCount = 1;
                            }
                            
                            //set weight to 1
                            filterWeightsForPitchChroma[j][k] = 1;
                        }
                    }
                }
                //next midi pitch
                chromaMidiPitch++;
            }
        }
        prevBinMidiPitch = currentBinMidiPitch;
    }
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

