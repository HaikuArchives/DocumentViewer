/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef BOOKMARKSVIEW_H
#define BOOKMARKSVIEW_H

#include <vector>

extern "C" {
    #include <fitz.h>
}

#include <Bitmap.h>
#include <GroupView.h>
#include <OutlineListView.h>
#include <ScrollBar.h>
#include <String.h>
#include <TextControl.h>

#include "Debug.h"

class BookmarkItem : public BListItem
{
public:
    BookmarkItem(const char* name);
    ~BookmarkItem();
    virtual void DrawItem(BView* owner, BRect itemRect, bool = false);
private:
	BString fName;   
};


class BookmarkListView : public BOutlineListView
{
public:
                    BookmarkListView(void);

private:

};


class BookmarksView : public BGroupView
{
public:
                    BookmarksView(void);

private:
    BScrollBar*         fVScrollBar;
    BScrollBar*         fHScrollBar;
    
    BTextControl*		fSearchTC;
    BookmarkListView* 	fBookmarkListView;
};

#endif
