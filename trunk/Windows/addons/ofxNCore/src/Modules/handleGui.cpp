/***************************************************************************
*
*  handleGui.cpp
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
#include "libresample.h"

#define AUDIO_FLOAT2SHORT(P) ( short(P * 32767.5 - 0.5) )
#define AUDIO_SHORT2FLOAT(P) ( (float(P) + 0.5) / 32767.5 )

void ofxNCoreAudio::callback_sourcePanel_record() {
    bool setBool;
    if (bRecording) {
        ofSoundStreamClose();
        bRecording = false;            
    }
    else {
        if (bPlaying) {
            curPlayPoint = 0;
            ofSoundStreamClose();
            bPlaying = false;

            setBool = false;
            controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
        }

        if (asr_mode != mode_commandpicking) {
            return;
        }

        if (audioBuf!=NULL) {
            delete[] audioBuf;
            audioBuf = NULL;
            audioBufSize = 0;
        }
        ofSoundStreamSetup(0, 1, this, SampleRate, AUDIO_SEGBUF_SIZE, 2);
        bRecording = true;
    }
    return;

}

void ofxNCoreAudio::callback_sourcePanel_stop() {
     bool setBool;
    if (bRecording) {            
        ofSoundStreamClose();
        bRecording = false;  

        setBool = false;
        controls->update(sourcePanel_record, kofxGui_Set_Bool, &setBool, sizeof(bool));
    }
    if (bPlaying) {
        curPlayPoint = 0;
        ofSoundStreamClose();
        bPlaying = false;

        setBool = false;
        controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
    }
    return;
}

void ofxNCoreAudio::callback_sourcePanel_playpause() {
    bool setBool;
    if (bRecording) {
        ofSoundStreamClose();
        bRecording = false;            

        setBool = false;
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
        if (asr_mode != mode_commandpicking) {
            return;
        }

        if (audioBufSize<AUDIO_SEGBUF_SIZE) {
            setBool = false;
            controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
        }
        else {
            ofSoundStreamSetup(2, 0, this, SampleRate, AUDIO_SEGBUF_SIZE, 4);
            bPlaying = true;
            bPaused = false;
        }
    }
    return;
}

void ofxNCoreAudio::callback_sourcePanel_sendToASR() {
    bool setBool;
    float *sentBuf = audioBuf;
    int sentBufSize = audioBufSize;
    int srcused;
    short *buf_16 = NULL;    
    time_t t;
    struct tm *current_time;
    char *result_tmp = NULL;
    string result;

    if (asr_mode != mode_commandpicking) {
        return;
    }

    if (bRecording) {            
        ofSoundStreamClose();
        bRecording = false;  

        setBool = false;
        controls->update(sourcePanel_record, kofxGui_Set_Bool, &setBool, sizeof(bool));
    }
    if (bPlaying) {
        curPlayPoint = 0;
        ofSoundStreamClose();
        bPlaying = false;

        setBool = false;
        controls->update(sourcePanel_playpause, kofxGui_Set_Bool, &setBool, sizeof(bool));
    }

    if (audioBufSize < AUDIO_SEGBUF_SIZE) {
        return;
    }

    if (! asrEngine->isEngineOpened()) {
        asrEngine->engineOpen();
    }

    if (SampleRate != model_sampleRate) {
        sentBufSize = int(audioBufSize * resample_factor);
        sentBuf = new float[sentBufSize];
        resample_process(resample_handle, resample_factor, audioBuf, audioBufSize, 1, &srcused, sentBuf, sentBufSize);
    }

    buf_16 = new short[sentBufSize];
    for (int i=0; i<sentBufSize; i++) {
        buf_16[i] = AUDIO_FLOAT2SHORT(sentBuf[i]);
    }

    asrEngine->engineSentAudio(buf_16, audioBufSize);
    asrEngine->engineClose();

    if (sentBuf != NULL && sentBuf != audioBuf) {
        delete[] sentBuf;
        sentBuf = NULL;
    }

    result_tmp = new char[maxSentenceLength];
    t = time(0);
    current_time = localtime(&t);
    sprintf(result_tmp, "[%2d:%2d:%2d] %s", current_time->tm_hour, current_time->tm_min, 
        current_time->tm_sec, asrEngine->engineGetText());
    result = result_tmp;
    rectPrint.addString(result);        
    delete[] result_tmp;
    result_tmp = NULL;
    printf("Test Converted: %s\n", result.c_str());
    return;
}

void ofxNCoreAudio::callback_outputPanel_switchPickingMode() {
    bool setBool;
    setBool = true;
    controls->update(outputPanel_switchPickingMode, kofxGui_Set_Bool, &setBool, sizeof(bool));
    setBool = false;
    controls->update(outputPanel_switchFreeMode, kofxGui_Set_Bool, &setBool, sizeof(bool));
    asr_mode = mode_commandpicking;
    if (asrEngine->isEngineOpened()) {
        asrEngine->engineClose();
    }
    return;
}

void ofxNCoreAudio::callback_outputPanel_switchFreeMode() {
    bool setBool;
    setBool = true;
    controls->update(outputPanel_switchFreeMode, kofxGui_Set_Bool, &setBool, sizeof(bool));
    setBool = false;
    controls->update(outputPanel_switchPickingMode, kofxGui_Set_Bool, &setBool, sizeof(bool));
    asr_mode = mode_freespeaking;
    return;
}

void ofxNCoreAudio::callback_outputPanel_clear() {
    rectPrint.clearAll();
    return;
}