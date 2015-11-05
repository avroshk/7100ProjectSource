#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    
    DEVICEID = 2;  // 2 - at home // 3 - at couch - SoundFlower
    //DEVICEID = 1;  // 1 - at home // 2 - at couch - Microphone Input

    
    plotHeight = 128;
    
    BUFFERSIZE = 256; //hopSize
    OVERLAPMULTIPLE = 4; // times the hopSize
    NBUFFERS = 1;
    SAMPLERATE = 44100;
    //INPUTS = 2; //stereo
    INPUTS = 1; //mono
    OUTPUTS = 0;
    NUMHOPS = 0;
    
    leftInput.assign(BUFFERSIZE, 0.0); //size of one hop
    
    if(INPUTS == 2) {
        rightInput.assign(BUFFERSIZE, 0.0); //size of one hop
    }
    
    soundStream.listDevices();
    
    //if you want to set a different device id
    soundStream.setDeviceID(DEVICEID); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
    
    features = new myFeatures(SAMPLERATE,BUFFERSIZE*OVERLAPMULTIPLE);
    
    block = new float[BUFFERSIZE*OVERLAPMULTIPLE];
    
    drawBins.resize(features->getFftSize());
    middleBins.resize(features->getFftSize());
    audioBins.resize(features->getFftSize());
    pitchChroma.resize(12);
    
    // ------ Sound Setup --------
    //ofSoundStreamSetup(0, 1, this, 44100, BUFFERSIZE, 4); //BUFFERSIZE is set here
    soundStream.setup(this, OUTPUTS, INPUTS, SAMPLERATE, BUFFERSIZE, NBUFFERS);
    
    ofBackground(0, 0, 0);
    
    // ------ Feature to Effect Mapping setup -----

    featureMap = new myMappingVector(features->getNumOfFeatures(),2);
    featureMap->routeFeature(1, 0, 0);
    featureMap->routeFeature(1, 1, 1);
    featureMap->routeFeature(0.8, 3, 0);
    
    // ------ Image setup -----------
    succ = myImage.loadImage("rains.jpg");

    getFreshMesh();
    
    fbo.allocate(784, 628);
    // clear fbo
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
    jitterCounter = 0;
    
    //Set up vertices
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            meshGrid.addVertex(ofPoint((x - W/2) * meshSize, (y - H/2) * meshSize, 0 )); // adding texure coordinates allows us to bind textures to it later // --> this could be made into a function so that textures can be swapped / updated
            meshGrid.addTexCoord(ofPoint(x * ( imageWidth/ W), y * (imageHeight / H)));
            meshGrid.addColor(ofColor(255, 255, 255));
        }
    }
    
    //Set up triangles' indices
    for (int y=0; y<H-1; y++) {
        for (int x=0; x<W-1; x++) {
            int i1 = x + W * y;
            int i2 = x+1 + W * y;
            int i3 = x + W * (y+1);
            int i4 = x+1 + W * (y+1);
            meshGrid.addTriangle( i1, i2, i3 );
            meshGrid.addTriangle( i2, i4, i3 );
        }
    }

}
//--------------------

//--------------------
void ofApp::getFreshMesh(){
    float intensityThreshold = 175.0;
    
    int w = myImage.getWidth();
    int h = myImage.getHeight();
    
    for (int x=0; x<w; ++x) {
        for (int y=0; y<h; ++y) {
            ofColor c = myImage.getColor(x, y);
            float intensity = c.getLightness();
            
            if (intensity >= intensityThreshold) {
                ofVec3f pos(x, y);
                mesh.addVertex(pos);
                mesh.addTexCoord(pos);
                //for jitter
                offsets.push_back(ofVec3f(ofRandom(0,100000), ofRandom(0,100000), ofRandom(0,100000)));
            }
        }
    }
    
    //cout<< mesh.getNumVertices();
}

