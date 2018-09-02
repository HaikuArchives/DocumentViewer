/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "SearchView.h"

#include <LayoutBuilder.h>

#include <ColumnTypes.h>
#include "Flags.h"
#include "Messages.h"


SearchListView::SearchListView(void)
	:
	BColumnListView("search_list_view", 0)
{
	
}


void
SearchListView::ItemInvoked(void)
{
	BIntegerField* pageField = (BIntegerField*)CurrentSelection()->GetField(0);

	if (pageField == nullptr)
        return;
	
	BMessage msg(MSG_HIGHLIGHT_RECT);
	msg.AddInt32("page", pageField->Value() - 1);
	msg.AddRect("rect", fRectVec[IndexOf(CurrentSelection())]);
    Window()->PostMessage(&msg);
    
    BColumnListView::ItemInvoked();	
}


void
SearchListView::Clear(void)
{
	fRectVec.clear();
	BColumnListView::Clear();
	ScrollTo(BPoint(0, 0));	
}


SearchView::SearchView(void)
	:
    BGroupView("find", B_VERTICAL, 0)
{
	fSearchTC   = new BTextControl("", "", new BMessage(M_TEXT_CHANGED));
	fStartButton = new BButton("Start", new BMessage(M_START));
	fStartButton->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fMatchCaseCB = new BCheckBox("Match Case");
	fWholeWordCB = new BCheckBox("Whole Word");
	
	fSearchListView = new SearchListView();
	
	BColumn* pageColumn = new BIntegerColumn("pg.", 45, 0, 1000);
	BColumn* contextColumn = new BStringColumn("Context", 500, 0, 1000, true);
	fSearchListView->AddColumn(pageColumn, 0);
	fSearchListView->AddColumn(contextColumn, 1);
	fSearchListView->SetSortColumn(pageColumn, true, true);
	fSearchListView->SetSortingEnabled(false);
	

    fVScrollBar = new BScrollBar("v_scrollbar",
                    fSearchListView, 0, 2000, B_VERTICAL);
    fHScrollBar = new BScrollBar("h_scrollbar",
                    fSearchListView, 0, 100, B_HORIZONTAL);

    BLayoutBuilder::Group<>(this)
    	.Add(fSearchTC)
    	.AddGroup(B_HORIZONTAL, 0)
    		.Add(fStartButton)
    		.AddGlue(0)
    	.End()
    	.Add(fMatchCaseCB)
    	.Add(fWholeWordCB)
    	.Add(fSearchListView)
    .End()
	;
}


void
SearchView::AttachedToWindow(void)
{
	fStartButton->SetTarget(this);
	fSearchTC->SetTarget(this);
}


void
SearchView::MessageReceived(BMessage* message)
{
    switch (message->what) {
        case M_START:
        {
        	fSearchTC->MakeFocus(false);
        	fSearchListView->Clear();
        	int32 flag = 0;
        	
        	if (fMatchCaseCB->Value() != false)
        		flag |= SEARCH_MATCH_CASE;
        		
        	if (fWholeWordCB->Value() != false)
        		flag |= SEARCH_WHOLE_WORD;
        	
        	fEngine->FindString(fSearchTC->Text(), Window(), this, flag);
        	break;
        }
        
        case M_TEXT_CHANGED:
        {
        	fSearchListView->Clear();
        	int32 flag = 0;
        	
        	if (fMatchCaseCB->Value() != false)
        		flag |= SEARCH_MATCH_CASE;
        		
        	if (fWholeWordCB->Value() != false)
        		flag |= SEARCH_WHOLE_WORD;
        	
        	fEngine->FindString(fSearchTC->Text(), Window(), this, flag);
        	break;
        }
        	
      	case MSG_SEARCH_RESULT:
      	{
      		int32 page = 0;
      		message->FindInt32("page", &page);
      		BString context;
      		message->FindString("context", &context);
      		BRect rect;
      		message->FindRect("rect", &rect);
      		fSearchListView->fRectVec.push_back(rect);
      		
      		BRow*	row = new BRow();
			row->SetField(new BIntegerField(page + 1), 0);
			row->SetField(new BStringField(context), 1);
			fSearchListView->AddRow(row);
      		
      		break;
      	}
        
        default:
        	BGroupView::MessageReceived(message);
        	break;
    }
}


void
SearchView::EngineChanged(BaseEngine* engine)
{
	if (engine == nullptr)
		return;
	
	fEngine = engine;	
	
	fSearchListView->Clear();		
}
