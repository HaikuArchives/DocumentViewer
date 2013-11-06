/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef MAINWINDOWRB_H
#define MAINWINDOWRB_H

#include <vector>

#include <GroupView.h>
#include <StringView.h>
#include <TextControl.h>

#include "Debug.h"
#include "ImageButton.h"
#include "ImagePopUp.h"
#include "ImageTabView.h"
#include "PageNavigationView.h"

class MainWindowRB : public BGroupView
{
public:
                    MainWindowRB(void);
    virtual void 	MouseMoved(BPoint where, uint32 code,
                                const BMessage* dragMessage);
    virtual void    AttachedToWindow(void);
    virtual void    MessageReceived(BMessage* message);
    virtual	void	MakeFocus(bool focusState = true);

            void    SetDocumentPages(int pages);

            void    SetActiveTab(int32 tab);
            int32   ActiveTab(void);

            void	SetAutohiding(bool autohiding = true);
            bool	IsAutohiding(void)	{ return fAutohiding; }

    enum{   M_PAGENUMBER = 'ip01'
        };

private:
            void     _GoToPage(int pageNumber);

    ImageTabView*           fTabView;
    std::vector<ImageTab*>  fTabsVec;

    BGroupLayout*	        fMainGroup;
    PageNavigationView*		fPageNavigationView;
    
    ImageButton*			fOpenFileButton;
    ImageButton*            fQuitButton;
    ImageButton*			fSupportButton;

    int                     fPages;
    int						fMaxHeight;

    bool			        fAutohiding;

    Debug                   out;
};

#endif // MAINWINDOWRB_H
