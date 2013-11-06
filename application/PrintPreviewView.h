/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PRINTPREVIEW_H
#define PRINTPREVIEW_H

#include <View.h>

#include "Debug.h"

class PrintPreviewView : public BView
{
public:
                    PrintPreviewView(void);

    virtual void    Draw(BRect updateRect);
    virtual	void    FrameResized(float newWidth, float newHeight);
    virtual void    MessageReceived(BMessage* message);
    virtual void    KeyDown(const char* bytes, int32 numBytes);
    virtual void 	MouseDown(BPoint where);
    virtual	void	MouseUp(BPoint where);
    virtual	void	MouseMoved(BPoint where, uint32 code,
                        const BMessage* dragMessage);

private:
	
    Debug               	out;
};

#endif // PRINTPREVIEW_H
