#include "Sequence.h"

Sequence::Sequence(){
    filename = "";
    duration = 0;
    numFrames = 0;
    playhead = 0;
    elapsedTime = 0;
    maxPatternsWindow = 12;
}

void Sequence::setup(const int maxMarkers){
    this->maxMarkers = maxMarkers;
    verdana.loadFont("fonts/verdana.ttf", 80, true, true);
}

void Sequence::update(){
    updatePlayhead();
}

void Sequence::record(const vector<irMarker>& markers){
    int frameNum = xml.addTag("frame");
    xml.pushTag("frame", frameNum);
    xml.setValue("timestamp", ofGetElapsedTimef(), frameNum);
    for(int markerIndex = 0; markerIndex < markers.size(); markerIndex++){
        int numMarker = xml.addTag("marker");
        xml.pushTag("marker", numMarker);
        xml.setValue("id", ofToString(markers[markerIndex].getLabel()), numMarker);
        xml.setValue("x",  markers[markerIndex].smoothPos.x, numMarker);
        xml.setValue("y",  markers[markerIndex].smoothPos.y, numMarker);
        xml.setValue("disappeared", markers[markerIndex].hasDisappeared, numMarker);
        xml.popTag();
    }
//    // fill with dummy markers if not enough markers
//    while(xml.getNumTags("marker") < maxMarkers){
//        int numMarker = xml.addTag("marker");
//        xml.pushTag("marker", numMarker);
//        xml.setValue("id", -1, numMarker);
//        xml.setValue("x",  -1, numMarker);
//        xml.setValue("y",  -1, numMarker);
//        xml.setValue("disappeared", 1, numMarker);
//        xml.popTag();
//    }
    xml.popTag();
}

void Sequence::draw(){

    // Draw entire sequence
    for(int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){

//        ofSetColor(255, 40);
//        markersPosition[markerIndex].draw();

        ofPoint currentPoint = getCurrentPoint(markerIndex);
        markersPastPoints[markerIndex].addVertex(currentPoint);

        if(markerIndex == 0) ofSetColor(255, 0, 0);
        if(markerIndex == 1) ofSetColor(0, 0, 255);
        ofSetLineWidth(1.2);
        markersPastPoints[markerIndex].draw();

        ofFill();
        ofCircle(currentPoint, 3);
    }
}

ofPoint Sequence::getCurrentPoint(int markerIndex){
    return markersPosition[markerIndex].getPointAtIndexInterpolated(calcCurrentFrameIndex());
}

void Sequence::load(const string path){

    if(!xml.load(path)) return;

    // Initialize polylines sequence
    markersPosition.clear();
    for(int i = 0; i < maxMarkers; i++){
        ofPolyline newPolyline;
        markersPosition.push_back(newPolyline);
    }

    // Initialize previous points polylines sequence
    markersPastPoints.clear();
    for(int i = 0; i < maxMarkers; i++){
        ofPolyline newPolyline;
        markersPastPoints.push_back(newPolyline);
    }

    // Number of frames of the sequence
    numFrames = xml.getNumTags("frame");
    float timestampFirstFrame = -1;
    float timestampLastFrame = -1;
    size_t emptyFrames = 0;

    // Load XML sequence in memory
    for(size_t frameIndex = 0; frameIndex < numFrames; frameIndex++){
        xml.pushTag("frame", frameIndex);

        const size_t nMarkers = xml.getNumTags("marker");

        // Frame timestamp
        if(timestampFirstFrame == -1 && nMarkers >= maxMarkers) timestampFirstFrame = xml.getValue("timestamp", -2.0);
        if(nMarkers >= maxMarkers) timestampLastFrame = xml.getValue("timestamp", -2.0);

        // If no recorded markers or less than maxMarkers we don't consider that frame
        if(nMarkers < maxMarkers){
            emptyFrames++;
            xml.popTag();
            continue;
        }

        int addedMarkers = 0;
        for(size_t markerIndex = 0; markerIndex < nMarkers; markerIndex++){
            xml.pushTag("marker", markerIndex);
            unsigned int id = xml.getValue("id", -2.0);
            const float px = xml.getValue("x", -2.0);
            const float py = xml.getValue("y", -2.0);
            const bool disappeared = xml.getValue("disappeared", -1.0);

            // Number of markers we still have to take the value from
            int aheadMarkers = (nMarkers-1) - markerIndex;
            // If marker is not disappeared in that frame we add it
            if(!disappeared){
                markersPosition[addedMarkers].addVertex(ofPoint(px, py));
                addedMarkers++;
            }
            // If marker is disappeared but not enough markers to fill maxMarkers we add it too
            else if(disappeared && aheadMarkers < (maxMarkers-addedMarkers)){
                markersPosition[addedMarkers].addVertex(ofPoint(px, py));
                addedMarkers++;
            }
            // If marker has disappeared but we have other markers in the list we don't add it
            else{}

            xml.popTag();

            // If we have already loaded more or equal maxMarkers we go to the next frame
            if(addedMarkers >= maxMarkers ){
                break;
            }
        }

//        // If not enough markers in the frame to fill maxMarkers we add dummy vertices to the polyline
//        while(addedMarkers < maxMarkers){
//            markersPosition[addedMarkers].addVertex(ofPoint(-1, -1));
//            addedMarkers++;
//        }

        xml.popTag();
    }

    filename = ofFilePath::getBaseName(path);

//    // Create n equal length patterns for debug
//    createPatterns(6);

    // Number of frames of the sequence
    numFrames = numFrames - emptyFrames;

    // Duration in seconds of the sequence
    duration = timestampLastFrame - timestampFirstFrame;
}

