/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "BaseEngine.h"

#include "Flags.h"
#include "Messages.h"

using namespace std;

pthread_mutex_t	BaseEngine::gEngineStopMutex;
pthread_mutex_t	BaseEngine::gTextSearchStopMutex;

BaseEngine::BaseEngine(void)
	:
  	fZoomFactor(1),
  	fPages(0),
 	fRotation(0),
   	fForwardCache(16),
  	fBackwardCache(16),
 	fCurrentPageNo(0),
 	fSearchFlag(0),
 	fDefaultRect(0, 0, 300, 500),
  	fStopThread(false),
  	fStopTextSearchThread(false),
  	fHighlightUnderText(false)
{
	fTextSearchThread = nullptr;
	fDrawingThread = nullptr;
}


BaseEngine::~BaseEngine()
{
	StopTextSearch();
	
	for (std::pair< BBitmap*, bool> bitmap : fBitmap) {
    	if (bitmap.first != nullptr) {
    		delete bitmap.first;
    		bitmap.first = nullptr;
    	}
	}
}


void
BaseEngine::Start(void)
{
	fPages = PageCount();

    fBitmap.resize(fPages, std::pair<BBitmap*, bool>((BBitmap*)nullptr, false));
    fMutex.resize(fPages);

	pthread_create(&fDrawingThread, nullptr, _DrawingThread, (void*)(this));
}


void
BaseEngine::Stop(void)
{
	pthread_mutex_lock(&gEngineStopMutex);
	fStopThread = true;
	pthread_mutex_unlock(&gEngineStopMutex);
	
	while (true) {
		usleep(1000);
		pthread_mutex_lock(&gEngineStopMutex);
		if (fStopThread == false) {
			pthread_mutex_unlock(&gEngineStopMutex);
			break;
		}
		pthread_mutex_unlock(&gEngineStopMutex);
	}
}


void*
BaseEngine::_DrawingThread(void* arg)
{
    BaseEngine*  engine =(BaseEngine*)arg;
    int& pages = engine->fPages;
    std::vector< std::pair<BBitmap*, bool> >& bitmap = engine->fBitmap;
    std::vector< pthread_mutex_t >& mutex = engine->fMutex;

    int upperbound = -1;
    int lowerbound = -1;
    int pageFuture = -1;
    int pagePast   = -1;
    int currentPage = -1;
    int deleteIndex = 0;
    bool forwardPriority = true;
	
	while(true) {
		pthread_mutex_lock(&gEngineStopMutex);
		if (engine->fStopThread) {
			engine->fStopThread = false;
			pthread_mutex_unlock(&gEngineStopMutex);
			return nullptr;			
		}
		pthread_mutex_unlock(&gEngineStopMutex);
		
        if (currentPage != engine->fCurrentPageNo) {
            if (engine->fCurrentPageNo < currentPage)
                forwardPriority = false;
            else
                forwardPriority = true;

            deleteIndex = 0;
            currentPage = engine->fCurrentPageNo;
            pageFuture = pagePast = currentPage;
            lowerbound =  currentPage - engine->fBackwardCache;
            upperbound =  currentPage + engine->fForwardCache;

            if (lowerbound < 0)         lowerbound = 0;
            if (upperbound >= pages)    upperbound = pages - 1;
        } else {
            for (; deleteIndex < pages; ++deleteIndex) {
                pthread_mutex_lock(&mutex[deleteIndex]);
                if (bitmap[deleteIndex].first != nullptr
                    && (deleteIndex < lowerbound || deleteIndex > upperbound)) {
                    delete bitmap[deleteIndex].first;
                    bitmap[deleteIndex].first = nullptr;
                    pthread_mutex_unlock(&mutex[deleteIndex]);
                    break;
                }
                pthread_mutex_unlock(&mutex[deleteIndex]);
            }

            if (forwardPriority) {
                if (pageFuture < upperbound) {
                    ++pageFuture;
                    pthread_mutex_lock(&mutex[pageFuture]);
                    if (bitmap[pageFuture].first == nullptr) {
                        bitmap[pageFuture] = engine->_RenderBitmap(pageFuture);
                    } else if (bitmap[pageFuture].second == true) {
                        delete bitmap[pageFuture].first;
                        bitmap[pageFuture] = engine->_RenderBitmap(pageFuture);
                    }
                    pthread_mutex_unlock(&mutex[pageFuture]);
                } else if (pagePast > lowerbound) {
                    --pagePast;
                    pthread_mutex_lock(&mutex[pagePast]);
                    if (bitmap[pagePast].first == nullptr) {
                        bitmap[pagePast] = engine->_RenderBitmap(pagePast);
                    } else if (bitmap[pageFuture].second == true) {
                        delete bitmap[pageFuture].first;
                        bitmap[pageFuture] = engine->_RenderBitmap(pageFuture);
                    }
                    pthread_mutex_unlock(&mutex[pagePast]);
                } else {
                    usleep(1000);
                }
            } else {
                if (pagePast > lowerbound) {
                    --pagePast;
                    pthread_mutex_lock(&mutex[pagePast]);
                    if (bitmap[pagePast].first == nullptr)
                        bitmap[pagePast] = engine->_RenderBitmap(pagePast);

                    pthread_mutex_unlock(&mutex[pagePast]);
                } else if (pageFuture < upperbound) {
                    ++pageFuture;
                    pthread_mutex_lock(&mutex[pageFuture]);
                    if (bitmap[pageFuture].first == nullptr)
                        bitmap[pageFuture] = engine->_RenderBitmap(pageFuture);

                    pthread_mutex_unlock(&mutex[pageFuture]);
                } else {
                    usleep(1000);
                }
            }
        }
    }
	return nullptr;
}


