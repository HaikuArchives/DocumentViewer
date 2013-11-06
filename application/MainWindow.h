/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <GroupLayout.h>
#include <CardLayout.h>
#include <CheckBox.h>
#include <Entry.h>
#include <Rect.h>
#include <SplitView.h>
#include <TextControl.h>
#include <Window.h>


#include "BookmarksView.h"
#include "Debug.h"
#include "ImageSpinner.h"
#include "ImageTabView.h"
#include "MainWindowRB.h"
#include "OutlineView.h"
#include "DocumentView.h"
#include "PreviewView.h"
#include "SearchView.h"

class MainWindow : public BWindow
{
public:
							MainWindow(void);
    virtual	void 			MessageReceived(BMessage* message);
    virtual bool 			QuitRequested(void); 				
    
    const	int&	CurrentPageNumber(void)
    {
    			return fDocumentView->CurrentPageNumber();
    }
    				

    enum{	M_EXIT = 'ie01', M_OPEN_FILE = 'io01', M_OPEN_FILE_PANEL,
    		M_SHOW_DOCUMENT = 'is01'
    	};
    
private:
			void			_OpenFile(const BString& path, BString const& fileType,
								BString password, int32 const& page = 0);
            void 			_SaveSettings(void);
            void 			_LoadSettings(void);
            		
            BString			_FileType(BString const& file);

    BCardLayout*            fCardLayout;
    BGroupLayout*           fDocumentLayout;
    BGroupLayout*           fIntroLayout;

    BSplitView*             fSplitView1;
    BSplitView*             fSplitView2;
    BSplitView*             fSplitView3;
    BSplitView*             fSplitView4;

	BButton*				fButton;
	
    ImageTabView*           fTabView1;

    std::vector<ImageTab*>  fTabsVec1;
    

    DocumentView*           fDocumentView;
    OutlineView*            fOutlineView;
    PreviewView*            fPreviewView;
    BookmarksView*          fBookmarksView;
    SearchView*             fSearchView;
    MainWindowRB*           fRibbon;
	
	ImageSpinner*			fImageSpinner;
	
    BRect                   fSavedFrame;

    bool                    fFullscreenIsOn;
    
    BString					fPassword;

    Debug                   out;
};


#endif
