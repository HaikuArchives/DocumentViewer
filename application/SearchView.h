
/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <vector>

extern "C" {
    #include <mupdf/fitz.h>
}

#include <Bitmap.h>
#include <Button.h>
#include <CheckBox.h>
#include <GroupView.h>
#include <ListView.h>
#include <ScrollBar.h>
#include <String.h>
#include <TextControl.h>

#include <columnlistview/ColumnListView.h>
#include "Debug.h"
#include "BaseEngine.h"


class SearchListView : public BColumnListView
{
public:
					SearchListView(void);
	virtual	void	ItemInvoked(void);
	
			void	Clear(void);
	
	friend class SearchView;
	
private:

	std::vector<BRect>	fRectVec;
	Debug			out;
	
};


class SearchView : public BGroupView
{
public:
                    SearchView(void);
    virtual	void	MessageReceived(BMessage* message);
    virtual void	AttachedToWindow(void);
    
 	virtual	void	EngineChanged(BaseEngine* engine);
 	
 	enum{
 			M_START = 'is01', M_TEXT_CHANGED = 'it01'
        };

private:
    BScrollBar*         fVScrollBar;
    BScrollBar*         fHScrollBar;
    
    BButton*			fStartButton;
    BCheckBox*			fWholeWordCB;
    BCheckBox*			fMatchCaseCB;
    BTextControl*		fSearchTC;
    
    SearchListView*		fSearchListView;
    
    BaseEngine*			fEngine;
};

#endif
