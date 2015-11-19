#include "ofMain.h"
#include "ofApp.h"

#define WIDTH 610 //shura1
#define HEIGHT 610

//========================================================================
int main( ){
//	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
//    ofSetupOpenGL(784,628,OF_WINDOW);			// <-------- rains.jpg
//        ofSetupOpenGL(610,610,OF_WINDOW);			// <-------- Shura1.png
            ofSetupOpenGL(WIDTH,HEIGHT,OF_WINDOW);			// <-------- liz.jpg

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
