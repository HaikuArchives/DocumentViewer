/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */

 #include "ImagePopUp.h"

 #include <PopUpMenu.h>

ImagePopUp::ImagePopUp(const char* name, const char* label, BMessage* message,
    uint32 flags)
        :
        BOptionControl(name, label, message, flags)
{

}


ImagePopUp::~ImagePopUp()
{

}


void
ImagePopUp::Draw(BRect updateRect)
{
    SetHighColor(255, 255, 255, 255);
    FillRect(Bounds());
}

void
ImagePopUp::MessageReceived(BMessage* message)
{

}


status_t
ImagePopUp::AddOption(const char* name, int32 value)
{

    return 0;
}


bool
ImagePopUp::GetOptionAt(int32 index, const char** _name, int32* _value)
{

    return true;
}


void
ImagePopUp::RemoveOptionAt(int32 index)
{


}


int32
ImagePopUp::CountOptions() const
{

    return 0;
}


status_t
ImagePopUp::AddOptionAt(const char* name, int32 value, int32 index)
{

    return 0;
}


int32
ImagePopUp::SelectedOption(const char** name, int32* outValue) const
{

    return 0;
}
