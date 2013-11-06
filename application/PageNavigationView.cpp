/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "PageNavigationView.h"

#include <stdlib.h>

#include <Box.h>
#include <Font.h>
#include <LayoutBuilder.h>

#include "ImageButton.h"

using namespace std;

PageNavigationView::PageNavigationView(unique_ptr<BMessage> message)
	:
    BGroupView("page_navigation_view", B_HORIZONTAL, 0),
    BInvoker(message.release(), NULL, NULL),
    fMaxValue(10000)
{
	SetFlags(Flags() | B_FRAME_EVENTS);
	unique_ptr<BMessage> pageNumberMsg(new BMessage(M_GOTO_PAGE));
	
    fPageNC = new NumberControl(move(pageNumberMsg));
    fTotalPagesSV = new BStringView("pages_sv", "");
    
    BGroupLayout* layout1;
    layout1 = BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
        .Add(fPageNC)
        .AddGroup(B_VERTICAL, 0)
        	.AddGlue(0)
        	.Add(fTotalPagesSV)
        	.AddGlue(0)
        .End()
    ;
    
    BBox* box1 = new BBox("box1");
    box1->SetBorder(B_FANCY_BORDER);
    box1->AddChild(layout1->View());
    
    fBackButton = new ImageButton("back",
        new BMessage(M_GOTO_PREVIOUS_PAGE), 0.3, 1);
        
    fNextButton = new ImageButton("next",
        new BMessage(M_GOTO_NEXT_PAGE), 0.3, 1);
    
    BLayoutBuilder::Group<>(this)
    	.Add(fBackButton)
		.Add(box1)
		.Add(fNextButton)
	;
}


void
PageNavigationView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case M_GOTO_PAGE:
			SetValue(fPageNC->Value());
			Invoke();
			break;
			
		case M_GOTO_NEXT_PAGE:
			if (fNextButton->PushedLong())
				SetValue(fMaxValue);
			else
				SetValue(fPageNC->Value() + 1);
			
			Invoke();
			break;
		
		case M_GOTO_PREVIOUS_PAGE:
			if (fBackButton->PushedLong())
				SetValue(1);
			else
				SetValue(fPageNC->Value() - 1);
			
			Invoke();
			break;
		
        default:
            BGroupView::MessageReceived(message);
            break;
    }
}


void
PageNavigationView::AttachedToWindow(void)
{
	fNextButton->SetTarget(this);
    fBackButton->SetTarget(this);
    fPageNC->SetTarget(this);
	BGroupView::AttachedToWindow();
}


void
PageNavigationView::FrameResized(float newWidth, float newHeight)
{
	fBackButton->SetExplicitMinSize(BSize(newHeight, newHeight));
    fBackButton->SetExplicitMaxSize(BSize(newHeight, newHeight));
	fNextButton->SetExplicitMinSize(BSize(newHeight, newHeight));
    fNextButton->SetExplicitMaxSize(BSize(newHeight, newHeight));
	
	BGroupView::FrameResized(newWidth, newHeight);	
}


void
PageNavigationView::MakeFocus(bool focus)
{
	BGroupView::MakeFocus(focus);
}


void
PageNavigationView::SetValue(int value)
{
	if (value < 1)
		value = 1;
	else if (value > fMaxValue)
		value = fMaxValue;
	
	fPageNC->SetValue(value);	
}

void
PageNavigationView::SetMaxValue(int const& value)
{
	fMaxValue = value;
	BString str = " / ";
	str << fMaxValue;
	fTotalPagesSV->SetText(str);
}


int
PageNavigationView::Value(void)
{
	return fPageNC->Value();
}
