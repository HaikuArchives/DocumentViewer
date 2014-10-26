/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "PDFEngine.h"

#include <UnicodeChar.h>
#include <memory>
#include <string>

#include "Flags.h"
#include "Messages.h"
#include "OutlineView.h"
#include "PasswordRequestWindow.h"

using namespace std;

pthread_mutex_t	PDFEngine::gRendermutex;

PDFEngine::PDFEngine(BString fileName, BString& password)
    :
    fFileName(fileName),
    fPassword(password)
{
	fHighlightUnderText = true;
    fDocument = nullptr;
    fz_var(fDocument);
   // fz_accelerate();
    fContext = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
	fz_register_document_handlers(fContext);
    // why bgr instead of rgb?
    fColorSpace = fz_device_bgr(fContext);
    
    if (!fContext) {
    	!out << "cannot init context" << endl;
    	exit(1);	
    }
    
	fz_try(fContext) {
    	fDocument = fz_open_document(fContext, const_cast<char*>(fileName.String()));
	} fz_catch(fContext) {
		!out << "can not open document" << endl;
	}

	if (fz_needs_password(fDocument)) {
		if (password.Length() == 0) {
			while (true) {
				auto t = (new PasswordRequestWindow())->Go();
				if (std::get<1>(t) == false)
					throw "wrong password";
				
				if (fz_authenticate_password(fDocument, std::get<0>(t))) {
					password = 	std::get<0>(t);
					break;
				}
			}	
		} else {
			int okay = fz_authenticate_password(fDocument,
            	const_cast<char*>(password.String()));
            	
          	if (!okay)
          		throw "wrong password";
		}
	}
	
	Start();
}


PDFEngine::~PDFEngine()
{
	Stop();

	if (fDocument) {
		fz_close_document(fDocument);
	}
	
	if (fContext)
		fz_free_context(fContext);
}


int
PDFEngine::PageCount(void) const
{
	int count = fz_count_pages(fDocument);
	return count;
}



void
PDFEngine::WriteOutline(BOutlineListView* list)
{
	auto outline = fz_load_outline(fDocument);
	
	std::function<void(fz_outline*, BOutlineListView*, BListItem*, int)> OutlineToList =
	[&OutlineToList](fz_outline* outline, BOutlineListView* list, BListItem* super, int level) {
		OutlineItem* item;
		while (outline) {
			if (outline->dest.kind == FZ_LINK_GOTO)
				item = new OutlineItem(outline->title, outline->dest.ld.gotor.page);
			else
				item = new OutlineItem(outline->title, 0);
			
			list->AddUnder(item, super);
			
			if (outline->down)
				OutlineToList(outline->down,list, item, level + 1);
				
	//		((OutlineListView*)list)->ReverseOrder(super);
					
			outline = outline->next;
		}	
	};
		
	OutlineToList(outline, list, nullptr, 0);
	fz_free_outline(fContext, outline);
}


namespace std {
// FIXME remove when the compiler supports this C++11 feature.
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}


std::tuple< std::vector<BString>, std::vector<BRect> >
PDFEngine::_FindString(BString const& name, int const& pageNumber)
{
	vector<BString> contextVec;
	vector<BRect>	rectVec;
#if 0
	bool needWholeWord = false;
    if (GHasFlag(fSearchFlag, SEARCH_WHOLE_WORD))
    	needWholeWord = true;
    	
    bool needMatchCase = false;
    if (GHasFlag(fSearchFlag, SEARCH_MATCH_CASE))
    	needMatchCase = true;
    
    fz_text_sheet* sheet = fz_new_text_sheet(fContext);
    fz_page* page = nullptr;
    page = fz_load_page(fDocument, pageNumber);
	
	fz_device* dev = nullptr;
	fz_var(dev);
	
#endif
	fz_text_page* text = nullptr;
	fz_var(text);
	
	fz_try(fContext) {
		text = fz_new_text_page(fContext);
		int boxCount = fz_search_text_page(fContext, text, name, NULL, 0);
		fz_rect boxes[boxCount];
		int count = fz_search_text_page(fContext, text, name, boxes, boxCount);

		for(fz_rect fr: boxes) {
			BRect br;
			br.top = fr.y0;
			br.bottom = fr.y1;
			br.left = fr.x0;
			br.right = fr.x1;
			rectVec.push_back(br);
			// TODO we need to extract the "context" for the search results (a
			// line of text or so)
			contextVec.push_back(name);
		}


#if 0
		dev = fz_new_text_device(fContext, sheet, text);
		fz_run_page(fDocument, page, dev, &fz_identity, nullptr);	
		fz_free_device(dev);
		dev = nullptr;
#endif
	
	} fz_catch(fContext) {
#if 0
		fz_free_device(dev);
#endif
		fz_free_text_page(fContext, text);
#if 0
		fz_free_page(fDocument, page);
#endif
		fz_rethrow(fContext);
	}
		
	return move(make_tuple(contextVec, rectVec));
}


