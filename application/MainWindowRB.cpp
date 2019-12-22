/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "MainWindowRB.h"

#include <memory>
#include <stdlib.h>

#include <Box.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <OptionPopUp.h>
#include <SeparatorView.h>

#include "Messages.h"

using namespace std;

MainWindowRB::MainWindowRB(void)
	:
    BGroupView("main_widow_ribbon", B_VERTICAL, 0),
    fPages(0),
    fMaxHeight(35),
    fAutohiding(false)
{
	SetFlags(Flags() | B_FULL_POINTER_HISTORY);

	unique_ptr<BMessage> navigationMessage(new BMessage(M_PAGENUMBER));
	fPageNavigationView = new PageNavigationView(move(navigationMessage));

	ImageButton* printButton = new ImageButton("print", 
		new BMessage(MSG_PRINT_DOCUMENT), 0.3, 1, "Print document");
    printButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    printButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

	fOpenFileButton = new ImageButton("open_document",
        new BMessage(MSG_OPEN_FILE_PANEL), 0.3, 1, "Open document");
    fOpenFileButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    fOpenFileButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    fQuitButton = new ImageButton("quit", new BMessage(MSG_APP_QUIT),
		0.3, 1, "Close document");
    fQuitButton->SetExplicitMinSize(BSize(fMaxHeight, 0));
    fQuitButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    // Home Tab
    BGroupLayout* homeLayout = BGroupLayoutBuilder(B_HORIZONTAL, 0)
        .SetInsets(2, 2, 2, 2)
        .Add(fPageNavigationView)
       	.AddGlue(1000)
        .Add(printButton)
        .Add(fOpenFileButton)
        .Add(fQuitButton)
        .RootLayout()
    ;

    ImageButton* fullscreenButton = new ImageButton("fullscreen",
        new BMessage(MSG_FULLSCREEN) , 0.3, 1, "Fullscreen");
    fullscreenButton->SetExplicitMinSize(BSize(fMaxHeight, 0));
    fullscreenButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* fitPageHeight = new ImageButton("fit_page_height",
        new BMessage(MSG_FIT_PAGE_HEIGHT), 0.3, 1, "Fit to height");
    fitPageHeight->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    fitPageHeight->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* fitPageWidth = new ImageButton("fit_page_width",
        new BMessage(MSG_FIT_PAGE_WIDTH), 0.3, 1, "Fit to width");
    fitPageWidth->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    fitPageWidth->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

	ImageButton* zoomOriginal = new ImageButton("zoom_original",
		new BMessage(MSG_NO_ZOOM), 0.3, 1, "Reset zoom");
    zoomOriginal->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    zoomOriginal->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* zoomOut = new ImageButton("zoom_out",
        new BMessage(MSG_ZOOM_OUT), 0.3, 1, "Zoom out");
    zoomOut->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    zoomOut->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* zoomIn = new ImageButton("zoom_in",
        new BMessage(MSG_ZOOM_IN), 0.3, 1, "Zoom in");
    zoomIn->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    zoomIn->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* rotateLeftButton = new ImageButton("rotate_left", nullptr,
		0.3, 1, "Rotate left");
    rotateLeftButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    rotateLeftButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* rotateRightButton = new ImageButton("rotate_right", nullptr,
		0.3, 1, "Rotate right");
    rotateRightButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    rotateRightButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    // View Tab
    BGroupLayout* viewLayout = BGroupLayoutBuilder(B_HORIZONTAL, 0)
        .SetInsets(2, 2, 2, 2)
        .AddGroup(B_VERTICAL, 0, 0)
            .AddGroup(B_HORIZONTAL, 0, 0)
            	.Add(fullscreenButton)
                .Add(new BSeparatorView(B_VERTICAL))
                .Add(fitPageHeight)
                .Add(fitPageWidth)
                .Add(new BSeparatorView(B_VERTICAL))
                .Add(zoomOriginal)
                .Add(zoomOut)
                .Add(zoomIn)
                .Add(new BSeparatorView(B_VERTICAL))
                .Add(rotateLeftButton)
                .Add(rotateRightButton)
                .AddGlue(0)
            .End()
        .End()
        .RootLayout()
    ;

    // Edit Tab
    BGroupLayout* editLayout = BGroupLayoutBuilder(B_VERTICAL, 0)
        .AddGlue(0)
        //.Add(new ImageButton("star_green", nullptr, 0.3))
        .AddGlue(0)
        .RootLayout()
    ;


    ImageButton* setupPrinterButton = new ImageButton("printer", 
    	new BMessage(MSG_SETUP_PRINTER), 0.3, 1, "Printer setup");
    setupPrinterButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    setupPrinterButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    // Configure Tab
    BGroupLayout* configureLayout = BGroupLayoutBuilder(B_HORIZONTAL, 0)
        .Add(setupPrinterButton)
        .AddGlue(0)
    	.RootLayout()
    ;

    ImageButton* helpButton = new ImageButton("help2",
    	new BMessage(MSG_HELP), 0.3, 1, "Help");
   	helpButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    helpButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));

    ImageButton* supportButton = new ImageButton("support",
    	new BMessage(MSG_SUPPORT), 0.3, 1, "Contribute");
   	supportButton->SetExplicitMinSize(BSize(fMaxHeight, fMaxHeight));
    supportButton->SetExplicitMaxSize(BSize(fMaxHeight, fMaxHeight));


    // Configure Tab
    BGroupLayout* helpLayout = BGroupLayoutBuilder(B_HORIZONTAL, 0)
        .Add(helpButton)
        .AddGlue(1000)
        .Add(supportButton)
        //.AddGlue(0)
        .RootLayout()
    ;

    fTabView = new ImageTabView("tab_view");
    //fTabView->SetTabWidth(B_WIDTH_FROM_LABEL);

    fTabsVec.push_back(new ImageTab());
	fTabView->AddTab(homeLayout->View(), fTabsVec.back());
	fTabsVec.back()->SetLabel("home");
    fTabsVec.push_back(new ImageTab());
	fTabView->AddTab(viewLayout->View(), fTabsVec.back());
	fTabsVec.back()->SetLabel("zoom_fit_best");
    fTabsVec.push_back(new ImageTab());
	fTabView->AddTab(editLayout->View(), fTabsVec.back());
	fTabsVec.back()->SetLabel("marker");
    fTabsVec.push_back(new ImageTab());
    fTabView->AddTab(configureLayout->View(), fTabsVec.back());
	fTabsVec.back()->SetLabel("configure");
	fTabsVec.push_back(new ImageTab());
    fTabView->AddTab(helpLayout->View(), fTabsVec.back());
	fTabsVec.back()->SetLabel("help");

    BLayoutBuilder::Group<>(this)
        .Add(fTabView)
    ;
    //fTabView->SetExplicitMaxSize(BSize(10000, 50));
  //  fTabView->SetExplicitMinSize(BSize(0, 50));
}


