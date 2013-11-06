/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <memory>

#include <Button.h>
#include <Bitmap.h>
#include <GradientLinear.h>
#include <Mime.h>
#include <Rect.h>
#include <Resources.h>
#include <String.h>



#include "Debug.h"
#include "Timer.h"

class ImageButton : public BButton
{
public:
                    ImageButton(BString imageName, BMessage *message = nullptr,
                        float marginProportion = 0, int frameBehaviour = 0,
                        const char* name = "im_button",
                        uint32 flags = B_FRAME_EVENTS | B_WILL_DRAW
                            | B_NAVIGABLE |B_FULL_UPDATE_ON_RESIZE
                            | B_DRAW_ON_CHILDREN);

    virtual void    Draw(BRect updateRect);
    virtual void    AttachedToWindow(void);
    virtual void    MouseMoved(BPoint where, uint32 code,
                        const BMessage* dragMessage);
    virtual void    MouseUp(BPoint where);
    virtual void    MouseDown(BPoint where);

    virtual	void    FrameResized(float newWidth, float newHeight);
    
    		bool	PushedLong(void);

private:
    BBitmap*        	_RescaleBitmap(const BBitmap* src, int32 width,
                        	int32 height);
    
    BString         		fImageName;
    BBitmap*        		fBitmap;
    BRect           		fImageRect;
    BPoint          		fOffset;
    float           		fMarginProportion;
    int             		fFrameBehaviour;

    BGradientLinear    		fGradientHighlight;
    BGradientLinear			fGradientPressed;

	std::unique_ptr<Timer>	fTimer;			

    rgb_color       		fBgColor;
    rgb_color       		fHighlightColor;
    rgb_color       		fButtonPressedColor;
    rgb_color       		fMarginColor;

    enum {
    	MOUSE_OUTSIDE, MOUSE_INSIDE, MOUSE_PRESSED
    };

    int             		fMouseState;
    bool            		fShowFrame;
    bool					fLongPush;
 	double					fLongPushLimit;
    
    Debug           		out;
};

#endif
