/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "DocumentView.h"

#include <LayoutBuilder.h>
#include <Button.h>

#include "DJVUEngine.h"
#include "ImageButton.h"
#include "Messages.h"
#include "PDFEngine.h"
#include "PrintingWindow.h"

using namespace std;


DocumentView::DocumentView(BString filename, BString fileType,
	BString& password)
		:
    	BGroupView("document_view", B_VERTICAL, 0)
{
    fBasicDocumentView = new BasicDocumentView(filename, fileType, password);

    fVScrollBar = new BScrollBar("v_scrollbar", fBasicDocumentView,
        0, 0, B_VERTICAL);
    fHScrollBar = new BScrollBar("h_scrollbar", fBasicDocumentView,
        0, 0, B_HORIZONTAL);

    BLayoutBuilder::Group<>(this)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fBasicDocumentView)
            .Add(fVScrollBar)
		.End()
        .AddGroup(B_HORIZONTAL, 0)
            .Add(fHScrollBar)
            .AddStrut(13)
        .End()
	;
}


void
DocumentView::MakeFocus(bool focus)
{
	fBasicDocumentView->MakeFocus(true);
}


const int&
DocumentView::CurrentPageNumber(void)
{
	return fBasicDocumentView->fCurrentPageNumber;
}


void
DocumentView::Print(void)
{
	new PrintingWindow(Window(), this);	
}


std::unique_ptr<BBitmap>
DocumentView::Cover(float const& height)
{
	return fBasicDocumentView->fEngine->RenderBitmap(0,0, height, 0);
}


void
DocumentView::FileChanged(const BString& file, BString const& fileType,
	BString& password)
{
	fBasicDocumentView->SetFile(file, fileType, password);
}


int
DocumentView::PageCount(void)
{
	if (fBasicDocumentView->fEngine == nullptr)
		return 1;
		
    return fBasicDocumentView->fEngine->PageCount();
}


void
DocumentView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_NO_FIT:
        case MSG_FIT_PAGE_HEIGHT:
        case MSG_FIT_PAGE_WIDTH:
        case MSG_NO_ZOOM:
        case MSG_ZOOM_OUT:
        case MSG_ZOOM_IN:
        case MSG_GOTO_PAGE:
        case MSG_HIGHLIGHT_RECT:
            Window()->PostMessage(message, fBasicDocumentView);
            break;

        default:
            BView::MessageReceived(message);
    }
}


BasicDocumentView::BasicDocumentView(BString filename, BString fileType,
	BString& password)
    	:
    	BView(BRect(), "basic_document_view", B_FOLLOW_ALL,
            B_WILL_DRAW | B_DRAW_ON_CHILDREN | B_FULL_UPDATE_ON_RESIZE
                | B_FRAME_EVENTS | B_NAVIGABLE),
    	fOldMousePos(0, 0),
    	fMouseButtons(0),
    	fCurrentFit(MSG_NO_FIT),
    	fZoomFactor(1),
    	fZoomStep(0.2),
    	fSmallScrollStep(25),
    	fCurrentPageNumber(0),
    	fIsPanning(false),
    	fIsHPanning(false),
    	fIsVPanning(false),
    	fNeedsResize(false),
    	fIsPrinting(false),
    	fHighlightUnderText(false),
    	fHighlightRect(make_tuple(-1, BRect()))
{
    SetFlags(Flags() | B_PULSE_NEEDED);
    SetViewColor(B_TRANSPARENT_32_BIT);
    SetDrawingMode(B_OP_ALPHA);
    SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
    
    ResizeTo(50, 50);
	
	if (filename.Length() == 0)
		fEngine = nullptr;
	else if (fileType == "pdf")
   		fEngine = unique_ptr<BaseEngine>(new PDFEngine(filename, password));
   	else if (fileType == "djvu")
   		fEngine = unique_ptr<BaseEngine>(new DJVUEngine(filename, password));
   	else
   		fEngine = nullptr;
   		
//    !out << "Prop: " << fEngine->GetProperty("Title") << std::endl;
    /*
    fCircleMenu = new CircleMenuView(BRect(0,0,200,200));
    AddChild(fCircleMenu);
    fCircleMenu->AddChild(new ImageButton("fullscreen",
        new BMessage(MSG_FULLSCREEN), 0.3, 2));
    fCircleMenu->AddChild(new ImageButton("zoom_in",
    new BMessage(MSG_ZOOM_IN), 0.3, 2));
    fCircleMenu->AddChild(new ImageButton("zoom_out",
        new BMessage(MSG_ZOOM_OUT), 0.3, 2));
    fCircleMenu->AddChild(new ImageButton("quit",
        new BMessage(MSG_APP_QUIT), 0.3, 2));
    fCircleMenu->AddChild(new ImageButton("fit_page_width",
        new BMessage(MSG_FIT_PAGE_WIDTH), 0.3, 2));
    fCircleMenu->AddChild(new ImageButton("fit_page_height",
        new BMessage(MSG_FIT_PAGE_HEIGHT), 0.3, 2));

    Invalidate();
    */
}


