/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "BookmarksView.h"

#include <LayoutBuilder.h>

#include "ImageButton.h"
#include "Messages.h"


BookmarkItem::BookmarkItem(const char* name)
	:
    BListItem(),
    fName(name)
{
}


BookmarkItem::~BookmarkItem()
{
}


void
BookmarkItem::DrawItem(BView *owner, BRect itemRect, bool complete)
{
	rgb_color kBlack = {0, 0, 0, 0};
	rgb_color kHighlight = {156, 154, 156, 0};

	if (IsSelected() || complete) {
		rgb_color color;
		if (IsSelected())
			color = kHighlight;
		else
			color = owner->ViewColor();

		owner->SetHighColor(color);
		owner->SetLowColor(color);
		owner->FillRect(itemRect);
		owner->SetHighColor(kBlack);

	} else {
		owner->SetLowColor(owner->ViewColor());
	}

	BFont font = be_plain_font;
	font_height	finfo;
	font.GetHeight(&finfo);

	BPoint point = BPoint(itemRect.left + 5, itemRect.bottom - finfo.descent + 1);

	owner->SetHighColor(kBlack);
	owner->SetFont(be_plain_font);
	owner->MovePenTo(point);

	owner->MovePenTo(point);
	owner->DrawString(fName);
}


BookmarkListView::BookmarkListView(void)
    :
    BOutlineListView("bookmark_listview")
{

}


BookmarksView::BookmarksView(void)
	:
    BGroupView("Bookmarks_view", B_VERTICAL, 0)
{  
    ImageButton* bookmarkDeleteButton = new ImageButton("quit",
        new BMessage(MSG_ZOOM_IN), 0.3, 1);
    bookmarkDeleteButton->SetExplicitMinSize(BSize(20, 20));
    bookmarkDeleteButton->SetExplicitMaxSize(BSize(20, 20));
    
    ImageButton* bookmarkAddButton = new ImageButton("plus",
        new BMessage(MSG_ZOOM_OUT), 0.3, 1);
    bookmarkAddButton->SetExplicitMinSize(BSize(20, 20));
    bookmarkAddButton->SetExplicitMaxSize(BSize(20, 20));
    
    ImageButton* bookmarkEditButton = new ImageButton("edit",
        new BMessage(MSG_NO_FIT), 0.3, 1);
    bookmarkEditButton->SetExplicitMinSize(BSize(20, 20));
    bookmarkEditButton->SetExplicitMaxSize(BSize(20, 20));
    
    BookmarkItem* item1 = new BookmarkItem("Test1");
   // item1->SetOutlineLevel(1);
    BookmarkItem* item2 = new BookmarkItem("Test2");
    //item2->SetOutlineLevel(0);
    BookmarkItem* item3 = new BookmarkItem("Test3");
    //item2->SetOutlineLevel(0);
    BookmarkItem* item4 = new BookmarkItem("Test4");
    BookmarkItem* item5 = new BookmarkItem("Test5");
    
    fBookmarkListView = new BookmarkListView();
    fBookmarkListView->AddItem(item1);
    fBookmarkListView->AddUnder(item3,  item1);
    fBookmarkListView->AddUnder(item2, item1);
    fBookmarkListView->AddItem(item4);
    fBookmarkListView->AddItem(item5);
    
    fVScrollBar = new BScrollBar("v_scrollbar",
                    fBookmarkListView, 0, 2000, B_VERTICAL);
    fHScrollBar = new BScrollBar("h_scrollbar",
                    fBookmarkListView, 0, 2000, B_HORIZONTAL);
                    
   	fSearchTC   = new BTextControl("", "", nullptr);
    
    BLayoutBuilder::Group<>(this)
    	.AddGroup(B_HORIZONTAL, 0)
    		.Add(bookmarkAddButton)
    		.Add(bookmarkEditButton)
    		.Add(bookmarkDeleteButton)
    		.Add(fSearchTC)
    		//.AddGlue(100)
    	.End()	
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fBookmarkListView)
            .Add(fVScrollBar)
		.End()
        .Add(fHScrollBar)
    .End()
	;
}
