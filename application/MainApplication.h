/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H


#include <Application.h>
#include <Entry.h>
#include <FilePanel.h>

#include "Debug.h"
#include "MainWindow.h"
#include "Messages.h"

class MainApplication : public BApplication
{
public:
    MainApplication(void)
        :
        BApplication("application/x-vnd.documentviewer")
    {
        fFilePanel = nullptr;
        fMainWindow = nullptr;
    }

    virtual void ReadyToRun(void)
    {
      	if (fMainWindow == nullptr)
    		fMainWindow = new MainWindow();
    }

    virtual void RefsReceived(BMessage* msg)
    {	
    	uint32 type = 0;
        int32 count = 0;

        msg->GetInfo ("refs", &type, &count);
        if (type != B_REF_TYPE)
            return;
    	
    	if (fMainWindow == nullptr)
    		fMainWindow = new MainWindow();
    		
    	entry_ref ref; 
    	for (int32 i = --count; i >= 0; --i)
            if (msg->FindRef("refs", i, &ref) == B_OK) {
            	BEntry entry(&ref, true);
				BPath path;
				entry.GetPath(&path);
				
				int32 page = 0;
				msg->FindInt32("page", &page);
				
				BString password = "";
				msg->FindString("password", &password);
				
				BMessage message(MSG_OPEN_FILE);
				message.AddString("file", path.Path());
				message.AddString("password", password);
				message.AddInt32("page", page);	
				fMainWindow->PostMessage(&message);  	
            }
    }

    virtual void ArgvReceived (int32 argc, char **argv)
    {
        // check command line
        if (!(argc == 2 || argc == 3 || argc == 4))
            exit(1);
		
		BString path = "";
        if (argc >= 2)
            path = argv[1];

		int page = 0;
        if (argc >= 3)
            page = atoi(argv[2]) - 1;
        
        BString password = "";
        if (argc == 4)
        	password = argv[3];
        	
        entry_ref ref;
        get_ref_for_path (path.String(), &ref);

        BMessage msg(B_REFS_RECEIVED);
        msg.AddRef("refs", &ref);
        msg.AddString("password", password);
        msg.AddInt32 ("page", page);
        PostMessage(&msg);
    }

    void MessageReceived(BMessage* msg)
    {
        switch (msg->what) {
    /*       case B_CANCEL:
                if (fMainWindow == nullptr)
                    PostMessage (B_QUIT_REQUESTED);

                break;
						*/
            default:
                BApplication::MessageReceived(msg);
        }
    }

    void OpenFilePanel(void)
    {
        if (fFilePanel == nullptr)
            fFilePanel = new BFilePanel (B_OPEN_PANEL,
                            nullptr, nullptr, B_FILE_NODE, true, nullptr, nullptr);

        fFilePanel->Show();
    }


private:
    MainWindow*		fMainWindow;
    BFilePanel*     fFilePanel;
    bool            fToOpenFilePanel;

    Debug           out;
};

#endif
