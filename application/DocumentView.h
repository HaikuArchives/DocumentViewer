/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <memory>
#include <vector>

#include <GroupView.h>
#include <ScrollBar.h>
#include <String.h>

#include "BaseEngine.h"
#include "CircleMenuView.h"
#include "Debug.h"
#include "ImageButton.h"


class BasicDocumentView : public BView
{
public:
                    BasicDocumentView(BString filename, BString fileType,
                    	BString& password);
                    ~BasicDocumentView();

    virtual void    Draw(BRect updateRect);
    virtual	void    FrameResized(float newWidth, float newHeight);
    virtual void    KeyDown(const char* bytes, int32 numBytes);
    virtual void    MessageReceived(BMessage* message);
    virtual void 	MouseDown(BPoint where);
    virtual	void	MouseUp(BPoint where);
    virtual	void	MouseMoved(BPoint where, uint32 code,
                        const BMessage* dragMessage);
    virtual	void	ScrollTo(BPoint where);
    virtual void    AllAttached(void);
    virtual void    Pulse();
    
    		void	SetFile(const BString& file, BString const& fileType,
    					BString& password);
    
    friend class 	DocumentView;

    enum {
    	M_INIT = 'ii01'
    };

private:
            void        _GoToPage(int32 pageNumber);
            void        _NotifyPageChange(const int& pageNumber);
            void        _SetScrollBarAtPage(int pageNumber);
            void        _ShowPage(const int& pagenumber);
            void        _ShowCurrentPage(void);
            void        _AdaptScrollBarRange(void);
            void        _AdaptCache(void);
            void        _FitPageHeight(void);
            void        _FitPageWidth(void);
            void        _ScrollDownBigStep(void);
            void        _ScrollDownSmallStep(void);
            void        _ScrollUpBigStep(void);
            void        _ScrollUpSmallStep(void);

         //   BRect       _PageFrame(const int& pageNumber);
            BRect       _PageFrame(BBitmap* bitmap, const int& pageNumber);

    std::unique_ptr<BaseEngine>    	fEngine;
    CircleMenuView*     			fCircleMenu;


    BPoint	            			fOldMousePos;
    uint32	            			fMouseButtons;
    uint32              			fCurrentFit;

    float               			fZoomFactor;
    float               			fZoomStep;
    float               			fSmallScrollStep;

    int                 			fCurrentPageNumber;
    bool                			fIsPanning;
    bool               				fIsHPanning;
    bool                			fIsVPanning;
    bool                			fNeedsResize;
    bool							fIsPrinting;
    bool							fHighlightUnderText;
    
    std::tuple<int32, BRect>		fHighlightRect;

    Debug               			out;
};


class DocumentView : public BGroupView
{
public:
                    	DocumentView(BString filename, BString fileType, BString& password);
    virtual void    	MessageReceived(BMessage* message);
    virtual void		FileChanged(const BString& file, BString const& fileType,
    						BString& password);
    
    		void		Print(void);
    		int     	PageCount(void);
    			
    		BBitmap*	ImagePage(int const& pageNumber, float const& height = 500);
    		
    		
   	
  	std::unique_ptr<BBitmap>
  						Cover(float const& height = 500);
    
    virtual void    	MakeFocus(bool focus);        
            
    const 	int&		CurrentPageNumber(void);

private:
    BasicDocumentView*      fBasicDocumentView;
    BScrollBar*             fVScrollBar;
    BScrollBar*             fHScrollBar;

    Debug                   out;
};

#endif
