/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "DJVUEngine.h"

#include <vector>

#include "Flags.h"
#include "Messages.h"
#include "OutlineView.h"
#include "PasswordRequestWindow.h"

using namespace std;

pthread_mutex_t	DJVUEngine::gRendermutex;

DJVUEngine::DJVUEngine(BString fileName, BString& password)
    :
    fFileName(fileName),
    fPassword(password)
{
    
	fContext = ddjvu_context_create("DJVUEngine");
	ddjvu_cache_set_size(fContext, 30E6);
	fDocument = ddjvu_document_create_by_filename_utf8(fContext,
					fileName.String(), TRUE);
					
	while (!ddjvu_document_decoding_done(fDocument))
		_HandleDjvuMessages(fContext, true);    
    
    if (ddjvu_document_decoding_error(fDocument))
        throw;
       
	Start();
}


DJVUEngine::~DJVUEngine()
{
	Stop();
	
	ddjvu_document_release(fDocument);
	ddjvu_context_release(fContext);
	
	minilisp_finish();
}


void
DJVUEngine::_HandleDjvuMessages(ddjvu_context_t *context, int wait)
{
  	if (wait)
    	ddjvu_message_wait(context);
 
	const ddjvu_message_t *msg;   
  	while ((msg = ddjvu_message_peek(context))) {
    	switch(msg->m_any.tag) { 
    		case DDJVU_ERROR:
    			!out << "djvu-error: " << msg->m_error.message << endl;
   				break;
 
    		default:
    			break;
    	}
    	ddjvu_message_pop(context);
	}
}


int
DJVUEngine::PageCount(void) const
{
	return ddjvu_document_get_pagenum(fDocument);
}


void
DJVUEngine::WriteOutline(BOutlineListView* list)
{
    std::function<void(miniexp_t, BOutlineListView*, BListItem*, int)> OutlineToList =
	[&OutlineToList](miniexp_t outline, BOutlineListView* list, BListItem* super, int level) {		
		for (miniexp_t rest = outline; miniexp_consp(rest); rest = miniexp_cdr(rest)) {
	        miniexp_t item = miniexp_car(rest);
	        if (!miniexp_consp(item) || !miniexp_consp(miniexp_cdr(item)) ||
	            !miniexp_stringp(miniexp_car(item)) || !miniexp_stringp(miniexp_cadr(item)))
	            	continue;
	
	        BString link(miniexp_to_str(miniexp_cadr(item)));
			BString name(miniexp_to_str(miniexp_car(item)));	
			
			link.RemoveAll("#");
			int pageNumber= atoi(link.String()) - 1;
			OutlineItem* listItem = new OutlineItem(name, pageNumber);
			list->AddUnder(listItem, super);
			OutlineToList(miniexp_cddr(item),list, listItem, level + 1);
    	}
	};
	
	miniexp_t outline;
	
	while ((outline = ddjvu_document_get_outline(fDocument)) == miniexp_dummy)
		_HandleDjvuMessages(fContext); 
	
    if (!miniexp_consp(outline) || miniexp_car(outline) != miniexp_symbol("bookmarks")) {
        ddjvu_miniexp_release(fDocument, outline);
        	return;
    }	
		
	OutlineToList(outline, list, nullptr, 0);
   
    
	ddjvu_miniexp_release(fDocument, outline);
}


BString
DJVUEngine::GetProperty(BString name)
{
    BString property;

    return property;
}


BString
DJVUEngine::FileName(void) const
{
    return fFileName;
}




