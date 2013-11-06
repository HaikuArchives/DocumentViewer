/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "ImageTabView.h"

#include <algorithm>

#include <Application.h>
#include <Point.h>
#include <Size.h>

#include "Tools.h"

ImageTab::ImageTab(BView* tabView)
    :
    BTab(tabView)
{
    fBitmap = nullptr;
}


void
ImageTab::DrawLabel(BView* owner, BRect frame)
{
    if (Label() == nullptr)
        return;

    int size = std::min(frame.Width(), frame.Height()) - 2;

    if (fBitmap == nullptr) {
        fBitmap = Tools::LoadBitmap(Label(), size);
    } else if (fBitmap->Bounds().Height() != size) {
        delete fBitmap;
        fBitmap = Tools::LoadBitmap(Label(), size);
    }

    owner->SetDrawingMode(B_OP_OVER);

    BPoint where;
    where.x = frame.left + (frame.Width() - fBitmap->Bounds().Width()) / 2;
    where.y = frame.top + (frame.Height() - fBitmap->Bounds().Height())/ 2;

    owner->DrawBitmap(fBitmap, where);

}


ImageTabView::ImageTabView(const char* name)
    :
    BTabView(name),
    fTabWidth(38)
{
	SetTabHeight(26);
}


BRect
ImageTabView::TabFrame(int32 index) const
{
    if (index < 0 || index > CountTabs())
        return BRect();
	
    return BRect(index*fTabWidth, 0, (index + 1) * fTabWidth, TabHeight());
}
