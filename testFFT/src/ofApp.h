#pragma once

#include "ofMain.h"
#include "ofxFft.h"

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
    
    void getFreshMesh();
    
   
    
    
    int plotHeight, bufferSize, overlapMultiple, nBuffers, sampleRate, nInputs, nOutputs;
    
    ofxFft* fft;
    ofSoundStream soundStream;

    ofMutex soundMutex;
    vector<float> drawBins, middleBins, audioBins, leftInput, leftWithOverlap, spectralFlux, spectralRollOff, fftData;
    float* block;
    
    vector<float> pitchChroma;
    
    bool succ,alphaBool,zeeBool,jitterBool;
    ofImage myImage;
   
    float instantaneousFlux, instantaneousRollOff,instantaneousPitch, curFreq, referencePitch;
    ofFbo fbo;
    ofPixels fboPixels;
    ofMesh mesh;
    vector<ofVec3f> offsets;
    int jitterCounter,numHops;
    
    float** midiBins;
};
