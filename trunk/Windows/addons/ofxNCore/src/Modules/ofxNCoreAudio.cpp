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

#define AUDIO_SEGBUF_SIZE 256

ofxNCoreAudio::ofxNCoreAudio()
{
    // Assign listeners
    ofAddListener(ofEvents.mousePressed, this, &ofxNCoreAudio::_mousePressed);
    ofAddListener(ofEvents.mouseDragged, this, &ofxNCoreAudio::_mouseDragged);
    ofAddListener(ofEvents.mouseReleased, this, &ofxNCoreAudio::_mouseReleased);
    ofAddListener(ofEvents.keyPressed, this, &ofxNCoreAudio::_keyPressed);
    ofAddListener(ofEvents.setup, this, &ofxNCoreAudio::_setup);
    ofAddListener(ofEvents.update, this, &ofxNCoreAudio::_update);
    ofAddListener(ofEvents.draw, this, &ofxNCoreAudio::_draw);
    ofAddListener(ofEvents.exit, this, &ofxNCoreAudio::_exit);

    // Variables for Recording and Playings
    audioBuf = NULL;
    audioBufSize = 0;
    curPlayPoint = 0;
    bRecording = false;
    bPlaying = false;
    bPaused = false;

    // The ASR Engine
    asrEngine = new ofxSphinxASR;
}

ofxNCoreAudio::~ofxNCoreAudio()
{		
    if (audioBuf!=NULL) {
        delete[] audioBuf;
        audioBuf = NULL;
    }
    if (asrEngine!=NULL) {
        delete asrEngine;
        asrEngine = NULL;
    }


}

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

    // Fonts
    verdana.loadFont("verdana.ttf", 8, true, true);	   // Font used for small images
    bigvideo.loadFont("verdana.ttf", 13, true, true);  // Font used for big images.

    // Static Images
    background.loadImage("images/background.jpg"); // Main Background

    // GUI Controls
    controls = ofxGui::Instance(this);
    setupControls();    

    /*****************************************************************************************************
    * Startup Modes
    ******************************************************************************************************/
    showConfiguration = true;
    if (bMiniMode)
    {
        bShowInterface = false;
        printf("Starting in Mini Mode...\n\n");
        ofSetWindowShape(190, 200); // minimized size
    }
    else{
        bShowInterface = true;
        printf("Starting in full mode...\n\n");
    }

#ifdef TARGET_WIN32
#ifndef _DEBUG
    FreeConsole();
#endif
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
    winWidth                    = XML.getValue("CONFIG:WINDOW:WIDTH", 950);
    winHeight                   = XML.getValue("CONFIG:WINDOW:HEIGHT", 600);	

    // MODES
    bMiniMode                   = XML.getValue("CONFIG:BOOLEAN:MINIMODE", 0);

    // Memory
    maxAudioSize                = XML.getValue("CONFIG:MEMORY:MAXAUDIOSIZE", 102400);

    // Formats
    SampleRate                  = XML.getValue("CONFIG:FORMAT:SAMPLERATE", 16000);

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
    // recording auto stop
    if (bRecording && audioBufSize>=maxAudioSize) {
        printf("Sound is too long. Try to set MAXAUDIOSIZE in config.xml.");
        finishRecord();

        bool setBool = false;
        controls->update(sourcePanel_record, kofxGui_Set_Bool, &setBool, sizeof(bool));
        bRecording = false;
    }
    
    // playing auto stop
    if (bPlaying && audioBufSize-curPlayPoint < AUDIO_SEGBUF_SIZE) {
        curPlayPoint = 0;
        ofSoundStreamClose();
        bPlaying = false;

        bool setBool = false;
        controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
    }
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
    bigvideo.drawString("Source Audio", 140, 30);
    bigvideo.drawString("Converted Text", 475, 30);

    // Draw input waveform viewer
    int inputViewerX = 40;
    int inputViewerY = 41;
    int viewerHeight = 229;
    int viewerWidth = 320;
	int viewerBackColor = 0x333333;
	int waveColor = 0x73BA51;
    int viewerMiddleY = inputViewerY + viewerHeight / 2;
    ofSetColor(viewerBackColor);
    ofRect(inputViewerX, inputViewerY, viewerWidth, viewerHeight);
    
    ofSetColor(waveColor);

    if (audioBufSize > viewerWidth) {
        for (int i = 0; i < viewerWidth; i++){
            if (bRecording) {
                // Dynamic waveform
                ofLine(inputViewerX+i,viewerMiddleY,inputViewerX+i,viewerMiddleY
                    +audioBuf[audioBufSize-viewerWidth+i]*100.0f);
            }
            else {
                // Static waveform
                ofLine(inputViewerX+i,viewerMiddleY,inputViewerX+i,viewerMiddleY
                    +audioBuf[i*audioBufSize/viewerWidth]*100.0f);
                ofLine(inputViewerX+i,viewerMiddleY,inputViewerX+i,viewerMiddleY
                    -audioBuf[i*audioBufSize/viewerWidth]*100.0f);
            }
        }

        if (bPlaying) {
            ofSetColor(0xFFFFFF);
            int curDrawPoint = int(curPlayPoint/(float)audioBufSize*viewerWidth);
            ofLine(inputViewerX+curDrawPoint, inputViewerY,
                inputViewerX+curDrawPoint, inputViewerY+viewerHeight);
            ofSetColor(waveColor);
        }
    }
	
	// Draw output text viewer
	int outputViewerX = 386;
    int outputViewerY = 41;
    ofSetColor(viewerBackColor);
    ofRect(outputViewerX, outputViewerY, viewerWidth, viewerHeight);
    ofSetColor(waveColor);

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
        audioBuf[audioBufSize++] = input[i];
    }
}

