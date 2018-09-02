/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef TOOLS_H
#define	TOOLS_H

#include <stdint.h>

#include <memory>
#include <vector>

#include <Bitmap.h>
#include <String.h>


#include <Application.h>
#include <IconUtils.h>
#include <Path.h>
#include <Resources.h>
#include <Roster.h>
#include <Window.h>

#include <StorageKit.h>
#include <TranslationKit.h>

class Tools {
public:
	static BString
	AppPath(void)
	{
	    app_info	info;
	    be_app->GetAppInfo(&info);
	
	    BPath path = BPath(&info.ref);
	    path.GetParent(&path);
	
	    return path.Path();
	}
	
	
	static BString
	AppName(void)
	{
	    app_info	info;
	    be_app->GetAppInfo(&info);
	
	    BPath path = BPath(&info.ref);
	    path.GetParent(&path);
	
	    return path.Leaf();
	}
	
	
	static BString
	SettingsPath(void)
	{
		BPath path;
	    if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
	        return "";
	
	    path.Append(AppName().Capitalize());
	    create_directory(path.Path(), 0755);
	    
	    BString str(path.Path());
	    str.Append("/");
	    return str;	
	}
	
	
	static std::vector<BString>
	Split(BString str, const char token, BString start = "", BString stop = "")
	{
	    std::vector<BString> vec;
	
	    int n1 = 0;
	    int n2 = str.Length();
	
	    if (start != "")
	        n1 = str.FindFirst(start, 0);
	
	    if (stop != "")
	        n2 = str.FindFirst(stop, n1 + 1);
	
	    if (n1 == B_ERROR || n2 == B_ERROR)
	        return vec;
	
	    n1 = n1 + start.Length();
	
	    int n = n1;
	
	    for (; n < n2; ++n)
	        if (str[n] == token) {
	            if (n > 0 && str[n-1] != token) {
	                char temp[n - n1 + 1];
	                str.CopyInto(temp, n1, n - n1);
	                temp[n - n1] = '\0';
	                vec.push_back(temp);
	                n1 = n + 1;
	            } else {
	                n1 = n + 1;
	            }
	        }
	
	    char temp[n - n1 + 1] ;
	    str.CopyInto(temp, n1, n - n1);
	    temp[n - n1] = '\0';
	    vec.push_back(temp);
	    return vec;
	}
	
	
	static std::vector<BString>
	Split(unsigned int length, BString str,
	    const char token, BString start, BString stop)
	{
	    std::vector<BString> vec;
	
	    int n1 = 0;
	    int n2 = str.Length();
	
	    if (start != "")
	        n1 = str.FindFirst(start, 0);
	
	    if (stop != "")
	        n2 = str.FindFirst(stop, n1 + 1);
	
	    if (n1 == B_ERROR || n2 == B_ERROR) {
	        vec.resize(length, "");
	        return vec;
	    }
	
	    n1 = n1 + start.Length();
	
	    int n = n1;
	
	    for (; n < n2; ++n)
	        if (str[n] == token) {
	            if (n > 0 && str[n-1] != token) {
	                char temp[n - n1 + 1];
	                str.CopyInto(temp, n1, n - n1);
	                temp[n - n1] = '\0';
	                vec.push_back(temp);
	                if (vec.size() == length)
	                    return vec;
	
	                n1 = n + 1;
	            } else {
	                n1 = n + 1;
	            }
	        }
	
	    char temp[n - n1 + 1] ;
	    str.CopyInto(temp, n1, n - n1);
	    temp[n - n1] = '\0';
	    vec.push_back(temp);
	    vec.resize(length, "");
	
	    return vec;
	}
	
	
	static BString
	Find(BString str, BString searchStr)
	{
	   BString result = "";
	
	  if (str.Length() == 0 || searchStr.Length() == 0)
	    return result;
	
	  int n1 = str.FindFirst(searchStr) + searchStr.Length();
	  int n2 = str.FindFirst(" ", n1);
	
	  if (n2 == B_ERROR)
		n2 = str.Length();
	
	  str.CopyInto(result, n1, n2 - n1);
	
	  return result;
	}
	
	
	static BBitmap*
	LoadBitmap(BString const& imageName, int size)
	{
	    BResources* res = be_app->AppResources();
	
		if (res == nullptr)
			return nullptr;
	
		size_t nbytes = 0;
		color_space cspace = B_RGBA32;
	
		const void* data = res->LoadResource('HVIF', imageName.String(), &nbytes);
	
	   // size--;
	
		BBitmap* bitmap = new BBitmap(BRect(0, 0, size, size), cspace);
	
		if (bitmap->InitCheck() != B_OK) {
	        delete bitmap;
	        bitmap = nullptr;
	    } else if (BIconUtils::GetVectorIcon((const uint8*)data, nbytes, bitmap)
	                 != B_OK) {
	        delete bitmap;
	        bitmap = nullptr;
	    }
	
	    res->RemoveResource(data);
		return bitmap;
	}
	
	
	static void
	ExportBitmap(BBitmap* bitmap,
		BString const& path, int32 const& format = B_PNG_FORMAT)
	{
    	BBitmapStream stream(bitmap);
    	BFile imageFile(path, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
    	BTranslatorRoster *roster = BTranslatorRoster::Default();
    	roster->Translate(&stream, NULL, NULL, &imageFile, B_PNG_FORMAT);
    	BBitmap* bmp;
    	stream.DetachBitmap(&bmp);
    		
	}
	

	static std::unique_ptr<BBitmap>
	RescaleBitmap(std::unique_ptr<BBitmap> src, int32 width, int32 height)
	{
		if (src == nullptr || src->IsValid() == false)
			throw "Tools::RescaleBitmap";
			
		if (height <= 0) {
			if (width <= 0)
				return src;
		
			height = (width * src->Bounds().Height()) / src->Bounds().Width();		
		} else if (width <= 0) {
			if (height <= 0)
				return src;
			
			width = (height * src->Bounds().Width()) / src->Bounds().Height();
		}
	
		BRect srcSize = src->Bounds();
		
		if (height < 0) {
			float srcProp = srcSize.Height()/srcSize.Width();
			height = (int32)(width * ceil(srcProp));
		}
		
		BBitmap* res = new BBitmap(BRect(0, 0, (float)width, (float)height),
	        src->ColorSpace());
	
		float dx = (srcSize.Width() + 1)/(float)(width + 1);
		float dy = (srcSize.Height() + 1)/(float)(height + 1);
		uint8 bpp = (uint8)(src->BytesPerRow()/ceil(srcSize.Width()));
	
		int srcYOff = src->BytesPerRow();
		int dstYOff = res->BytesPerRow();
	
		void* dstData = res->Bits();
		void* srcData = src->Bits();
	
		for (int32 y = 0; y <= height; y++) {
			void* dstRow = (void*)((uintptr_t)dstData + (uintptr_t)(y * dstYOff));
			void* srcRow = (void*)((uintptr_t)srcData + ((uintptr_t)(y * dy) * srcYOff));
	
			for (int32 x = 0; x <= width; x++)
				memcpy((void*)((uintptr_t)dstRow + (x * bpp)), (void*)((uintptr_t)srcRow
	                                              + ((uintptr_t)(x * dx) * bpp)), bpp);
		}
		
		std::unique_ptr<BBitmap> bitmap(res);
		return move(bitmap);
	}	

	
private:
			Tools(void);	
};

#endif
