/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef IMAGESPINNER_H
#define IMAGESPINNER_H

#include <list>
#include <memory>
#include <tuple>

#include <Bitmap.h>
#include <File.h>
#include <GroupView.h>
#include <Point.h>
#include <String.h>

#include "Debug.h"
#include "ImageButton.h"

class BasicImageSpinner : public BView
{
public:
						BasicImageSpinner(void);
	virtual 			~BasicImageSpinner();
	
	virtual void    	AttachedToWindow(void);
	virtual void    	Draw(BRect updateRect);
    virtual	void 	  	FrameResized(float newWidth, float newHeight);
    virtual void    	KeyDown(const char* bytes, int32 numBytes);
    virtual void    	MessageReceived(BMessage* message);
    virtual void 		MouseDown(BPoint where);
    virtual	void		MouseUp(BPoint where);
    virtual	void		MouseMoved(BPoint where, uint32 code,
                        	const BMessage* dragMessage);
                        
        	void		Add(BString const& str, std::unique_ptr<BBitmap>&& bitmap);
        	
        	void		Next(void);
        	void		Back(void);
        	
        	bool		NeedsFile(BString const& filePath);
        	
        	void		RemoveBackItem(void);
			void		RemoveSelectedItem(void);
			
			void		MoveLeftSelectedItem(void);
			void		MoveRightSelectedItem(void);

private:
			void		_AdaptScrollBarRange(void);
			void		_Add(BString const& filePath,
							std::unique_ptr<BBitmap>&& bitmap, int const& bitmapID);
			void		_SaveSettings(void);
			void		_LoadSettings(void);
			void		_Last(void);
			void		_Next(void);
			void		_Back(void);
			void		_First(void);
			void		_HighlightPosition(BPoint const& pos);
			void		_Select(BPoint const& pos);
			void		_SelectIndex(int const& index);
			void		_ScrollToSelection(void);
			
			BString		_RandomName(void);
			
	
	std::list< std::tuple<std::unique_ptr<BBitmap>, BString, int> >		fItems;
	
	BPoint						fOldMousePos;
	uint32	            		fMouseButtons;
	
	float						fSpace;
	float						fTotalWidth;
	bool						fIsPanning;
	int							fMaxItemsNumber;
	int							fCurrentIndex;
	BString						fAppName;
	BString						fSettingsPath;
	
		
	Debug						out;
};

class ImageSpinner : public BGroupView
{
public:
					ImageSpinner(float const& imageHeight = 500);
	virtual void    MakeFocus(bool focus);
	virtual void    MessageReceived(BMessage* message);
	virtual void    AttachedToWindow(void);
					
			void	Add(BString const& filePath, std::unique_ptr<BBitmap>&& bitmap);
			
			bool	NeedsFile(BString const& filePath);
			
			float	PreferredImageHeight(void);
										
private:

	ImageButton*		fDeleteButton;
	ImageButton*		fBackButton;
	ImageButton*		fNextButton;
	ImageButton*		fOpenButton;
	
	
	BasicImageSpinner*	fBasicImageSpinner;
	
	float				fImageHeight;
	
	enum {
		M_BACK = 'ib01', M_NEXT = 'in01', 
		M_MOVE_LEFT = 'im01', M_MOVE_RIGHT = 'im02',
		M_DELETE = 'id01'	
	};
	
};

#endif // IMAGESPINNER_H
