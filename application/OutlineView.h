
/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef OUTLINEVIEW_H
#define OUTLINEVIEW_H

#include <memory>
#include <vector>


extern "C" {
    #include <mupdf/fitz.h>
}


#include <Bitmap.h>
#include <CheckBox.h>
#include <GroupView.h>
#include <OutlineListView.h>
#include <ScrollBar.h>
#include <String.h>
#include <TextControl.h>
#include <Window.h>

#include "BaseEngine.h"
#include "Debug.h"
#include "ImageButton.h"


class OutlineItem : public BListItem
{
public:
    OutlineItem(const BString& name, const int& pageNumber = 0,
    	uint32 outlineLevel = 0, bool expanded = true);
    ~OutlineItem();
    virtual void 		DrawItem(BView* owner, BRect itemRect, bool = false);
    
    		void 		SetName(const BString& name);
    		BString 	Name(void) { return fName;	}
    		
    		const int&	PageNumber(void) const 
    		{
    			return fPageNumber;
    		}
    		
    		void		SetPageNumber(const int& pageNumber)
    		{
    			fPageNumber = pageNumber;
    		}
    
private:
	BString 	fName;
	int			fPageNumber;
};


class OutlineListView : public BOutlineListView
{
public:
                    		OutlineListView(void);
	virtual status_t		Invoke(BMessage* message = nullptr);
	virtual void			MouseDown(BPoint where);
	virtual bool			InitiateDrag(BPoint point, int32 itemIndex,
								bool initialySelected);
	
			void			RemoveCurrentSelection(void);
			void			SetEngine(BaseEngine* engine);
			void			ReverseOrder(OutlineItem* super);
			void			ReverseOrder(void);
			void			MoveFirstToEnd(OutlineItem* super);
			void			Find(const BString& name);

private:
			OutlineItem*	_SelectedItem(void);
	inline 	void			_GoToPage(const int& pageNumber);
	
	BaseEngine*				fEngine;
	
	BString					fSearchString;
	int32					fStartSearchIndex;
	
	Debug					out;

};


class EditListItemWindow : public BWindow
{
public:
					EditListItemWindow(OutlineItem* item, const BString& name = "Edit");
						
	virtual	void	MessageReceived(BMessage* message);
	
			void	SetResponse(const int& code, BLooper* looper, BHandler* handler = nullptr);
	
	enum{	M_OK = 'io01'
		
		};

private:
	BTextControl*			fNameTC;
	BTextControl*			fPageTC;
	BCheckBox*				fGoToPageCB;
	OutlineItem * 			fItem;
	BLooper*				fTargetLooper;
	BHandler*				fTargetHandler;
	int						fCode;
	
	Debug					out;

};


class OutlineView : public BGroupView
{
public:
                    OutlineView(void);
	virtual	void	MessageReceived(BMessage* message);
	virtual void	AttachedToWindow(void);
	
	virtual	void	EngineChanged(BaseEngine* engine);					

                    	
	enum{   M_ADD_ITEM = 'ic01', M_DELETE_ITEM = 'id01',
			M_EDIT_ITEM = 'ie01', M_FIND_NEXT = 'if01',
			M_ITEM_CHANGED = 'ii01'
        };

private:
			int		_CurrentPageNumber(void);
			void	_ShowEditWindow(void);
			
    BScrollBar*         fVScrollBar;
    BScrollBar*         fHScrollBar;
    
    BTextControl*		fSearchTC;
 	
 	ImageButton* 		fEditItem;
 	ImageButton* 		fDeleteItem;
 	ImageButton* 		fAddItem;
 	ImageButton*		fFindNext;
 	   
    OutlineListView* 	fOutlineListView;
    Debug				out;
};

#endif
