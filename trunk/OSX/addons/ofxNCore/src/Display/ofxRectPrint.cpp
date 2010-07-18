/***************************************************************************
*
*  ofxRectPrint.cpp
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

#include "ofxRectPrint.h"

ofRectPrint::ofRectPrint()
{
    lastStringIdx = -1;
    lineHeight = 20;
}

void ofRectPrint::init(ofRectangle &_rect, ofColor &_bgColor, ofColor &_fgColor, string fontFile, int fontSize) 
{
    rect.x = _rect.x;
    rect.y = _rect.y;
    rect.height = _rect.height;
    rect.width = _rect.width;

    bgColor.r = _bgColor.r;
    bgColor.g = _bgColor.g;
    bgColor.b = _bgColor.b;
    bgColor.a = _bgColor.a;

    fgColor.r = _fgColor.r;
    fgColor.g = _fgColor.g;
    fgColor.b = _fgColor.b;
    fgColor.a = _fgColor.a;

    font.loadFont(fontFile, fontSize);
    if (! font.bLoadedOk) {
        printf("load font file failed: %s\n", fontFile);
        return;
    }
}

void ofRectPrint::addString(std::string s)
{
    stringQueue.push_back(s);
    lastStringIdx = stringQueue.size() - 1;
}

void ofRectPrint::upScroll(int lines)
{
    lastStringIdx -= lines;
    if (lastStringIdx < 0) {
        lastStringIdx = 0;
    }
}

void ofRectPrint::downScroll(int lines)
{
    lastStringIdx += lines;
    if (lastStringIdx > stringQueue.size() - 1) {
        lastStringIdx = stringQueue.size() - 1;
    }
}

void ofRectPrint::clearAll()
{
    stringQueue.clear();
    lastStringIdx = -1;
}

void ofRectPrint::setLineHeight(int height)
{
    lineHeight = height;
}

void ofRectPrint::draw()
{
    if (! font.bLoadedOk) {
        printf("Font file has not been loaded.\n");
        return;
    }    

    // Clear rect
    ofSetColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    ofRect(rect.x, rect.y, rect.width, rect.height);    
    ofFill();

    if (lastStringIdx < 0 || lastStringIdx > stringQueue.size() - 1)  {
        printf("No strings in the output queue.\n");
        return;
    }    

    // Draw strings
    ofSetColor(fgColor.r, fgColor.g, fgColor.b, fgColor.a);
    int maxLineNum = rect.height / lineHeight;
    int startLine = lastStringIdx - maxLineNum;
    if (startLine < 0)
        startLine = 0;

    for (int i=0; i<=lastStringIdx-startLine; i++) {
        font.drawString(stringQueue[startLine+i], rect.x, rect.y + i*lineHeight);
    }    
}
