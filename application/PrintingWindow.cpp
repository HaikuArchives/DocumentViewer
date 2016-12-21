/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "PrintingWindow.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <Application.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>

#include "Messages.h"
#include "Settings.h"

using namespace std;

PrintingWindow::PrintingWindow(BWindow* window, DocumentView* document)
	:
    BWindow(BRect(), "Print", B_TITLED_WINDOW_LOOK,
        B_FLOATING_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS
            | B_AUTO_UPDATE_SIZE_LIMITS | B_OUTLINE_RESIZE
            /*| B_WILL_ACCEPT_FIRST_CLICK*/),
    fTargetWindow(window),
    fDocument(document)
{
	auto nameNameSV = new BStringView("name", "Name:");
	nameNameSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	auto statusNameSV = new BStringView("status", "Status:");
	statusNameSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	auto typeNameSV = new BStringView("type", "Type:");
	typeNameSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	
	fNameOP = new BOptionPopUp("name_op", "", NULL);
	fNameOP->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fNameOP->AddOptionAt("Name", 0, 0);
	fStatusSV = new BStringView("ready", "Ready");
	fStatusSV->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fTypeSV = new BStringView("color", "Color");
	fTypeSV->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	
	auto propertiesButton = new BButton("Configure...", new BMessage(MSG_SETUP_PRINTER));
	propertiesButton->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	auto printWhatSV = new BStringView("print_what", "Print What:");
	printWhatSV->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	
	auto printWhatOP = new BOptionPopUp("print_what_op", "", nullptr);
	printWhatOP->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	printWhatOP->AddOptionAt("Document", 0, 0);
	printWhatOP->AddOptionAt("Document and Annotations", 1, 1);
	printWhatOP->AddOptionAt("Annotations", 2, 2);
	printWhatOP->SetValue(1);
	
	BGroupLayout* 
	printerLayoutV1 = BLayoutBuilder::Group<>(B_VERTICAL, 5)
		.Add(nameNameSV)
    	.Add(statusNameSV)
    	.Add(typeNameSV)
	;
	
	BGroupLayout* 
	printerLayoutV2 = BLayoutBuilder::Group<>(B_VERTICAL, 5)
		.Add(fNameOP)
    	.Add(fStatusSV)
    	.Add(fTypeSV)
    ;
    
    BGroupLayout* 
	printerLayoutV3 = BLayoutBuilder::Group<>(B_VERTICAL, 5)	
    	.Add(propertiesButton)
    	.Add(printWhatSV)
    	.Add(printWhatOP)
    ;
    
    printerLayoutV1->AlignLayoutWith(printerLayoutV2, B_VERTICAL);
    printerLayoutV2->AlignLayoutWith(printerLayoutV3, B_VERTICAL);
    
    BGroupLayout* 
	printerLayout = BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
		.SetInsets(5, 5, 5, 5)
    	.Add(printerLayoutV1)
    	.AddStrut(10)
    	.Add(printerLayoutV2)
    	.AddStrut(30)
    	.Add(printerLayoutV3)
    	.AddGlue(0)
	;
    
    BBox* printerBox = new BBox("printer");
    printerBox->SetLabel("Printer");
    printerBox->SetBorder(B_FANCY_BORDER);
    printerBox->AddChild(printerLayout->View());
    
    fAllRB = new BRadioButton("All", nullptr);
    fAllRB->SetValue(true);
    fCurrentViewRB = new BRadioButton("Current view", nullptr);
    fCurrentViewRB->SetEnabled(false);
    fCurrentPageRB = new BRadioButton("Current page", nullptr);
    fPageFromToRB = new BRadioButton("Page from:", nullptr);
    fPagesRB = new BRadioButton("Pages", nullptr);
    
    BGroupLayout* 
	printRangeLayoutV1 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.Add(fAllRB)
    	.Add(fCurrentViewRB)
    	.Add(fCurrentPageRB)
    	.Add(fPageFromToRB)
    	.Add(fPagesRB)
	;
	
	fPageFromTC = new BTextControl("", "", nullptr);
    fPageToTC = new BTextControl("", "", nullptr);
    fPagesTC = new BTextControl("", "", nullptr);
	
	BGroupLayout* 
	printRangeLayoutV2 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.AddGlue()
    	.AddGlue()
    	.AddGlue()
    	.AddGroup(B_HORIZONTAL, 0)
    		.Add(fPageFromTC)
   			.AddStrut(5)
    		.Add(new BStringView("to_page", "to:"))
    		.Add(fPageToTC)
    	.End()
    	.Add(fPagesTC)
	;
	
	auto subsetSV = new BStringView("subset", "Subset:");
    subsetSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	
	BGroupLayout* 
	printRangeLayoutV3 = BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
		.Add(subsetSV)	
	;
	
	auto subsetOP = new BOptionPopUp("subset_op", "", nullptr);
	subsetOP->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	subsetOP->AddOptionAt("All pages in range", 0, 0);
	subsetOP->AddOptionAt("Odd pages only", 1, 1);
	subsetOP->AddOptionAt("Even pages only", 2, 2);
	
	fReverseCB = new BCheckBox("Reverse pages", nullptr);
	
	BGroupLayout* 
	printRangeLayoutV4 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		//.AddGroup(B_HORIZONTAL, 0)
		.Add(subsetOP)
		//.AddStrut(3)
		.Add(fReverseCB)	
		//.End()
	;
	
	printRangeLayoutV1->AlignLayoutWith(printRangeLayoutV2, B_VERTICAL);
	printRangeLayoutV1->AlignLayoutWith(printRangeLayoutV3, B_HORIZONTAL);
	printRangeLayoutV2->AlignLayoutWith(printRangeLayoutV4, B_HORIZONTAL);
	printRangeLayoutV3->AlignLayoutWith(printRangeLayoutV4, B_VERTICAL);
    
    BGroupLayout* 
	printRangeLayout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.SetInsets(5, 5, 5, 5)
		.AddGroup(B_HORIZONTAL, 0)
 			.Add(printRangeLayoutV1)
 		//	.AddGlue(0)
 			.AddStrut(5)
 			.Add(printRangeLayoutV2)
 		.End()
 		.AddStrut(5)
 		.Add(new BSeparatorView("", "",B_HORIZONTAL,  B_FANCY_BORDER))
 		.AddStrut(5)
 		.AddGroup(B_HORIZONTAL, 0)
 			.Add(printRangeLayoutV3)
 		//	.AddGlue(0)
 			.AddStrut(5)
 			.Add(printRangeLayoutV4)
 		.End()
 	;

    BBox* printRangeBox = new BBox("print_range");
    printRangeBox->SetLabel("Print Range");
    printRangeBox->SetBorder(B_FANCY_BORDER);
    printRangeBox->AddChild(printRangeLayout->View());
    
    fCopiesTC = new BTextControl("Number of copies:", "1", nullptr);
    fCollateCB = new BCheckBox("Collate", nullptr);
    
    BGroupLayout* 
	copiesLayout = BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
		.SetInsets(5, 5, 5, 5)
		.Add(fCopiesTC)
		.AddStrut(3)
    	.Add(fCollateCB)
    	.AddGlue(0)
	;
    
    BBox* copiesBox = new BBox("copies");
    copiesBox->SetLabel("Copies");
    copiesBox->SetBorder(B_FANCY_BORDER);
    copiesBox->AddChild(copiesLayout->View());
    
    
    auto scalingTypeSV = new BStringView("scaling_name", "Scaling type:");
	scalingTypeSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	
	fScalingTypeOP = new BOptionPopUp("scaling_type_op", "", new BMessage(M_SCALING));
	fScalingTypeOP->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fScalingTypeOP->AddOptionAt("None", SCALE_NONE, 0);
	fScalingTypeOP->AddOptionAt("Fit to printer margins", FIT_MARGINS, 1);
	fScalingTypeOP->AddOptionAt("Fit to printer page", FIT_PAGE, 2);
	fScalingTypeOP->AddOptionAt("Custom scale", SCALE_CUSTOM, 3);
	fScalingTypeOP->AddOptionAt("Multiple pages per sheet", MULTIPLE_PAGES, 4);
    
	fCardLayout1 = new BCardLayout();
	
	BGroupLayout* 
	printHandlingLayoutV1 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.Add(scalingTypeSV)
		.Add(fCardLayout1)
		.AddGlue(0)
	;
    
    auto scaleSV = new BStringView("scale_sv", "Scale:");
	scaleSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	
	fScaleCLI1 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
        .Add(scaleSV)
        .AddGlue(0)
    ;
    
    auto pagesPerSheetSV = new BStringView("", "Pages per sheet:");
    auto pageOrderSV = new BStringView("", "Page order:");
    
    pagesPerSheetSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
    pageOrderSV->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
    
   	fMultiplePagesCLI1 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
        .Add(pagesPerSheetSV)
        .Add(pageOrderSV)
        //.AddGlue(1)
        .Add(new BStringView("", ""))
    ;
	
	fCardLayout1->AddItem(fScaleCLI1);
	fCardLayout1->AddItem(fMultiplePagesCLI1);
	//fCardLayout1->SetVisibleItem(fScaleCLI1);
    
	fCardLayout2 = new BCardLayout();
	
	auto autoCenterCB = new BCheckBox("Auto-Center", nullptr);
	auto autoRotateCB = new BCheckBox("Auto-Rotate", nullptr);
	
	autoCenterCB->SetValue(true);
	autoRotateCB->SetValue(true);
	
	BGroupLayout* 
	printHandlingLayoutV2 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.Add(fScalingTypeOP)
		.Add(fCardLayout2)
		.Add(autoCenterCB)
		.Add(autoRotateCB)
	;
		
	fScaleTC = new BTextControl("", "100%", nullptr);
	
	fScaleCLI2 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
        .Add(fScaleTC)
        .AddGlue(0)
    ;
    
    fPagesPerSheetOP = new BOptionPopUp("", "", new BMessage(M_PAGES_PER_SHEET));
    fPagesPerSheetOP->AddOptionAt("2", 2, 0);
    fPagesPerSheetOP->AddOptionAt("4", 4, 1);
    fPagesPerSheetOP->AddOptionAt("6", 6, 2);
    fPagesPerSheetOP->AddOptionAt("9", 9, 3);
    fPagesPerSheetOP->AddOptionAt("16", 16, 4);
    fPagesPerSheetOP->AddOptionAt("Custom...", 0, 5);
    
    fXPagesPerSheetTC = new BTextControl("", "", nullptr);
    fXPagesPerSheetTC->SetEnabled(false);
    fXPagesPerSheetTC->SetText("1");
    fYPagesPerSheetTC = new BTextControl("", "", nullptr);
    fYPagesPerSheetTC->SetEnabled(false);
    fYPagesPerSheetTC->SetText("2");
    
    fPageOrderOP = new BOptionPopUp("", "", new BMessage(M_PAGE_ORDER));
    fPageOrderOP->AddOptionAt("Horizontal", ORDER_HORIZONTAL, 0);
    fPageOrderOP->AddOptionAt("Horizontal Reversed", ORDER_HORIZONTAL_R, 1);
    fPageOrderOP->AddOptionAt("Vertical", ORDER_VERTICAL, 2);
    fPageOrderOP->AddOptionAt("Vertical Reversed", ORDER_VERTICAL_R, 3);
    
    auto drawPagesBorderCB = new BCheckBox("Draw pages border", nullptr);
    
    fMultiplePagesCLI2 = BLayoutBuilder::Group<>(B_VERTICAL, 0)
    	.AddGroup(B_HORIZONTAL, 0)
        	.Add(fPagesPerSheetOP)
        	.AddStrut(2)
        	.Add(fXPagesPerSheetTC)
        	.AddStrut(2)
        	.Add(new BStringView("", "by"))
        	.AddStrut(2)
        	.Add(fYPagesPerSheetTC)
        	.AddGlue(1000)
        .End()
        .Add(fPageOrderOP)
        .Add(drawPagesBorderCB)
    ;
	
	fCardLayout2->AddItem(fScaleCLI2);
	fCardLayout2->AddItem(fMultiplePagesCLI2);
	//fCardLayout2->SetVisibleItem(fScaleCLI2);
	
	fCardLayout1->SetVisible(false);
	fCardLayout2->SetVisible(false);
	
	printHandlingLayoutV2->AlignLayoutWith(printHandlingLayoutV1, B_VERTICAL);
	
    BGroupLayout* 
	printHandlingLayout = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.SetInsets(5, 5, 5, 5)
		//.AddStrut(20)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(printHandlingLayoutV1)
			.AddStrut(5)
			.Add(printHandlingLayoutV2)
			.AddGlue(0)
		.End()
		.AddGlue(0)
    ;

    BBox* printHandlingBox = new BBox("print_handling");
    printHandlingBox->SetLabel("Print Handling");
    printHandlingBox->SetBorder(B_FANCY_BORDER);
    printHandlingBox->AddChild(printHandlingLayout->View());
    
  	
  	fPreviewPageTC = new BTextControl("", "1", nullptr);
  	auto width = fPreviewPageTC->StringWidth("100 of 100");
  	fPreviewPageTC->SetExplicitMinSize(BSize(width, 0));
        				
  	unique_ptr<BMessage> navigationMessage(new BMessage(M_PAGENUMBER));
	fPageNavigationView = new PageNavigationView(move(navigationMessage));
	fPageNavigationView->SetValue(1);
	fPageNavigationView->SetMaxValue(100);
	fPageNavigationView->SetExplicitMinSize(BSize(B_SIZE_UNSET, 30));
    
    fPrintPreviewView = new PrintPreviewView();
	
	BGroupLayout* 
	printPreviewLayout = BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(5, 5, 5, 5)
		.AddGroup(B_HORIZONTAL, 0)
    		.AddGlue(0)
    		.Add(fPrintPreviewView)
    	.End()
    	.AddGroup(B_HORIZONTAL, 0)
    		.AddGlue(0)
    		.Add(fPageNavigationView, 0)
    		.AddGlue(0)
    	.End()
    ;

    BBox* printPreviewBox = new BBox("preview");
    printPreviewBox->SetLabel("Preview");
    printPreviewBox->SetBorder(B_FANCY_BORDER);
    printPreviewBox->AddChild(printPreviewLayout->View());
    
    
    auto okBT = new BButton("ok_button", "Print Now", new BMessage(M_OK));
    auto cancelBT = new BButton("cancel", "Cancel", new BMessage(M_CANCEL));
    
    okBT->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
    cancelBT->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	

    BLayoutBuilder::Group<>(this, B_VERTICAL, 5)
        .SetInsets(10, 10, 10, 10)
        .Add(printerBox)
        .AddGroup(B_HORIZONTAL, 5, 1)
        	.AddGroup(B_VERTICAL, 5, 0)
        		.Add(printRangeBox)
        		.Add(copiesBox)
        		.Add(printHandlingBox)
        	.End()
        	.Add(printPreviewBox, 1)
        .End()
        //.AddGlue(0)
        .AddGroup(B_HORIZONTAL, 5)
        	.AddGlue(0)
        	.Add(okBT)
        	.Add(cancelBT)
        .End()
    ;
    
    _LoadSettings();
    CenterIn(fTargetWindow->Frame());
    Show();
}


