/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "ImageSpinner.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <functional>

#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <ScrollBar.h>


#include <TranslationKit.h>

#include "Messages.h"
#include "Settings.h"
#include "Tools.h"


using namespace std;

ImageSpinner::ImageSpinner(float const& imageHeight)
	:
	BGroupView("image_spinner", B_VERTICAL, 0),
	fImageHeight(imageHeight)
{

	fBasicImageSpinner = new BasicImageSpinner;
	fBasicImageSpinner->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, fImageHeight + 6));
	fBasicImageSpinner->SetExplicitMinSize(BSize(0, 0/*fImageHeight + 6*/));

	fDeleteButton = new ImageButton("quit",
		new BMessage(M_DELETE), 0.3, 1, "Remove document");
	fBackButton = new ImageButton("back",
		new BMessage(M_MOVE_LEFT), 0.3, 1, "Previous document");
	fNextButton = new ImageButton("next",
		new BMessage(M_MOVE_RIGHT), 0.3, 1, "Next document");
    fOpenButton = new ImageButton("open_document",
    	new BMessage(MSG_OPEN_FILE_PANEL), 0.3, 1, "Open document");


  	BGroupLayout* navigationLayout = BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
  		.Add(fDeleteButton)
    	.Add(fBackButton)
    	.Add(fNextButton)
    	.Add(fOpenButton, 10)
    	//.AddStrut(12)
    ;

    navigationLayout->SetExplicitMinSize(BSize(0, 20));
    navigationLayout->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 50));

    auto scrollbar = new BScrollBar("h_scrollbar",
                    fBasicImageSpinner, 0, 0, B_HORIZONTAL);
	scrollbar->SetSteps(200, 200);

	BLayoutBuilder::Group<>(this)
		.Add(scrollbar)
		.AddGlue(0)
		.Add(fBasicImageSpinner)
		//.AddStrut(10)
		.AddGlue(0)
    	.Add(navigationLayout)
    .End()
	;
}


void
ImageSpinner::AttachedToWindow(void)
{
	fDeleteButton->SetTarget(this);
	fBackButton->SetTarget(this);
	fNextButton->SetTarget(this);
	fBasicImageSpinner->MakeFocus(true);
	BGroupView::AttachedToWindow();
}


void
ImageSpinner::MakeFocus(bool focus)
{
	BGroupView::MakeFocus(focus);
	fBasicImageSpinner->MakeFocus(focus);
}

void
ImageSpinner::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case M_BACK:
			fBasicImageSpinner->Back();
            break;

        case M_NEXT:
        	fBasicImageSpinner->Next();
            break;

       	case M_DELETE:
       		fBasicImageSpinner->RemoveSelectedItem();
       		break;

      	case M_MOVE_LEFT:
      		fBasicImageSpinner->MoveLeftSelectedItem();
      		break;

      	case M_MOVE_RIGHT:
      		fBasicImageSpinner->MoveRightSelectedItem();
      		break;

        default:
            BGroupView::MessageReceived(message);
            break;
    }
}


void
ImageSpinner::Add(BString const& str, std::unique_ptr<BBitmap>&& bitmap)
{
	if (bitmap->Bounds().Height() != fImageHeight) {
		auto res = Tools::RescaleBitmap(move(bitmap), 0, fImageHeight);
		fBasicImageSpinner->Add(str, move(res));
	} else {
		fBasicImageSpinner->Add(str, move(bitmap));
	}
}


bool
ImageSpinner::NeedsFile(BString const& filePath)
{
	return fBasicImageSpinner->NeedsFile(filePath);
}


float
ImageSpinner::PreferredImageHeight(void)
{
	return fImageHeight;
}


BasicImageSpinner::BasicImageSpinner(void)
	:
    BView(BRect(), "basic_image_view", B_FOLLOW_ALL,
            B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_FRAME_EVENTS),
    fOldMousePos(0, 0),
    fMouseButtons(0),
  	fSpace(5),
  	fTotalWidth(fSpace),
  	fIsPanning(false),
  	fMaxItemsNumber(10),
  	fCurrentIndex(0)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
    //SetDrawingMode(B_OP_ALPHA);
    //SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);


    fSettingsPath = Tools::SettingsPath();

	_LoadSettings();
}