unique_ptr<BBitmap>
DJVUEngine::RenderBitmap(int const& pageNumber,
	int const& width, int const& height, int const& rotation)
{	
	if (pageNumber < 0 || pageNumber >= fPages) {
    	return unique_ptr<BBitmap>(nullptr);
	}
	
    ddjvu_page_t *page = ddjvu_page_create_by_pageno(fDocument, pageNumber);
    if (!page)
        return unique_ptr<BBitmap>(nullptr);
        
    while (!ddjvu_page_decoding_done(page))
    	_HandleDjvuMessages(fContext);
        
    if (ddjvu_page_decoding_error(page))
        return unique_ptr<BBitmap>(nullptr);
    
    
    ddjvu_rect_t prect = {0,0, width, height};
    
  	if (prect.w <= 0) {
  		if (prect.h <= 0)
  			throw "DJVUEngine::RenderBitmap - illegal size";
  			
  		prect.w = prect.h * ddjvu_page_get_width(page) / ddjvu_page_get_height(page);
  	} else if (prect.h <= 0) {
  		prect.h = prect.w * ddjvu_page_get_height(page) / ddjvu_page_get_width(page); 
  	}

    
    ddjvu_rect_t rrect = prect;
    
    BBitmap* bitmap;
    ddjvu_render_mode_t mode;
    ddjvu_format_t *fmt;
    
    if (DDJVU_PAGETYPE_BITONAL == ddjvu_page_get_type(page)) {
    	bitmap = new BBitmap(BRect(0, 0, prect.w - 1, prect.h - 1), B_GRAY8);
    	fmt = ddjvu_format_create(DDJVU_FORMAT_GREY8, 0, NULL);
    	mode = DDJVU_RENDER_MASKONLY;
    } else {
    	bitmap = new BBitmap(BRect(0, 0, prect.w - 1, prect.h - 1), B_RGB24);
    	fmt = ddjvu_format_create(DDJVU_FORMAT_BGR24, 0, NULL);
    	mode = DDJVU_RENDER_COLOR;
    }
    
    ddjvu_format_set_row_order(fmt, TRUE);
   
    if (!ddjvu_page_render(page, mode, &prect, &rrect, fmt, bitmap->BytesPerRow(), bitmap->Bits()))
    	return unique_ptr<BBitmap>(nullptr);
	
    ddjvu_format_release(fmt);
    ddjvu_page_release(page);
    
    return unique_ptr<BBitmap>(bitmap);
}


std::pair<BBitmap*, bool>
DJVUEngine::_RenderBitmap(int const& pageNumber)
{
	pthread_mutex_lock(&gRendermutex);
	
    ddjvu_page_t *page = ddjvu_page_create_by_pageno(fDocument, pageNumber);
    if (!page) {
    	pthread_mutex_unlock(&gRendermutex);
        return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);
    }
        
    while (!ddjvu_page_decoding_done(page))
    	_HandleDjvuMessages(fContext); 
        
    if (ddjvu_page_decoding_error(page)) {
    	pthread_mutex_unlock(&gRendermutex);
        return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);
	}
    
    ddjvu_rect_t prect = {0, 0, 0, 0};
    prect.w = ddjvu_page_get_width(page) * fZoomFactor * 0.1;
    prect.h = ddjvu_page_get_height(page) * fZoomFactor * 0.1;
    ddjvu_rect_t rrect = prect;
    
    BBitmap* bitmap;
    ddjvu_render_mode_t mode;
    ddjvu_format_t *fmt;
    
    if (DDJVU_PAGETYPE_BITONAL == ddjvu_page_get_type(page)) {
    	bitmap = new BBitmap(BRect(0, 0, prect.w - 1, prect.h - 1), B_GRAY8);
    	fmt = ddjvu_format_create(DDJVU_FORMAT_GREY8, 0, NULL);
    	mode = DDJVU_RENDER_MASKONLY;
    } else {
    	bitmap = new BBitmap(BRect(0, 0, prect.w - 1, prect.h - 1), B_RGB24);
    	fmt = ddjvu_format_create(DDJVU_FORMAT_BGR24, 0, NULL);  	
    	mode = DDJVU_RENDER_COLOR;
    }
    
    ddjvu_format_set_row_order(fmt, /* top_to_bottom */ TRUE);
   
    if (!ddjvu_page_render(page, mode, &prect, &rrect, fmt, bitmap->BytesPerRow(), bitmap->Bits())) {
    	pthread_mutex_unlock(&gRendermutex);
    	return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);
	}
    ddjvu_format_release(fmt);
    ddjvu_page_release(page);
    pthread_mutex_unlock(&gRendermutex);
    
	return std::pair<BBitmap*, bool>(bitmap, false);
}