int const&
BaseEngine::PageRotation(int pageNumber)
{
    return fRotation;
}


void
BaseEngine::SetZoom(float zoomFactor)
{
    if (fZoomFactor == zoomFactor)
        return;

    fZoomFactor = zoomFactor;
    fDefaultRect.right 	= 300 * fZoomFactor;
    fDefaultRect.bottom = 500 * fZoomFactor;

	// Mark all existing bitmap as dirty as they need to be rendered again.
    for (int i = 0; i < fPages; ++i)
        if (fBitmap[i].first != nullptr)
            fBitmap[i].second = true;
}


BBitmap*
BaseEngine::Page(int pageNumber)
{
    if (pageNumber < 0)
        pageNumber = 0;
    else if (pageNumber >= fPages - 1)
        pageNumber = fPages - 1;

    pthread_mutex_lock(&fMutex[pageNumber]);
    if (fBitmap[pageNumber].first == nullptr) {
        fBitmap[pageNumber] = _RenderBitmap(pageNumber);
    } else if (fBitmap[pageNumber].second == true) {
        delete fBitmap[pageNumber].first;
        fBitmap[pageNumber] = _RenderBitmap(pageNumber);
    }
    pthread_mutex_unlock(&fMutex[pageNumber]);
    fCurrentPageNo = pageNumber;

    return fBitmap[pageNumber].first;
}


BRect
BaseEngine::PageMediabox(int pageNumber)
{
    BRect rect(0, 0, 1, 1);

    pthread_mutex_lock(&fMutex[pageNumber]);
    if (fBitmap[pageNumber].first != nullptr)
        rect = fBitmap[pageNumber].first->Bounds();

    pthread_mutex_unlock(&fMutex[pageNumber]);

    return rect;
}


BRect
BaseEngine::PageContentBox(int pageNumber)
{
	return PageMediabox(pageNumber);
}


BString
BaseEngine::GetPageLabel(int pageNumber)
{
    BString label;
    label << pageNumber;

    return label;
}


BString
BaseEngine::FileName(void) const
{
    return BString("");
}


int
BaseEngine::GetPageByLabel(BString label)
{
	return atoi(label.String());
}


BString
BaseEngine::GetDecryptionKey(void) const
{
	return BString("");
}


void
BaseEngine::SetCacheSize(int forwardCache, int backwardCache)
{
    fForwardCache = forwardCache;
    fBackwardCache = backwardCache;
}


void
BaseEngine::MultiplyZoom(float factor)
{
    SetZoom(fZoomFactor * factor);
}


void
BaseEngine::WriteOutline(BOutlineListView* list)
{

}

void
BaseEngine::StopTextSearch(void)
{
	if (fTextSearchThread != nullptr) {
		pthread_mutex_lock(&gTextSearchStopMutex);
		fStopTextSearchThread = true;
		pthread_mutex_unlock(&gTextSearchStopMutex);
		void* status;
		pthread_join(fTextSearchThread, &status);
		fTextSearchThread = nullptr;
	}
}

void
BaseEngine::FindString(BString const& name, BLooper* looper, BHandler* handler, int32 flag)
{
	StopTextSearch();
	
	if (name.Length() < 2)
		return;
	
	fSearchString = name;
	fSearchFlag = flag;
	fTargetLooper = looper;
	fSearchHandler = handler;
	fStopTextSearchThread = false;
	pthread_create(&fTextSearchThread, nullptr, _TextSearchThread,(void*)(this));
}


tuple< vector<BString>, vector<BRect> >
BaseEngine::_FindString(BString const& name, int const& page)
{
	//empty
}


void*
BaseEngine::_TextSearchThread(void* arg)
{
	BaseEngine*  engine =(BaseEngine*)arg;
    int& pages = engine->fPages;

	BString name = engine->fSearchString;
	for (int page = 0;  page < pages; ++page) {
		pthread_mutex_lock(&gTextSearchStopMutex);
		if (engine->fStopTextSearchThread) {
			engine->fStopTextSearchThread = false;
			pthread_mutex_unlock(&gTextSearchStopMutex);
			break;
			//return nullptr;
		}
		pthread_mutex_unlock(&gTextSearchStopMutex);
		
		auto t = engine->_FindString(name, page);
		
		for (int i = 0; i < get<0>(t).size(); ++i) {
			BMessage msg(MSG_SEARCH_RESULT);
			msg.AddInt32("page", page);
			msg.AddString("context", move(get<0>(t)[i]));
			msg.AddRect("rect", move(get<1>(t)[i]));
			engine->fTargetLooper->PostMessage(&msg, engine->fSearchHandler);
		}
	}
	
	return nullptr;
}


bool
BaseEngine::HighlightUnderText(void)
{
	return fHighlightUnderText;	
}

