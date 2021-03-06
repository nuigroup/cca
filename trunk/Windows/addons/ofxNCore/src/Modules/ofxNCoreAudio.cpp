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

#include <time.h>
#include "ofxNCoreAudio.h"
#include "libresample.h"

#ifdef USE_SPHINX
#include "ofxSphinxASR.h"
#endif

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
	
    audioBuf = NULL;
    audioBufSize = 0;
    curPlayPoint = 0;
    bRecording = false;
    bPlaying = false;
    bPaused = false;
    curAsrEngine = NULL;
    asrEngine_1 = NULL;
    asrEngine_2 = NULL;
    resample_handle = NULL;
}

ofxNCoreAudio::~ofxNCoreAudio()
{		
    	
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
	
    // ASR Engine: commandpicking
    asrEngine_1 = new ofxSphinxASR;
    ofAsrEngineArgs *engineArgs = new ofAsrEngineArgs;
    engineArgs->sphinxmodel_am = sphinxmodel_am;
    engineArgs->sphinxmodel_lm = sphinxmodel_lm;
    engineArgs->sphinxmodel_dict = sphinxmodel_dict;
    engineArgs->sphinxmodel_fdict = sphinxmodel_fdict;
    engineArgs->sphinx_mode = 2;
    engineArgs->samplerate = model_sampleRate;
    FILE *fp_list = fopen(commandList.c_str(), "rt");
    if (fp_list==NULL) {
        printf("Error: Can not find command list file %s.", commandList.c_str());
    }
    char *sentence = new char[maxSentenceLength];
    while (fgets(sentence, maxSentenceLength, fp_list)) {
        engineArgs->sphinx_candidate_sentences.push_back(sentence);
    }
    if (asrEngine_1->engineInit(engineArgs) != OFXASR_SUCCESS) {
        printf("Initial ASR Engine Failed!");
    }
    delete []sentence;
    curAsrEngine = asrEngine_1;

    // ASR Engine: freespeaking
    asrEngine_2 = new ofxSphinxASR;
    engineArgs->sphinx_mode = 4;
    if (asrEngine_2->engineInit(engineArgs) != OFXASR_SUCCESS) {
        printf("Initial ASR Engine Failed!");
    }
    delete engineArgs;
	
    // Display
    ofColor outBgColor;
    outBgColor.r = outBgColor.g = outBgColor.b = 0x33;
    ofColor outFgColor;
    outFgColor.r = 0; outFgColor.g = 255; outFgColor.b = 0;
    ofRectangle outRect;
    int outRectBorder = 20;
    outRect.x = 386 + outRectBorder/2;
    outRect.y = 41 + outRectBorder;
    outRect.height = 229 - 2 * outRectBorder;
    outRect.width = 320 - 2 * outRectBorder/2;
    rectPrint.init(outRect, outBgColor, outFgColor, "verdana.ttf", 8);
    rectPrint.setLineHeight(15);

    // Resample
    resample_factor = (float)model_sampleRate / SampleRate;
    resample_handle = resample_open(1, resample_factor, resample_factor);
    tcpServer.setup(tcpPort);
	
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
	
    // Command Candidate
    commandList                 = XML.getValue("CONFIG:ASR:LIST", "");
    maxSentenceLength           = XML.getValue("CONFIG:ASR:MAXCOMMANDLENGTH", 32);;

	// ASR
	model_sampleRate            = XML.getValue("CONFIG:ASR:SAMPLERATE", 16000);
#ifdef USE_SPHINX
	
    sphinxmodel_am              = XML.getValue("CONFIG:ASR:AM", "");
    sphinxmodel_lm              = XML.getValue("CONFIG:ASR:LM", "");
    sphinxmodel_dict            = XML.getValue("CONFIG:ASR:DICT", "");
    sphinxmodel_fdict           = XML.getValue("CONFIG:ASR:FDICT", "");
#endif
    
    // Communication
    outputMode                 = OutputMode(XML.getValue("CONFIG:COMMUNICATION:NETWORKMODE", 0));
    tcpPort                    = XML.getValue("CONFIG:COMMUNICATION:TCPPORT", 11999);
    printf("XML Loaded.\n");
    
	
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
        ofSoundStreamClose();
		
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
        if (!bMiniMode) {
            controls->draw();
        }
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
    
    // Draw network message
    if (outputMode!=screen_only)
	{
		//Draw Port and IP to screen
		ofSetColor(0xffffff);
		char buf[256];
		if(outputMode==tcp_plaintext)
			sprintf(buf, "Sending TCP Plain messages on:\n Port: %i", tcpPort);
		else if(outputMode==tcp_xml)
            sprintf(buf, "Sending TCP XML messages on:\n Port: %i", tcpPort);
        else {}
        
		verdana.drawString(buf, 740, 480);
	}
	
    // Draw link to CCA website
    ofSetColor(79, 79, 79);
    ofFill();
    ofRect(721, 584, 228, 16);
    ofSetColor(0xFFFFFF);
    ofDrawBitmapString("nuicode.com/projects/cca-alpha", 725, 596);
	
    ofSetColor(0xFF0000);
    // verdana.drawString("Press spacebar for mini mode", 748, 572);
	
    rectPrint.draw();
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
				break;
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
 * ON EXIT
 *****************************************************************************/
void ofxNCoreAudio::_exit(ofEventArgs &e)
{
    saveSettings();

    asrEngine_1->engineExit();
    delete asrEngine_1;
    asrEngine_1 = NULL;

    asrEngine_2->engineExit();
    delete asrEngine_2;
    asrEngine_2 = NULL;

    if (audioBuf!=NULL) {
        delete[] audioBuf;
        audioBuf = NULL;
    }
    if (curAsrEngine!=NULL) {
        curAsrEngine = NULL;
    }
	
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
    ofxGuiPanel* srcPanel = controls->addPanel(appPtr->sourcePanel, "Input Audio Panel", 41, 270, OFXGUI_PANEL_BORDER, OFXGUI_PANEL_SPACING);
    srcPanel->addButton(appPtr->sourcePanel_record, "RECORD SOUND", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Switch);
    srcPanel->addButton(appPtr->sourcePanel_sendToASR, "SEND TO RECOGNIZE ENGINE", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
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
	
    // Output Panel
    ofxGuiPanel* outputPanel = controls->addPanel(appPtr->outputPanel, "Convert Text Panel", 387, 270, OFXGUI_PANEL_BORDER, OFXGUI_PANEL_SPACING);
    outputPanel->addButton(appPtr->outputPanel_switchPickingMode, "COMMAND PICKING MODE", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_On, kofxGui_Button_Switch);
    outputPanel->addButton(appPtr->outputPanel_switchFreeMode, "FREE SPEAKING MODE", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Switch);
    outputPanel->addButton(appPtr->outputPanel_clear, "CLEAR SCREEN", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
    srcPanel->mObjHeight = 85;
    outputPanel->mObjWidth = 319;
    outputPanel->mObjects[0]->mObjY = 42;
    outputPanel->mObjects[1]->mObjY = 65;
    outputPanel->mObjects[2]->mObjX = 180;
    outputPanel->mObjects[2]->mObjY = 65;
	
    srcPanel->adjustToNewContent(100, 0);
    
    // Communication
    ofxGuiPanel* tcpPanel = controls->addPanel(appPtr->tcpPanel, "Communication", 735, 10, 12, OFXGUI_PANEL_SPACING);
	tcpPanel->addButton(appPtr->tcpPanel_tcp_plaintext, "TCP Plain Text", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, outputMode==tcp_plaintext ? kofxGui_Button_On : kofxGui_Button_Off, kofxGui_Button_Switch);
	tcpPanel->addButton(appPtr->tcpPanel_tcp_xml, "TCP XML", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, outputMode==tcp_xml ? kofxGui_Button_On : kofxGui_Button_Off, kofxGui_Button_Switch);
	tcpPanel->mObjWidth = 200;
	
    // do update while inactive
    controls->forceUpdate(false);
    controls->activate(true);
}

void ofxNCoreAudio ::handleGui(int parameterId, int task, void* data, int length)
{	
    switch(parameterId)
    {
		case sourcePanel_record:
			callback_sourcePanel_record();
			break;			
		case sourcePanel_stop:
			callback_sourcePanel_stop();
			break;
		case sourcePanel_playpause:
			callback_sourcePanel_playpause();
			break;
		case sourcePanel_sendToASR:
			callback_sourcePanel_sendToASR();
			break;
		case outputPanel_switchPickingMode:
			callback_outputPanel_switchPickingMode();
			break;
		case outputPanel_switchFreeMode:
			callback_outputPanel_switchFreeMode();
			break;
		case outputPanel_clear:
			callback_outputPanel_clear();
			break;
        case tcpPanel_tcp_plaintext:
            callback_tcpPanel_tcp_plaintext();
            break;
        case tcpPanel_tcp_xml:
            callback_tcpPanel_tcp_xml();
            break;
		default:
			1;
    }
}
