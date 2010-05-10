/***************************************************************************
*
*  ofxNCoreAudio.cpp
*  NUI Group Community Core Audio
* 
*  Author: Jimbo Zhang <dr.jimbozhang@gmail.com>
*  Copyright 2010 NUI Group. All rights reserved.       
*
*
* This file is part of Community Core Audio.
*
* Community Core Audio is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Community Core Audio is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with Community Core Audio.  If not, see <http:// www.gnu.org/licenses/>.
*
*
* Web: http:// nuicode.com/projects/cca-alpha
*
***************************************************************************/

#include "ofxNCoreAudio.h"
#include "../Controls/gui.h"

/******************************************************************************
* The setup function is run once to perform initializations in the application
*****************************************************************************/
void ofxNCoreAudio::_setup(ofEventArgs &e)
{	
	// set the title
	ofSetWindowTitle(" Community Core Audio ");	

	// Load Settings from config.xml file
	loadXMLSettings();

	// Setup Window Properties
	ofSetWindowShape(winWidth,winHeight);
	ofSetVerticalSync(false);

	// Fonts - Is there a way to dynamically change font size?
	verdana.loadFont("verdana.ttf", 8, true, true);	   // Font used for small images
	bigvideo.loadFont("verdana.ttf", 13, true, true);  // Font used for big images.

	// Static Images
	background.loadImage("images/background.jpg"); // Main (Temp?) Background

	// GUI Controls
	controls = ofxGui::Instance(this);
	setupControls();

	/*****************************************************************************************************
	* Startup Modes
	******************************************************************************************************/
	// If Standalone Mode (not an addon)
	if (bStandaloneMode)
	{
		printf("Starting in standalone mode...\n\n");
		showConfiguration = true;
	}
	if (bMiniMode)
	{
		showConfiguration = true;
		bShowInterface = false;
		printf("Starting in Mini Mode...\n\n");
		ofSetWindowShape(190, 200); // minimized size
	}
	else{
		bShowInterface = true;
		printf("Starting in full mode...\n\n");
	}
	
	#ifdef TARGET_WIN32
	    // get rid of the console window
        FreeConsole();
	#endif

	printf("Community Core Vision is setup!\n\n");
}

/****************************************************************
*	Load/Save config.xml file Settings
****************************************************************/
void ofxNCoreAudio::loadXMLSettings()
{
	//  TODO: a separate XML to map keyboard commands to action
	message = "Loading config.xml...";
	//  Can this load via http?
	if ( XML.loadFile("config.xml"))
		message = "Settings Loaded!\n\n";
	else
		message = "No Settings Found...\n\n"; // FAIL
	
	// --------------------------------------------------------------
	//   START BINDING XML TO VARS
	// --------------------------------------------------------------
	winWidth					= XML.getValue("CONFIG:WINDOW:WIDTH", 950);
	winHeight					= XML.getValue("CONFIG:WINDOW:HEIGHT", 600);	
	
	// MODES
	bMiniMode                   = XML.getValue("CONFIG:BOOLEAN:MINIMODE",0);
	// --------------------------------------------------------------
	//   END XML SETUP
}

void ofxNCoreAudio::saveSettings()
{
	XML.setValue("CONFIG:BOOLEAN:MINIMODE", bMiniMode);
	XML.saveFile("config.xml");
}


/******************************************************************************
* The update function runs continuously. Use it to update states and variables
*****************************************************************************/
void ofxNCoreAudio::_update(ofEventArgs &e)
{
}

/******************************************************************************
* The draw function paints the textures onto the screen. It runs after update.
*****************************************************************************/
void ofxNCoreAudio::_draw(ofEventArgs &e)
{
	if (showConfiguration) 
	{
		// if mini mode
		if (bMiniMode)
		{
			drawMiniMode();
		}

		// if full mode
		else if (bShowInterface)
		{
			drawFullMode();
		}

		// draw gui controls
		if (!bMiniMode) {controls->draw();}
	}
}

void ofxNCoreAudio::drawFullMode()
{
	ofSetColor(0xFFFFFF);
	// Draw Background Image
	background.draw(0, 0);
	// Draw arrows
	ofSetColor(187, 200, 203);
	ofFill();
	ofTriangle(680, 420, 680, 460, 700, 440);
	ofTriangle(70, 420, 70, 460, 50, 440);
	ofSetColor(255, 255, 0);

	ofSetColor(0x969696);

	bigvideo.drawString("Source", 168, 20);
	bigvideo.drawString("Output", 509, 20);

	//draw link to CCA website
	ofSetColor(79, 79, 79);
	ofFill();
	ofRect(721, 584, 228, 16);
	ofSetColor(0xFFFFFF);
	ofDrawBitmapString("nuicode.com/projects/cca-alpha", 725, 596);

	//Display Application information in bottom right
	string str = "Calculation Time: ";

	ofSetColor(0xFF0000);
	verdana.drawString("Press spacebar for mini mode", 748, 572);
}

void ofxNCoreAudio::drawMiniMode()
{
	//black background
	ofSetColor(0,0,0);
	ofRect(0,0,ofGetWidth(), ofGetHeight());

	//draw grey rectagles for text information
	ofSetColor(128,128,128);
	ofFill();
	ofRect(0,ofGetHeight() - 83, ofGetWidth(), 20);
	ofRect(0,ofGetHeight() - 62, ofGetWidth(), 20);
	ofRect(0,ofGetHeight() - 41, ofGetWidth(), 20);
	ofRect(0,ofGetHeight() - 20, ofGetWidth(), 20);

	//draw text
	ofSetColor(250,250,250);
	verdana.drawString("Communication:  " ,10, ofGetHeight() - 9 );
}

/*****************************************************************************
* KEY EVENTS
*****************************************************************************/
void ofxNCoreAudio::_keyPressed(ofKeyEventArgs &e)
{
	if (showConfiguration)
	{
		switch (e.key)
		{
		case ' ':
			if (bMiniMode)
			{
				bMiniMode = false;
				bShowInterface = true;
				ofSetWindowShape(950,600); //default size
			}
			else
			{
				bMiniMode = true;
				bShowInterface = false;
				ofSetWindowShape(190,200); //minimized size
			}
		}
	}
}

/*****************************************************************************
*	MOUSE EVENTS
*****************************************************************************/
void ofxNCoreAudio::_mouseDragged(ofMouseEventArgs &e)
{
	if (showConfiguration)
		controls->mouseDragged(e.x, e.y, e.button); // guilistener
}

void ofxNCoreAudio::_mousePressed(ofMouseEventArgs &e)
{
	if (showConfiguration)
	{
		controls->mousePressed(e.x, e.y, e.button); // guilistener
		if (e.x > 722 && e.y > 586){ofLaunchBrowser("http://nuicode.com/projects/cca-alpha");}
	}
}

void ofxNCoreAudio::_mouseReleased(ofMouseEventArgs &e)
{
	if (showConfiguration)
		controls->mouseReleased(e.x, e.y, 0); // guilistener
}

/*****************************************************************************
* ON EXIT
*****************************************************************************/
void ofxNCoreAudio::_exit(ofEventArgs &e)
{
	// save/update log file
	if((stream = freopen(fileName, "a", stdout)) == NULL){}

	saveSettings();

	//  -------------------------------- SAVE STATE ON EXIT	
	printf("CCA module has exited!\n");
}

