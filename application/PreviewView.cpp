/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "PreviewView.h"

#include <stdexcept>

#include <LayoutBuilder.h>

#include "Messages.h"
#include "PDFEngine.h"
#include "DJVUEngine.h"


PreviewView::PreviewView(void)
	:
    BGroupView("preview_view", B_VERTICAL, 0)
{	
    fColumnOption = new BOptionPopUp("column_option", "",
                        new BMessage(M_COLUMN_WIDTH));
    fColumnOption->AddOptionAt("Fit Width", FIT_WIDTH, 0);
    fColumnOption->AddOptionAt("Small Width", SMALL_COLUMN, 1);
    fColumnOption->AddOptionAt("Medium Width", MEDIUM_COLUMN, 2);
    fColumnOption->AddOptionAt("Big Width", BIG_COLUMN, 3);

    fBasicPreviewView = new BasicPreviewView();

    fVScrollBar = new BScrollBar("v_scrollbar",
                    fBasicPreviewView, 0, 10000, B_VERTICAL);

    BLayoutBuilder::Group<>(this)
        .AddGroup(B_HORIZONTAL, 0)
            .Add(fColumnOption)
        .End()
       // .AddStrut(1)
        .AddGroup(B_HORIZONTAL, 0)
            .Add(fBasicPreviewView)
            .Add(fVScrollBar)
        .End()
    .End()
	;
}


void
PreviewView::AttachedToWindow(void)
{
    fColumnOption->SetTarget(this);
}


void
PreviewView::Show(void)
{
	//if (fBasicPreviewView->fEngine == nullptr)
	//	return;
		
	BGroupView::Show();	
}



void
PreviewView::MessageReceived(BMessage* message)
{
	switch (message->what) {
        case M_COLUMN_WIDTH:
        {
            int32 value = 0;
            fColumnOption->SelectedOption(nullptr, &value);
            fBasicPreviewView->SetColumnWidth(value);
            break;
        }

        default:
            BView::MessageReceived(message);
            break;
    }
}


void
PreviewView::FileChanged(const BString& file, BString const& fileType,
	BString& password)
{
	fBasicPreviewView->SetFile(file, fileType, password);
	Invalidate();
}


void
PreviewView::SelectPage(const int& pageNumber)
{
    fBasicPreviewView->SelectPage(pageNumber);
}


BasicPreviewView::BasicPreviewView(void)
    :
    BView(BRect(), "basic_preview_view", B_FOLLOW_ALL,
            B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_FRAME_EVENTS ),
    fOldMousePos(0, 0),
    fZoom(1),
    fPageHeight(0),
    fHSpace(5),
    fVSpace(5),
    fHMargin(1),
    fVMargin(1),
    fColumns(1),
    fRows(0),
    fColumnWidth(0),
    fMouseButtons(0),
    fCurrentPageNumber(0),
    fHighlightPageNumber(0),
    fIsPanning(false),
    fScrollToCurrentPage(0),
    fAdaptCache(false)
{
    SetFlags(Flags() | B_PULSE_NEEDED);

    fEngine = nullptr;

    SetViewColor(B_TRANSPARENT_32_BIT);
    SetDrawingMode(B_OP_ALPHA);
    //SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

    Invalidate();
}


void
BasicPreviewView::MessageReceived(BMessage* message)
{
	switch (message->what) {

        default:
            BView::MessageReceived(message);
            break;
    }
}


void
BasicPreviewView::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1) {
		switch (bytes[0]) {
            case B_PAGE_UP:
            case B_UP_ARROW:
            case B_LEFT_ARROW:
                _ScrollUpBigStep();
				break;

            case B_PAGE_DOWN:
			case B_DOWN_ARROW:
            case B_RIGHT_ARROW:
                _ScrollDownBigStep();
                break;

            case B_HOME:
                _ShowPage(0);
                break;

            case B_END:
                _ShowPage(fEngine->PageCount() - 1);
                break;

    	  	default:
				BView::KeyDown(bytes, numBytes);
       		  	break;
        }
    }
}


void
BasicPreviewView::AllAttached(void)
{
    Window()->SetPulseRate(1E6);
    BView::AllAttached();
}


