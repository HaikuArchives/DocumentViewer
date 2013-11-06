/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */

#include "CircleMenuView.h"

#include <complex>
#include <numeric>

using namespace std;

CircleMenuView::CircleMenuView(BRect const& frame, int width, BString const& name)
    :
    BView(frame, name, B_FOLLOW_LEFT | B_FOLLOW_TOP,
        B_FULL_UPDATE_ON_RESIZE | B_WILL_DRAW | B_DRAW_ON_CHILDREN)
{
    SetEventMask(EventMask() | B_FULL_POINTER_HISTORY);
    SetDrawingMode(B_OP_ALPHA);
    SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
    SetPenSize(3);

    fBgColor = make_color(180, 180, 255, 180);
    SetViewColor(fBgColor);
    fMarginColor = tint_color(fBgColor, B_DARKEN_4_TINT);
/*
	m_nWidth = nWidth;
	m_cBounds = cFrame.Bounds();

	int nROut = static_cast< int >( m_cBounds.Width() / 2 );
	int nRIn = nROut - m_nWidth ;

	m_cCenter = IPoint( nROut, nROut );

	m_vRMiddle = nRIn + m_nWidth / 2;

	m_pcImage = DrawRing( nROut, nRIn, 0.0, 0.0, 0.0, 0.6 );

	Hide();
    */
    fCenterPoint = BPoint(frame.Width() / 2, frame.Height() / 2);
    fROut = std::min(frame.Width() / 2, frame.Height() / 2);
    fROut -= 10;
    fRIn = fROut- width;
    fRMiddle = (fROut + fRIn) / 2;
    if (IsHidden() == false)
        Hide();
}


void
CircleMenuView::Draw(BRect updateRect)
{
    SetHighColor(fBgColor);
    BPoint center(Bounds().Width()/2, Bounds().Height()/2);
    // FillEllipse(center, center.x, center.x - 10);
    FillArc(fCenterPoint, fROut, fROut, 0, 360);
    SetHighColor(B_TRANSPARENT_32_BIT);
    FillArc(fCenterPoint, fRIn, fRIn, 0, 360);
    SetHighColor(fMarginColor);
    StrokeArc(fCenterPoint, fROut, fROut, 0, 360);
    StrokeArc(fCenterPoint, fRIn, fRIn, 0, 360);
}


void
CircleMenuView::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
    BView::MouseMoved(where, code, dragMessage);
    if((pow((where.x - fCenterPoint.x),2) +
                pow((where.y - fCenterPoint.y),2)) >= pow(fROut,2)) {
        if (IsHidden() == false)
            Hide();

        Parent()->MakeFocus(true);
    }
}


void
CircleMenuView::AddChild(BView* view)
{
    BView::AddChild(view);
    AlignViews();
}


void
CircleMenuView::RemoveChild(BView* view)
{


}


void
CircleMenuView::AlignViews(void)
{
    if (Parent() == NULL)
        return;

    float children = CountChildren();
    float numberOfChild = 0;
    BView*  child = ChildAt(0);

    while (child != NULL) {
        complex<float> pos(polar(fRMiddle, static_cast< float >(M_PI/2) +
            numberOfChild*2* static_cast< float >(M_PI) / children));

        float width = child->Bounds().Width();
		float height = child->Bounds().Height();

        //rect.right = fCenterPoint.x + pos.real() + width/2;
        //rect.bottom = fCenterPoint.y - pos.imag() + height/2;

        child->MoveTo(fCenterPoint.x + pos.real() - width/2,
            fCenterPoint.y - pos.imag() - height/2);
        child->ResizeTo(49, 49);
        child->ResizeBy(1, 1);
        child->Invalidate();
        //child->Show();
        ++numberOfChild;
        child = child->NextSibling();
    }

    Invalidate();
}


void
CircleMenuView::AttachedToWindow(void)
{
    BView::AttachedToWindow();

    AlignViews();
    Invalidate();
}
