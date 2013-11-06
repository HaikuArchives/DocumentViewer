/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef IMAGEPOPUP_H
#define IMAGEPOPUP_H

#include <MenuField.h>
#include <OptionControl.h>

class ImagePopUp : public BOptionControl
{
public:
                                ImagePopUp(const char* name,
									const char* label, BMessage* message,
									uint32 flags = B_WILL_DRAW);
    virtual						~ImagePopUp();

    virtual void                Draw(BRect updateRect);

	virtual	void				MessageReceived(BMessage* message);

			status_t			AddOption(const char* name, int32 value);
	virtual	bool				GetOptionAt(int32 index, const char** _name,
									int32* _value);
	virtual	void				RemoveOptionAt(int32 index);
	virtual	int32				CountOptions() const;
	virtual	status_t			AddOptionAt(const char* name, int32 value,
									int32 index);
	virtual	int32				SelectedOption(const char** name = nullptr,
									int32* outValue = nullptr) const;

private:

};


#endif
