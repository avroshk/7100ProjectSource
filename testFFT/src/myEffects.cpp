//
//  myEffects.cpp
//  testFFT
//
//  Created by Avrosh Kumar on 11/8/15.
//
//

#include "myEffects.hpp"

myEffects::myEffects(string imageFileName,int width, int height) {
    succ = myImage.loadImage(imageFileName);
    mirrorImage = myImage;
    mirrorImage.mirror(false, true);
    
    imageHeight = height;
    imageWidth = width;
    
    setUpMeshVertices();
    setUpTriangleIndices();
    backupMesh = meshGrid;
    
    
}

ofImage* myEffects::getImage() {
    return &myImage;
}

ofImage* myEffects::getMirrorImage() {
    return &mirrorImage;
}

ofMesh* myEffects::getMeshGrid() {
    return &meshGrid;
}

ofMesh* myEffects::getMesh() {
    return &mesh;
}

void myEffects::setUpMeshVertices() {
//    float intensityThreshold = 50;
    
    //Set up vertices
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            
            
            meshGrid.addVertex(ofPoint((x - W/2) * meshSize, (y - H/2) * meshSize, 0));
            // textures can be swapped / updated by calling this function
//            ofColor c = myImage.getColor(x, y);
//            float intensity = c.getLightness();
//            if (intensity >= intensityThreshold) {
//                mesh.addVertex(ofPoint((x - W/2) * meshSize, (y - H/2) * meshSize, 0 ));
//                mesh.addTexCoord(ofPoint(x * ( imageWidth/ W), y * (imageHeight / H)));
//                mesh.addColor(ofColor(255,0,0));
//            }
            
            meshGrid.addTexCoord(ofPoint(x * ( imageWidth/ W), y * (imageHeight / H)));
            meshGrid.addColor(ofColor(255, 255, 255));
            offsets.push_back(ofVec3f(ofRandom(0,100000), ofRandom(0,100000), ofRandom(0,100000)));
        }
    }
}

void myEffects::setUpTriangleIndices() {
    //Set up triangles' indices
    for (int y=0; y<H-1; y++) {
        for (int x=0; x<W-1; x++) {
            int i1 = x + W * y;
            int i2 = x+1 + W * y;
            int i3 = x + W * (y+1);
            int i4 = x+1 + W * (y+1);
            meshGrid.addTriangle( i1, i2, i3 );
            meshGrid.addTriangle( i2, i4, i3 );
            
//            mesh.addTriangle( i1, i2, i3 );
//            mesh.addTriangle( i2, i4, i3 );
        }
    }
    
}

void myEffects::applyNoiseToMesh(float feature1, float feature2, float feature3) {
    //Apply noise to mesh
    //Change vertices
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            
            //Vertex index
            int i = x + W * y;
            ofPoint p = meshGrid.getVertex( i );
            
            //Change z-coordinate of vertex
            
//            p.z = ofNoise(x * 0.05, y * 0.05, ofGetElapsedTimef() * 0.5) * 100;
              p.z = ofNoise(x * feature1, y * feature2, ofGetElapsedTimef() * 0.3) * 100; //control the third argument using a feature that is prolonged over the time
            meshGrid.setVertex( i, p );
            
            //Change color of vertex
            meshGrid.setColor(i , ofColor(255, 255, 255));
        }
    }
}

void myEffects::applyAlphaToTexture(float alpha, float alpha2) {
    //Apply alpha to texture
    //Change vertices
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            
            //Vertex index
            int i = x + W * y;
            
            //Change color of vertex
            meshGrid.setColor(i , ofColor(255, 255, 255,alpha));
        }
    }
}

void myEffects::refreshMesh() {
    meshGrid = backupMesh;
}

void myEffects::applyJitterToMesh(float displacementScale) {
    
    for (int j=0; j<meshGrid.getNumVertices(); ++j) {
        ofVec3f vert = meshGrid.getVertex(j);
        
        float time = ofGetElapsedTimef();
        float timeScale = 5.0;
//        float displacementScale = 0.75; // keyword - this parameter can be controlled
        ofVec3f timeOffsets = offsets[j];
    
        vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
        vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
        vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
        
        meshGrid.setVertex(j, vert);
    }
}


