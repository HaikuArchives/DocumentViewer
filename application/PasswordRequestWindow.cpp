/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "PasswordRequestWindow.h"

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>

#include "Settings.h"

PasswordRequestWindow::PasswordRequestWindow()
	:
    BWindow(BRect(), "Enter Password", B_TITLED_WINDOW_LOOK,
                B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS
                    | B_NOT_CLOSABLE | B_NOT_MOVABLE
                    | B_AUTO_UPDATE_SIZE_LIMITS | B_OUTLINE_RESIZE
                    | B_NOT_RESIZABLE | B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE ),
    fWaiting(true),
    fIsOk(false)
{	
	fPasswordTC = new BTextControl("Password:", "password", NULL);
   	fPasswordTC->TextView()->HideTyping(true);
    fShowCB = new BCheckBox("Show Characters", new BMessage(M_SHOW_CHARS));
	
	BButton* cancelButton = new BButton("Cancel", new BMessage(M_CANCEL));
	BButton* okButton = new BButton("OK", new BMessage(M_OK));
	
	SetDefaultButton(okButton);
	
    BLayoutBuilder::Group<>(this, B_VERTICAL, 10)
        .SetInsets(10, 10, 10, 10)
        .Add(fPasswordTC)
        .Add(fShowCB)
        .AddGroup(B_HORIZONTAL, 0, 1)
            .Add(cancelButton)
            .AddStrut(50)
            .Add(okButton)
        .End()
        
    .End()
    ;
}


void
PasswordRequestWindow::MessageReceived(BMessage* message)
{
    switch (message->what) {
        case M_OK:
        	fWaiting = false;
        	fIsOk = true;
        	Close();
            break;

        case M_CANCEL:
        	fWaiting = false;
            Close();
            break;
            
      	case M_SHOW_CHARS:
      		if (fShowCB->Value() == 0)
      			fPasswordTC->TextView()->HideTyping(true);
      		else
      			fPasswordTC->TextView()->HideTyping(false);
      			
      		fPasswordTC->TextView()->Invalidate();
      		fPasswordTC->MakeFocus();
      		break;

		default:
            BWindow::MessageReceived(message);
            break;
	}
}


std::tuple<BString, bool>
PasswordRequestWindow::Go(void)
{
	CenterOnScreen();
    Show();
    while (fWaiting) {
    	usleep(10000);	
    }
  
   return std::tuple<BString, bool>(fPasswordTC->Text(), fIsOk);
}