tuple< vector<BString>, vector<BRect> > 
DJVUEngine::_FindString(BString const& name, int const& page)
{
	vector<BString> contextVec;
	vector<BRect>	rectVec;
	
	vector<BString> textVec;
		
	bool needWholeWord = false;
    if (GHasFlag(fSearchFlag, SEARCH_WHOLE_WORD))
    	needWholeWord = true;
    	
    bool needMatchCase = false;
    if (GHasFlag(fSearchFlag, SEARCH_MATCH_CASE))
    	needMatchCase = true;
    	
    float pageWidth = 1;
    float pageHeight = 1;
	
	function<void(miniexp_t)> _SearchInExpression =
	[&](miniexp_t expression) {
		miniexp_t type = miniexp_car(expression);
	    if (!miniexp_symbolp(type))
	        return;
	        
	    expression = miniexp_cdr(expression);
		BRect rect;
    	
    	rect.left = miniexp_to_int(miniexp_car(expression));
    	expression = miniexp_cdr(expression);
    	rect.bottom = miniexp_to_int(miniexp_car(expression));
    	expression = miniexp_cdr(expression);
    	rect.right = miniexp_to_int(miniexp_car(expression));
    	expression = miniexp_cdr(expression);
    	rect.top = miniexp_to_int(miniexp_car(expression));
    	expression = miniexp_cdr(expression);
		Debug out;
	    miniexp_t str = miniexp_car(expression);
	    
	    if (miniexp_symbol("page") == type) {
	    	pageWidth = rect.Width();
	    	pageHeight = -rect.Height();
	    }
	   
	    
	    if (miniexp_stringp(str) && !miniexp_cdr(expression)) {
	        BString tempStr = miniexp_to_str(str);
	        bool foundMatch = false;
	   		if (needMatchCase) {
				if (tempStr.FindFirst(name) != B_ERROR) {
					if (needWholeWord) {
						if (tempStr.Length() == name.Length())
							foundMatch = true;
					} else {
						foundMatch = true;	
					}
				}		
			} else {
				if (tempStr.IFindFirst(name) != B_ERROR) {
					if (needWholeWord) {
						if (tempStr.Length() == name.Length())
							foundMatch = true;
					} else {
						foundMatch = true;	
					}
				}
			}
			
			if (foundMatch) {
				rect.left = rect.left / pageWidth;
				rect.right = rect.right/ pageWidth;
				rect.top = (pageHeight - rect.top) / pageHeight;
				rect.bottom = (pageHeight - rect.bottom) / pageHeight;
				rectVec.push_back(rect);
			}
	        	        
	        textVec.push_back(move(tempStr));
	        expression = miniexp_cdr(expression);
	    }
	    
	    while (miniexp_consp(str)) {
	      	_SearchInExpression(str);
	        expression = miniexp_cdr(expression);
	        str = miniexp_car(expression);
	    }
	    
	    return;
	};
	
	auto _SearchInPage = [&](int page) {
		miniexp_t pagetext;
    	while ((pagetext = ddjvu_document_get_pagetext(fDocument, page, "word")) == miniexp_dummy)
        	_HandleDjvuMessages(fContext, true);
    	
    	if (miniexp_nil == pagetext)
        	return;
        	
       	_SearchInExpression(pagetext);
	
		ddjvu_miniexp_release(fDocument, pagetext);
	};

	_SearchInPage(page);
	
	int deltaIndexLeft = 4;
	int deltaIndexRight = 4;
	
	bool foundMatch = false;
	
	for (int i = 0; i < textVec.size(); ++i) {
		foundMatch = false;
		
		if (needMatchCase) {
			if (textVec[i].FindFirst(name) != B_ERROR) {
				if (needWholeWord) {
					if (textVec[i].Length() == name.Length())
						foundMatch = true;
				} else {
					foundMatch = true;	
				}
			}		
		} else {
			if (textVec[i].IFindFirst(name) != B_ERROR) {
				if (needWholeWord) {
					if (textVec[i].Length() == name.Length())
						foundMatch = true;
				} else {
					foundMatch = true;	
				}
			}
		}
		
		if (foundMatch == true) {
			int from = i - deltaIndexLeft;
			if (from < 0)
				from = 0;
				
			int to	 = i + deltaIndexRight;
			if (to >= textVec.size())
				to = textVec.size() - 1;
			
			BString str;
			for (int j = from; j < to; ++j) { 
				str += textVec[j];
				str += " ";
			}
				
			str += textVec[to];
			
			contextVec.push_back(str);
		}
	}
	
    return move(make_tuple(contextVec, rectVec));
}
