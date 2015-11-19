#pragma once
//
//#define BUFFERSIZE 2048 //hopSize
//#define OVERLAPMULTIPLE 2 // times the hopSize
//#define NBUFFERS 1
//#define SAMPLERATE 44100
////#define INPUTS = 2; //stereo
//#define INPUTS 1 //mono
//#define OUTPUTS 0
//#define NUMHOPS 0
//#define DEVICEID 2
//#define WIDTH 300 //liz
//#define HEIGHT 398
//#define WIDTH 590 //paris
//#define HEIGHT 590
//#define WIDTH 800 //helen
//#define HEIGHT 800
#define WIDTH 610 //shura1
#define HEIGHT 610


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
    
    //Test features ------
    void testFeatures();
    ofSoundPlayer audioclip;
    
    //Graphs ----------
    int plotHeight, BUFFERSIZE, OVERLAPMULTIPLE, NBUFFERS, SAMPLERATE, NUMHOPS, INPUTS, OUTPUTS,DEVICEID;

    
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
    ofMesh mesh, *mesh1;
    
    ofFbo fbo;
    ofPixels fboPixels;
    
    bool succ,alphaBool,zeeBool,drawBool;
    ofImage* myImage;
    
    //---- temp
    float tempMax = 0;
    bool flag = false;
    ofstream myfile;
    //---- temp
    

};
