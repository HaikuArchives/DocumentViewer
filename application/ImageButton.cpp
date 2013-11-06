/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "ImageButton.h"

#include <algorithm>

#include <Application.h>
#include <Point.h>
#include <Screen.h>
#include <Size.h>

#include "Tools.h"

using namespace std;

ImageButton::ImageButton(BString imageName,BMessage *message,
                        float marginProportion, int frameBehaviour,
                        const char* name,  uint32 flags)
    :
    BButton(name, "", message, flags),
    fImageName(imageName),
    fBitmap(nullptr),
    fMarginProportion(1 - marginProportion),
    fFrameBehaviour(frameBehaviour),
    fMouseState(MOUSE_OUTSIDE),
    fShowFrame(false),
    fLongPush(false),
    fLongPushLimit(500)
{
    SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
    SetExplicitPreferredSize(BSize(50, 30));

    //SetViewColor(B_TRANSPARENT_32_BIT);
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    SetDrawingMode(B_OP_ALPHA);
    SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
    /*
    BScreen screen;

    monitor_info info;
    screen.GetMonitorInfo(&info);

    !out << "Width: " << info.width << "  Height " << info.height << std::endl;
    */
    
    fTimer = unique_ptr<Timer>(new Timer);

    fGradientHighlight.SetStart(0, 0);
    fGradientPressed.SetStart(0, 0);
}


void
ImageButton::Draw(BRect updateRect)
{
    switch (fFrameBehaviour) {
        case 0: case 1:
            break;

        case 2:


            break;

        default:
            break;
    }


    switch (fFrameBehaviour) {
        case 0:
            BButton::Draw(updateRect);
            break;

        case 1:
            if (fMouseState == MOUSE_OUTSIDE) {
                SetHighColor(Parent()->ViewColor());
                FillRect(Bounds());
            } else {
                BButton::Draw(updateRect);
            }
            break;

        case 2:
            switch (fMouseState) {
                case MOUSE_OUTSIDE:
                    if (Parent()->ViewColor().alpha == 255) {
                        SetHighColor(Parent()->ViewColor());
                        FillRect(Bounds());
                    }
                    break;

                case MOUSE_INSIDE:
                    FillEllipse(Bounds(), fGradientHighlight);
                    break;

                case MOUSE_PRESSED:
                    FillEllipse(Bounds(), fGradientPressed);
                    SetHighColor(fMarginColor);
                    StrokeEllipse(Bounds());
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    if (fBitmap == nullptr)
        fBitmap = Tools::LoadBitmap(fImageName, fImageRect.Width());

    DrawBitmapAsync(fBitmap, fImageRect.LeftTop() + fOffset);

    Sync();
}


void
ImageButton::AttachedToWindow(void)
{
    BButton::AttachedToWindow();
    if (Parent() != nullptr)
        fBgColor = Parent()->ViewColor();
    else
        fBgColor = ViewColor();

    fGradientHighlight.AddColor(tint_color(fBgColor, B_LIGHTEN_1_TINT), 0);
    fGradientHighlight.AddColor(tint_color(fBgColor, B_DARKEN_3_TINT), 255);

    fGradientPressed.AddColor(tint_color(fBgColor, B_DARKEN_1_TINT), 0);
    fGradientPressed.AddColor(tint_color(fBgColor, B_DARKEN_4_TINT), 255);

    fMarginColor = tint_color(fBgColor, B_DARKEN_2_TINT);
}


void
ImageButton::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{

	switch (code) {
		case B_EXITED_VIEW:
            MakeFocus(false);
            fMouseState = MOUSE_OUTSIDE;
            Invalidate();
            break;

		case B_ENTERED_VIEW:
            fMouseState = MOUSE_INSIDE;
            Invalidate();
			break;

		default:
            break;
	}

    BButton::MouseMoved(where, code, dragMessage);
}


void
ImageButton::MouseUp(BPoint where)
{	
	if (fTimer->MS() > fLongPushLimit)
		fLongPush = true;
		
    fMouseState = MOUSE_INSIDE;
    fShowFrame = false;
    fOffset = BPoint(0, 0);
    Invalidate();
    BButton::MouseUp(where);
}


void
ImageButton::MouseDown(BPoint where)
{
	fTimer->Restart();
    fMouseState = MOUSE_PRESSED;
    fLongPush = false;
    fShowFrame = true;
    fOffset = BPoint(1, 1);
    Invalidate();
    BButton::MouseDown(where);
}


bool
ImageButton::PushedLong(void)
{
	return fLongPush;	
}

void
ImageButton::FrameResized(float newWidth, float newHeight)
{
    BButton::FrameResized(newWidth, newHeight);

    int width = std::min(newWidth, newHeight) * fMarginProportion;
    fImageRect = BRect(0, 0, width, width);

    fImageRect.OffsetTo((newWidth - width)/2, (newHeight - width)/2);

    delete fBitmap;
    fBitmap = Tools::LoadBitmap(fImageName, fImageRect.Width());
    if (fBitmap == nullptr)
        !out << "Bitmap == nullptr" << std::endl;
    Invalidate();

    fGradientHighlight.SetEnd(0, newHeight);
    fGradientPressed.SetEnd(0, newHeight);
}


BBitmap*
ImageButton::_RescaleBitmap(const BBitmap* src, int32 width, int32 height)
{
	if (!src || !src->IsValid())
		return nullptr;

	BRect srcSize = src->Bounds();

	if (height < 0) {
		float srcProp = srcSize.Height()/srcSize.Width();
		height = (int32)(width * ceil(srcProp));
	}

	BBitmap* res = new BBitmap(BRect(0, 0, (float)width, (float)height),
        src->ColorSpace());

	float dx = (srcSize.Width() + 1)/(float)(width + 1);
	float dy = (srcSize.Height() + 1)/(float)(height + 1);
	uint8 bpp = (uint8)(src->BytesPerRow()/ceil(srcSize.Width()));

	int srcYOff = src->BytesPerRow();
	int dstYOff = res->BytesPerRow();

	void* dstData = res->Bits();
	void* srcData = src->Bits();

	for (int32 y = 0; y <= height; y++) {
		void* dstRow = (void*)((uint32)dstData + (uint32)(y * dstYOff));
		void* srcRow = (void*)((uint32)srcData + ((uint32)(y * dy) * srcYOff));

		for (int32 x = 0; x <= width; x++)
			memcpy((void*)((uint32)dstRow + (x * bpp)), (void*)((uint32)srcRow
                                              + ((uint32)(x * dx) * bpp)), bpp);
	}

	return res;
}
