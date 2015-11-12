//
//  myEffects.hpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/8/15.
//
//

#ifndef myEffects_hpp
#define myEffects_hpp

#include <stdio.h>
#include "ofMain.h"

class myEffects {
public:
    myEffects(string); //constructor
    void applyNoiseToMesh(float,float,float);
    void applyAlphaToTexture(float,float);
    void applyJitterToMesh(float);
    void refreshMesh();
    ofImage* getImage();
    ofMesh* getMeshGrid();
    ofMesh* getMesh();
    
private:
    ofImage myImage;
    ofMesh mesh, meshGrid, backupMesh;
    
    vector<ofVec3f> offsets;
    
    bool succ;
    
    void setUpMeshVertices();
    void setUpTriangleIndices();
    
    int imageHeight = 628;
    int imageWidth = 784;
    int W = 100; //Grid size
    int H = 100;
    int meshSize = 6;
    
};

#endif /* myEffects_hpp */
