/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PAGENAVIGATIONVIEW_H
#define PAGENAVIGATIONVIEW_H

#include <memory>

#include <GroupView.h>
#include <StringView.h>
#include <TextView.h>

#include "Debug.h"
#include "ImageButton.h"
#include "NumberControl.h"


class PageNavigationView : public BGroupView, public BInvoker
{
public:
                    	PageNavigationView(std::unique_ptr<BMessage> message);
                    	
    virtual void		FrameResized(float newWidth, float newHeight);
    virtual void		MessageReceived(BMessage* message);
    virtual void    	AttachedToWindow(void);
    virtual void    	MakeFocus(bool focus);        
            
  			int			Value(void);
  			void		SetValue(int value);
  			void		SetMaxValue(int const& value);
  			
  	enum{   M_GOTO_PAGE = 'ig01', M_GOTO_NEXT_PAGE = 'ig02',
            M_GOTO_PREVIOUS_PAGE = 'ig03'
        };

private:
	NumberControl*			fPageNC;
	BStringView*			fTotalPagesSV;
	
	ImageButton* 			fBackButton;
    ImageButton*   			fNextButton;
    
	int						fMaxValue;
	
	Debug					out;
};

#endif // PAGENAVIGATIONVIEW_H