BasicImageSpinner::~BasicImageSpinner()
{
	_SaveSettings();
}


void
BasicImageSpinner::AttachedToWindow(void)
{
	_AdaptScrollBarRange();
	BView::AttachedToWindow();
}


void
BasicImageSpinner::Draw(BRect updateRect)
{
	BRect bounds = Bounds();
    SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    FillRect(bounds);

    if (fItems.empty())
    	return;

    SetPenSize(2);
    SetHighColor(0, 0, 0, 255);

    float height = get<0>(fItems.front())->Bounds().Height();

    BPoint offset(fSpace, (bounds.Height() - height) / 2);

    if (offset.y < 0)
    	offset.y = 0;

    bool hasDrawn = false;
   	int index = 0;
    for (auto it = fItems.begin(); it != fItems.end(); ++it, ++index) {
    	BBitmap* bitmap = (BBitmap*)(&*get<0>(*it));
    	auto rect = bitmap->Bounds().OffsetByCopy(offset);
    	if (rect.Intersects(updateRect)) {
    		hasDrawn = true;
	    	DrawBitmapAsync(bitmap, rect/*.LeftTop()*/);
	    	if (index == fCurrentIndex) {
	    		SetHighColor(0, 0, 255, 255);
	    		SetPenSize(3);
	    		StrokeRect(rect);
	    		SetPenSize(2);
	    		SetHighColor(0, 0, 0, 255);
	    	} else {
	    		StrokeRect(get<0>(*it)->Bounds().OffsetByCopy(offset));
	    	}
    	} else if (hasDrawn) {
        	break;
        }
    	offset.x += get<0>(*it)->Bounds().Width() + fSpace;
    }

	Sync();
	BView::Draw(updateRect);
}



void
BasicImageSpinner::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1) {
		switch (bytes[0]) {
			case B_ENTER:
				_SelectIndex(fCurrentIndex);
				break;

			case B_DELETE:
				RemoveSelectedItem();
				break;

			case B_DOWN_ARROW: case B_RIGHT_ARROW:
				_Next();
                break;

      		case B_UP_ARROW: case B_LEFT_ARROW:
				_Back();
                break;

            case B_PAGE_UP:
				break;

            case B_PAGE_DOWN:
                break;

            case B_HOME:
                _First();
                break;

            case B_END:
                _Last();
                break;


    	  	default:
				BView::KeyDown(bytes, numBytes);
       		  	break;
        }
    }
}



void
BasicImageSpinner::FrameResized(float newWidth, float newHeight)
{

	_AdaptScrollBarRange();
	BView::FrameResized(newWidth, newHeight);
}


void
BasicImageSpinner::MessageReceived(BMessage* message)
{

	BView::MessageReceived(message);
}


void
BasicImageSpinner::MouseMoved(BPoint where, uint32 code,
    const BMessage* dragMessage)
{
    switch (code) {
		case B_EXITED_VIEW:
            fIsPanning  = false;
            break;

		default:
            break;
	}

    if (fIsPanning == true) {
        BPoint delta = fOldMousePos - where;

        ScrollBar(B_HORIZONTAL)->SetValue(
            ScrollBar(B_HORIZONTAL)->Value() + delta.x);
    }

    BView::MouseMoved(where, code, dragMessage);
}


void
BasicImageSpinner::MouseDown(BPoint where)
{
    MakeFocus(true);
    GetMouse(&fOldMousePos, &fMouseButtons);

    int32 clicks = 0;
    Window()->CurrentMessage()->FindInt32("clicks", &clicks);

    if (clicks == 2) {
    	_Select(fOldMousePos);
    } else {
    	_HighlightPosition(fOldMousePos);
        fIsPanning = true;
    }

    BView::MouseDown(where);
}


void
BasicImageSpinner::MouseUp(BPoint where)
{
    fIsPanning = false;

    BView::MouseUp(where);
}


bool
BasicImageSpinner::NeedsFile(BString const& filePath)
{
	for (auto it = fItems.begin(); it != fItems.end(); ++it) {
		if (filePath == get<1>(*it)) {
			auto temp = move(*it);
			fItems.remove(*it);
			fItems.push_front(move(temp));
			return false;
		}
	}

	return true;
}


