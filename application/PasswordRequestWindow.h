/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PASSWORDREQUESTWINDOW_H
#define PASSWORDREQUESTWINDOW_H

#include <tuple>

#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <Window.h>

#include "Debug.h"

class PasswordRequestWindow : public BWindow
{
public:
                    PasswordRequestWindow();
    virtual void    MessageReceived(BMessage* message);
    
    std::tuple<BString, bool>	Go(void);
    
    enum {
    	M_CANCEL = 'ic01', M_OK = 'io01', M_SHOW_CHARS = 'is01'
    };
private:
            void    _Apply(void);

    BWindow*                fTarget;
    
    BTextControl*           fPasswordTC;
    BCheckBox*              fShowCB;
    
    bool					fWaiting;
    bool					fIsOk;
 
    Debug                   out;
};


#endif
