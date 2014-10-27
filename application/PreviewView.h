/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PREVIEWVIEW_H
#define PREVIEWVIEW_H

#include <memory>
#include <vector>

#include <Bitmap.h>
#include <GroupView.h>
#include <OptionPopUp.h>
#include <Path.h>
#include <ScrollBar.h>
#include <String.h>

#include "BaseEngine.h"
#include "Debug.h"

class BasicPreviewView : public BView
{
public:
                    BasicPreviewView(void);

    virtual void    Draw(BRect updateRect);
    virtual	void    FrameResized(float newWidth, float newHeight);
    virtual void    MessageReceived(BMessage* message);
    virtual void    KeyDown(const char* bytes, int32 numBytes);
    virtual void 	MouseDown(BPoint where);
    virtual	void	MouseUp(BPoint where);
    virtual	void	MouseMoved(BPoint where, uint32 code,
                        const BMessage* dragMessage);
    virtual	void	ScrollTo(BPoint where);

            void    SetFile(BString file, BString fileType, BString& password);
            void    SetColumnWidth(const int32& width);
            void    SelectPage(const int& pageNumber);

    friend class PreviewView;

private:
            void        _AdaptScrollBarRange(void);
            void        _GoToPage(int pageNumber);
            void        _ScrollDownBigStep(void);
            void        _ScrollUpBigStep(void);
            int         _ConvertToPage(BPoint const& where);
    inline  int         _ConvertToRow(const BPoint& where);
    inline  float       _PageHeight(const int& pageNumber);
    inline  float       _PageHeight(void);
    inline  float       _PageWidth(void);
    inline  int         _Pages(void);
    inline  void        _ShowPage(const int& pageNumber);
    inline  void        _ShowPage(void);
    inline  void        _SetScrollBarAtPage(int pageNumber);
    inline  void        _AdaptCache(void);
    inline  void        _SetColumns(const int& columns);
    inline  BBitmap*    _Page(const int& pageNumber);
    inline  int         _PageToRow(const int& pageNumber);


    BaseEngine*						fEngine;
    BPoint	            			fOldMousePos;
    float               			fZoom;
    float               			fPageHeight;
    int                 			fHSpace;
    int                 			fVSpace;
    int                 			fHMargin;
    int                 			fVMargin;
    int                 			fColumns;
    int                 			fRows;
    int                 			fCurrentRow;

    int32               			fColumnWidth;
    uint32	            			fMouseButtons;

    int                 			fCurrentPageNumber;
    int                 			fHighlightPageNumber;
    bool                			fIsPanning;
    int                 			fScrollToCurrentPage;

    Debug               			out;
};


class PreviewView : public BGroupView
{
public:
                    	PreviewView(void);
    virtual void    	MessageReceived(BMessage* message);
    virtual void    	AttachedToWindow(void);
    virtual void		Show(void);
    virtual void		FileChanged(const BString& file, BString const& fileType,
    						BString& password);

            void    	SetFile(BString file, BString& password);
            void    	SelectPage(const int& pageNumber);
            
            
			BaseEngine*	Engine(void)
			{
				return fBasicPreviewView->fEngine;	
			}

private:
    BOptionPopUp*       fColumnOption;
    BasicPreviewView*   fBasicPreviewView;

    BScrollBar*         fVScrollBar;
    BScrollBar*         fHScrollBar;

    Debug               out;

    enum{
        FIT_WIDTH = 0, SMALL_COLUMN = 75, MEDIUM_COLUMN = 125, BIG_COLUMN = 200
    };

    enum{
        M_COLUMN_WIDTH = 'ic01'
    };
};

#endif
