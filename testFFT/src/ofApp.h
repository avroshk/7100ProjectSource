#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "myFeatures.hpp"
#include "myMappingVector.hpp"
#include "myEffects.hpp"
#include "ofxLibsndFileRecorder.h"
#include "sndfile.h"

const int HEIGHT = 600;//rains
const int WIDTH = 728;
const string IMGFILE = "helen.jpg";

//const int HEIGHT = 610;//shura
//const int WIDTH = 610;
//const string IMGFILE = "Shura1.png";
//const int HEIGHT = 610;//shura
//const int WIDTH = 610;
//const string IMGFILE = "Shura1.png";


const int BUFFERSIZE = 256;
const int OVERLAPMULTIPLE = 4;
const int NBUFFERS = 1;
const int SAMPLERATE = 44100;
const int NUMHOPS = 4;
const int INPUTS = 1;
const int OUTPUTS = 0;
/* the device id corresponds to all audio devices, including  input-only and output-only devices.
  0 - at home - Microphone Input
  1 - at couch - Microphone Input
  2 - at home - SoundFlower
  3 - at couch - SoundFlower */
const int DEVICEID = 3  ;

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
    vector<float> downMixAudio(float* leftInput, float* rightInput, int bufferSize);
    void blockAndProcessAudioData(float* input, int bufferSize, int nChannels);
    void processBlock(float* block, int windowBuffer, int nChannels);
    
    //Test features ------
//     ofstream myfile;
//        void testFeatures();
#ifdef TEST
    ofstream myfile;
    SndfileHandle myAudioFile;
    void testFeatures();
#endif
    
    //Graphs ----------
    int plotHeight;
    ofFbo fbo;
    ofPixels fboPixels;
    
    //Audio --------
    
    myFeatures *features;
    ofSoundStream soundStream; //For live audio
    ofMutex soundMutex;
    
    vector<float> drawBins, middleBins, audioBins, leftInput, rightInput, downMixedInput, pitchChroma, normalizedInput,drawInput, middleInput, downMixed;
    
    float* block;
    float* buffer;
    int numHops;
    
    //Mapping ---------
    
    myMappingVector *featureMap;
    
    //Mesh -----------
    
    myEffects *effects;
    ofMesh *meshGrid;
    ofMesh mesh, *mesh1;
    ofImage *myImage;
    ofImage img1, img2, img3, img4;
    
    bool succ,alphaBool,zeeBool,drawBool,flipBool;
    bool bool1,bool2,bool3,bool4,boolAll;
 
    
    //---- temp
    float tempMax = 0;
    //---- temp
    
};
