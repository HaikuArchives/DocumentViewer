/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */

#ifndef CIRCLEMENUVIEW
#define CIRCLEMENUVIEW

#include <list>

#include <GradientLinear.h>
#include <Rect.h>
#include <String.h>
#include <View.h>

#include "Debug.h"

class CircleMenuView : public BView
{
public:
                    CircleMenuView(BRect const& frame, int width = 60,
                        BString const& name = "circle_menu");

    virtual void    Draw(BRect updateRect);
    virtual void    MouseMoved(BPoint where, uint32 code,
                        const BMessage* dragMessage);
    virtual void    AddChild(BView* view);
	virtual void    RemoveChild(BView* view);
    virtual void    AlignViews(void);
    virtual void    AttachedToWindow(void);

private:
    std::list<BView*>       fViewList;
    float                   fROut;
    float                   fRIn;
    float                   fRMiddle;
    BPoint                  fCenterPoint;

    rgb_color               fBgColor;
    rgb_color               fMarginColor;

    BGradientLinear         fTransparentGradient;
    Debug                   out;
};

#endif
