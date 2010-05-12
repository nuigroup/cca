/***************************************************************************
*
*  Gui.h
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
* with Community Core Audio.  If not, see <http://www.gnu.org/licenses/>.
*
*
* Web: http://nuicode.com/projects/cca-alpha
*
***************************************************************************/

#ifndef GUI_CONTROLS_H
#define GUI_CONTROLS_H

#include "../Modules/ofxNCoreAudio.h"

#include <stdio.h>

ofxNCoreAudio  *appPtr;

void ofxNCoreAudio::setupControls()
{
	appPtr = this;

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
    srcPanel->addButton(appPtr->sourcePanel_playpause, "PLAY/PAUSE", OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, kofxGui_Button_Off, kofxGui_Button_Trigger);
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
            ofSoundStreamStop();
            bRecording = false;
            if (lastAudioSavename.length()>0) {
                FILE* lastAudio_fp = fopen(lastAudioSavename.c_str(), "wb");
                for (int i=0; i<audioBufSize; i++) {
                    short f = short(audioBuf[i]);
                    fwrite(&f, sizeof(short), 1, lastAudio_fp);
                }
                fclose(lastAudio_fp); 
            }
        }
        else {
            if (audioBuf!=NULL) {
                delete[] audioBuf;
                audioBuf = NULL;
                audioBufSize = 0;
            }
            ofSoundStreamStart();
            bRecording = true;
        }
        break;

	default:
		1;
	}
}
#endif

