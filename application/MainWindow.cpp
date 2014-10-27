/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "MainWindow.h"

#include <memory>

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <Mime.h>
#include <MimeType.h>
#include <Path.h>
#include <PrintJob.h>
#include <Screen.h>
#include <SeparatorView.h>
#include <SpaceLayoutItem.h>
#include <SplitLayoutBuilder.h>
#include <SplitView.h>
#include <StringView.h>
#include <View.h>

#include "MainApplication.h"
#include "Messages.h"
#include "Settings.h"
#include "Tools.h"



MainWindow::MainWindow(void)
    :
    BWindow(BRect(), "DocumentViewer", B_DOCUMENT_WINDOW_LOOK,
        B_NORMAL_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS
            | B_QUIT_ON_WINDOW_CLOSE),
    fFullscreenIsOn(false),
    fPassword("")
{
	fTabView1 = new ImageTabView("tab_view1");

	fPreviewView = new PreviewView();
	fTabView1->AddTab(fPreviewView, new ImageTab());

	fOutlineView = new OutlineView();
	fTabView1->AddTab(fOutlineView, new ImageTab());

	fSearchView = new SearchView();
	fTabView1->AddTab(fSearchView, new ImageTab());

	fTabView1->Select(0);
	fDocumentView = new DocumentView("", "", fPassword);
	fRibbon = new MainWindowRB();

	fDocumentLayout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.AddSplit(B_HORIZONTAL, 0, 7).GetSplitView(&fSplitView1)
			.AddGroup(B_VERTICAL, 0, 1)
				.Add(fTabView1)
			.End()
			.AddSplit(B_VERTICAL, 0, 6).GetSplitView(&fSplitView2)
				.Add(fRibbon, 1)
				.AddGroup(B_VERTICAL, 0, 6)
					.Add(fDocumentView)
				.End()
			.End()
		.End()
	;

	fImageSpinner = new ImageSpinner();

	fIntroLayout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.Add(fImageSpinner)
		.AddGlue(0)
	;

	fCardLayout = new BCardLayout();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fCardLayout)
		;

	fCardLayout->AddItem(fIntroLayout);
	fCardLayout->AddItem(fDocumentLayout);
	fCardLayout->SetVisibleItem(fIntroLayout);
	_LoadSettings();
	Show();
}