BasicDocumentView::~BasicDocumentView()
{

}


void
BasicDocumentView::AllAttached(void)
{
    BView::AllAttached();
    Window()->SetPulseRate(1E6);
}



void
BasicDocumentView::SetFile(const BString& filename, BString const& fileType,
	BString& password)
{
	if (filename.Length() == 0)
		return;
	
	if (fileType == "pdf")
		fEngine = unique_ptr<BaseEngine>(new PDFEngine(filename, password));
	else if (fileType == "djvu")
		fEngine = unique_ptr<BaseEngine>(new DJVUEngine(filename, password));
	else
		return;
		
	fHighlightUnderText = fEngine->HighlightUnderText();
	
	_ShowCurrentPage();
}


void
BasicDocumentView::Pulse(void)
{
    if (fEngine == nullptr)
        return;

    if (fNeedsResize) {
        fNeedsResize = false;
        switch(fCurrentFit) {
            case MSG_FIT_PAGE_HEIGHT:
                _FitPageHeight();
                break;

            case MSG_FIT_PAGE_WIDTH:
                _FitPageWidth();
                break;

            case MSG_NO_FIT:
                break;

            default:
                break;
        }
    }
}


void
BasicDocumentView::MessageReceived(BMessage* message)
{	
	if (fEngine == nullptr) {
		BView::MessageReceived(message);
		return;	
	}
	
	switch (message->what) {
        case MSG_NO_FIT:
            fCurrentFit = message->what;
            break;

        case MSG_FIT_PAGE_HEIGHT:
            _FitPageHeight();
            break;

        case MSG_FIT_PAGE_WIDTH:
            _FitPageWidth();
            break;

        case MSG_ZOOM_IN:
            fCurrentFit = MSG_NO_FIT;
            fZoomFactor += fZoomStep;
            
            fEngine->SetZoom(fZoomFactor);
            _AdaptCache();
            _ShowCurrentPage();
            break;

        case MSG_ZOOM_OUT:
            fCurrentFit = MSG_NO_FIT;
            fZoomFactor -= fZoomStep;
            if (fZoomFactor < 0.1)
                fZoomFactor = 0.1;
		
            fEngine->SetZoom(fZoomFactor);
            _AdaptCache();
            _ShowCurrentPage();
            break;
           
     	case MSG_NO_ZOOM:		
     		fCurrentFit = MSG_NO_FIT;
     		fZoomFactor = 1;
     		fEngine->SetZoom(fZoomFactor);
            _AdaptCache();
            _ShowCurrentPage();
     		break;
     	
        case MSG_GOTO_PAGE:
        {
            MakeFocus(true);

            int32 value;
            message->FindInt32("info", &value);

            if (value < 0) {
                fCurrentPageNumber = 0;
            } else if (value >= fEngine->PageCount()) {
                fCurrentPageNumber = fEngine->PageCount() - 1;
            } else {
                fCurrentPageNumber = value;
            }

            _ShowCurrentPage();
            break;
        }
        
        case MSG_HIGHLIGHT_RECT:
        {
        	int32 page = 0;
        	message->FindInt32("page", &page);
        	BRect rect;
        	message->FindRect("rect", &rect);
        	
        	auto frame = _PageFrame(fEngine->Page(page), page);
        	
        	rect.left 	*= fZoomFactor / 2;
        	rect.right 	*= fZoomFactor / 2;
        	rect.top 	*= fZoomFactor / 2;
        	rect.bottom *= fZoomFactor / 2;
        	
        	rect.OffsetBy(frame.LeftTop());
        		
        	rect.left 	-= 2;
        	rect.right 	+= 2;
        	rect.top	-= 2;
        	rect.bottom += 2;
        	
        	get<0>(fHighlightRect) = page;
        	get<1>(fHighlightRect) = rect;
 
        	ScrollTo(rect.LeftTop() + BPoint(-Bounds().Width() / 2,
        		-Bounds().Height() / 2));
   			Invalidate();
        	break;	
        }

		default:
			BView::MessageReceived(message);
            break;
	}
}


