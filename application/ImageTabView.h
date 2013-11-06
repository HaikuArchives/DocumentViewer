/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef IMAGETAB_H
#define IMAGETAB_H

#include <Bitmap.h>
#include <Rect.h>
#include <String.h>
#include <TabView.h>

#include "Debug.h"

class ImageTab : public BTab
{
public:
                    ImageTab(BView* tabView = nullptr);
    virtual	void	DrawLabel(BView* owner, BRect frame);

private:
    BBitmap*        fBitmap;
    Debug           out;
};


class ImageTabView : public BTabView
{
public:
                    ImageTabView(const char* name = "im_tabview");
    virtual	BRect	TabFrame(int32 index) const;
    

private:
    float       fTabWidth;
};

#endif
