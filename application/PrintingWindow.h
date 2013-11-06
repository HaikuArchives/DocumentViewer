/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PRINTINGWINDOW_H
#define PRINTINGWINDOW_H

#include <vector>

#include <CardLayout.h>
#include <InterfaceKit.h>
#include <GroupLayout.h>
#include <OptionPopUp.h>
#include <Window.h>

#include "Debug.h"
#include "DocumentView.h"
#include "PageNavigationView.h"
#include "PrintPreviewView.h"

class PrintingWindow : public BWindow
{
public:
                   	PrintingWindow(BWindow* window, DocumentView* doc);
    virtual void    MessageReceived(BMessage* message);
    virtual bool    QuitRequested(void);

    enum {
    	M_CANCEL 			= 'ic01', 
    	M_HELP 				= 'ih01', 
    	M_OK 				= 'io01',
    	M_PAGENUMBER		= 'ip01',
    	M_PAGES_PER_SHEET 	= 'ip02',	
    	M_PAGE_ORDER 		= 'ip03',  	
    	M_SCALING 			= 'is01',
 	};
 	
 	enum {
 		FIT_MARGINS, FIT_PAGE, MULTIPLE_PAGES,
 		SCALE_CUSTOM, SCALE_NONE,
 	};
 	
 	enum {
 		ORDER_HORIZONTAL, ORDER_HORIZONTAL_R,
 		ORDER_VERTICAL,   ORDER_VERTICAL_R,	
 	};
 	
private:
            void    _LoadSettings(void);
            void    _SaveSettings(void);

    BWindow*                	fTargetWindow;
    
    BGroupLayout* 				fScaleCLI1;
    BGroupLayout* 				fScaleCLI2;
    BGroupLayout* 				fMultiplePagesCLI1;
    BGroupLayout* 				fMultiplePagesCLI2;
    
    BCardLayout*            	fCardLayout1;
    BCardLayout*            	fCardLayout2;
    
    BOptionPopUp*				fNameOP;
    BOptionPopUp*				fScalingTypeOP;
    BOptionPopUp*				fPagesPerSheetOP;
    BOptionPopUp*				fPageOrderOP;
    
    BStringView*				fStatusSV;
    BStringView*				fTypeSV;

    BTextControl*           	fPageFromTC;
    BTextControl*           	fPageToTC;
    BTextControl*           	fPagesTC;
    BTextControl*           	fCopiesTC;
    BTextControl*				fPreviewPageTC;
    BTextControl*				fScaleTC;
    BTextControl*				fXPagesPerSheetTC;
    BTextControl*				fYPagesPerSheetTC;
    
    
    
    BRadioButton*           	fAllRB;
    BRadioButton*           	fCurrentViewRB;
    BRadioButton*           	fCurrentPageRB;
    BRadioButton*           	fPageFromToRB;
    BRadioButton*           	fPagesRB;
    
    BCheckBox*					fCollateCB;
    BCheckBox*					fReverseCB;

	DocumentView*				fDocument;
	
	PrintPreviewView*			fPrintPreviewView;
	
	PageNavigationView*			fPageNavigationView;
	
    Debug                   	out;
};


#endif 
