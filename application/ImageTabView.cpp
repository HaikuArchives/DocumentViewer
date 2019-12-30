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

ImageTab::ImageTab(const char* imgName, BView* tabView)
    :
    BTab(tabView)
{
    fBitmap = nullptr;
    fImageName.SetTo(imgName);
}


void
ImageTab::DrawLabel(BView* owner, BRect frame)
{

    int size = std::min(frame.Width(), frame.Height()) - 2;

    if (fBitmap == nullptr) {
        fBitmap = Tools::LoadBitmap(fImageName, size);
    } else if (fBitmap->Bounds().Height() != size) {
        delete fBitmap;
        fBitmap = Tools::LoadBitmap(fImageName, size);
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

void
ImageTabView::MouseMoved(BPoint where, uint32 code, const BMessage * dragMessage)
{
	int32 numTabs = CountTabs();
	if (code == B_INSIDE_VIEW || code == B_ENTERED_VIEW) {
		for (int i = 0; i < numTabs; i++) {
			const BRect tabRect = TabFrame(i);
			if (tabRect.Contains(where)) {
				SetToolTip(TabAt(i)->Label());
			}
		}
	} else {
		SetToolTip("");
	}

}
