//
//  myMappingVector.cpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/5/15.
//
//

#include "myMappingVector.hpp"

myMappingVector::myMappingVector(int nFeatures, int nEffects) {
    numFeatures = nFeatures;
    numEffects = nEffects;
    
    //Initialize 3-dimensional mapping vector
    
    mappingVector.resize(numFeatures); //LENGTH
    for (int i=0; i<numFeatures; i++) {
        mappingVector[i].resize(numEffects); //WIDTH
    }
    countAlphaValues.resize(numEffects);
}

void myMappingVector::routeFeature(float alphaRatio, int featureId, int effectId) {
    
    //put validation to make sure alphaRatio is between 0 and 1
    
    float remainderRatio = 0;
    
    if (countAlphaValues[effectId] > 0) {
        remainderRatio = 1 - alphaRatio;
        
        for (int i = 0; i<numFeatures; i++) {
            if (mappingVector[i][effectId] != 0) {
                mappingVector[i][effectId] *= remainderRatio;
            }
        }
        mappingVector[featureId][effectId] = alphaRatio;
    }
    else {
        mappingVector[featureId][effectId] = 1;
    }
    
    if (alphaRatio == 1) {
        countAlphaValues[effectId] = 1;
    }
    else {
        countAlphaValues[effectId]++;
    }
}

float myMappingVector::getFeatureForEffect(int effectId, vector<float> featureVector) {
    float sumMappedFeatures = 0;
    for (int i=0; i<numFeatures; i++) {
        sumMappedFeatures += featureVector[i]*mappingVector[i][effectId];
    }
    return sumMappedFeatures/countAlphaValues[effectId];
}