/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "OutlineView.h"

#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>

#include "ImageButton.h"
#include "MainWindow.h"
#include "Messages.h"

using namespace std;


EditListItemWindow::EditListItemWindow(OutlineItem* item, const BString& name)
		:
		BWindow(BRect(0, 0, 300, 100), name, B_MODAL_WINDOW_LOOK,
        	B_FLOATING_ALL_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS
            	| B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE
            	| B_NOT_CLOSABLE
            	/*| B_OUTLINE_RESIZE*/
           	/* | B_WILL_ACCEPT_FIRST_CLICK*/),
         fCode(0)
{
	fNameTC = new BTextControl("Name ", "", nullptr);
	fNameTC->SetText(item->Name());
	fPageTC = new BTextControl("Page ", "", nullptr);
	BString str;
	str << item->PageNumber() + 1;
	
	fPageTC->SetText(str);
	
	BButton* okButton = new BButton("ok_button", "OK", new BMessage(M_OK));
	SetDefaultButton(okButton);
	
	fGoToPageCB = new BCheckBox("Go to page ");
    fGoToPageCB->SetValue(1);

    BGroupLayout* layout;
    layout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
        .SetInsets(5, 5, 5, 5)
        .Add(fGoToPageCB)
    ;

    BBox* box1 = new BBox("box1");
    box1->SetBorder(B_FANCY_BORDER);
    box1->AddChild(layout->View());

	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 10)
        .SetInsets(10, 10, 10, 10)
        .Add(fNameTC)
        .Add(fPageTC)
        //.Add(box1)
        .AddGlue(0)
        .AddGroup(B_HORIZONTAL, 0)
        	.AddGlue(0)
        	.Add(okButton)
        .End()
   	.End();
	
	fNameTC->MakeFocus();
	fItem = item;
}


void
EditListItemWindow::SetResponse(const int& code, BLooper* looper, BHandler* handler)
{
	fTargetLooper = looper;
	fTargetHandler = handler;
	fCode = code;
}


void
EditListItemWindow::MessageReceived(BMessage* message)
{
    switch (message->what) {
        case M_OK:
        {
            fItem->SetName(fNameTC->Text());
            fItem->SetPageNumber(atoi(fPageTC->Text()) - 1);
            fTargetLooper->PostMessage(fCode, fTargetHandler);
            Close();
            
            break;
        }
        
        
		default:
            BWindow::MessageReceived(message);
            break;
	}
}


OutlineItem::OutlineItem(const BString& name, const int& pageNumber,
	uint32 outlineLevel, bool expanded)
		:
    	BListItem(),
    	fName(name),
    	fPageNumber(pageNumber)
{
}


OutlineItem::~OutlineItem()
{
}


void
OutlineItem::SetName(const BString& name)
{
	fName = name;
}


void
OutlineItem::DrawItem(BView *owner, BRect itemRect, bool complete)
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


OutlineListView::OutlineListView(void)
    :
    BOutlineListView("outline_list_view"),
    fStartSearchIndex(0)
{

}


void
OutlineListView::ReverseOrder(OutlineItem* super)
{
	if (super == nullptr)
		return;
		
	int32 end = CountItemsUnder(super, true) - 1;
	if (end < 1)
		return;

	for (int start = 0; start < end; ++start, --end)
		SwapItems(FullListIndexOf(ItemUnderAt(super, true, start)),
			FullListIndexOf(ItemUnderAt(super, true, end)));
}


void
OutlineListView::MoveFirstToEnd(OutlineItem* super)
{
	// move the element fromt the front to the end of the list 
	int32 transpositions = CountItemsUnder(super, true) - 1;
	for (int i = 0; i < transpositions; ++i)
		SwapItems(FullListIndexOf(ItemUnderAt(super, true, i)),
			FullListIndexOf(ItemUnderAt(super, true, i + 1)));
}

void
OutlineListView::MouseDown(BPoint where)
{
	int32 idx =  CurrentSelection();
	BOutlineListView::MouseDown(where);
	
	if (CurrentSelection() == idx) {
		DeselectAll();
	}
}


void
OutlineListView::SetEngine(BaseEngine* engine)
{
	if (engine == nullptr)
		return;
		
	MakeEmpty();
	fEngine = engine;
	fEngine->WriteOutline(this);
	ReverseOrder();
}


void
OutlineListView::ReverseOrder(void)
{
	int32 size = FullListCountItems();
	
	for (int i = 0; i < size; ++i)
		ReverseOrder(FullListItemAt(i));
}


void
OutlineListView::Find(const BString& name)
{
	if (name.Length() == 0)
		return;
		
	if (name != fSearchString) {
		fStartSearchIndex = 0;
		fSearchString = name;	
	}
	
	int32 size = FullListCountItems();
	OutlineItem* item;
	
	for (int j = 0; j < 2; ++j) {
		for (int i = fStartSearchIndex; i < size; ++i) {
			item = FullListItemAt(i); 
			if (item->Name().IFindFirst(name) != B_ERROR) {
				DeselectAll();
				Select(i, true);
				ScrollToSelection();
				fStartSearchIndex = (i + 1) % size;
				return;
			}
		}
		fStartSearchIndex = 0;
	}
}


OutlineItem*
OutlineListView::_SelectedItem(void)
{
	return static_cast< OutlineItem* >(FullListItemAt(CurrentSelection()));
}


inline void
OutlineListView::_GoToPage(const int& pageNumber)
{
    BMessage msg(MSG_GOTO_PAGE);
    msg.AddInt32("info", pageNumber);
    Window()->PostMessage(&msg);
}


status_t			
OutlineListView::Invoke(BMessage* message)
{
	auto item =  _SelectedItem();
	if (item)
		_GoToPage(item->PageNumber());
		
	BListView::Invoke(message);
}


