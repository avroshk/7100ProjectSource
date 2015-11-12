#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    
    DEVICEID = 2;  // 2 - at home // 3 - at couch - SoundFlower
//    DEVICEID = 1;  // 0 - at home // 1 - at couch - Microphone Input

    
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
    
    drawBins.resize(*features->getFftSize());
    middleBins.resize(*features->getFftSize());
    audioBins.resize(*features->getFftSize());
    pitchChroma.resize(12);
    
    // ------ Sound Setup --------
    //ofSoundStreamSetup(0, 1, this, 44100, BUFFERSIZE, 4); //BUFFERSIZE is set here
    soundStream.setup(this, OUTPUTS, INPUTS, SAMPLERATE, BUFFERSIZE, NBUFFERS);
    
    ofBackground(0, 0, 0);
    
    // ------ Feature to Effect Mapping setup -----

    featureMap = new myMappingVector(*features->getNumOfFeatures(),2);
    featureMap->routeFeature(1, 0, 0);
    featureMap->routeFeature(1, 1, 1);
    featureMap->routeFeature(0.8, 3, 0);
    
    // ------ Image setup -----------
    
    effects = new myEffects("Helen.jpg");
    
    myImage = effects->getImage();
    
//    succ = myImage.loadImage("Helen.jpg");

//    getFreshMesh();
    
    fbo.allocate(784, 628);
    // clear fbo
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
}

//--------------------------------------------------------------
void ofApp::update(){
    // ------ Get features -------
    float feature1 = featureMap->getFeatureForEffect(0, features->getNormalizedFeatureSet());
    float feature2 = featureMap->getFeatureForEffect(1, features->getNormalizedFeatureSet());
    
    features->setAlphaFlux(*features->getSpectralFlatness());
    
    effects->applyNoiseToMesh(ofMap(*features->getSpectralRollOff(),0,0.5,0,0.05),ofMap(*features->getSpectralDecrease(),0,0.5,0,0.005));
    
//    effects->applyNoiseToMesh(*features->getSpectralRollOff(),*features->getSpectralDecrease());
    
    effects->applyJitterToMesh(*features->getPitchChromaCrestFactor());
    
    meshGrid = effects->getMeshGrid();
    
    if (features->spectralFluxLevelCrossingRateChanged()) {
        effects->refreshMesh();
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

    
    // effects :--------------------------------------------------

//    float alpha = ofMap(exp(instantaneousFlux*50.0f), 1, 50, 180, 255);
//    float alpha = exp(instantaneousFlux*50);
     //float alpha = ofMap(features->getSpectralFlux()*30.0f, 0.5, 1, 180, 255);
//     float alpha = ofMap(features->getSpectralDecrease(), 0, 0.5, 180, 255); // good feature mapping
//    float alpha = ofMap(features->getSpectralFlatness(), 0, 0.5, 180, 255); //no bad
//     float alpha = ofMap(features->getPitchChromaFlatness(), 0, 0.5, 180, 255); // good feature (kinetoscope effect)
    float alpha = ofMap(*features->getPitchChromaCrestFactor(), 0, 0.7, 180, 255);
     float alpha2 = ofMap(*features->getSpectralRollOff()*10.0f, 0, 1, 180, 255);
//    float alpha = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
    
    if(*features->getPitchChromaCrestFactor() > tempMax) { tempMax = *features->getPitchChromaCrestFactor();}
    
    //string msgFlux = "Flux " + ofToString((float)features->getSpectralCentroid()*50.0f);
    string msgFlux = "Flux " + ofToString((float)*features->getSpectralFlux()*100.0f);
    ofDrawBitmapString(msgFlux, 80, ofGetHeight() - 5);
    
    string msgRollOff ="Roll off " + ofToString((float)*features->getSpectralRollOff()*50.0f);
    ofDrawBitmapString(msgRollOff, 80, ofGetHeight() - 15);
    
    string msgAlpha = "Alpha " + ofToString((float) alpha2);
    ofDrawBitmapString(msgAlpha, 250, ofGetHeight() - 5);
    
    string msgPitch ="Pitch " + ofToString((float)*features->getPitch()) + " Hz";
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
  myImage->rotate90(0);
    
    if (zeeBool){
        myImage->draw(0,0,alpha-80);
        for(int x = 0; x<mesh.getNumVertices(); x++){
            ofVec3f pos = mesh.getVertex(x);
            pos.z=alpha-80;
            mesh.setVertex(x, pos);
        }
    }
    else{
        myImage->draw(0,0);
    }
    
    effects->applyAlphaToTexture(alpha2, alpha);
    
    myImage->bind();
    
    ofPushMatrix(); //Store the coordinate system
    ofTranslate( ofGetWidth()/2, ofGetHeight()/2, 0 );  //Move the coordinate center to screen's center
//    meshGrid.drawWireframe();
    meshGrid->draw();
    ofPopMatrix(); //Restore the coordinate system
    
    mesh.draw();
    myImage->unbind();
    
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

    //task : Use circular mapping to pointer to improve performance
    
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
    }
    if (key == 'j') {
        drawBool = ! drawBool;
    }
    if (key == 'k') {

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