void ofxNCoreAudio::audioRequested(float * output, int bufferSize, int nChannels)
{
    float pan = 0.5f;
    float leftScale = 1 - pan;
    float rightScale = pan;

    for (int i=0; i<bufferSize; i++) {
        output[i*nChannels    ] = audioBuf[curPlayPoint] * leftScale;
        output[i*nChannels + 1] = audioBuf[curPlayPoint] * rightScale;
        curPlayPoint++;
    }

}

/*****************************************************************************
* Record 
*****************************************************************************/
void ofxNCoreAudio::finishRecord()
{
    ofSoundStreamClose();
    if (lastAudioSavename.length()>0) {
        FILE* lastAudio_fp = fopen(lastAudioSavename.c_str(), "wb");
        if (lastAudio_fp==NULL) {
            printf("Open file %s failed.\n", lastAudioSavename.c_str());
        }
        else {
            for (int i=0; i<audioBufSize; i++) {
                short f = short(audioBuf[i] * 32767.5 - 0.5);
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

/*****************************************************************************
* GUI
*****************************************************************************/
void ofxNCoreAudio::setupControls()
{
    ofxNCoreAudio  *appPtr = this;

    // panel border color
    controls->mGlobals->mBorderColor.r = 0;
    controls->mGlobals->mBorderColor.g = 0;
    controls->mGlobals->mBorderColor.b = 0;
    controls->mGlobals->mBorderColor.a = .3;
    // panel color
    controls->mGlobals->mCoverColor.r = 1;
    controls->mGlobals->mCoverColor.g = 1;
    controls->mGlobals->mCoverColor.b = 1;
    controls->mGlobals->mCoverColor.a = .4;
    // control outline color
    controls->mGlobals->mFrameColor.r = 0;
    controls->mGlobals->mFrameColor.g = 0;
    controls->mGlobals->mFrameColor.b = 0;
    controls->mGlobals->mFrameColor.a = .3;
    // text color
    controls->mGlobals->mTextColor.r = 0;
    controls->mGlobals->mTextColor.g = 0;
    controls->mGlobals->mTextColor.b = 0;
    controls->mGlobals->mTextColor.a = 1;
    // button color
    controls->mGlobals->mButtonColor.r = 1;
    controls->mGlobals->mButtonColor.g = 0;
    controls->mGlobals->mButtonColor.b = 0;
    controls->mGlobals->mButtonColor.a = .8;
    // slider tip color
    controls->mGlobals->mHandleColor.r = 0;
    controls->mGlobals->mHandleColor.g = 0;
    controls->mGlobals->mHandleColor.b = 0;
    // slider color
    controls->mGlobals->mSliderColor.r = 1;
    controls->mGlobals->mSliderColor.g = 0;
    controls->mGlobals->mSliderColor.b = 0;
    controls->mGlobals->mSliderColor.a = .8;

    // Source Image
    ofxGuiPanel* srcPanel = controls->addPanel(appPtr->sourcePanel, "Source Audio", 41, 270, OFXGUI_PANEL_BORDER, OFXGUI_PANEL_SPACING);
    srcPanel->addButton(appPtr->sourcePanel_record, "RECORD SOUND", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Switch);
    srcPanel->addButton(appPtr->sourcePanel_readfile, "READ SOUND FILE", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
    srcPanel->addButton(appPtr->sourcePanel_playpause, "PLAY/PAUSE", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Switch);
    srcPanel->addButton(appPtr->sourcePanel_stop, "STOP", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
    srcPanel->mObjHeight = 85;
    srcPanel->mObjWidth = 319;
    srcPanel->mObjects[0]->mObjY = 42;
    srcPanel->mObjects[1]->mObjX = 130;
    srcPanel->mObjects[1]->mObjY = 42;
    srcPanel->mObjects[2]->mObjX = 130;
    srcPanel->mObjects[2]->mObjY = 65;
    srcPanel->mObjects[3]->mObjY = 65;

    srcPanel->adjustToNewContent(100, 0);

    // do update while inactive
    controls->forceUpdate(false);
    controls->activate(true);

}

void ofxNCoreAudio ::handleGui(int parameterId, int task, void* data, int length)
{
    switch(parameterId)
    {
    case sourcePanel_record:
        if (bRecording) {
            finishRecord();
            bRecording = false;            
        }
        else {
            if (audioBuf!=NULL) {
                delete[] audioBuf;
                audioBuf = NULL;
                audioBufSize = 0;
            }
            ofSoundStreamSetup(0, 1, this, SampleRate, AUDIO_SEGBUF_SIZE, 2);
            bRecording = true;
        }
        break;

    case sourcePanel_stop:
        if (bRecording) {            
            finishRecord();
            bRecording = false;  

            bool setBool = false;
            controls->update(sourcePanel_record, kofxGui_Set_Bool, &setBool, sizeof(bool));
        }
        if (bPlaying) {
            curPlayPoint = 0;
            ofSoundStreamClose();
            bPlaying = false;

            bool setBool = false;
            controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
        }
        break;
    case sourcePanel_playpause:
        if (bRecording) {
            finishRecord();
            bRecording = false;            

            bool setBool = false;
            controls->update(sourcePanel_record, kofxGui_Set_Bool, &setBool, sizeof(bool));
        }

        if (bPlaying) {            
            if (bPaused) {
                ofSoundStreamStart();
                bPaused = false;
            }
            else {
                ofSoundStreamStop();
                bPaused = true;
            }
        }
        else {
            if (audioBufSize<AUDIO_SEGBUF_SIZE) {
                bool setBool = false;
                controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
            }
            else {
                ofSoundStreamSetup(2, 0, this, SampleRate, AUDIO_SEGBUF_SIZE, 4);
                bPlaying = true;
                bPaused = false;
            }
        }
        break;
    default:
        1;
    }
}
