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

    // Setup audio input
    ofSoundStreamSetup(0, 1, this, 16000, 256, 2);
    ofSoundStreamStop();

    /*****************************************************************************************************
    * Startup Modes
    ******************************************************************************************************/
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
    // FreeConsole();
#endif

    printf("Community Core Audio is setup!\n\n");
}

/****************************************************************
*	Load/Save config.xml file Settings
****************************************************************/
void ofxNCoreAudio::loadXMLSettings()
{
    message = "Loading config.xml...";
    if ( XML.loadFile("config.xml"))
        message = "Settings Loaded!\n\n";
    else
        message = "No Settings Found...\n\n";

    // --------------------------------------------------------------
    //   START BINDING XML TO VARS
    // --------------------------------------------------------------
    winWidth					= XML.getValue("CONFIG:WINDOW:WIDTH", 950);
    winHeight					= XML.getValue("CONFIG:WINDOW:HEIGHT", 600);	

    // MODES
    bMiniMode                   = XML.getValue("CONFIG:BOOLEAN:MINIMODE", 0);

    // Memory
    maxAudioSize                = XML.getValue("CONFIG:MEMORY:MAXAUDIOSIZE", 102400);

    // Logs
    lastAudioSavename           = XML.getValue("CONFIG:LOGS:LASTAUDIOFILENAME", "");
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

    ofSetColor(0x969696);
    bigvideo.drawString("Source", 168, 20);
    bigvideo.drawString("Text", 509, 20);

    // Draw input waveform
    ofSetColor(0x333333);
    ofRect(40, 41, 320, 229);
    ofSetColor(0x73BA51);
    if (audioBufSize>320) {
        for (int i = 0; i < 320; i++){
            ofLine(40+i,156,41+i,156+audioBuf[audioBufSize-320+i]*100.0f);
        }
    }

    // Draw link to CCA website
    ofSetColor(79, 79, 79);
    ofFill();
    ofRect(721, 584, 228, 16);
    ofSetColor(0xFFFFFF);
    ofDrawBitmapString("nuicode.com/projects/cca-alpha", 725, 596);

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
    verdana.drawString("CCA" ,10, ofGetHeight() - 9 );
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
                ofSetWindowShape(winWidth, winHeight); //default size
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

    // printf("Mouse pressed at x=%d, y=%d\n", e.x, e.y);
}

void ofxNCoreAudio::_mouseReleased(ofMouseEventArgs &e)
{
    if (showConfiguration)
        controls->mouseReleased(e.x, e.y, 0); // guilistener
}

void ofxNCoreAudio::audioReceived( float * input, int bufferSize, int nChannels )
{    
    if (audioBuf==NULL) {
        audioBuf = new float[maxAudioSize];
        audioBufSize = 0;
    }
    for (int i=0; i<bufferSize; i++) {
        if (audioBufSize>=maxAudioSize) {
            printf("Sound is too long. Try set MAXAUDIOSIZE in config.xml.");
            ofSoundStreamStop();
        }
        else {
            audioBuf[audioBufSize++] = input[i];
        }
    }
}

/*****************************************************************************
* Record 
*****************************************************************************/
void ofxNCoreAudio::finishRecord()
{
    ofSoundStreamStop();
    if (lastAudioSavename.length()>0) {
        FILE* lastAudio_fp = fopen(lastAudioSavename.c_str(), "wb");
        if (lastAudio_fp==NULL) {
            printf("Open file %s failed.\n", lastAudioSavename.c_str());
        }
        else {
            for (int i=0; i<audioBufSize; i++) {
                short f = short(audioBuf[i]);
                fwrite(&f, sizeof(short), 1, lastAudio_fp);
            }
            fclose(lastAudio_fp); 
        }
    }
}

/*****************************************************************************
* ON EXIT
*****************************************************************************/
void ofxNCoreAudio::_exit(ofEventArgs &e)
{
    saveSettings();

    //  -------------------------------- SAVE STATE ON EXIT	
    printf("CCA module has exited!\n");
}