bool
PrintingWindow::QuitRequested(void)
{
    return true;
}


void
PrintingWindow::MessageReceived(BMessage* message)
{
    switch (message->what) {
    	case M_SCALING:
    	{
    		switch (fScalingTypeOP->Value()) {
    			case SCALE_CUSTOM:
    				fCardLayout1->SetVisible(true);
    				fCardLayout1->SetVisibleItem(fScaleCLI1);
    				fCardLayout2->SetVisible(true);
    				fCardLayout2->SetVisibleItem(fScaleCLI2);
    				break;
    				
    			case MULTIPLE_PAGES:
    				fCardLayout1->SetVisible(true);
    				fCardLayout1->SetVisibleItem(fMultiplePagesCLI1);
    				fCardLayout2->SetVisible(true);
    				fCardLayout2->SetVisibleItem(fMultiplePagesCLI2);
    				break;
    				
    			case SCALE_NONE:
    			case FIT_MARGINS:
    			case FIT_PAGE:
    				fCardLayout1->SetVisible(false);
    				fCardLayout2->SetVisible(false);
    				break;
    		
    			default:
    				break;
    		}
    		
    		break;	
    	}
    	
    	case M_PAGES_PER_SHEET:
    	{
    		auto value = fPagesPerSheetOP->Value();		
    		switch (value) {
    			case 0:
    				fXPagesPerSheetTC->SetEnabled(true);
    				fYPagesPerSheetTC->SetEnabled(true);
    				break;
    				
    			case 2:
    				fXPagesPerSheetTC->SetText("1");
    				fYPagesPerSheetTC->SetText("2");
    				fXPagesPerSheetTC->SetEnabled(false);
    				fYPagesPerSheetTC->SetEnabled(false);
    				break;
    				
    			case 6:
    				fXPagesPerSheetTC->SetText("2");
    				fYPagesPerSheetTC->SetText("3");
    				fXPagesPerSheetTC->SetEnabled(false);
    				fYPagesPerSheetTC->SetEnabled(false);
    				break;
    				
    			default:
    			{
    				BString valueStr;
    				valueStr << static_cast<int>(std::sqrt(value));
    				fXPagesPerSheetTC->SetText(valueStr);
    				fYPagesPerSheetTC->SetText(valueStr);
    				fXPagesPerSheetTC->SetEnabled(false);
    				fYPagesPerSheetTC->SetEnabled(false);
    				break;
    			}
    		}
    		
    		break;
    	}
    	
        case M_OK:
            _SaveSettings();
            Close();
            break;

        case M_CANCEL:
            Close();
            break;

        case M_HELP:
            break;
          
            
     	case MSG_SETUP_PRINTER:
     		fTargetWindow->PostMessage(message);
     		break;
     		
     		

		default:
            BWindow::MessageReceived(message);
            break;
	}
}


void
PrintingWindow::_SaveSettings(void)
{
    Settings settings("printing_wnd");
}


void
PrintingWindow::_LoadSettings(void)
{
    Settings settings("printing_wnd");
}
