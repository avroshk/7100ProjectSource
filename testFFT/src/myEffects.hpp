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
    myEffects(string,int,int); //constructor
    void applyNoiseToMesh(float,float,float);
    void applyAlphaToTexture(float,float);
    void applyJitterToMesh(float);
    void refreshMesh();
    ofImage* getImage();
    ofImage* getMirrorImage();
    ofMesh* getMeshGrid();
    ofMesh* getMesh();
    
private:
    ofImage myImage, mirrorImage;
    ofMesh mesh, meshGrid, backupMesh;
    
    vector<ofVec3f> offsets;
    
    bool succ;
    
    void setUpMeshVertices();
    void setUpTriangleIndices();
    void setMeshTexture(ofMesh m);
    
    int imageHeight;
    int imageWidth;
    int W = 100; //Grid size
    int H = 100;
    int meshSize = 6;
    
};

#endif /* myEffects_hpp */
