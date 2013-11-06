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
    // why bgr instead of rgb?
    fColorSpace = fz_device_bgr;
    fDocument = nullptr;
    fz_var(fDocument);
   // fz_accelerate();
    fContext = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    
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
	fz_count_pages(fDocument);
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


tuple< vector<BString>, vector<BRect> >
PDFEngine::_FindString(BString const& name, int const& pageNumber)
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
    
    fz_text_sheet* sheet = fz_new_text_sheet(fContext);
    fz_page* page = nullptr;
    page = fz_load_page(fDocument, pageNumber);
	
	fz_device* dev = nullptr;
	fz_var(dev);
	
	fz_text_page* text = nullptr;
	fz_var(text);
	
	fz_try(fContext) {
		text = fz_new_text_page(fContext, fz_bound_page(fDocument, page));
		dev = fz_new_text_device(fContext, sheet, text);
		fz_run_page(fDocument, page, dev, fz_identity, nullptr);	
		fz_free_device(dev);
		dev = nullptr;
	
	} fz_catch(fContext) {
		fz_free_device(dev);
		fz_free_text_page(fContext, text);
		fz_free_page(fDocument, page);
		fz_rethrow(fContext);
	}
		
	fz_text_block *block;
	fz_text_line *line;
	fz_text_span *span;
	fz_text_char *ch;
	char utf[10];
	int i, n;
	
	BString str;
	BString character;
	BRect rect;
	
	auto pageBox = fz_bound_page(fDocument, page);
	auto pageWidth = pageBox.x1 - pageBox.x0;
    auto pageHeight = pageBox.y1 - pageBox.y0;
	fz_rect tempRect;
	
	for (block = text->blocks; block < text->blocks + text->len; block++) {
	for (line = block->lines; line < block->lines + block->len; line++) {
	for (span = line->spans; span < line->spans + line->len; span++) {
	for (ch = span->text; ch < span->text + span->len; ch++) {
		character = "";
		n = fz_runetochar(utf, ch->c);
		for (i = 0; i < n; i++)
			character += utf[i];	
					
		if (character == " ") {
			if (str.Length() > 0) {
				bool foundMatch = false;
	   			if (needMatchCase) {
					if (str.FindFirst(name) != B_ERROR) {
						if (needWholeWord) {
							if (str.Length() == name.Length())
								foundMatch = true;
						} else {
							foundMatch = true;	
						}
					}		
				} else {
					if (str.IFindFirst(name) != B_ERROR) {
						if (needWholeWord) {
							if (str.Length() == name.Length())
								foundMatch = true;
						} else {
							foundMatch = true;	
						}
					}
				}
			
				if (foundMatch) {
					#if 0
					ch--;
					rect.right = ch->bbox.x1;
					rect.bottom = ch->bbox.y1;
					ch++;
					#endif
					rect.right = tempRect.x1;
					rect.bottom = tempRect.y1;
					
					rect.left = rect.left / pageWidth;
					rect.right = rect.right/ pageWidth;
					rect.top = (rect.top) / pageHeight;
					rect.bottom = (rect.bottom) / pageHeight;
					
					rectVec.push_back(rect);
				}
				textVec.push_back(str);
			}
						
			str = "";
		} else {
			if (str.Length() == 0) {
				rect.left = ch->bbox.x0;
				rect.top  = ch->bbox.y0;
			}
			
			tempRect = ch->bbox;
			str += character;
		}
	}
	}
	}
	}

	fz_free_text_page(fContext, text);
	fz_free_text_sheet(fContext, sheet);
	
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
    	bitmap = new BBitmap(BRect(0, 0, width, height), B_RGBA32);
    }
    
    if (stop)
    	return unique_ptr<BBitmap>(bitmap);
	
	fz_try(fContext) {
    	list = fz_new_display_list(fContext);
    	dev = fz_new_list_device(fContext, list);
    	fz_run_page(fDocument, page, dev, fz_identity, nullptr);
	} fz_catch(fContext) {
    	fz_free_device(dev);
    	fz_free_display_list(fContext, list);
    	fz_free_page(fDocument, page);
    	pthread_mutex_unlock(&gRendermutex);
    	stop = true;
    	bitmap = new BBitmap(BRect(0, 0, width, height), B_RGBA32);
	}
	
	if (stop)
    	return unique_ptr<BBitmap>(bitmap);
    	
	fz_free_device(dev);
	dev = nullptr;

    fz_matrix ctm;
    fz_bbox bbox;
    
	fz_pixmap* image = static_cast<fz_pixmap*>(nullptr);
	fz_var(image);
	
	fz_rect bounds = fz_bound_page(fDocument, page);
	
	float zoomFactor = 1;
	
	if (width <= 0) {
		zoomFactor = height / (bounds.y1 - bounds.y0);
	} else {
		zoomFactor = width / (bounds.x1 - bounds.x0);
	}
	
  	ctm = fz_scale(zoomFactor, zoomFactor);
    ctm = fz_concat(ctm, fz_rotate(fRotation));
    fz_rect bounds2 = fz_transform_rect(ctm, bounds);
    fz_bbox pageBox = fz_round_rect(bounds2);
	
	fz_try(fContext) {
    	image = fz_new_pixmap_with_bbox(fContext, fColorSpace, pageBox);
    	
    	//save alpha
    	//fz_clear_pixmap(fContext, image); 
    	fz_clear_pixmap_with_value(fContext, image, 255);
    	dev = fz_new_draw_device(fContext, image);
    	if (list)
    		fz_run_display_list(list, dev, ctm, pageBox, nullptr);
   		 else
   	 		fz_run_page(fDocument, page, dev, ctm, nullptr);
    	
    	fz_free_device(dev);
    	dev = nullptr;
	} fz_catch(fContext) {
		fz_free_device(dev);
		fz_drop_pixmap(fContext, image);
    	fz_free_display_list(fContext, list);
		fz_free_page(fDocument, page);
		pthread_mutex_unlock(&gRendermutex);
    	stop = true;
    	bitmap = new BBitmap(BRect(0, 0, width, height), B_RGBA32);
	}
	
	if (stop)
    	return unique_ptr<BBitmap>(bitmap);
    	
    fz_flush_warnings(fContext);
    
    int imageWidth = pageBox.x1 - pageBox.x0;
    int imageHeight = pageBox.y1 - pageBox.y0;

    bitmap = new BBitmap(BRect(0, 0, imageWidth - 1, imageHeight - 1), B_RGBA32);
    bitmap->SetBits(fz_pixmap_samples(fContext, image),
    	imageWidth * imageHeight * fz_pixmap_components(fContext, image), 0, B_RGBA32);
    
    fz_free_device(dev);
	fz_drop_pixmap(fContext, image);
    fz_free_display_list(fContext, list);
	fz_free_page(fDocument, page);
	return unique_ptr<BBitmap>(bitmap);
}