//--------------------------------------------------------------
void ofApp::update(){
    // ------ Get feature ---
    float feature1 = featureMap->getFeatureForEffect(0, features->getNormalizedFeatureSet());
    float feature2 = featureMap->getFeatureForEffect(1, features->getNormalizedFeatureSet());
    
    
    //Apply noise to mesh
    //Change vertices
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            
            //Vertex index
            int i = x + W * y;
            ofPoint p = meshGrid.getVertex( i );
            
            //Change z-coordinate of vertex
            
            p.z = ofNoise(x * 0.05, y * 0.05, ofGetElapsedTimef() * 0.5) * 100;
            meshGrid.setVertex( i, p );
            
            //Change color of vertex
            meshGrid.setColor(i , ofColor(255, 255, 255));
        }
    }

    
    if(jitterBool){
        
        for (int j=0; j<mesh.getNumVertices(); ++j) {
            ofVec3f vert = mesh.getVertex(j);
            
            jitterCounter++;
            
            float time = ofGetElapsedTimef();
            float timeScale = 5.0;
            float displacementScale = 0.75; // keyword - this parameter can be controlled
            ofVec3f timeOffsets = offsets[j];
            
            
            if (features->getPitch()< 2500){
                if (vert.x > 0 && vert.x < 2*628/4) {
                    vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
                    vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
                    vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
                }
            }
            else if (features->getPitch() > 2500){
                if (vert.x > 2*628/4 && vert.x < 628) {
                    vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
                    vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
                    vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
                }
            }
            
            mesh.setVertex(j, vert);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(16, 16);
    
    soundMutex.lock();
    drawBins = middleBins;
    soundMutex.unlock();
    
    ofDrawBitmapString("Frequency Domain", 0, 0);
    plot(drawBins, -plotHeight, plotHeight / 2);
    ofPopMatrix();
    string msg = ofToString((int) ofGetFrameRate()) + " fps";
    ofDrawBitmapString(msg, ofGetWidth() - 80, ofGetHeight() - 20);
    
    // draw the left channel:
    ofPushStyle();
        ofPushMatrix();
            ofTranslate(16, 200, 0);
            
            ofSetColor(225);
            ofDrawBitmapString("Input Channel", 4, 18);
            
            ofSetLineWidth(1);
            ofRect(0, 0, 512, 200);
            
            ofSetColor(245, 58, 135);
            ofSetLineWidth(3);
    
            ofBeginShape();
            for (unsigned int i = 0; i < leftInput.size(); i++){
                ofVertex(2*i, 100 -leftInput[i]*180.0f);
            }
            ofEndShape(false);
        ofPopMatrix();
    ofPopStyle();
    
    // draw the pitch chroma:
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(16, 440, 0);
    
    ofSetColor(225);
    ofDrawBitmapString("Pitch Chroma", 4, -18);
    
    ofSetLineWidth(1);
    ofRect(0, 0, 480, 160);
    
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    string pitches[] = {"A","A#","B","C","C#","D","D#","E","F","F#","G","G#"};
    pitchChroma = features->getPitchChroma();
    for(int i =0; i<12;i++){
        ofRect(0+40*i,0,40, pitchChroma[i]*200);
        ofDrawBitmapString(pitches[i], 0+40*i,-4);
    }
    
    ofPopMatrix();
    ofPopStyle();

    
    // flux :--------------------------------------------------

//    float alpha = ofMap(exp(instantaneousFlux*50.0f), 1, 50, 180, 255);
//    float alpha = exp(instantaneousFlux*50);
     //float alpha = ofMap(features->getSpectralFlux()*30.0f, 0.5, 1, 180, 255);
     float alpha = ofMap(features->getSpectralDecrease(), 0, 0.5, 180, 255);
     float alpha2 = ofMap(features->getSpectralRollOff()*10.0f, 0, 1, 180, 255);
//    float alpha = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
    
    if(features->getSpectralDecrease() > tempMax) { tempMax = features->getSpectralDecrease();}
    
    //string msgFlux = "Flux " + ofToString((float)features->getSpectralCentroid()*50.0f);
    string msgFlux = "Flux " + ofToString((float)features->getSpectralDecrease()*50.0f);
    ofDrawBitmapString(msgFlux, 80, ofGetHeight() - 5);
    
    string msgRollOff ="Roll off " + ofToString((float)features->getSpectralRollOff()*50.0f);
    ofDrawBitmapString(msgRollOff, 80, ofGetHeight() - 15);
    
    string msgAlpha = "Alpha " + ofToString((float) alpha2);
    ofDrawBitmapString(msgAlpha, 250, ofGetHeight() - 5);
    
    string msgPitch ="Pitch " + ofToString((float)features->getPitch()) + " Hz";
    ofDrawBitmapString(msgPitch, 400, ofGetHeight() - 5);
    
    string msgTemp ="Temp " + ofToString((float)tempMax);
    ofDrawBitmapString(msgTemp, 550, ofGetHeight() - 45);

    

    fbo.begin();
    if (alphaBool){
        ofSetColor(255,255,255, alpha2);
    }
    else{
        ofSetColor(255,255,255, 180);
    }
    //    ofSetColor(255,255,255, 180);

    for(int x = 0; x<mesh.getNumVertices(); x++){
        
        
        ofVec3f pos = mesh.getVertex(x);
        
        if (pos.x > 300 and pos.x < 500){
//            ofColor c = myImage.getColor(pos.x,pos.y);
//            //        float intensity = c.getLightness();
//            
//            c.setBrightness(alpha);
//            
//            myImage.setColor(pos.x, pos.y, c);
           
        }
    }
  myImage.rotate90(0);
    
    
    if (zeeBool){
        myImage.draw(0,0,alpha-80);
        for(int x = 0; x<mesh.getNumVertices(); x++){
            ofVec3f pos = mesh.getVertex(x);
            pos.z=alpha-80;
            mesh.setVertex(x, pos);
        }
    }
    else{
        myImage.draw(0,0);
        
    }
    
    myImage.bind();
    
    //ofPushMatrix(); //Store the coordinate system
//    ofTranslate( ofGetWidth()/2, ofGetHeight()/2, 0 );  //Move the coordinate center to screen's center
//    //meshGrid.drawWireframe();
//    meshGrid.draw();
//    ofPopMatrix(); //Restore the coordinate system
//
    
    mesh.draw();
    myImage.unbind();
    
    fbo.end();
    
    fbo.draw(0,0);
}
//-----------

void ofApp::plot(vector<float>& buffer, float scale, float offset) {
    ofNoFill();
    int n = buffer.size();
    ofRect(0, 0, n, plotHeight);
    glPushMatrix();
    glTranslatef(0, plotHeight / 2 + offset, 0);
    ofBeginShape();
    for (int i = 0; i < n; i++) {
        ofVertex(i, sqrt(buffer[i]) * scale);
    }
    ofEndShape();
    glPopMatrix();
}
//--------

void ofApp::processBlock(float* window, int windowBufferSize, int nChannels){
    
    features->extractFeatures(window,nChannels);
    
    audioBins = features->getFftData();
    
    soundMutex.lock();
    middleBins = audioBins;
    soundMutex.unlock();
}

//---------

void ofApp::audioReceived(float* input, int BUFFERSIZE, int nChannels) {
    
    float* inputLeft = input;
    float* inputRight;
    
    if (nChannels == 2) {
       inputRight = input + BUFFERSIZE; //not used for now
    }

    //task : Use circular mapping of pointer to improve performance
    
    if (soundStream.getTickCount() > OVERLAPMULTIPLE-1) {
        processBlock(block,BUFFERSIZE*OVERLAPMULTIPLE,nChannels);
        //Shift block data to left
        copy(block+BUFFERSIZE, block+BUFFERSIZE*OVERLAPMULTIPLE, &block[0]);
        //Push the last hop into the block
        copy(inputLeft, inputLeft + BUFFERSIZE, &block[BUFFERSIZE*(OVERLAPMULTIPLE-1)]);
    }
    else {
        copy(inputLeft, inputLeft + BUFFERSIZE, &block[NUMHOPS*BUFFERSIZE]);
        NUMHOPS++;
    }
    
    //Store the input in the leftInput
    for(int i = 0; i < BUFFERSIZE; i++) {
        leftInput[i] = input[i];
    }
    
    //Store the input in the rightInput if stereo -- not being used for now
    if (nChannels==2) {
        for(int i = BUFFERSIZE; i < 2*BUFFERSIZE; i++) {
            rightInput[i] = input[i];
        }
    }
    
    
    
}
//--------------------------------------------------------------

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'm') {
        alphaBool = ! alphaBool;
    }
    if (key == 'n') {
        zeeBool = ! zeeBool;
//        for(int x = 0; x<mesh.getNumVertices(); x++){
//            ofVec3f pos = mesh.getVertex(x);
//            pos.z=0;
//            mesh.setVertex(x, pos);
//        }
        getFreshMesh();
    }
    if (key == 'j') {
        jitterBool = ! jitterBool;
        getFreshMesh();
    }
    if (key == 'k') {
        getFreshMesh();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