void
BasicImageSpinner::Add(BString const& filePath, std::unique_ptr<BBitmap>&& bitmap)
{
	uniform_int_distribution<int> distribution(0, 999);
    mt19937 engine(time(NULL)); //Mersenn twister
    auto generate = bind(distribution, engine);  // RNG


    int idNumber;
    bool isInvalid;

    // generate a random Number, and make sure it's unique.
    do {
    	isInvalid = false;
   		idNumber = generate();
    	for (auto it = fItems.begin(); it != fItems.end(); ++it) {
    		if (get<2>(*it) == idNumber) {
    			isInvalid = true;
    			break;
    		}
    	}
    } while (isInvalid);

    // save bitmap to disk
    BString settingsPath(Tools::SettingsPath());
    BString path(settingsPath);
    path << idNumber;
    Tools::ExportBitmap(bitmap.get(), path);

    _Add(filePath, move(bitmap), idNumber);
}


void
BasicImageSpinner::_Add(BString const& filePath,
	std::unique_ptr<BBitmap>&& bitmap, int const& bitmapID)
{
	fItems.push_front(tuple<unique_ptr<BBitmap>, BString, int>
		(move(bitmap), filePath, bitmapID));

	fTotalWidth += get<0>(fItems.front())->Bounds().Width() + fSpace;

	if (fItems.size() > fMaxItemsNumber)
		RemoveBackItem();

	_AdaptScrollBarRange();
}


void
BasicImageSpinner::MoveLeftSelectedItem(void)
{
	if (fCurrentIndex < 1 || fItems.size() < 2)
		return;



	auto it = fItems.begin();
	for (int i = 1; i < fCurrentIndex; ++i)
		++it;

	auto left = it;
	++it;
	swap(*left, *it);
	_Back();
}


void
BasicImageSpinner::MoveRightSelectedItem(void)
{
	if (fCurrentIndex > (fItems.size() - 2)
		|| fItems.size() < 2)
		return;

	float width = fSpace;
	auto it = fItems.begin();
	for (int i = 0; i < fCurrentIndex; ++i)
		++it;

	auto left = it;
	++it;
	swap(*left, *it);

	_Next();
}


void
BasicImageSpinner::RemoveBackItem(void)
{
	fTotalWidth -= (get<0>(fItems.back())->Bounds().Width() + fSpace);

	BString path(Tools::SettingsPath());
	path << get<2>(fItems.back());
	BEntry entry(path);
	entry.Remove();
	fItems.pop_back();
	_AdaptScrollBarRange();
	Invalidate();
}


void
BasicImageSpinner::RemoveSelectedItem(void)
{
	if (fItems.empty() || fCurrentIndex < 0)
		return;

	auto it = fItems.begin();
	for (int i = 0; i < fCurrentIndex; ++i)
		++it;

	fTotalWidth -= (get<0>(*it)->Bounds().Width() + fSpace);

	BString path(Tools::SettingsPath());
	path << get<2>(*it);
	BEntry entry(path);
	entry.Remove();
	fItems.remove(*it);

	--fCurrentIndex;
	if (fCurrentIndex < 0)
		fCurrentIndex = 0;

	_AdaptScrollBarRange();
	Invalidate();
}


void
BasicImageSpinner::_Select(BPoint const& pos)
{
	float temp = 0;
	for (auto it = fItems.begin(); it != fItems.end(); ++it) {
		temp += get<0>(*it)->Bounds().Width() + fSpace;
		if (temp < pos.x)
			continue;

		BMessage msg(MSG_OPEN_FILE);
		msg.AddString("file", get<1>(*it));
		Window()->PostMessage(&msg);
		_First();
		break;
	}
}


void
BasicImageSpinner::_HighlightPosition(BPoint const& pos)
{
	float temp = 0;
	int counter = 0;
	for (auto it = fItems.begin(); it != fItems.end(); ++it, ++counter) {
		temp += get<0>(*it)->Bounds().Width() + fSpace;
		if (temp >= pos.x)
			break;
	}

	fCurrentIndex = counter;

	if (fCurrentIndex >= fItems.size())
		fCurrentIndex = -1;

	Invalidate();
}