std::pair<BBitmap*, bool>
PDFEngine::_RenderBitmap(int const& pageNumber)
{
	pthread_mutex_lock(&gRendermutex);
		
    if (pageNumber < 0 || pageNumber >= fPages)
    	return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);

	fz_page* page;
	fz_display_list* list = nullptr;
	fz_device* dev = nullptr;
	
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
    	return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);
	
	fz_try(fContext) {
    	list = fz_new_display_list(fContext);
    	dev = fz_new_list_device(fContext, list);
    	fz_run_page(fDocument, page, dev, fz_identity, nullptr);
	} fz_catch(fContext) {
    	fz_free_device(dev);
    	fz_free_display_list(fContext, list);
    	fz_free_page(fDocument, page);
    	pthread_mutex_unlock(&gRendermutex);
    	stop = true;
	}
	
	if (stop)
    	return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);
	
	fz_free_device(dev);
	dev = nullptr;

    fz_matrix ctm;
    fz_bbox bbox;
    
	fz_pixmap* image = nullptr;
	fz_var(image);
	
	fz_rect bounds = fz_bound_page(fDocument, page);
	
   	ctm = fz_scale(fZoomFactor, fZoomFactor);
    ctm = fz_concat(ctm, fz_rotate(fRotation));
    fz_rect bounds2 = fz_transform_rect(ctm, bounds);
    fz_bbox pageBox = fz_round_rect(bounds2);
	
	fz_try(fContext) {
    	image = fz_new_pixmap_with_bbox(fContext, fColorSpace, pageBox);
    	
    	//save alpha
    	//fz_clear_pixmap(fContext, image); 
    	fz_clear_pixmap_with_value(fContext, image, 255);
    	dev = fz_new_draw_device(fContext, image);
    	if (list)
    		fz_run_display_list(list, dev, ctm, pageBox, nullptr);
   		 else
   	 		fz_run_page(fDocument, page, dev, ctm, nullptr);
    	
    	fz_free_device(dev);
    	dev = nullptr;
    	//fz_unmultiply_pixmap(fContext, image);
	} fz_catch(fContext) {
		fz_free_device(dev);
		fz_drop_pixmap(fContext, image);
    	fz_free_display_list(fContext, list);
		fz_free_page(fDocument, page);
		pthread_mutex_unlock(&gRendermutex);
    	stop = true;
	}
	
	if (stop)
    	return std::pair<BBitmap*, bool>(new BBitmap(fDefaultRect, B_RGBA32), false);
	
    fz_flush_warnings(fContext);
    
    int imageWidth  = pageBox.x1 - pageBox.x0;
    int imageHeight = pageBox.y1 - pageBox.y0;
    
    BBitmap* bitmap = new BBitmap(BRect(0, 0, imageWidth - 1, imageHeight - 1), B_RGBA32);
   	bitmap->SetBits(fz_pixmap_samples(fContext, image),
   		imageWidth * imageHeight * fz_pixmap_components(fContext, image), 0, B_RGBA32);
    
    fz_free_device(dev);
	fz_drop_pixmap(fContext, image);
    fz_free_display_list(fContext, list);
	fz_free_page(fDocument, page);
	pthread_mutex_unlock(&gRendermutex);
    
    return std::pair<BBitmap*, bool>(bitmap, false);
}
