#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "myFeatures.hpp"
#include "myMappingVector.hpp"
#include "myEffects.hpp"

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
    int plotHeight, BUFFERSIZE, OVERLAPMULTIPLE, NBUFFERS, SAMPLERATE, NUMHOPS, INPUTS, OUTPUTS, DEVICEID;
    
    //Audio --------
    
    myFeatures *features;

    ofSoundStream soundStream;

    ofMutex soundMutex;
    
    vector<float> drawBins, middleBins, audioBins, leftInput, rightInput, pitchChroma;
    
    float* block;
    //Mapping ---------
    
    myMappingVector *featureMap;
    
    //Mesh -----------
    
    myEffects *effects;
    ofMesh *meshGrid;
    ofMesh mesh;
    
    ofFbo fbo;
    ofPixels fboPixels;
    
    bool succ,alphaBool,zeeBool,drawBool;
    ofImage* myImage;
    
    //---- temp
    float tempMax = 0;
    //---- temp
    

};