void
BasicImageSpinner::_SelectIndex(int const& index)
{
	int counter = 0;
	for (auto it = fItems.begin(); it != fItems.end(); ++it) {
		if (counter < index) {
			++counter;
			continue;
		}

		BMessage msg(MSG_OPEN_FILE);
		msg.AddString("file", get<1>(*it));
		Window()->PostMessage(&msg);
		_First();
		break;
	}
}


void
BasicImageSpinner::_SaveSettings(void)
{
#if 0 // This is known to crash DocumentViewer in some cases, see issue #2
	BString settingsPath(Tools::SettingsPath());
	BString path = settingsPath;
	path.Append("RecentlyOpened");
	BFile file(path, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);

    BString str;
    for (auto it = fItems.rbegin(); it != fItems.rend(); ++it) {
    	str = get<1>(*it);
    	str << "\t" << get<2>(*it) << "\n";
    	file.Write(str, str.Length());
    }
#endif
}


void
BasicImageSpinner::_LoadSettings(void)
{
#if 0 // This is known to crash DocumentViewer in some cases, see issue #2
	BString settingsPath = Tools::SettingsPath();
	BString path = settingsPath;
	path.Append("RecentlyOpened");

	BFile file(path, B_READ_ONLY | B_CREATE_FILE);
	off_t size;
  	file.GetSize(&size);
  	char buffer[size];
  	file.Read(&buffer, size);

  	fItems.clear();
  	BString str = "";
  	for (int i = 0; i < size; ++i) {
  		if (buffer[i] != '\n') {
  			str += buffer[i];
  		} else {
  			auto vec = Tools::Split(str, '\t');
  			str = "";
  			if (vec.size() != 2)
  				continue;

  			path = settingsPath;
  			path.Append(vec[1]);
  			unique_ptr<BBitmap> bitmap(BTranslationUtils::GetBitmap(path));

  			if (bitmap == nullptr)
  				return;

  			stringstream ss;
  			ss << vec[1];
  			int idNumber;
  			ss >>idNumber;
  			_Add(vec[0], move(bitmap), idNumber);

  		}
  	}
#endif
}


void
BasicImageSpinner::_ScrollToSelection(void)
{
	// ToDO : Not yet implemented

	Invalidate();
}


void
BasicImageSpinner::Next(void)
{
	_Next();
}


void
BasicImageSpinner::_Next(void)
{
	++fCurrentIndex;
	if (fCurrentIndex < 0)
		fCurrentIndex = 0;
	else if (fCurrentIndex >= fItems.size())
		fCurrentIndex = fItems.size() - 1;

	float width = fSpace;
	auto it = fItems.begin();
	for (int i = 0; i <= fCurrentIndex; ++i, ++it)
		width += get<0>(*it)->Bounds().Width() + fSpace;

	if (Bounds().right < width)
		ScrollTo(width - Bounds().Width(), 0);

	Invalidate();
}


void
BasicImageSpinner::_Back(void)
{
	--fCurrentIndex;
	if (fCurrentIndex < 0)
		fCurrentIndex = 0;
	else if (fCurrentIndex >= fItems.size())
		fCurrentIndex = fItems.size() - 1;

	float width = 0;
	auto it = fItems.begin();
	for (int i = 0; i < fCurrentIndex; ++i, ++it)
		width += get<0>(*it)->Bounds().Width() + fSpace;

	if (Bounds().left > width)
		ScrollTo(width, 0);

	Invalidate();
}


void
BasicImageSpinner::Back(void)
{
	_Back();
}


void
BasicImageSpinner::_First(void)
{
	fCurrentIndex = 0;
	Invalidate();
}


void
BasicImageSpinner::_Last(void)
{
	fCurrentIndex = fItems.size() - 1;
	Invalidate();
}


void
BasicImageSpinner::_AdaptScrollBarRange(void)
{
	float width = -Bounds().Width();
	if (width > 0)
		return;

	width += fTotalWidth;
	auto scrollbar = ScrollBar(B_HORIZONTAL);
	scrollbar->SetRange(0, width);
	scrollbar->SetProportion(1 - width/fTotalWidth);
}


BString
BasicImageSpinner::_RandomName(void)
{
	uniform_int_distribution<int> distribution(0,999);
    mt19937 engine(time(NULL)); //Mersenn twister
    auto random = bind(distribution, engine);  // RNG

    BString str;
    str << random();

	return str;
}


