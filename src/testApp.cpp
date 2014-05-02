#include "testApp.h"

#define PROXIMITY_THRESHOLD 20.0


float distance2D(ofPoint a, ofPoint b)
{
    float delta_x = a.x - b.x;
    float delta_y = a.y - b.y;
    return sqrt(delta_x*delta_x + delta_y*delta_y);
}
//--------------------------------------------------------------
//----------------------  Params -------------------------------
//--------------------------------------------------------------
Params param;        //Definition of global variable
int last_x;
int last_y;
float speed;
float angle;
void Params::setup() {
	eCenter = ofPoint( ofGetWidth() / 2, ofGetHeight() / 2 );
    //eCenter = ofPoint(last_x, last_y);
	eRad = 10;//originally 100
	velRad = 0;
	lifeTime = 2000.0; //originally 2.0
    
	rotate = 0;
	force = 1000;
	spinning = 1000;
	friction = 0.5; //originally .05
}

//--------------------------------------------------------------
//-------------------  DRAWN LETTER  ---------------------------
//--------------------------------------------------------------
DrawnLetter:: DrawnLetter(){
    setup();
}

void DrawnLetter::setup()
{
    isComplete = false;
    
    //Allocate drawing buffer
	int w = ofGetWidth();
	int h = ofGetHeight();
	fbo.allocate( w, h, GL_RGB32F_ARB );
    
	//Fill buffer with white color
	fbo.begin();
	ofBackground(255, 255, 255);
	fbo.end();
    
	//Set up parameters
	//
	history = 0.0;//0.9;
	bornRate = 500;
    
	bornCount = 0;
	time0 = ofGetElapsedTimef();
    
    bendIdx = -1;
}

void DrawnLetter::draw()
{
    //First draw any particles, then shapes and lines
    
    
    //1. Drawing to buffer
    /*
     fbo.begin();
     
     //Draw the particles
     ofFill();
     for (int i=0; i<p.size(); i++) {
     p[i].draw();
     }
     for( int i = 0; i < 5; i++) {
     path.arc( i * 50 + 20, i * 50 + 20, i * 40 + 10, i * 40 + 10, 0, 360); // creates a new ofSubPath
     path.close();
     }
     fbo.end();
     
     //2. Draw buffer on the screen
     ofSetColor( 255, 255, 255 );
     fbo.draw( 0, 0 );
     */
    
    if (isComplete){
        
        ofSetColor(0,0,255);
        ofFill();
        
        //Show all the lines assocated with a shape!
        ofBeginShape();
        for (int i=0; i<lines.size(); i++) {
            ofSetColor(0,255,0);
            //lines[i].draw();
            ofPolyline poly = lines[i];
            for( int j = 0; j < poly.getVertices().size(); j++) {
                ofVertex(poly.getVertices().at(j).x, poly.getVertices().at(j).y);
            }
            
        }
        ofEndShape();
        
    }
    
    //If the shape is in the works, show the vertices
    else{//if (!isComplete){
        ofSetColor(255,0,0);
        if (verts.size()>0){
            ofCircle(verts[verts.size()-1].x, verts[verts.size()-1].y, 10);
            
            //Mark the first point of the shape
            ofSetColor(0, 255, 255,125);
            ofCircle(verts[0].x, verts[0].y, 10.0);
        }
        
        //Mark all the other click
        ofSetColor(0, 0, 255,125);
        for (int i=1; i<verts.size(); i++) {
            ofCircle(verts[i].x, verts[i].y, 10.0);
        }
    }
    
    //Show all the lines assocated with a shape!
    for (int i=0; i<lines.size(); i++) {
        ofSetColor(255,0,0);
        if (i == bendIdx)
        {
            makeBend(bendIdx,ofPoint(last_x,last_y));
        }
        lines[i].draw();
	}
    
    /*
     if (isHoveringOverPoint)
     ofCircle(clicks.back().x, clicks.back().y, 10.0*5);
     
     if (clicks.size()>0 && posft.size()>0)
     
     */
}
void DrawnLetter::showSampleLine(ofPoint mousePos)
{
    if (verts.size())
        ofLine(verts.back().x, verts.back().y, mousePos.x, mousePos.y);
}

void DrawnLetter::makeBend(int lineIdx, ofPoint mousePos){
    int line1Z[7] = {0, 0, 0, 0, 0, 0, 0};
    
    ofPolyline lineWithDuplicate;
    ofSetColor(128, 128, 128);
    
    lineWithDuplicate.curveTo(verts[lineIdx].x, verts[lineIdx].y, line1Z[0]);
    lineWithDuplicate.curveTo(verts[lineIdx].x, verts[lineIdx].y, line1Z[0]);
    
    lineWithDuplicate.curveTo(mousePos.x, mousePos.y, line1Z[0]);
    lineWithDuplicate.curveTo(verts[lineIdx+1].x, verts[lineIdx+1].y, line1Z[1]);
    lineWithDuplicate.curveTo(verts[lineIdx+1].x, verts[lineIdx+1].y, line1Z[1]);
    lines[lineIdx] = lineWithDuplicate;
    //    lineWithDuplicate.draw();
}

