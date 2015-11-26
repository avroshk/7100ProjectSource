#include "ofApp.h"
#include <fstream>
#include <iostream>




//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    plotHeight = 128;
    
//    const char* testoutput_fn= "/Users/avrosh/Documents/of_v0.8.4_osx_release/apps/myApps/testFFT/bin/data/features.csv";
//    myfile.open(testoutput_fn);
//    myfile.clear();
    
#ifdef TEST
    /* write results into a csv file */
    const char*  testoutput_fn = "/Users/avrosh/Documents/of_v0.8.4_osx_release/apps/myApps/testFFT/bin/data/features.csv";
    const char*  audiosample_fn = "/Users/avrosh/Documents/of_v0.8.4_osx_release/apps/myApps/testFFT/bin/data/funfair.wav";
    
    myfile.open (testoutput_fn);
    myfile.clear();
    myAudioFile = SndfileHandle(audiosample_fn);
    
    SAMPLERATE = myAudioFile.samplerate();
    INPUTS = myAudioFile.channels();
    
    // -- for audio streamed from file -- // end //
#else
    soundStream.listDevices(); //uncomment to print list of available devices
    soundStream.setDeviceID(DEVICEID);
    soundStream.setup(this, OUTPUTS, INPUTS, SAMPLERATE, BUFFERSIZE, NBUFFERS);
#endif
    
    leftInput.resize(BUFFERSIZE*OVERLAPMULTIPLE);//size of one hop
    drawInput.resize(BUFFERSIZE*OVERLAPMULTIPLE, 0.0);
    middleInput.resize(BUFFERSIZE*OVERLAPMULTIPLE,0.0);
    
    block = new float[BUFFERSIZE*OVERLAPMULTIPLE];
    numHops = 0;
    tempMax = 0;
    
    //Stereo
    if(INPUTS == 2) {
        rightInput.assign(BUFFERSIZE, 0.0); //size of one hop
        downMixedInput.assign(BUFFERSIZE, 0.0); //size of one hop
        buffer = new float[BUFFERSIZE*2];
    }
    //Mono
    else if(INPUTS == 2) {
        buffer = new float[BUFFERSIZE];
    }
    
    // ------ Features setup -------
    
    features = new myFeatures(SAMPLERATE,BUFFERSIZE*OVERLAPMULTIPLE);
    features->LCRFluxThreshold = 50;
    features->instantaneousFluxThreshold = 0.0005;
    
    drawBins.resize(features->getFftSize());
    middleBins.resize(features->getFftSize());
    audioBins.resize(features->getFftSize());
    pitchChroma.resize(12);
    normalizedInput.resize(BUFFERSIZE);
    
    // ------ Feature to Effect Mapping setup -----

    featureMap = new myMappingVector(features->getNumOfFeatures(),2);
    featureMap->routeFeature(1, 0, 0);
    featureMap->routeFeature(1, 1, 1);
    featureMap->routeFeature(0.8, 3, 0);
    
    // ------ Image setup -----------
    
    /*create a offset with the background image by sending different image dimensions here */
    effects = new myEffects(IMGFILE,WIDTH,HEIGHT);
    
    myImage = effects->getImage();
    img1 = *myImage;
    img1.crop(0, 0, myImage->getWidth()/2, myImage->getHeight()/2);
    img1.resize(img1.getWidth()*2, img1.getHeight()*2);
    img2 = *myImage;
    img2.crop(myImage->getWidth()/2, 0, myImage->getWidth()/2, myImage->getHeight()/2);
    img2.resize(img2.getWidth()*2, img2.getHeight()*2);
    img3 = *myImage;
    img3.crop(0, myImage->getHeight()/2, myImage->getWidth()/2, myImage->getHeight()/2);
    img3.resize(img3.getWidth()*2, img3.getHeight()*2);
    img4 = *myImage;
    img4.crop(myImage->getWidth()/2, myImage->getHeight()/2, myImage->getWidth()/2, myImage->getHeight()/2);
    img4.resize(img4.getWidth()*2, img4.getHeight()*2);
    
    // ------ Graphics setup -----
    ofBackground(0, 0, 0);
    fbo.allocate(WIDTH, HEIGHT);
    // clear fbo
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
}