void
BasicDocumentView::_FitPageHeight(void)
{
    fCurrentFit = MSG_FIT_PAGE_HEIGHT;
    fZoomFactor = fZoomFactor * Bounds().Height() /
        fEngine->PageMediabox(fCurrentPageNumber).Height();
    fEngine->SetZoom(fZoomFactor);
    _AdaptCache();
    _ShowCurrentPage();
}


void
BasicDocumentView::_FitPageWidth(void)
{
    fCurrentFit = MSG_FIT_PAGE_WIDTH;
    fZoomFactor = fZoomFactor * Bounds().Width() /
        fEngine->PageMediabox(fCurrentPageNumber).Width();
    fEngine->SetZoom(fZoomFactor);
    _AdaptCache();
    _ShowCurrentPage();
}


void
BasicDocumentView::ScrollTo(BPoint where)
{
	if (fEngine == nullptr) {
		BView::ScrollTo(where);	
		return;
	}
	
	BBitmap* bitmap = fEngine->Page(fCurrentPageNumber);
    int page = where.y / bitmap->Bounds().Height();

    if (page != fCurrentPageNumber) {
        fCurrentPageNumber = page;
        _NotifyPageChange(fCurrentPageNumber);
        _AdaptScrollBarRange();
    }

    BView::ScrollTo(where);
}


void
BasicDocumentView::MouseDown(BPoint where)
{
    MakeFocus(true);
    GetMouse(&fOldMousePos, &fMouseButtons);
    fIsHPanning = true;
    fIsVPanning = true;
    fIsPanning  = true;
    BView::MouseDown(where);
}


void
BasicDocumentView::MouseUp(BPoint where)
{

/*
    if (fMouseButtons == B_SECONDARY_MOUSE_BUTTON) {
        fCircleMenu->MoveTo(where - BPoint(fCircleMenu->Bounds().Width()/2,
            fCircleMenu->Bounds().Height()/2));
        fCircleMenu->Show();
    }
*/

    fIsPanning  = false;
    fIsHPanning = false;
    fIsVPanning = false;

    BView::MouseUp(where);
}


void
BasicDocumentView::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
    switch (code) {
		case B_EXITED_VIEW:
            fIsPanning  = false;
            fIsHPanning = false;
            fIsVPanning = false;
            break;

		default:
            break;
	}

    if (fIsPanning == true) {
        BPoint delta = fOldMousePos - where;

        if (fIsVPanning)
            ScrollBar(B_VERTICAL)->SetValue(
                ScrollBar(B_VERTICAL)->Value() + delta.y);

        if (fIsHPanning)
            ScrollBar(B_HORIZONTAL)->SetValue(
                ScrollBar(B_HORIZONTAL)->Value() + delta.x);
    }

    BView::MouseMoved(where, code, dragMessage);
}


inline void
BasicDocumentView::_GoToPage(int32 pageNumber)
{
    BMessage msg(MSG_GOTO_PAGE);
    msg.AddInt32("info", pageNumber);
    Window()->PostMessage(&msg);
}


inline void
BasicDocumentView::_NotifyPageChange(const int& pageNumber)
{
    BMessage msg(MSG_ACTIVE_PAGE);
    msg.AddInt32("info", pageNumber);
    Window()->PostMessage(&msg);
}