void DrawnLetter::update()
{
    //Compute dt
	float time = ofGetElapsedTimef();
	float dt = ofClamp( time - time0, 0, 0.1 );
	time0 = time;
    
	//Delete inactive particles
	/*
     int i=0;
     while (i < p.size()) {
     if ( !p[i].live ) {
     p.erase( p.begin() + i );
     }
     else {
     i++;
     }
     }
     */
    
	//Born new particles
	bornCount += dt * bornRate;      //Update bornCount value
	if ( bornCount >= 1 ) {          //It's time to born particle(s)
		int bornN = int( bornCount );//How many born
		bornCount -= bornN;          //Correct bornCount value
		for (int i=0; i<bornN; i++) {
			Particle newP;
			newP.setup();            //Start a new particle
			p.push_back( newP );     //Add this particle to array
		}
	}
    
	//Update the particles
	for (int i=0; i<p.size(); i++) {
		p[i].update( dt );
	}
    
}
//Create a line that goes from a to b, possibly curving through c
ofPolyline DrawnLetter::createLineSegment(ofPoint a, ofPoint b, ofPoint mid)
{
    ofPolyline lineSeg;
    lineSeg.curveTo(a.x, a.y, 0);
    lineSeg.curveTo(a.x, a.y, 0);
    if (mid.x>0 and mid.y>0)
        lineSeg.curveTo(mid.x, mid.y, 0);
    lineSeg.curveTo(b.x, b.y, 0);
    lineSeg.curveTo(b.x, b.y, 0);
    
    return lineSeg;
}
//Handle the user click in draw mode
//If this is the letter's first click, simply append to the verts
//Otherwise, check if it closes the shape
bool DrawnLetter::handleClick(ofPoint clickPos){
    
    if (verts.size()==0){
        verts.push_back(clickPos);
        return false; //The shape is not closed
    }
    
    if (bendIdx !=-1)
    {
        makeBend(bendIdx, clickPos);
        bendIdx = -1;
        return false;
    }
    //How far the click from the original point?
    float distance = distance2D(verts[0], clickPos);
    
    //If it's not the first click and it's sufficinetly close to the first point
    //Close the shape
    if (distance <= PROXIMITY_THRESHOLD)
    {
        //Create a line back to the origin of the shape
        lines.push_back(createLineSegment(verts.back(), verts[0], ofPoint(0,0)));
        closeShape(); //Adds the original point to the end, clears particles
        return true; //This shape is closed
    }
    
    //Otherwise, add the point to the polyline vertices
    for (int i=1; i<lines.size(); i++) {
        distance = distance2D(verts[i], clickPos);
        
        if (distance <= PROXIMITY_THRESHOLD)
        {
            bendIdx = i;
            return false;
        }
    }
    
    
    //If the code has made it all the way here, then it is a new vertex with a
    //new line segment to it
    //Create a line to the point
    lines.push_back(createLineSegment(verts.back(), clickPos, ofPoint(0,0)));
    //And then append the new position to the verts
    verts.push_back(clickPos);
    //And make sure no line segements are selected
    bendIdx = -1;
    return false; //This shape is not closed
    
}
void DrawnLetter::closeShape()
{
    //The last coordinate is the starting spot
    verts.push_back(verts[0]);
    
    //Mark it internally as complete
    isComplete = true;
    //Clear all particles
    p.clear();
}

//--------------------------------------------------------------
//----------------------  Particle  ----------------------------
//--------------------------------------------------------------
Particle::Particle() {
	live = false;
}

//--------------------------------------------------------------
ofPoint randomPointInCircle( float maxRad ){
	ofPoint pnt;
	float rad = ofRandom( 0, maxRad );
	float angle = ofRandom( 0, M_TWO_PI );
	pnt.x = cos( angle ) * rad;
	pnt.y = sin( angle ) * rad;
	return pnt;
}

//--------------------------------------------------------------
void Particle::setup() {
	pos = ofPoint(last_x,last_y) + randomPointInCircle( param.eRad );
	
    //vel = randomPointInCircle( param.velRad );
    float rad = ofRandom( 0, 30*speed);
    
    vel = ofPoint(cos(angle)*rad, sin(angle)*rad);
    
    time = 0;
	lifeTime = param.lifeTime;
	live = true;
}

//--------------------------------------------------------------
void Particle::update( float dt ){
	if ( live ) {
		//Rotate vel
		vel.rotate( 0, 0, param.rotate * dt );
        
		ofPoint acc;         //Acceleration
		ofPoint delta = pos - param.eCenter;
		float len = delta.length();
		/*
         if ( ofInRange( len, 0, param.eRad ) ) {
         delta.normalize();
         
         //Attraction/repulsion force
         acc += delta * param.force;
         
         //Spinning force
         //acc.x += -delta.y * param.spinning;
         //acc.y += delta.x * param.spinning;
         }
         */
		vel += acc * dt;            //Euler method
		vel *= ( 1 - param.friction );  //Friction
		
		//Update pos
		pos += vel * dt;    //Euler method
        
		//Update time and check if particle should die
		time += dt;
		
        if ( time >= lifeTime ) {
			live = false;   //Particle is now considered as died
		}
        
	}
}

//--------------------------------------------------------------
void Particle::draw(){
	if ( live ) {
		//Compute size
		float size = ofMap( fabs(time - lifeTime/2), 0, lifeTime/2, 3, 1 );
        
		//Compute color
		ofColor color = ofColor::red;
		//float hue = ofMap( time, 0, lifeTime, 128, 255 );
		//color.setHue( hue );
		ofSetColor( ofColor(0) );
        
		ofCircle( pos, size );  //Draw particle
	}
}

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
    
    param.setup();		//Global parameters
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
        letters[i].draw();
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