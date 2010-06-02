/***************************************************************************
*
*  ofxBaseASR.h
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

#ifndef OFXBASEASR_H
#define OFXBASEASR_H

#include "ofMain.h"

class ofxASR
{
public:
    // Operation of the ASR Engine
    virtual void initEngine() = 0;
    virtual void exitEngine() = 0;
    virtual void openEngine() = 0;
    virtual void closeEngine() = 0;

    // Audio Receive Event
    virtual void _audioReceived(ofAudioEventArgs &e) = 0;
};

#endif