void
MainWindowRB::AttachedToWindow(void)
{
    fPageNavigationView->SetTarget(this);
    //int size = fTabView->CountTabs();

  //  for (int i = 0; i < size; ++i)
    //    fTabView->ViewForTab(i)->SetViewColor(220,220,255,255);
    BGroupView::AttachedToWindow();

}


int32
MainWindowRB::ActiveTab(void)
{
    return fTabView->Selection();
}


void
MainWindowRB::SetActiveTab(int32 tab)
{
    fTabView->Select(tab);
}


inline void
MainWindowRB::_GoToPage(int pageNumber)
{
    BMessage msg(MSG_GOTO_PAGE);
    msg.AddInt32("info", pageNumber);
    Window()->PostMessage(&msg);
}


void
MainWindowRB::SetDocumentPages(int pages)
{
    fPages = pages;
    fPageNavigationView->SetMaxValue(pages);
}


void
MainWindowRB::MessageReceived(BMessage* message)
{
	switch (message->what) {

        case MSG_ACTIVE_PAGE:
        case MSG_GOTO_PAGE:
        {
            int32 value;
            message->FindInt32("info", &value);
            ++value;
			fPageNavigationView->SetValue(value);

            break;
        }

        case M_PAGENUMBER:
        	_GoToPage(fPageNavigationView->Value() - 1);
        	break;

        default:
            BGroupView::MessageReceived(message);
    }
}


void
MainWindowRB::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	switch (code) {
		case B_EXITED_VIEW:
		//	fMainGroup->SetVisible(false);
			break;

		case B_ENTERED_VIEW:
			if (fAutohiding) {
				fMainGroup->SetVisible(true);
				MakeFocus(true);
			}
			break;

		default:
			break;
	}

    BGroupView::MouseMoved(where, code, dragMessage);
}


void
MainWindowRB::MakeFocus(bool focusState)
{
	if (fAutohiding) {
		if (focusState == false)
			fMainGroup->SetVisible(false);
	}

	BGroupView::MakeFocus(focusState);
}


void
MainWindowRB::SetAutohiding(bool autohiding)
{
	fAutohiding = autohiding;

	if (fAutohiding)
		fMainGroup->SetVisible(false);
	else
		fMainGroup->SetVisible(true);
}