//--------------------------------------------------------------
void ofApp::update() {
    
#ifdef TEST
    //Stereo
    if(INPUTS==2) {
        myAudioFile.read (buffer, BUFFERSIZE*2);
        audioReceived(buffer, BUFFERSIZE*2, INPUTS);
    }
    //Mono
    else if(INPUTS==1) {
        myAudioFile.read (buffer, BUFFERSIZE);
        audioReceived(buffer, BUFFERSIZE, INPUTS);
    }
#endif
    
    // ------ Update features -------
    float feature1 = featureMap->getFeatureForEffect(0, features->getNormalizedFeatureSet());
    float feature2 = featureMap->getFeatureForEffect(1, features->getNormalizedFeatureSet());
    
    effects->applyNoiseToMesh(ofMap(features->getSpectralRollOff(0.5),0,1,0,0.05),ofMap(features->getSpectralFlux(0.5),0,0.1,0,0.05),ofMap(features->getSpectralRollOff(0.5), 0, 0.5, 0.05, 0.20));
    
    effects->applyJitterToMesh(features->getPitchChromaCrestFactor());
    
    // ------ Update mesh -------
    meshGrid = effects->getMeshGrid();
    mesh1 = effects->getMesh();
    
    if (features->spectralFluxLevelCrossingRateChanged()) {
        effects->refreshMesh();
    }
    
    if(features->getMostNotableOnsets()) {
        tempMax++;
        if (tempMax>10) {
            flipBool = !flipBool;
            tempMax = 0;
        }
    }
    
    if(features->getSpectralDecrease() < -0.1) {
        bool1 = !bool1;
    }
    
    if(features->getSpectralRollOff(0.5) > 0.1) {
        bool2 = !bool2;
    }
    
    if(features->getSpectralFlatness() > 0.02) {
        bool3 = !bool3;
    }
    if(features->getPitchChromaCrestFactor() > 0.02) {
        bool4 = !bool4;
    }
    
    
//     if(features->getMostNotableOnsets()) { tempMax++;}
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    
    // effects :--------------------------------------------------
    
    //    float alpha = ofMap(exp(instantaneousFlux*50.0f), 1, 50, 180, 255);
//    float alpha = ofMap(*features->getSpectralFlux(0.5)*30.0f, 0.5, 1, 180, 255);
         float alpha = ofMap(features->getSpectralDecrease(), 0, 0.5, 180, 255); // good feature mapping
    //    float alpha = ofMap(features->getSpectralFlatness(), 0, 0.5, 180, 255); //no bad
    //     float alpha = ofMap(features->getPitchChromaFlatness(), 0, 0.5, 180, 255); // good feature (kinetoscope effect)
//    float alpha = ofMap(features->getSpectralFluxLog(0.5), 0, 1, 180, 255);
    float alpha2 = ofMap(features->getSpectralRollOff(0.5)*10.0f, 0, 1, 180, 255);
    //    float alpha = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
    
   
    
    //----------------------------------------------------------
    
    if (drawBool) {
        ofSetColor(255);
        ofPushMatrix();
        ofTranslate(16, 16);
        
        soundMutex.lock();
        drawBins = middleBins;
        drawInput = middleInput;
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
        for (unsigned int i = 0; i < drawInput.size(); i++){
            ofVertex(2*i, 100 -drawInput[i]*180.0f);
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
        string pitches[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        pitchChroma = features->getPitchChroma();
        for(int i =0; i<12;i++){
            ofRect(0+40*i,0,40, pitchChroma[i]*200);
            ofDrawBitmapString(pitches[i], 0+40*i,-4);
        }
        
        ofPopMatrix();
        ofPopStyle();
        
        
        //string msgFlux = "Flux " + ofToString((float)features->getSpectralCentroid()*50.0f);
        string msgFlux = "Flux " + ofToString((float)features->getSpectralFlux(0.5));
        ofDrawBitmapString(msgFlux, 80, ofGetHeight() - 5);
        
        string msgRollOff ="Roll off " + ofToString((float)features->getSpectralRollOff(0.5));
        ofDrawBitmapString(msgRollOff, 80, ofGetHeight() - 15);
        
        string msgAlpha = "Alpha " + ofToString((float) alpha2);
        ofDrawBitmapString(msgAlpha, 250, ofGetHeight() - 5);
        
        string msgPitch ="Pitch " + ofToString((float)features->getPitch()) + " Hz";
        ofDrawBitmapString(msgPitch, 400, ofGetHeight() - 5);
        
        string msgTemp ="Temp " + ofToString((float)tempMax);
        ofDrawBitmapString(msgTemp, 550, ofGetHeight() - 45);
        
    }

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
            ofColor c = myImage->getColor(pos.x,pos.y);
            //        float intensity = c.getLightness();
            
            c.setBrightness(alpha);
            
            myImage->setColor(pos.x, pos.y, c);
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
            if (bool1) {
                img1.draw(0,0);
            }
            else if (bool2) {
                img2.draw(0,0);
            }
            else if (bool3) {
                img3.draw(0,0);
            }
            else if (bool4) {
                img4.draw(0,0);
            }
            else {
                myImage->draw(0,0);
            }
    }
    
    effects->applyAlphaToTexture(alpha2, alpha);
    
    if (flipBool) {
        myImage->bind();
    }
    else {
        effects->getMirrorImage()->bind();
    }
    
    
    ofPushMatrix(); //Store the coordinate system
    ofTranslate( ofGetWidth()/2, ofGetHeight()/2, 0 );  //Move the coordinate center to screen's center
//    meshGrid.drawWireframe();
    meshGrid->draw();
//    mesh1->draw();
    ofPopMatrix(); //Restore the coordinate system
    
//    mesh.draw();
    if (flipBool) {
        myImage->unbind();
    }
    else {
        effects->getMirrorImage()->unbind();
    }
    
    
    fbo.end();
    
    fbo.draw(0,0);
}
//-----------------------------------------------

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

void ofApp::audioReceived(float* input, int bufferSize, int nChannels) {
    //Stereo
    if (nChannels == 2) {
        
        //We may need two different functions for downmixing interleaved and non-interleaved audio
        
        /* sndfile-> returns interleaved stereo */
        downMixedInput = downMixAudio(input, input+1, bufferSize);

        /* ofSoundStream returns non-interleaved stereo by default*/
//        downMixedInput = downMixAudio(input, input+bufferSize, bufferSize);
        blockAndProcessAudioData(&downMixedInput[0],BUFFERSIZE,nChannels);
    }
    //Mono
    else if(nChannels == 1) {
        blockAndProcessAudioData(input,BUFFERSIZE,nChannels);
    }
   
}
//--------------------------------------------------------------
//should ideally downmix the audio in myFeatures

vector<float> ofApp::downMixAudio(float* inputLeft, float* inputRight, int bufferSize) {

    downMixed.resize(BUFFERSIZE);
    
    for(int i = 0; i < bufferSize; i=i+2) {
//        leftInput[i/2] = inputLeft[i];
//        rightInput[i/2] = inputRight[i];
        
        downMixed[i/2] = (inputLeft[i]+inputRight[i])*0.5;
    }
    return  downMixed;
}
//--------------------------------------------------------------

void ofApp::blockAndProcessAudioData(float *input, int bufferSize, int nChannels) {
    
    //task : Use circular mapping to pointer to improve performance
    
    if (numHops > OVERLAPMULTIPLE-1) {
        processBlock(block,BUFFERSIZE*OVERLAPMULTIPLE,nChannels);
        //Shift block data to left
        copy(block+BUFFERSIZE, block+BUFFERSIZE*OVERLAPMULTIPLE, &block[0]);
        //Push the last hop into the block
        copy(input, input + BUFFERSIZE, &block[BUFFERSIZE*(OVERLAPMULTIPLE-1)]);
    }
    else {
        copy(input, input + BUFFERSIZE, &block[NUMHOPS*BUFFERSIZE]);
        numHops++;
    }

}

//-----------------------------------------------

void ofApp::processBlock(float* window, int windowBufferSize, int nChannels){
    
    for (int i=0; i<windowBufferSize; i++) {
        leftInput[i] = window[i];
    }
    
    soundMutex.lock();
    middleInput = leftInput;
    soundMutex.unlock();
    
    features->extractFeatures(window,nChannels);
    
    audioBins = features->getNormalizedFftData();
    
    soundMutex.lock();
    middleBins = audioBins;
    soundMutex.unlock();

#ifdef TEST
    testFeatures();
#endif
    
}

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
        flipBool = !flipBool;
    }
    if (key == '1') {
        bool1 = !bool1;
    }
    if (key == '2') {
        bool2 = !bool2;
    }
    if (key == '3') {
        bool3 = !bool3;
    }
    if (key == '4') {
        bool4 = !bool4;
    }
    if (key == '5') {
        boolAll = !boolAll;
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

#ifdef TEST
//--------------------------------------------------------------
void ofApp::testFeatures() {
    
    if (myfile.is_open()) {

//        //Test Input
//        soundMutex.lock();
//        drawInput = middleInput;
//        soundMutex.unlock();
//
//        for(int i=0;i<BUFFERSIZE*OVERLAPMULTIPLE;i++) {
//            if (i==BUFFERSIZE*OVERLAPMULTIPLE-1) {
//                myfile << drawInput[i]<<"\n";
//            }
//            else {
//                myfile << drawInput[i]<<",";
//            }
//        }
//        
//        //Test Fft
//        for(int i=0;i<513;i++) {
//            if (i==512) {
//                myfile << middleBins[i]<<"\n";
//            }
//            else {
//                myfile << middleBins[i]<<",";
//            }
//        }

         //Test Features
//        pitchChroma = features->getPitchChroma();
//        
//        myfile << features->getSpectralCentroid()<<","
//        << features->getSpectralDecrease()<<","
//        << features->getSpectralFlatness()<<","
//        << features->getSpectralFlux(0)<<","
//        << features->getSpectralRollOff(0)<<","
//        << features->getSpectralSpread()<<","
//        << pitchChroma[0]<<","
//        << pitchChroma[1]<<","
//        << pitchChroma[2]<<","
//        << pitchChroma[3]<<","
//        << pitchChroma[4]<<","
//        << pitchChroma[5]<<","
//        << pitchChroma[6]<<","
//        << pitchChroma[7]<<","
//        << pitchChroma[8]<<","
//        << pitchChroma[9]<<","
//        << pitchChroma[10]<<","
//        << pitchChroma[11]<<"\n";
        
        bool todo = features->getMostNotableOnsets();
        if (todo) {
            tempMax++;
        }
        
        myfile << features->getSpectralFlux(0.0)<<","
                <<features->getAdaptiveThreshold()<<","
                <<todo<<"\n";

    }
}

#endif


