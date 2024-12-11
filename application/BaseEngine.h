/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef BASEENGINE_H
#define BASEENGINE_H

#include <memory>
#include <stdlib.h>
#include <utility>
#include <vector>

#include <Bitmap.h>
#include <Handler.h>
#include <Looper.h>
#include <OutlineListView.h>
#include <Point.h>
#include <Rect.h>

#include <ColumnListView.h>
#include "Debug.h"


class BaseEngine
{
public:
        					BaseEngine(void);
    virtual 				~BaseEngine();

    void					Start(void);
    void					Stop(void);

    void					StopTextSearch(void);

    // the name of the file this engine handles
    virtual BString 		FileName(void) const;
    // number of pages the loaded document contains
    virtual int 			PageCount(void) const = 0;

    // the box containing the visible page content
    // (usually BRect(0, 0, pageWidth, pageHeight))
    virtual BRect 			PageMediabox(int pageNumber);
    // the box inside PageMediabox that actually contains any relevant content
    // (used for auto-cropping in Fit Content mode, can be PageMediabox)
    virtual BRect 			PageContentBox(int pageNumber);

    virtual void			SetZoom(float zoomFactor);

    // the angle in degrees the given page is rotated natively (usually 0 deg)
    virtual int const&		PageRotation(int pageNumber);

    virtual std::unique_ptr<BBitmap> RenderBitmap(int const& pageNumber,uint32 const& width,
    										uint32 const& height, int const& rotation = 0) = 0;

    virtual BBitmap* 		Page(int pageNumber);

    // access to various document properties (such as Author, Title, etc.)
    virtual BString 		GetProperty(BString name) { return BString(""); }

	virtual BString 		GetPageLabel(int pageNumber);

    // reverts GetPageLabel by returning the first page number
    // having the given label
    virtual int 			GetPageByLabel(BString label);

    // returns a string to remember when the user wants to save
    // a document's password (don't implement for document types
    // that don't support password protection)
    virtual BString 		GetDecryptionKey(void) const;

    virtual void 			SetCacheSize(int forwardCache, int backwardCache = 0);
    virtual void 			MultiplyZoom(float factor);

    virtual	void 			WriteOutline(BOutlineListView* list);

    virtual	void			FindString(BString const& name, BLooper* looper, BHandler* handler,
    							int32 flag = 0);

   			bool			HighlightUnderText(void);

	bool					fStopThread;

protected:
    virtual std::pair<BBitmap*, bool> 	_RenderBitmap(int const& pageNumber) = 0;
    virtual std::tuple< std::vector<BString>, std::vector<BRect> >
    									_FindString(BString const& name, int const& page);


	static void* 						_DrawingThread(void* arg);

	pthread_t	                            	fDrawingThread;

	static pthread_mutex_t						gEngineStopMutex;


    float                           			fZoomFactor;
    int                             			fPages;
    int                             			fRotation;
    int                             			fForwardCache;
    int                             			fBackwardCache;
    int                             			fCurrentPageNo;

    int32										fSearchFlag;

    std::vector<std::pair< BBitmap*, bool> >	fBitmap;
    std::vector<pthread_mutex_t>            	fMutex;

    BRect										fDefaultRect;
    bool										fHighlightUnderText;

	Debug										out;

private:
    static void* 						_TextSearchThread(void* arg);

    pthread_t	                        fTextSearchThread;

    bool								fStopTextSearchThread;

    static pthread_mutex_t				gTextSearchStopMutex;

    BLooper*							fTargetLooper;
    BHandler*							fSearchHandler;

    BString								fSearchString;
};

#endif
