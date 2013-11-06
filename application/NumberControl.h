/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef NUMBERCONTROL_H
#define NUMBERCONTROL_H

#include <memory>

#include <Invoker.h>
#include <Message.h>
#include <TextView.h>

#include "Debug.h"

class NumberControl : public BTextView, public BInvoker
{
public:
                    	NumberControl(std::unique_ptr<BMessage> message);
                    	
    virtual	void		KeyDown(const char* bytes, int32 numBytes);
			void		SetValue(float const& value);
			float		Value(void);    
private:
	
	float				fValue;
	Debug				out;
    
};

#endif