void
OutlineListView::RemoveCurrentSelection(void)
{
	int32 idx = FullListCurrentSelection();
	RemoveItem(idx);
	Select(idx - 1);
}


bool
OutlineListView::InitiateDrag(BPoint point, int32 itemIndex, bool initialySelected)
{
	!out << "DRAG" << endl;
	return BOutlineListView::InitiateDrag(point, itemIndex, initialySelected);	
}



OutlineView::OutlineView(void)
	:
    BGroupView("outline_view", B_VERTICAL, 0)
{   
	const float iconHeight = 30;    
  	fDeleteItem = new ImageButton("quit",
        new BMessage(M_DELETE_ITEM), 0.3, 1);
    fDeleteItem->SetExplicitMinSize(BSize(iconHeight, iconHeight));
    fDeleteItem->SetExplicitMaxSize(BSize(iconHeight, iconHeight));
    
    fAddItem = new ImageButton("plus",
        new BMessage(M_ADD_ITEM), 0.3, 1);
    fAddItem->SetExplicitMinSize(BSize(iconHeight, iconHeight));
    fAddItem->SetExplicitMaxSize(BSize(iconHeight, iconHeight));
    
    fEditItem = new ImageButton("edit",
        new BMessage(M_EDIT_ITEM), 0.3, 1);
    fEditItem->SetExplicitMinSize(BSize(iconHeight, iconHeight));
    fEditItem->SetExplicitMaxSize(BSize(iconHeight, iconHeight));
    
    fFindNext = new ImageButton("find_next",
        new BMessage(M_FIND_NEXT), 0.3, 1);
    fFindNext->SetExplicitMinSize(BSize(iconHeight, iconHeight));
    fFindNext->SetExplicitMaxSize(BSize(iconHeight, iconHeight));
    
    fOutlineListView = new OutlineListView();
    
 	fVScrollBar = new BScrollBar("v_scrollbar",
                    fOutlineListView, 0, 100, B_VERTICAL);
    fHScrollBar = new BScrollBar("h_scrollbar",
                    fOutlineListView, 0, 100, B_HORIZONTAL);   

	fSearchTC   = new BTextControl("", "", new BMessage(M_FIND_NEXT));

	BGroupLayout* 
	controlsLayout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
    	.AddGroup(B_HORIZONTAL, 0)
    		.Add(fAddItem)
    		.Add(fEditItem)
    		.Add(fDeleteItem)
    		.Add(fSearchTC)
    		.Add(fFindNext)
    	.End()
    ;

    BBox* box1 = new BBox("box1");
    box1->SetBorder(B_FANCY_BORDER);
    box1->AddChild(controlsLayout->View());
    
    BGroupLayout*
	listLayout = BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
    	.Add(fOutlineListView)
       	.Add(fVScrollBar)
   	;
   	
   	BBox* box2 = new BBox("box2");
    box2->SetBorder(B_FANCY_BORDER);
    box2->AddChild(listLayout->View());
	
    BLayoutBuilder::Group<>(this)
    	.Add(box1)
		.Add(box2)
    .End()
	;
}


void
OutlineView::AttachedToWindow(void)
{
	fDeleteItem->SetTarget(this);
	fAddItem->SetTarget(this);
	fEditItem->SetTarget(this);
	fSearchTC->SetTarget(this);
	fFindNext->SetTarget(this);	
    BGroupView::AttachedToWindow();
}


void
OutlineView::EngineChanged(BaseEngine* engine)
{
	//ToDo: save outline
	fOutlineListView->SetEngine(engine);
	Invalidate();	
}									


void
OutlineView::_ShowEditWindow(void)
{
	int idx = fOutlineListView->CurrentSelection();	
	if (idx < 0)
		return;
		
	OutlineItem* item = static_cast<OutlineItem*>(fOutlineListView->FullListItemAt(idx));
	item->SetPageNumber(_CurrentPageNumber());
	auto temp = new EditListItemWindow(item);
	temp->SetResponse(M_ITEM_CHANGED, Window(), this);
	temp->CenterIn(Window()->Frame());
	temp->Show();
}


int
OutlineView::_CurrentPageNumber(void)
{
	return static_cast<MainWindow*>(Window())->CurrentPageNumber();
}


void
OutlineView::MessageReceived(BMessage* message)
{
    switch (message->what) {
        case M_DELETE_ITEM:
        	fOutlineListView->RemoveCurrentSelection();
			break;
			
		case M_ADD_ITEM:
		{
			OutlineItem* item = new OutlineItem("", _CurrentPageNumber());
			OutlineItem* super = nullptr;
			int idx = fOutlineListView->CurrentSelection();
			if (idx < 0) {
				fOutlineListView->AddItem(item);
			} else {
				super = fOutlineListView->FullListItemAt(idx);
				fOutlineListView->AddUnder(item, super);
			}
			
			fOutlineListView->MoveFirstToEnd(super);
			
			fOutlineListView->Select(fOutlineListView->IndexOf(item));
			fOutlineListView->Invalidate();
			
			_ShowEditWindow();
			
			break;
		}
		
		case M_EDIT_ITEM:
			_ShowEditWindow();
			break;
		
		case M_ITEM_CHANGED:
		{
			fOutlineListView->Invalidate();
			/*
			auto compare = [](const OutlineItem* i1, const OutlineItem* i2){
				return i1->PageNumber() - i2->PageNumber();
			};
			
			fOutlineListView->FullListSortItems(compare);
			*/
			break;
		}
		
		case M_FIND_NEXT:
			fOutlineListView->Find(fSearchTC->Text());
			break;
		
		default:
			BGroupView::MessageReceived(message);
			break;
    }
}
