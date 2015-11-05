#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "myFeatures.hpp"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    void plot(vector<float>& buffer, float scale, float offset);
    void audioReceived(float* input, int bufferSize, int nChannels);
    void processBlock(float* block, int windowBuffer, int nChannels);
    void changeDeviceId();
    
    void getFreshMesh();
    
    //Graphs ----------
    int plotHeight, bufferSize, overlapMultiple, nBuffers, sampleRate, numHops, nInputs, nOutputs, deviceId;
    
    //Audio --------
    
    myFeatures *features;

    ofSoundStream soundStream;

    ofMutex soundMutex;
    
    vector<float> drawBins, middleBins, audioBins, leftInput, rightInput, pitchChroma;
    
    float* block;
    
    //Mesh -----------
    ofFbo fbo;
    ofPixels fboPixels;
    ofMesh mesh, meshGrid;
    
    vector<ofVec3f> offsets;
    int jitterCounter;
    
    bool succ,alphaBool,zeeBool,jitterBool;
    ofImage myImage;
    
    int imageHeight = 628;
    int imageWidth = 784;
    int W = 100; //Grid size
    int H = 100;
    int meshSize = 6;
    //---- temp
    float tempMax = 0;
    //---- temp
    

};
