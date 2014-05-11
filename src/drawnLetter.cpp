//
//  drawnLetter.cpp
//  emptyExample
//
//  Created by Daniel Goodwin on 5/10/14.
//
//

#include "drawnLetter.h"
#define PROXIMITY_THRESHOLD 20.0

float distance2D(ofPoint a, ofPoint b)
{
    float delta_x = a.x - b.x;
    float delta_y = a.y - b.y;
    return sqrt(delta_x*delta_x + delta_y*delta_y);
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

    
	//Fill buffer with white color
    
	//Set up parameters
	//
	history = 0.0;//0.9;
	bornRate = 500;
    
	bornCount = 0;
	time0 = ofGetElapsedTimef();
    
    bendIdx = -1;
}

void DrawnLetter::draw(int mouse_x, int mouse_y)
{
    
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
            //makeBend(bendIdx,ofPoint(last_x,last_y));
            makeBend(bendIdx,ofPoint(mouse_x,mouse_y));
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
}

