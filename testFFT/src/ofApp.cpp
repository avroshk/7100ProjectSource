#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
 
    
    ofSetVerticalSync(true);
    
    plotHeight = 128;
    bufferSize = 256; //hopSize
    overlapMultiple = 4; // times the hopSize
    nBuffers = 1;
    sampleRate = 44100;
    //nInputs = 2; //stereo
    nInputs = 1; //mono
    nOutputs = 0;
    numHops = 0;
    
    leftInput.assign(bufferSize, 0.0); //size of one hop
    
    if(nInputs == 2) {
        rightInput.assign(bufferSize, 0.0); //size of one hop
    }
    
    leftWithOverlap.assign(bufferSize*overlapMultiple, 0.0); //size of hop*overlapMultiple
    
    soundStream.listDevices();
    
    //if you want to set a different device id
    soundStream.setDeviceID(3); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
    
    //fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HAMMING);
    fft = ofxFft::create(bufferSize*overlapMultiple, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
    
    block = new float[bufferSize*overlapMultiple];
    
    drawBins.resize(fft->getBinSize());
    middleBins.resize(fft->getBinSize());
    audioBins.resize(fft->getBinSize());
    spectralFlux.resize(fft->getBinSize());
    spectralRollOff.resize(fft->getBinSize());
    fftData.resize(fft->getBinSize());
    pitchChroma.resize(12);
    
    //ofSoundStreamSetup(0, 1, this, 44100, bufferSize, 4); //bufferSize is set here
    
    soundStream.setup(this, nOutputs, nInputs, sampleRate, bufferSize, nBuffers);
    
    instantaneousFlux = 0;
    instantaneousFluxPrev = 0;
    instantaneousRollOff = 0;
    instantaneousRollOffPrev = 0;
    instantaneousPitch = 0;
    
    ofBackground(0, 0, 0);
    
    // ------- Pitch chroma setup ------
    referencePitch = 440;
    curFreq = 0;
    int length = fftData.size();
    int height = 2;
    
    midiBins = new float*[height];
    midiBins[0] = new float[length];
    midiBins[1] = new float[length];
    
    float tempCalc = sampleRate/(bufferSize*overlapMultiple);
    
    for (unsigned int k = 0; k < fftData.size(); k++){
        
        curFreq = k*tempCalc;
        
        if(curFreq == 0)
            midiBins[0][k] = 0;
        else
            midiBins[0][k] = round(69+12*log2f(curFreq/referencePitch));
    }
    
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
void ofApp::changeDeviceId() {
    if (micInput == 1) {
        soundStream.setDeviceID(1);
    }
    else {
        soundStream.setDeviceID(3);
    }
}

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
    
    //Max pitch chroma
    float maxPitch = 0;
    for (int i =0; i<pitchChroma.size(); i++) {
        if (maxPitch < pitchChroma[i]) {
            maxPitch = pitchChroma[i];
        }
    }
    
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
    alphaFactor = 0.5;
    
    //Instantaneous Flux ---------------
    instantaneousFlux = 0;
    for (unsigned int i = 0; i < spectralFlux.size(); i++){
        instantaneousFlux = instantaneousFlux + spectralFlux[i];
//        if(instantaneousFlux < spectralFlux[i]){
//            instantaneousFlux = spectralFlux[i];
//        }
    }
    //instantaneousFlux = 2*sqrt(instantaneousFlux)/spectralFlux.size();
    instantaneousFlux = 2*instantaneousFlux/spectralFlux.size();
    //instantaneousFlux = instantaneousFlux/20;
    
    instantaneousFlux = (1-alphaFactor)*instantaneousFlux + alphaFactor*instantaneousFluxPrev;
    
    instantaneousFluxPrev = instantaneousFlux;
    
    //Instantaneous RollOff ---------------
    for (unsigned int i = 0; i < spectralRollOff.size(); i++){
        instantaneousRollOff = instantaneousRollOff + spectralRollOff[i];
    }
    instantaneousRollOff = 2*instantaneousRollOff/spectralRollOff.size();
    
    instantaneousRollOff = (1-alphaFactor)*instantaneousRollOff + alphaFactor*instantaneousRollOffPrev;

    instantaneousRollOffPrev = instantaneousRollOff;
    
    //Find pitch --------------------
    float maxBinValue = 0;
    int maxBinLoc = 0;
    float prevMidiPitch = -1;
    int curAvgCount = 1;
    float tempSum = 0;

    
    for (unsigned int k = 0; k < fftData.size(); k++){
        
        midiBins[1][k] = fftData[k];
        
        if (k>0) {
            if(midiBins[0][k-1] == midiBins[0][k]) {
                curAvgCount++;
            }
            else {
                if(curAvgCount>1) {
                    for(int i =0;i<curAvgCount; i++){
                        tempSum += midiBins[1][k-i-1];
                        midiBins[1][k-i-1] = -1;
                    }
                    midiBins[1][k-1] = tempSum/curAvgCount;
                    curAvgCount = 1;
                    tempSum = 0;
                }
               
            }
        }
        
        if(maxBinValue < fftData[k]){
            maxBinValue = fftData[k];
            maxBinLoc = k;
        }
    }
    
    float chromaSum = 0;
    int lowestMidiPitch = 21;
    
    for (int j=0; j<12; j++) {
        for (int k=0; k<fftData.size(); k++) {
            if ((int(midiBins[0][k]) - lowestMidiPitch) % 12 == 0) {
                if (midiBins[1][k] != -1) {
                      chromaSum += midiBins[1][k];
                }
            }
        }
        pitchChroma[j] = chromaSum;
        chromaSum = 0;
        lowestMidiPitch++;
    }
    
    instantaneousPitch =  maxBinLoc*sampleRate/(bufferSize*overlapMultiple);
    
    if(jitterBool){
        
        for (int j=0; j<mesh.getNumVertices(); ++j) {
            ofVec3f vert = mesh.getVertex(j);
            
            jitterCounter++;
            
            float time = ofGetElapsedTimef();
            float timeScale = 5.0;
            float displacementScale = 0.75; // keyword - this parameter can be controlled
            ofVec3f timeOffsets = offsets[j];
            
            
            if (instantaneousPitch< 2500){
                if (vert.x > 0 && vert.x < 2*628/4) {
                    vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
                    vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
                    vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
                }
            }
            else if (instantaneousPitch > 2500){
                if (vert.x > 2*628/4 && vert.x < 628) {
                    vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
                    vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
                    vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
                }
            }
            
            mesh.setVertex(j, vert);
        }
    }
    
    //cout<<(string)soundStream.getTickCount() + " ";
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
    /*--
    // draw the left channel with overlap:
    ofPushStyle();
        ofPushMatrix();
            ofTranslate(16, 400, 0);
            
            ofSetColor(225);
            ofDrawBitmapString("Input Channel with overlap", 4, 18);
            
            ofSetLineWidth(1);
            ofRect(0, 0, 512, 200);
            
            ofSetColor(245, 58, 135);
            ofSetLineWidth(3);
            
            ofBeginShape();
            for (unsigned int i = 0; i < leftWithOverlap.size(); i++){
                ofVertex(i/2, 100 -leftWithOverlap[i]*45.0f);
            }
            
            ofEndShape(false);
        ofPopMatrix();
    ofPopStyle();
    */
    
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
    for(int i =0; i<12;i++){
        ofRect(0+40*i,0,40, pitchChroma[i]*50);
        ofDrawBitmapString(pitches[i], 0+40*i,-4);
    }
    
    ofPopMatrix();
    ofPopStyle();

    
    // flux :--------------------------------------------------

//    float alpha = ofMap(exp(instantaneousFlux*50.0f), 1, 50, 180, 255);
//    float alpha = exp(instantaneousFlux*50);
     float alpha = ofMap(instantaneousFlux*30.0f, 0.5, 1, 180, 255);
     float alpha2 = ofMap(instantaneousRollOff*10.0f, 0, 1, 180, 255);
//    float alpha = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);

    
    string msgFlux = "Flux " + ofToString((float)instantaneousFlux*50.0f);
    ofDrawBitmapString(msgFlux, 80, ofGetHeight() - 5);
    
    string msgRollOff ="Roll off " + ofToString((float)instantaneousRollOff*50.0f);
    ofDrawBitmapString(msgRollOff, 80, ofGetHeight() - 15);
    
    string msgAlpha = "Alpha " + ofToString((float) alpha2);
    ofDrawBitmapString(msgAlpha, 250, ofGetHeight() - 5);
    
    string msgPitch ="Pitch " + ofToString((float)instantaneousPitch) + " Hz";
    ofDrawBitmapString(msgPitch, 400, ofGetHeight() - 5);
    
    //Max pitch chroma
    float maxPitch = 0;
    for (int i =0; i<pitchChroma.size(); i++) {
        if (maxPitch < pitchChroma[i]) {
            maxPitch = pitchChroma[i];
        }
    }
    
    string msgMaxPitchChroma ="Max Pitch Chroma " + ofToString((float)maxPitch);
    ofDrawBitmapString(msgMaxPitchChroma, 550, ofGetHeight() - 5);
    
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
    
    float maxValue = 0;
    
    for(int i = 0; i < windowBufferSize; i++) {
        if(abs(window[i]) > maxValue) {
            maxValue = abs(window[i]);
        }
        //Store the input in the leftInput
        leftWithOverlap[i] = window[i];
    }
    
    for(int i = 0; i < windowBufferSize; i++) {
        window[i] /= maxValue;
    }
    
    fft->setSignal(window);
    
    float* curFft = fft->getAmplitude();
    //    memcpy(&audioBins[0], curFft, sizeof(float) * fft->getBinSize());
    copy(curFft, curFft + fft->getBinSize(), audioBins.begin());
    
    
    maxValue = 0;
    for(int i = 0; i < fft->getBinSize(); i++) {
        if(abs(audioBins[i]) > maxValue) {
            maxValue = abs(audioBins[i]);
        }
    }
    for(int i = 0; i < fft->getBinSize(); i++) {
        audioBins[i] /= maxValue;
    }
    
    fftData = audioBins;
    
    //Calculate Spectral flux
    if(soundStream.getTickCount() > 1){ //handle this before sending FFT to myFeatures.
        for(int i = 0; i < fft->getBinSize(); i++) {
            spectralFlux[i] = ((middleBins[i]-audioBins[i])*(middleBins[i]-audioBins[i]));
        }
    }
    
    //Calculate Spectral Roll off
    
    if(soundStream.getTickCount() > 1){
        for(int i = 0; i < fft->getBinSize(); i++) {
            spectralRollOff[i] = abs(audioBins[i]);
            //------ temp
            if(tempMax < spectralRollOff[i]) {
                tempMax = spectralRollOff[i];
            }
            //------ temp
        }
    }
    
    soundMutex.lock();
    middleBins = audioBins;
    soundMutex.unlock();
    
}


