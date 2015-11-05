//
//  myMappingVector.hpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/5/15.
//
//

#ifndef myMappingVector_hpp
#define myMappingVector_hpp

#include <stdio.h>
#include "ofMain.h"

class myMappingVector {
    
public:
    myMappingVector(int numFeatures, int numEffects); //Constructor

    void routeFeature(float alphaValue, int featureId, int effectId);
    float getFeatureForEffect(int effectId, vector<float> featureVector);
    
private:
    int numFeatures, numEffects;
    vector<int> countAlphaValues;
    vector<vector<float> > mappingVector;
    
};

#endif /* myMappingVector_hpp */
