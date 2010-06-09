/***************************************************************************
*
*  ofxSphinxASR.cpp
* 
*  Author: Jimbo Zhang <dr.jimbozhang@gmail.com>
*  Copyright 2010 Jimbo Zhang. All rights reserved.       
*
*
* This file is part of ofxASR, an openframeworks addon for speech
* recognition.
*
* ofxASR is free software: you can redistribute it and/or
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
***************************************************************************/

#include "ofxSphinxASR.h"

ofxSphinxASR::ofxSphinxASR()
{
    ofAddListener(ofEvents.audioReceived, this, &ofxSphinxASR::_audioReceived);
    bEngineInitialed = false;
    bEngineStarted = false;
    decoder = NULL;
}

ofxSphinxASR::~ofxSphinxASR()
{

}

void ofxSphinxASR::engineInit()
{
    decoder = new s3_decode_t;

}

void ofxSphinxASR::engineExit()
{
    if (decoder != NULL) {
        delete decoder;
        decoder = NULL;
    }

}

void ofxSphinxASR::engineOpen()
{

}

void ofxSphinxASR::engineClose()
{

}

void ofxSphinxASR::engineReset()
{

}

void ofxSphinxASR::engineSentAudio(char *audioBuf, int audioSize)
{

}

char * ofxSphinxASR::engineGetText()
{
    return NULL;
}

void ofxSphinxASR::_audioReceived(ofAudioEventArgs &e)
{

}
