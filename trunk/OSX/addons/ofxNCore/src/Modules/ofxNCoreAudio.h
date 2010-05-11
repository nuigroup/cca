/***************************************************************************
*
*  ofxNCoreAudio.h
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

#ifndef _ofxNCoreAudio_H
#define _ofxNCoreAudio_H

// Main
#include "ofMain.h"
// Addons
#include "ofxXmlSettings.h"

// Our Addon
#include "ofxNCore.h"

class ofxNCoreAudio : public ofxGuiListener
{
    // ofxGUI setup stuff
    enum
    {	
        sourcePanel,
        sourcePanel_record,
        sourcePanel_readfile,
        sourcePanel_playpause,
        sourcePanel_stop,
    };

public:
    ofxNCoreAudio()
    {
        ofAddListener(ofEvents.mousePressed, this, &ofxNCoreAudio::_mousePressed);
        ofAddListener(ofEvents.mouseDragged, this, &ofxNCoreAudio::_mouseDragged);
        ofAddListener(ofEvents.mouseReleased, this, &ofxNCoreAudio::_mouseReleased);
        ofAddListener(ofEvents.keyPressed, this, &ofxNCoreAudio::_keyPressed);
        ofAddListener(ofEvents.setup, this, &ofxNCoreAudio::_setup);
        ofAddListener(ofEvents.update, this, &ofxNCoreAudio::_update);
        ofAddListener(ofEvents.draw, this, &ofxNCoreAudio::_draw);
        ofAddListener(ofEvents.exit, this, &ofxNCoreAudio::_exit);
    }

    ~ofxNCoreAudio()
    {		
    }

    /****************************************************************
    *						Public functions
    ****************************************************************/
    // Basic Events called every frame
    void _setup(ofEventArgs &e);
    void _update(ofEventArgs &e);
    void _draw(ofEventArgs &e);
    void _exit(ofEventArgs &e);
    // Mouse Events
    void _mousePressed(ofMouseEventArgs &e);
    void _mouseDragged(ofMouseEventArgs &e);
    void _mouseReleased(ofMouseEventArgs &e);
    // Key Events
    void _keyPressed(ofKeyEventArgs &e);

    // GUI
    void setupControls();
    void		handleGui(int parameterId, int task, void* data, int length);
    ofxGui*		controls;

    // drawing
    void drawMiniMode();
    void drawFullMode();	

    //Load/save settings
    void loadXMLSettings();
    void saveSettings();

    /****************************************************************
    *            Variables in config.xml Settings file
    *****************************************************************/
    int					winWidth;
    int					winHeight;

    bool  				bMiniMode;
    /****************************************************
    *            End config.xml variables
    *****************************************************/

private:
    // Fonts
    ofTrueTypeFont		verdana;
    ofTrueTypeFont      sidebarTXT;
    ofTrueTypeFont		bigvideo;

    // Images
    ofImage				background;

    // XML Settings Vars
    ofxXmlSettings		XML;
    string				message;

    // Gui Settings
    bool				showConfiguration;
    bool				bShowInterface;
    bool  				bFullscreen;
};

#endif