void
BasicPreviewView::Draw(BRect updateRect)
{	
   	BRect bounds = Bounds();
    SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    FillRect(bounds);
    
    if (fEngine == nullptr) {
    	BView::Draw(updateRect);
    	return;
    }
    
    SetHighColor(0, 0, 0, 255);
    BPoint where(0, 0);

    if (fColumnWidth > 0 && fColumnWidth < bounds.Width()) {
        fRows = ceil(static_cast<float>(_Pages()) / fColumns);
        _SetColumns((Bounds().Width() - 2*fHMargin + fHSpace) / (fColumnWidth + fHSpace));

        fPageHeight = _PageHeight();
        int lowerbound = fCurrentPageNumber;
        int upperbound = 2 + (bounds.Height() -fVSpace) / (fPageHeight + fVSpace);
        upperbound *= fColumns;
        upperbound += fCurrentPageNumber;

        if (upperbound > _Pages())
            upperbound = _Pages();

        BRect rect;
        int i = lowerbound;
        where.y = fVMargin + fCurrentRow * (fPageHeight + fVSpace);
        for ( ; i < upperbound;) {
            where.x = fHMargin;

            for (int j = 0; j < fColumns && i < upperbound; ++j, ++i) {
                rect.left = 0;
                rect.top  = 0;
                rect.right = fColumnWidth;
                rect.bottom = fPageHeight = _PageHeight(i);
                rect.OffsetBySelf(where);
                if (rect.Intersects(updateRect)) {
                    DrawBitmapAsync(_Page(i), rect);
                    if (i == fHighlightPageNumber) {
                        SetHighColor(0, 0, 255, 255);
                        SetPenSize(2);
                        StrokeRect(rect);
                        SetHighColor(0, 0, 0, 255);
                        SetPenSize(1);
                    } else {
                        StrokeRect(rect);
                    }
                }
                where.x += fColumnWidth + fHSpace;
            }
            where.y += rect.Height() + fVSpace;
        }
        fZoom = fColumnWidth / fEngine->Page(upperbound - 1)->Bounds().Width();
    } else {
        where.x = fHMargin;
        fRows = fEngine->PageCount();
        fPageHeight = _PageHeight();
        float width = bounds.Width()  - 2*fHMargin;
        int upperbound = bounds.Height() / fPageHeight;
        upperbound = upperbound + fCurrentPageNumber + 2;

        if (upperbound > fEngine->PageCount())
            upperbound = fEngine->PageCount();

        if (fScrollToCurrentPage == 0)
            where.y = fVMargin + fCurrentRow * (fPageHeight + fVSpace);
        else
            where.y = ScrollBar(B_VERTICAL)->Value();
		
		bool hasDrawn = false;
        for (int i = fCurrentPageNumber; i < upperbound; ++i) {
            fPageHeight = _PageHeight(i);
            BRect rect(0, 0, width, fPageHeight);
            rect.OffsetBySelf(where);
            if (rect.Intersects(updateRect)) {
            	hasDrawn = true;
                DrawBitmapAsync(_Page(i), rect);
                if (i == fHighlightPageNumber) {
                    SetHighColor(0, 0, 255, 255);
                    SetPenSize(2);
                    StrokeRect(rect);
                    SetHighColor(0, 0, 0, 255);
                    SetPenSize(1);
                } else {
                    StrokeRect(rect);
                }
            } else if (hasDrawn) {
            	break;	
            }
            where.y += (fPageHeight + fVSpace);
        }
        fZoom =  width / fEngine->Page(upperbound - 1)->Bounds().Width();
    }
    
    if (fScrollToCurrentPage == 1)
            ++fScrollToCurrentPage;

    Sync();
    BView::Draw(updateRect);
}


void
BasicPreviewView::FrameResized(float newWidth, float newHeight)
{
    BView::FrameResized(newWidth, newHeight);
    if (fEngine == nullptr)
        return;

    fScrollToCurrentPage = 1;
    fAdaptCache = true;
}


void
BasicPreviewView::SetFile(BString file, BString fileType, BString& password)
{
	delete fEngine;
	if (fileType == "djvu")
    	fEngine = new DJVUEngine(file, password);
    else if (fileType == "pdf")
    	fEngine = new PDFEngine(file, password);
    
    _AdaptScrollBarRange();
    Invalidate();   
}


