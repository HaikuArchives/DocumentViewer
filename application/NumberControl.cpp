/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "NumberControl.h"

#include <stdlib.h>

#include <Font.h>
#include <Window.h>

using namespace std;

NumberControl::NumberControl(unique_ptr<BMessage> message)
	:
    BTextView("number_control"),
    BInvoker(message.release(), NULL, NULL)
{
	BFont font;
    font.SetSize(16);
    font.SetFlags(Flags() | B_FORCE_ANTIALIASING);
    font.SetFace(B_CONDENSED_FACE | B_BOLD_FACE);
    rgb_color color = make_color(0, 0, 0, 255);
    
    SetFontAndColor(&font, B_FONT_ALL, &color);
    
    SetAlignment(B_ALIGN_HORIZONTAL_CENTER);
    //TODO:  
    //SetWordWrap(false);
    
    font_height fontHeight;
    GetFontHeight(&fontHeight);
    auto height = fontHeight.ascent + fontHeight.descent + fontHeight.leading;
    height *= 1.35;
    SetExplicitMinSize(BSize(StringWidth("999999"), height));
    SetExplicitMaxSize(BSize(StringWidth("999999"), height));
}
	

void
NumberControl::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1) {
		auto value = bytes[0];
		switch (value) {
			case B_ENTER:
				SetValue(atoi(Text()));
				Invoke();
				MakeFocus(false);
				break;
			
			case B_DELETE:
			case B_INSERT:
			case B_BACKSPACE:
				BTextView::KeyDown(bytes, numBytes);
				break;
				
    	  	default:
    	  		if (value >= '0' && value <= '9' && TextLength() < 4)
					BTextView::KeyDown(bytes, numBytes);
       		  	
       		  	break;
        }
    }
}


float
NumberControl::Value(void)
{
	return fValue;	
}


void
NumberControl::SetValue(float const& value)
{
	fValue = value;
	BString str;
	str << static_cast<int>(fValue);
	SetText(str);	
}