void
BasicDocumentView::KeyDown(const char* bytes, int32 numBytes)
{
	if (fEngine == nullptr) {
		BView::KeyDown(bytes, numBytes);
		return;
	}
		
	if (numBytes == 1) {
		switch (bytes[0]) {
            case '+':
                Window()->PostMessage(MSG_ZOOM_IN);
                break;

            case '-':
                Window()->PostMessage(MSG_ZOOM_OUT);
                break;

            case B_UP_ARROW: case B_LEFT_ARROW:
                if (modifiers() & B_SHIFT_KEY)
                    _ScrollUpBigStep();
                else if (modifiers() & B_CONTROL_KEY)
                    Window()->PostMessage(MSG_ZOOM_OUT);
                else
                    _ScrollUpSmallStep();

                break;

			case B_DOWN_ARROW: case B_RIGHT_ARROW:
                if (modifiers() & B_SHIFT_KEY)
                    _ScrollDownBigStep();
                else if (modifiers() & B_CONTROL_KEY)
                    Window()->PostMessage(MSG_ZOOM_IN);
                else
                    _ScrollDownSmallStep();
                break;

            case B_PAGE_UP:
                _ScrollUpBigStep();
				break;

            case B_PAGE_DOWN:
                _ScrollDownBigStep();
                break;

            case B_HOME:
                _GoToPage(0);
                break;

            case B_END:
                _GoToPage(fEngine->PageCount() - 1);
                break;


    	  	default:
				BView::KeyDown(bytes, numBytes);
       		  	break;
        }
    }
}


void
BasicDocumentView::Draw(BRect updateRect)
{
	if (fIsPrinting)
		return;

	BRect bounds = Bounds();

	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(bounds);

	if (fEngine == nullptr) {
		BView::Draw(updateRect);
		return;
	}


	SetHighColor(0, 0, 0, 255);

	BBitmap* bitmap;
	BPoint where(0, 0);
	BRect frame;

	bitmap = fEngine->Page(fCurrentPageNumber);

	int upperbound = bounds.Height() /
		bitmap->Bounds().Height();
	upperbound = upperbound + fCurrentPageNumber + 2;
	if (upperbound > fEngine->PageCount())
		upperbound = fEngine->PageCount();

	bool hasDrawn = false;
	for (int i = fCurrentPageNumber; i < upperbound; ++i) {
		bitmap = fEngine->Page(i);
		frame = _PageFrame(bitmap, i);

		if (frame.Intersects(updateRect)) {
			hasDrawn = true;

			DrawBitmapAsync(bitmap, frame.LeftTop());
			StrokeRect(frame);

			if (i == get<0>(fHighlightRect)) {
				SetHighColor(0, 0, 255, 60);
				//SetHighColor(255, 255, 0, 80);
				FillRect(get<1>(fHighlightRect));
				SetHighColor(0, 0, 0, 255);
				StrokeRect(get<1>(fHighlightRect));
			}
		} else if (hasDrawn) {
			break;
		}
	}
}


inline void
BasicDocumentView::_SetScrollBarAtPage(int pageNumber)
{
	if (fEngine == nullptr)
		return;

	BBitmap* bitmap = fEngine->Page(pageNumber);

	_AdaptScrollBarRange();
	ScrollBar(B_VERTICAL)->SetValue(bitmap->Bounds().Height() * pageNumber);
}


inline void
BasicDocumentView::_AdaptCache(void)
{
	if (fEngine == nullptr)
		return;

	BBitmap* bitmap = fEngine->Page(fCurrentPageNumber);

	int cache = 16;

	int size = abs(floor(cache * Bounds().Height() /
		bitmap->Bounds().Height()));

	if (size > 50)
		size  = 50;

	fEngine->SetCacheSize(size, size);
}


