/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */

#include "PrintPreviewView.h"

#include <Window.h>

PrintPreviewView::PrintPreviewView(void)
    :
    BView(BRect(), "print_preview_view", B_FOLLOW_ALL,
            B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_FRAME_EVENTS )
{
    SetViewColor(B_TRANSPARENT_32_BIT);
    //SetDrawingMode(B_OP_ALPHA);
    //SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

    Invalidate();
}


void
PrintPreviewView::MessageReceived(BMessage* message)
{
	switch (message->what) {

        default:
            BView::MessageReceived(message);
            break;
    }
}


void
PrintPreviewView::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1) {
		switch (bytes[0]) {
    	  	default:
				BView::KeyDown(bytes, numBytes);
       		  	break;
        }
    }
}

#if 0
void
PrintPreviewView::AllAttached(void)
{
    Window()->SetPulseRate(1E6);
    BView::AllAttached();
}
#endif


void
PrintPreviewView::Draw(BRect updateRect)
{	
   	BRect bounds = Bounds();
    SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    FillRect(bounds);
    
    
    SetHighColor(0, 0, 0, 255);
    BPoint where(0, 0);
    
    SetPenSize(2);
    StrokeRect(bounds);
    Sync();
    BView::Draw(updateRect);
}


void
PrintPreviewView::FrameResized(float newWidth, float newHeight)
{
	
    BView::FrameResized(newWidth, newHeight);
}


void
PrintPreviewView::MouseMoved(BPoint where, uint32 code,
    const BMessage* dragMessage)
{
 

    BView::MouseMoved(where, code, dragMessage);
}


void
PrintPreviewView::MouseDown(BPoint where)
{
    MakeFocus(true);
    //GetMouse(&fOldMousePos, &fMouseButtons);

    int32 clicks = 0;
    Window()->CurrentMessage()->FindInt32("clicks", &clicks);

    if (clicks == 2) {
        
    }

    BView::MouseDown(where);
}


void
PrintPreviewView::MouseUp(BPoint where)
{

    BView::MouseUp(where);
}