//---------

void ofApp::audioReceived(float* input, int bufferSize, int nChannels) {
    
    float* inputLeft = input;
    float* inputRight;
    
    if (nChannels == 2) {
       inputRight = input + bufferSize; //not used for now
    }
    
    rms = 0;
    if(soundStream.getTickCount() > 1) {
        for(int i = 0; i < bufferSize; i++) {
            rms = rms + input[i]*input[i];
        }
        rms = sqrt(rms/bufferSize);
        if (rms != 0) { // check for silence
            
            //Use circular mapping of pointer to improve performance
            if (soundStream.getTickCount() > overlapMultiple-1) {
                processBlock(block,bufferSize*overlapMultiple,nChannels);
                //Shift block data to left
                copy(block+bufferSize, block+bufferSize*overlapMultiple, &block[0]);
                //Get the last hop data into the block
                copy(inputLeft, inputLeft + bufferSize, &block[bufferSize*(overlapMultiple-1)]);
            }
            else {
                copy(inputLeft, inputLeft + bufferSize, &block[numHops*bufferSize]);
                numHops++;
            }
            
            //Store the input in the leftInput
            for(int i = 0; i < bufferSize; i++) {
                leftInput[i] = input[i];
            }
            
            //Store the input in the rightInput if stereo -- not being used for now
            if (nChannels==2) {
                for(int i = bufferSize; i < 2*bufferSize; i++) {
                    rightInput[i] = input[i];
                }
            }
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
    if (key == 'd') {
        micInput = ! micInput;
        changeDeviceId();
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