void
BasicPreviewView::MouseMoved(BPoint where, uint32 code,
    const BMessage* dragMessage)
{
    switch (code) {
		case B_EXITED_VIEW:
            fIsPanning  = false;
            break;

		default:
            break;
	}

    if (fIsPanning == true) {
        BPoint delta = fOldMousePos - where;

        ScrollBar(B_VERTICAL)->SetValue(
            ScrollBar(B_VERTICAL)->Value() + delta.y);
    }

    BView::MouseMoved(where, code, dragMessage);
}


void
BasicPreviewView::MouseDown(BPoint where)
{
    MakeFocus(true);
    GetMouse(&fOldMousePos, &fMouseButtons);

    int32 clicks = 0;
    Window()->CurrentMessage()->FindInt32("clicks", &clicks);

    if (clicks == 2) {
        fHighlightPageNumber = _ConvertToPage(fOldMousePos);
        _GoToPage(fHighlightPageNumber);
        Invalidate();
    } else {
        fIsPanning = true;
    }

    BView::MouseDown(where);
}


void
BasicPreviewView::MouseUp(BPoint where)
{
    fIsPanning = false;

    BView::MouseUp(where);
}


void
BasicPreviewView::ScrollTo(BPoint where)
{
	if (fEngine == nullptr)
    	return;
    	
    BView::ScrollTo(where);
    
    where.x = fHMargin;
    int row = _ConvertToRow(where);

    if (row != fCurrentRow) {
        fCurrentRow = row;
        fCurrentPageNumber = row * fColumns;
        _AdaptScrollBarRange();
    }
}


inline void
BasicPreviewView::_GoToPage(int pageNumber)
{
    BMessage msg(MSG_GOTO_PAGE);
    msg.AddInt32("info", pageNumber);
    Window()->PostMessage(&msg);
}


inline int
BasicPreviewView::_ConvertToPage(BPoint const& where)
{
    if (fEngine == nullptr)
        return 0;

    if (fColumns == 1) {
        return _ConvertToRow(where);
    } else {
        int x = (where.x - fHMargin) / (fColumnWidth + fHSpace);
        int y = (where.y - fVMargin) / (_PageHeight() + fVSpace);

        return x + y*fColumns;
    }
}


inline void
BasicPreviewView::_AdaptScrollBarRange(void)
{
    if (fEngine == nullptr)
        return;

    if (fColumnWidth == 0) {
        ScrollBar(B_VERTICAL)->SetRange(0, fVMargin + (fVSpace + _PageHeight()) * fRows
            - Bounds().Height());

        float bigStepV = std::min(Bounds().Height(), _PageHeight());
        float smallStepV = Bounds().Height() / 27;
        ScrollBar(B_VERTICAL)->SetSteps(smallStepV, bigStepV);
    } else {
        ScrollBar(B_VERTICAL)->SetRange(0, fVMargin + (fVSpace + _PageHeight()) * fRows
            - Bounds().Height());

        float bigStepV = Bounds().Height();
        float smallStepV = Bounds().Height() / 27;
        ScrollBar(B_VERTICAL)->SetSteps(smallStepV, bigStepV);
    }
}


void
BasicPreviewView::_ScrollDownBigStep(void)
{
    float height = Bounds().Height();
    int value1 = ScrollBar(B_VERTICAL)->Value() + height;

    ScrollBar(B_VERTICAL)->SetValue(value1);
}


void
BasicPreviewView::_ScrollUpBigStep(void)
{
    int value1 = ScrollBar(B_VERTICAL)->Value() - Bounds().Height();
    ScrollBar(B_VERTICAL)->SetValue(value1);
}


inline float
BasicPreviewView::_PageHeight(const int& pageNumber)
{
    if (fColumnWidth == 0) {
        float zoom = Bounds().Width() / fEngine->Page(pageNumber)->Bounds().Width();
        return fEngine->Page(pageNumber)->Bounds().Height() * zoom;
    } else {
        float zoom = fColumnWidth / fEngine->Page(pageNumber)->Bounds().Width();
        return fEngine->Page(pageNumber)->Bounds().Height() * zoom;
    }
}