inline void
BasicDocumentView::_AdaptScrollBarRange(void)
{
	if (fEngine == nullptr)
        return;
        
 	BBitmap* bitmap = fEngine->Page(fCurrentPageNumber);
		
/*
    if (fCircleMenu->IsHidden() == false)
        fCircleMenu->Hide();
*/

    ScrollBar(B_VERTICAL)->SetRange(0,
        bitmap->Bounds().Height() *
            fEngine->PageCount() - Bounds().Height());

    ScrollBar(B_HORIZONTAL)->SetRange(0,
        fEngine->Page(fCurrentPageNumber)->Bounds().Width() - Bounds().Width());

    float bigStepV = std::min(Bounds().Height(),
        fEngine->Page(fCurrentPageNumber)->Bounds().Height());
    float smallStepV = Bounds().Height() / 27;
    ScrollBar(B_VERTICAL)->SetSteps(smallStepV, bigStepV);

}


inline void
BasicDocumentView::_ShowPage(const int& pageNumber)
{
	if (fEngine == nullptr)
		return; 
		
    _SetScrollBarAtPage(pageNumber);
    Invalidate();
}


inline void
BasicDocumentView::_ShowCurrentPage(void)
{
    _ShowPage(fCurrentPageNumber);
}


void
BasicDocumentView::FrameResized(float newWidth, float newHeight)
{
    BView::FrameResized(newWidth, newHeight);

    // keep circle menu at the same global position
    /*
    if (fCircleMenu->IsHidden() == false)
        ;
*/

  	BBitmap* bitmap = fEngine->Page(fCurrentPageNumber);
    
    fNeedsResize = true;

    ScrollBar(B_VERTICAL)->SetRange(0,
        bitmap->Bounds().Height() *
            (fEngine->PageCount()) - newHeight);
}


/*
BRect
BasicDocumentView::_PageFrame(const int& pageNumber)
{
    BPoint where;
    BRect frame = fEngine->Page(pageNumber)->Bounds();
    where.y = frame.Height() * pageNumber;
    where.x = (Bounds().Width() - frame.Width()) / 2;
    if (where.x < 0)
        where.x = 0;

    return frame.OffsetBySelf(where);
}
*/


BRect
BasicDocumentView::_PageFrame(BBitmap* bitmap, const int& pageNumber)
{
    BPoint where;
    BRect frame = bitmap->Bounds();
    where.y = frame.Height() * pageNumber;
    where.x = (Bounds().Width() - frame.Width()) / 2;
    if (where.x < 0)
        where.x = 0;

    return frame.OffsetBySelf(where);
}


void
BasicDocumentView::_ScrollDownBigStep(void)
{
	BBitmap* bitmap = fEngine->Page(fCurrentPageNumber + 1);
	
    float height = Bounds().Height();
    int value1 = ScrollBar(B_VERTICAL)->Value() + height;
    int value2 = bitmap->Bounds().Height() * (fCurrentPageNumber + 1);
    if (value1 >= value2)
        ScrollBar(B_VERTICAL)->SetValue(value2);
    else if ((value2 - value1) < height)
        ScrollBar(B_VERTICAL)->SetValue(value2 - height);
    else
        ScrollBar(B_VERTICAL)->SetValue(value1);
}


inline void
BasicDocumentView::_ScrollDownSmallStep(void)
{
    ScrollBar(B_VERTICAL)->SetValue(ScrollBar(B_VERTICAL)->Value() +
        fSmallScrollStep);
}


void
BasicDocumentView::_ScrollUpBigStep(void)
{
	if (fEngine == nullptr)
		return;
	
	BBitmap* bitmap = fEngine->Page(fCurrentPageNumber);
		
    float height = Bounds().Height();
    int value1 = ScrollBar(B_VERTICAL)->Value() - height;
    int value2 = bitmap->Bounds().Height() * (fCurrentPageNumber);
    if ((value2 - ScrollBar(B_VERTICAL)->Value()) == 0) {  
    	BBitmap* bitmapBefore = fEngine->Page(fCurrentPageNumber - 1);
        value2 = bitmapBefore->Bounds().Height() * (fCurrentPageNumber - 1);
    }

    if (value1 < value2)
        ScrollBar(B_VERTICAL)->SetValue(value2);
    else
        ScrollBar(B_VERTICAL)->SetValue(value1);
}


inline void
BasicDocumentView::_ScrollUpSmallStep(void)
{
	if (fEngine == nullptr)
		return;
	
    ScrollBar(B_VERTICAL)->SetValue(ScrollBar(B_VERTICAL)->Value() -
        fSmallScrollStep);
}