void Sequence::drawPatterns(map<int, float> currentPatterns){
    // If there are more patterns than maximum able to show in the window
    int nPatterns = MIN(maxPatternsWindow, patterns.size());

    // TODO: show most important patterns if there are more than maxPatternsToShow

    // Draw gesture patterns
    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
        int patternPosition = patternIndex + 1;
        bool highlight = false;
        float percent = 0;
        // If the pattern is is inside the map
        if(currentPatterns.find(patternIndex) != currentPatterns.end()){
            highlight = true;
            percent = currentPatterns[patternIndex];
        }
//        else{ // Either here or inside drawPattern
//            // If is not inside the map we clear the polyline
//            for(int markerIndex = 0; markerIndex < patterns[patternIndex].size(); markerIndex++){
//                patternsPastPoints[patternIndex][markerIndex].clear();
//            }
//        }
        drawPattern(patternPosition, patternIndex, percent, highlight);
    }
}

void Sequence::drawPattern(const int patternPosition, const int patternIndex, const float percent, const bool highlight){

    // Drawing window parameters
    float width = 640.0;
    float height = 480.0;
    float scale = 5.5;
    float margin = 40.0;
    float guiHeight = 800;

    ofPushMatrix();

        ofScale(1.0/scale, 1.0/scale);
        ofTranslate(0, guiHeight);

        // Position window pattern in the screen
        if(patternPosition%2 == 0){
            ofTranslate(width+margin, (patternPosition/2 - 1) * (height+margin));
        }
        else{
            ofTranslate(0, ((patternPosition+1)/2 - 1) * (height+margin));
        }

        int opacity = 60;
        if(highlight) opacity = 255;
        else          opacity = 60;

        // Background pattern window box
        ofSetColor(0);
        ofFill();
        ofRect(0, 0, width, height);

        // Contour pattern window box
        ofSetColor(255, opacity);
        ofSetLineWidth(2);
        ofNoFill();
        ofRect(0, 0, width, height);

        for(int markerIndex = 0; markerIndex < patterns[patternIndex].size(); markerIndex++){
            // Pattern lines
            ofSetColor(120, opacity);
            ofSetLineWidth(2);
            patterns[patternIndex][markerIndex].draw();

            if(highlight){
                ofPoint currentPoint = patterns[patternIndex][markerIndex].getPointAtPercent(percent);
                patternsPastPoints[patternIndex][markerIndex].addVertex(currentPoint);

                // Pattern already processed lines
                ofSetColor(255);
                ofSetLineWidth(2);
                patternsPastPoints[patternIndex][markerIndex].draw();

                // Pattern current processing point
                ofSetColor(240, 0, 20, opacity);
                ofCircle(currentPoint, 10);
            }
            else{
                patternsPastPoints[patternIndex][markerIndex].clear();
            }
        }

        // Pattern label number
        ofSetColor(255, opacity+30);
        verdana.drawString(ofToString(patternIndex+1), 30, 100);

    ofPopMatrix();
}

void Sequence::loadPatterns(vector< vector<ofPolyline> > patterns){
    this->patterns = patterns;

    // Clear and initialize memory of previous points polylines patterns
    patternsPastPoints.clear();
    for(int patternIndex = 0; patternIndex < patterns.size(); patternIndex++){
        vector<ofPolyline> newPattern;
        for(int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){
            ofPolyline newPolyline;
            newPattern.push_back(newPolyline);
        }
        patternsPastPoints.push_back(newPattern);
    }
}

void Sequence::createPatterns(int nPatterns){
    // Clear and initialize memory of polylines patterns
    patterns.clear();
    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
        vector<ofPolyline> newPattern;
        for(int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){
            ofPolyline newPolyline;
            newPattern.push_back(newPolyline);
        }
        patterns.push_back(newPattern);
    }

    // Clear and initialize memory of previous points polylines patterns
    patternsPastPoints.clear();
    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
        vector<ofPolyline> newPattern;
        for(int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){
            ofPolyline newPolyline;
            newPattern.push_back(newPolyline);
        }
        patternsPastPoints.push_back(newPattern);
    }

    // Break sequence in n patterns
    for(int patternIndex = 0; patternIndex < nPatterns; patternIndex++){
        for(int markerIndex = 0; markerIndex < maxMarkers; markerIndex++){
            int startIndex = markersPosition[markerIndex].getIndexAtPercent(patternIndex * (1.01/nPatterns));
            int endIndex = markersPosition[markerIndex].getIndexAtPercent((patternIndex+1) * (1.01/nPatterns))+1;
            if (endIndex == 1) endIndex = markersPosition[markerIndex].size();
            for(int i = startIndex; i < endIndex; i++){
                patterns[patternIndex][markerIndex].addVertex(markersPosition[markerIndex][i]);
            }
        }
    }
}

void Sequence::save(const string path) {
    xml.saveFile(path);
}

void Sequence::startRecording(){
    xml.clear();
}

void Sequence::clearPlayback(){
    // Clear and Initialize previous points polylines sequence
    markersPastPoints.clear();
    for(int i = 0; i < maxMarkers; i++){
        ofPolyline newPolyline;
        markersPastPoints.push_back(newPolyline);
    }
    playhead = 0;
    elapsedTime = 0;
}

void Sequence::updatePlayhead()
{
    elapsedTime += ofGetLastFrameTime();

    if(elapsedTime > duration){
        clearPlayback();
    }

    playhead = (elapsedTime / duration);
}

size_t Sequence::calcCurrentFrameIndex()
{
    size_t frameIndex = floor(numFrames * playhead);

    if(frameIndex >= numFrames) frameIndex = numFrames-1;

    return frameIndex;
}