void
MainWindow::MessageReceived(BMessage* message)
{	
	switch (message->what) {
		case MSG_NO_FIT:
		case MSG_NO_ZOOM:
        case MSG_FIT_PAGE_HEIGHT:
        case MSG_FIT_PAGE_WIDTH:
        case MSG_ZOOM_IN:
        case MSG_ZOOM_OUT:
            PostMessage(message, fDocumentView);
            break;

        case MSG_ACTIVE_PAGE:
        {
            PostMessage(message, fRibbon);
            int32 pageNumber = 0;
            message->FindInt32("info", &pageNumber);
            fPreviewView->SelectPage(pageNumber);
            break;
        }
		
        case MSG_GOTO_PAGE:
        {
            PostMessage(message, fDocumentView);
            PostMessage(message, fRibbon);

            int32 pageNumber = 0;
            message->FindInt32("info", &pageNumber);
            fPreviewView->SelectPage(pageNumber);
            break;
        }

        case MSG_FULLSCREEN:
            fFullscreenIsOn = !fFullscreenIsOn;

            if (fFullscreenIsOn) {
                fSavedFrame = Frame();
                BScreen screen(this);
                BRect rect(screen.Frame());
                MoveTo(rect.left, rect.top);
                ResizeTo(rect.Width(), rect.Height());
            } else {
                Hide();
                MoveTo(fSavedFrame.left, fSavedFrame.top);
                ResizeTo(fSavedFrame.Width(), fSavedFrame.Height());
                Show();
            }
            break;

        case MSG_APP_QUIT:
        {
   			fCardLayout->SetVisibleItem((BLayoutItem*)fIntroLayout);
   			fImageSpinner->MakeFocus(true);
   			SetTitle("DocumentViewer");
			//be_app->PostMessage(B_QUIT_REQUESTED);
			break;
        }
        
        case MSG_HELP:
        {	
        	char const* args[] = {"http://haiku.bplaced.net/DocumentViewer/help.html", 0};
        	be_roster->Launch("text/html", 1, args);
        	break;
        }
        
        
        case MSG_HIGHLIGHT_RECT:
        	PostMessage(message, fDocumentView);
        	break;
        
        case MSG_SUPPORT:
        {	
        	char const* args[] = {"http://haiku.bplaced.net/DocumentViewer/support.html", 0};
        	be_roster->Launch("text/html", 1, args);
        	break;
        }
        
        case MSG_PRINT_DOCUMENT:
        {	
        	fDocumentView->Print();
        	break;
        }
        
        
        case MSG_SETUP_PRINTER:
        {
        	BPrintJob job("document's name");
        	job.ConfigPage();	
        	break;
        }
        
        case MSG_OPEN_FILE_PANEL:
        case M_OPEN_FILE_PANEL:
        	((MainApplication*)be_app)->OpenFilePanel();
        	break;
        	
        case MSG_OPEN_FILE:
        {
        	BString file;
        	message->FindString("file", &file);
        	BString password = "";
        	message->FindString("password", &password);
        	int32 page = 0;
        	message->FindInt32("page", &page);
        	auto type = _FileType(file);
    		_OpenFile(file, type, password, page);
        	break;
        }
        	
        case M_SHOW_DOCUMENT:
        	fCardLayout->SetVisibleItem((BLayoutItem*)fDocumentLayout);
        	break;
        	

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


BString
MainWindow::_FileType(BString const& file)
{
	BMimeType mime;
    BMimeType::GuessMimeType(file, &mime);
    BMessage msg;
	uint32 i=0;
	char *ptr;
	mime.GetFileExtensions(&msg);

	BString type = "";
	while (true) {
   		if (msg.FindString("extensions", i++, &type) != B_OK)
  			break;
  			   		
   		if (type == "djvu" || type == "pdf")
   			break;
	}
	
	// hack, delte later, after types are added to haiku
	if (type == "") {
		if (file.IFindLast("djvu") != B_ERROR)
			type = "djvu";	
	}
		
	return type;
}


void
MainWindow::_OpenFile(BString const& path, BString const& fileType,
	BString password, int32 const& page)
{
    SetTitle(path);
    try {
    	fDocumentView->FileChanged(path, fileType, password);
    	fPreviewView->FileChanged(path, fileType, password);
    } catch(...) {
    	fCardLayout->SetVisibleItem((BLayoutItem*)fIntroLayout);
  		return;	
    }
    fOutlineView->EngineChanged(fPreviewView->Engine());
    fSearchView->EngineChanged(fPreviewView->Engine());
    fRibbon->SetDocumentPages(fDocumentView->PageCount());
    
    if (fImageSpinner->NeedsFile(path)) {
    	auto height = fImageSpinner->PreferredImageHeight();
    	fImageSpinner->Add(path, move(fDocumentView->Cover(height)));
    }
    
    BMessage msg(MSG_GOTO_PAGE);
    msg.AddInt32("info", page);
    PostMessage(&msg);
    
    fCardLayout->SetVisibleItem((BLayoutItem*)fDocumentLayout);
    fDocumentView->MakeFocus(true);
}


/*
void
MainWindow::_AddDocumentLayout
{
	fDocumentLayout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
        .AddSplit(B_HORIZONTAL, 0, 7).GetSplitView(&fSplitView1)
            .AddGroup(B_VERTICAL, 0, 6)
                .Add(fTabView1)
            .End()
            .AddSplit(B_VERTICAL, 0, 1).GetSplitView(&fSplitView2)
                .Add(fRibbon, 1)
                .AddGroup(B_VERTICAL, 0, 6)
                    .Add(fDocumentView)
                .End()
            .End()
        .End()
    ;	
}

*/

void
MainWindow::_SaveSettings(void)
{
    if (fFullscreenIsOn == false)
        fSavedFrame = Frame();

    Settings settings("MainWindow");
    
    settings << "SavedFrame"     << fSavedFrame;
    
    if (fDocumentLayout != nullptr) {
    	settings
        	<< "Split10"        << fSplitView1->ItemWeight((int32)0)
        	<< "Split11"        << fSplitView1->ItemWeight(1)
        	<< "Split20"        << fSplitView2->ItemWeight((int32)0)
        	<< "Split21"        << fSplitView2->ItemWeight(1)
        	<< "RibbonTab"      << fRibbon->ActiveTab()
        	<< "SidebarTab"     << fTabView1->Selection()
        	<< "Fullscreen"     << (int32)fFullscreenIsOn
        	<< std::endl;
    }
}


void
MainWindow::_LoadSettings()
{
    Settings settings("MainWindow");

    settings << "SavedFrame" >> fSavedFrame;

    if (fSavedFrame == BRect()) {
        Zoom();
    } else {
        MoveTo(fSavedFrame.left, fSavedFrame.top);
        ResizeTo(fSavedFrame.Width(), fSavedFrame.Height());
    }
    
    if (fDocumentLayout == nullptr)
    	return;

    float weight;
    weight = 1.0;
    settings << "Split10" >> weight;   	
    fSplitView1->SetItemWeight(0, weight, true);
    if (weight == 0)
    	fSplitView1->SetItemCollapsed(0, false);
    
    weight = 5.0;
    settings << "Split11" >> weight;
    fSplitView1->SetItemWeight(1, weight, true);
    if (weight == 0)
    	fSplitView1->SetItemCollapsed(1, false);
    	
    weight = 1.0;
    settings << "Split20" >> weight;
    fSplitView2->SetItemWeight(0, weight, true);
    if (weight == 0)
    	fSplitView2->SetItemCollapsed(0, false);
    	
    weight = 6.0;
    settings << "Split21" >> weight;
    fSplitView2->SetItemWeight(1, weight, true);
	if (weight == 0)
    	fSplitView2->SetItemCollapsed(1, false);

    int32 value;
    value = 0;
    settings << "RibbonTab" >> value;
    fRibbon->SetActiveTab(value);
    value = 0;
    settings << "SidebarTab" >> value;
    fTabView1->Select(value);
    value = 0;
    settings << "Fullscreen" >> value;
    if (value != 0)
        PostMessage(MSG_FULLSCREEN);
}


bool
MainWindow::QuitRequested(void)
{
    Hide();
    _SaveSettings();
    return true;
}