BString
PDFEngine::GetProperty(BString name)
{
    BString property;
    /*
    fz_obj* info = fz_dict_gets(fDocument->trailer, "Info");

	if (info) {
		fz_obj* obj = fz_dict_gets(info, const_cast<char*>(name.String()));
		if (obj)
			property = pdf_to_utf8(obj);
	}
	*/

    return property;
}


BString
PDFEngine::FileName(void) const
{
    return fFileName;
}


unique_ptr<BBitmap>
PDFEngine::RenderBitmap(int const& pageNumber,
	int const& width, int const& height, int const& rotation)
{
	BBitmap* bitmap = nullptr;
		
	if (pageNumber < 0 || pageNumber >= fPages) {
    	return unique_ptr<BBitmap>(bitmap);
	}
	
	fz_page *page;
	fz_display_list *list = nullptr;
	fz_device *dev = nullptr;
	
	fz_var(list);
	fz_var(dev);
	
    bool stop = false;	// variable for avoiding return in fz_catch
    fz_try(fContext) {
		page = fz_load_page(fDocument, pageNumber);
    } fz_catch(fContext) {
    	pthread_mutex_unlock(&gRendermutex);
    	stop = true;
    }
    
    if (stop)
    	return std::make_unique<BBitmap>(BRect(0, 0, width, height), B_RGBA32);
	
	fz_try(fContext) {
    	list = fz_new_display_list(fContext);
    	dev = fz_new_list_device(fContext, list);
    	fz_run_page(fDocument, page, dev, &fz_identity, nullptr);
	} fz_catch(fContext) {
    	fz_free_device(dev);
    	fz_drop_display_list(fContext, list);
    	fz_free_page(fDocument, page);
    	pthread_mutex_unlock(&gRendermutex);
    	stop = true;
	}
	
	if (stop)
    	return std::make_unique<BBitmap>(BRect(0, 0, width, height), B_RGBA32);
    	
	fz_free_device(dev);
	dev = nullptr;

    fz_matrix ctm = fz_identity;
    fz_rect bbox;
    
	fz_pixmap* image = nullptr;
	fz_var(image);
	
	fz_rect bounds;
	fz_bound_page(fDocument, page, &bounds);
	
	float zoomFactor = 1;
	
	if (width <= 0) {
		zoomFactor = height / (bounds.y1 - bounds.y0);
	} else {
		zoomFactor = width / (bounds.x1 - bounds.x0);
	}

  	fz_scale(&ctm, zoomFactor, zoomFactor);
    fz_rotate(&ctm, fRotation);
    fz_transform_rect(&bounds, &ctm);
    fz_irect pageBox;
	fz_round_rect(&pageBox, &bounds);
	
	fz_try(fContext) {
    	image = fz_new_pixmap_with_bbox(fContext, fColorSpace, &pageBox);
    	
    	//save alpha
    	//fz_clear_pixmap(fContext, image); 
    	fz_clear_pixmap_with_value(fContext, image, 255);
    	dev = fz_new_draw_device(fContext, image);
    	if (list)
    		fz_run_display_list(list, dev, &ctm, &bounds, nullptr);
   		 else
   	 		fz_run_page(fDocument, page, dev, &ctm, nullptr);
    	
    	fz_free_device(dev);
    	dev = nullptr;
	} fz_catch(fContext) {
		fz_free_device(dev);
		fz_drop_pixmap(fContext, image);
    	fz_drop_display_list(fContext, list);
		fz_free_page(fDocument, page);
		pthread_mutex_unlock(&gRendermutex);
    	stop = true;
	}
	
	if (stop)
    	return std::make_unique<BBitmap>(BRect(0, 0, width, height), B_RGBA32);
    	
    fz_flush_warnings(fContext);
    
    int imageWidth = pageBox.x1 - pageBox.x0;
    int imageHeight = pageBox.y1 - pageBox.y0;

    bitmap = new BBitmap(BRect(0, 0, imageWidth - 1, imageHeight - 1), B_RGBA32);
    bitmap->SetBits(fz_pixmap_samples(fContext, image),
    	imageWidth * imageHeight * fz_pixmap_components(fContext, image), 0, B_RGBA32);
    
    fz_free_device(dev);
	fz_drop_pixmap(fContext, image);
    fz_drop_display_list(fContext, list);
	fz_free_page(fDocument, page);
	return unique_ptr<BBitmap>(bitmap);
}


std::pair<BBitmap*, bool>
PDFEngine::_RenderBitmap(int const& pageNumber)
{
	pthread_mutex_lock(&gRendermutex);
	unique_ptr<BBitmap> bitmap = RenderBitmap(pageNumber, fDefaultRect.Width(),
		fDefaultRect.Height(), 0);
	pthread_mutex_unlock(&gRendermutex);
    
    return std::pair<BBitmap*, bool>(bitmap.release(), false);
}
