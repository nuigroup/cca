/***************************************************************************
*
*  ofxRectPrint.h
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

#ifndef OFXRECTPRINT_H
#define OFXRECTPRINT_H

#include <string>
#include <vector>
#include "ofMain.h"

class ofRectPrint {
public:
    ofRectPrint();
    void init(ofRectangle &_rect, ofColor &_bgColor, ofColor &_fgColor, string fontFile, int fontSize);
    void addString(std::string s);
    void upScroll(int lines);
    void downScroll(int lines);
    void clearAll();
    void draw();

private:
    std::vector<std::string> stringQueue;
    ofRectangle rect;
    ofColor bgColor;
    ofColor fgColor;
    ofTrueTypeFont font;
    int lastStringIdx;
};


#endif