inline float
BasicPreviewView::_PageHeight(void)
{
    return _PageHeight(fCurrentPageNumber);
}


inline float
BasicPreviewView::_PageWidth(void)
{
    if (fColumnWidth == 0)
        return Bounds().Width();
    else
        return fColumnWidth;
}


inline int
BasicPreviewView::_Pages(void)
{
    if (fEngine == nullptr)
        return 0;

    return fEngine->PageCount();
}


void
BasicPreviewView::Pulse(void)
{
    if (fEngine == nullptr)
        return;

    if (fZoom != 1) {
        fEngine->MultiplyZoom(fZoom);
        fZoom = 1;
        Invalidate();
    }

    if (fAdaptCache) {
        _AdaptCache();
        fAdaptCache = false;
    }

    if (fScrollToCurrentPage == 2) {
        fScrollToCurrentPage = 0;
        _ShowPage(fCurrentPageNumber);
    }
}


inline void
BasicPreviewView::_ShowPage(const int& pageNumber)
{
    _SetScrollBarAtPage(pageNumber);
    Invalidate();
}


inline void
BasicPreviewView::_ShowPage(void)
{
    _ShowPage(fCurrentPageNumber);
}


inline void
BasicPreviewView::_SetScrollBarAtPage(int pageNumber)
{
    _AdaptScrollBarRange();
    ScrollBar(B_VERTICAL)->SetValue(fVMargin +
        (_PageHeight(pageNumber) + fVSpace) * _PageToRow(pageNumber));
}


void
BasicPreviewView::SetColumnWidth(const int32& width)
{
    if (fColumnWidth == width || fEngine == nullptr)
        return;
     
    // TODO: Not a good solution
    BBitmap* bitmap = fEngine->Page(fCurrentPageNumber);
    while (bitmap == nullptr) {
		++fCurrentPageNumber;
		if (fCurrentPageNumber >= fEngine->PageCount())
			return;
			
		bitmap = fEngine->Page(fCurrentPageNumber);
	}
	
	if (bitmap == nullptr)
		!out << "NULL" << endl;
		
    int page = fCurrentPageNumber;

    fColumnWidth = width;
    fPageHeight = _PageHeight();
  	
    
    if (width <= 0) {
        fZoom = Bounds().Width() / bitmap->Bounds().Width();
        fColumns = 1;
        _AdaptCache();
    } else {
        fZoom = fColumnWidth / bitmap->Bounds().Width();
        fColumns = (Bounds().Width() - 2*fHMargin + fHSpace) / (fColumnWidth + fHSpace);
        if (fColumns < 1)
        	fColumns = 1;
        	
        fAdaptCache = true;
    }
    _ShowPage(page);
}


void
BasicPreviewView::SelectPage(const int& pageNumber)
{
    fHighlightPageNumber = pageNumber;
   // _ShowPage(fHighlightPageNumber);
}


int
BasicPreviewView::_ConvertToRow(const BPoint& where)
{
    return (where.y - fVMargin) / (_PageHeight() + fVSpace);
}


void
BasicPreviewView::_AdaptCache(void)
{
    int cache;
    if (fColumnWidth == 0)
        cache = 16;
    else
        cache = 6;

    int size = cache * fColumns * (Bounds().Height() - 2*fVMargin) / (_PageHeight() + fVSpace);

    /*
    if (fColumnWidth == 0) {
        fZoom = Bounds().Width() / fEngine->Page(fCurrentPageNumber)->Bounds().Width();
    } else {
        fZoom = fColumnWidth / fEngine->Page(fCurrentPageNumber)->Bounds().Width();
    }
*/
    fEngine->MultiplyZoom(fZoom);
    fZoom = 1;

    fEngine->SetCacheSize(size, size);
}


void
BasicPreviewView::_SetColumns(const int& columns)
{
    if (fColumns == columns)
        return;

    fColumns = columns;
    fAdaptCache = true;
    _AdaptScrollBarRange();
}


BBitmap*
BasicPreviewView::_Page(const int& pageNumber)
{
    return fEngine->Page(pageNumber);
}


int
BasicPreviewView::_PageToRow(const int& pageNumber)
{
    if (fColumns < 1)
        throw std::range_error("fColumns < 1");
	
    return pageNumber / fColumns;
}
