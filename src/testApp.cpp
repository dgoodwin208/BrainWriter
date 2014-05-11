#include "testApp.h"

int last_x;
int last_y;
float speed;
float angle;



//--------------------------------------------------------------
//----------------------  testApp  -----------------------------
//--------------------------------------------------------------
void testApp::setup(){
	
    
    image.loadImage("bg.jpg");
    
    ofSetFrameRate( 60 );	//Set screen frame rate
    
    last_x = ofGetWidth() / 2;
    last_y = ofGetHeight() / 2;
    
    //Initialize the first letter
    letters.push_back(DrawnLetter());
    
    hoverTime = 3;
    startHoverTime = 0;
    elapsedTime = 0;
    isHoveringOverPoint = false;
    
    //param.setup();		//Global parameters
}

//--------------------------------------------------------------
void testApp::update(){
    //Distance between the last clicked point and the first click
    
    if (newClick)
    {
        bool letterCompleted = letters.back().handleClick(lastClick);
        
        //If the letter is completed, move to the next one!
        if (letterCompleted) {
            letters.push_back(DrawnLetter());
        }
        
        newClick = false;
        //Is the mouse hovering over the last clicked point?
        //Take the distance between the last click and the last seen mousePosition
        /*
         float distance = distance2D(clicks.back(), posft.back());
         if (!justClicked){
         if (distance < PROXIMITY_THRESHOLD)
         isHoveringOverPoint = true;
         else if (isHoveringOverPoint and distance < 5*PROXIMITY_THRESHOLD)
         isHoveringOverPoint = true;
         else{
         isHoveringOverPoint = false;
         elapsedTime = 0;
         }
         }
         else
         {
         if (distance > PROXIMITY_THRESHOLD)
         justClicked = false;
         }
         */
    }
}

void testApp::inferClicks(){
    
    //If there are fewer than N clicks, then ignore it
    if (posft.size()<5)
        return;
    
    int x = 0;
    int y = 0;
    
    float ave_x; float ave_y;
    for (int i = 0; i<posft.size(); ++i) {
        ave_x += posft[i].x;
        ave_y += posft[i].y;
    }
    ave_x = ave_x*1.0/posft.size();
    ave_y = ave_y*1.0/posft.size();
    
    //StdDeviation
    
    mousePressed(x,y,0);
    
}
//--------------------------------------------------------------
void testApp::draw(){
    
	ofBackground( 255, 255, 255 );  //Set white background
    
    
    //Draw semi-transparent white rectangle
	//to slightly clearing a buffer (depends on history value)
	ofEnableAlphaBlending();         //Enable transparency
    
	//float alpha = (1-history) * 255;
	ofSetColor( 255, 255, 255, 255 );
	ofFill();
	ofRect( 0, 0, ofGetWidth(), ofGetHeight() );
    
	ofDisableAlphaBlending();        //Disable transparency
    image.draw(0,0);
    
    for (int i=0; i<letters.size(); i++) {
        letters[i].draw(last_x, last_y);
    }
    
    
    //Vizualize only the last click point
    if (posft.size()>0 && letters.back().bendIdx==-1)
    {
        letters.back().showSampleLine(posft.back());
    }
    
    
    
    //Draw the hover circle
    //Draw the back (50% opacity)
    /*
     if(isHoveringOverPoint){
     ofPushMatrix();
     ofTranslate(clicks.back().x, clicks.back().y);
     ofSetColor(255,0,255);
     for(int i=0;i<365;i++){
     ofRotate(1);
     if(i<elapsedTime+1){
     ofRect(0,0,10*5,1);
     }
     else
     break;
     }
     elapsedTime+=4;
     ofPopMatrix();
     }
     */
    
    
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    speed = sqrt((x - last_x)*(x - last_x) - (y - last_y)*(y - last_y));
    if (x<last_x || y<last_y)
        speed = speed*-1;
    angle = atan((y - last_y)/((x - last_x)*1.0));
    
    //cout << "Last angle = " << angle << " \n";
    
    last_x = x;
    last_y = y;
    
    //Update the posft vector
    posft.push_back(ofPoint(x,y));
    //cout << "Last speed = " << last_x << " \n";
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    lastClick = ofPoint(x,y);
    //    clicks.push_back();
    newClick = true;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}