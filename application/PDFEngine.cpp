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
#include <functional>

#include "Flags.h"
#include "Messages.h"
#include "OutlineView.h"
#include "PasswordRequestWindow.h"

using namespace std;


struct fz_lock_impl {
	fz_locks_context context;

	fz_lock_impl() {
		for (int i = 0; i < FZ_LOCK_MAX; i++)
			locks[i] = PTHREAD_MUTEX_INITIALIZER;

		context.user = this;
		context.lock = lock;
		context.unlock = unlock;
	}

	static void lock(void* user, int lock) {
		pthread_mutex_lock(&((fz_lock_impl*)user)->locks[lock]);
	}
	static void unlock(void* user, int lock) {
		pthread_mutex_unlock(&((fz_lock_impl*)user)->locks[lock]);
	}

private:
	pthread_mutex_t locks[FZ_LOCK_MAX];
};


PDFEngine::PDFEngine(BString fileName, BString& password)
    :
    fFileName(fileName),
    fPassword(password)
{
	fHighlightUnderText = true;
    fDocument = nullptr;
    fz_var(fDocument);

	fz_lock_impl* locks = new fz_lock_impl;
	fContext = fz_new_context(nullptr, &locks->context, FZ_STORE_DEFAULT);
	fz_register_document_handlers(fContext);
    // why bgr instead of rgb?
    fColorSpace = fz_device_bgr(fContext);

    if (!fContext) {
    	!out << "cannot init context" << endl;
    	exit(1);
    }

	fz_try(fContext) {
		fDocument = fz_open_document(fContext, fileName.String());
	} fz_catch(fContext) {
		!out << "can not open document" << endl;
		throw;
	}

	if (fz_needs_password(fContext, fDocument)) {
		if (password.Length() == 0) {
			while (true) {
				auto t = (new PasswordRequestWindow())->Go();
				if (std::get<1>(t) == false)
					throw "wrong password";

				if (fz_authenticate_password(fContext, fDocument, std::get<0>(t))) {
					password = 	std::get<0>(t);
					break;
				}
			}
		} else {
			int okay = fz_authenticate_password(fContext, fDocument,
            	const_cast<char*>(password.String()));

          	if (!okay)
          		throw "wrong password";
		}
	}

	// Contexts for alternate threads
	fRenderContext = fz_clone_context(fContext);

	Start();
}


PDFEngine::~PDFEngine()
{
	Stop();

	if (fDocument) {
		fz_drop_document(fContext, fDocument);
	}

	if (fContext) {
		fz_lock_impl* locks = static_cast<fz_lock_impl*>(fContext->locks->user);
		fz_drop_context(fContext);
		delete locks;
	}
}


int
PDFEngine::PageCount(void) const
{
	int count = fz_count_pages(fContext, fDocument);
	return count;
}



void
PDFEngine::WriteOutline(BOutlineListView* list)
{
	auto outline = fz_load_outline(fContext, fDocument);

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
	fz_drop_outline(fContext, outline);
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

	bool needWholeWord = false;
    if (GHasFlag(fSearchFlag, SEARCH_WHOLE_WORD))
    	needWholeWord = true;

    bool needMatchCase = false;
    if (GHasFlag(fSearchFlag, SEARCH_MATCH_CASE))
    	needMatchCase = true;

    fz_page* page = nullptr;
    page = fz_load_page(fContext, fDocument, pageNumber);

    fz_text_sheet* sheet = fz_new_text_sheet(fContext);

	fz_device* dev = nullptr;
	fz_var(dev);

	fz_text_page* text = nullptr;
	fz_var(text);

	fz_try(fContext) {
		text = fz_new_text_page(fContext);
		dev = fz_new_text_device(fContext, sheet, text);

		// Load the text from the page
		fz_run_page(fContext, page, dev, &fz_identity, NULL);
		fz_drop_device(fContext, dev);
		dev = nullptr;

		// Now actually get the boxes
		static const int MAX_SEARCHES = 500;
		fz_rect boxes[MAX_SEARCHES];
		int count = fz_search_text_page(fContext, text, name, boxes, MAX_SEARCHES);

		for(int i = 0; i < count; i++) {
			fz_rect fr = boxes[i];
			BRect br;
			br.top = fr.y0;
			br.bottom = fr.y1;
			br.left = fr.x0;
			br.right = fr.x1;

			// Get some context: extract some text around the match by enlarging
			// the rect to cover a greater part of the text
			fr.x0 -= 1000;
			fr.x1 += 1000;

			BString context(fz_copy_selection(fContext, text, fr));
			int pos = context.FindFirst(name);

			// Check that we get a case match if requested
			if (needMatchCase && pos < 0)
				continue;

			// Filter out non-whole word results
			if (needWholeWord) {
				if (pos > 0 && isalpha(context[pos - 1]))
					continue;
				if (isalpha(context[pos + name.Length()]))
					continue;
			}

			rectVec.push_back(br);
			contextVec.push_back(context);
		}


	} fz_catch(fContext) {
		fz_drop_device(fContext, dev);
		fz_drop_text_page(fContext, text);
		fz_drop_text_sheet(fContext, sheet);
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
	if (pageNumber < 0 || pageNumber >= fPages) {
    	return unique_ptr<BBitmap>(nullptr);
	}

	fz_page *page;
	fz_display_list *list = nullptr;
	fz_device *dev = nullptr;

	fz_var(list);
	fz_var(dev);

	pthread_mutex_lock(&fRendermutex);

    bool stop = false;	// variable for avoiding return in fz_catch
	fz_try(fRenderContext) {
		page = fz_load_page(fRenderContext, fDocument, pageNumber);
	} fz_catch(fRenderContext) {
		pthread_mutex_unlock(&fRendermutex);
    	stop = true;
    }

    if (stop)
    	return std::make_unique<BBitmap>(BRect(0, 0, width, height), B_RGBA32);

	fz_try(fRenderContext) {
		list = fz_new_display_list(fRenderContext);
		dev = fz_new_list_device(fRenderContext, list);
		fz_run_page(fRenderContext, page, dev, &fz_identity, nullptr);
	} fz_catch(fRenderContext) {
		fz_drop_device(fRenderContext, dev);
		fz_drop_display_list(fRenderContext, list);
		fz_drop_page(fRenderContext, page);
		pthread_mutex_unlock(&fRendermutex);
    	stop = true;
	}

	if (stop)
    	return std::make_unique<BBitmap>(BRect(0, 0, width, height), B_RGBA32);

	fz_drop_device(fRenderContext, dev);
	dev = nullptr;

    fz_matrix ctm = fz_identity;

	fz_pixmap* image = nullptr;
	fz_var(image);

	fz_rect bounds;
	fz_bound_page(fRenderContext, page, &bounds);

	float zoomFactor = 1;

	if (width <= 0) {
		zoomFactor = height / (bounds.y1 - bounds.y0);
	} else {
		zoomFactor = width / (bounds.x1 - bounds.x0);
	}

    fz_rotate(&ctm, fRotation);
  	fz_pre_scale(&ctm, zoomFactor, zoomFactor);
    fz_irect storage;
	fz_irect* pageBox = fz_round_rect(&storage, fz_transform_rect(&bounds, &ctm));

	fz_try(fRenderContext) {
		image = fz_new_pixmap_with_bbox(fRenderContext, fColorSpace, pageBox);

    	//save alpha
		//fz_clear_pixmap(fRenderContext, image);
		fz_clear_pixmap_with_value(fRenderContext, image, 255);
		dev = fz_new_draw_device(fRenderContext, image);
    	if (list)
			fz_run_display_list(fRenderContext, list, dev, &ctm, &bounds, nullptr);
   		 else
			fz_run_page(fRenderContext, page, dev, &ctm, nullptr);

		fz_drop_device(fRenderContext, dev);
    	dev = nullptr;
	} fz_catch(fRenderContext) {
		fz_drop_device(fRenderContext, dev);
		fz_drop_pixmap(fRenderContext, image);
		fz_drop_display_list(fRenderContext, list);
		fz_drop_page(fRenderContext, page);
		pthread_mutex_unlock(&fRendermutex);
    	stop = true;
	}

	if (stop)
    	return std::make_unique<BBitmap>(BRect(0, 0, width, height), B_RGBA32);

	fz_flush_warnings(fRenderContext);

    int imageWidth = pageBox->x1 - pageBox->x0;
    int imageHeight = pageBox->y1 - pageBox->y0;


    BBitmap* bitmap = new BBitmap(BRect(0, 0, imageWidth - 1, imageHeight - 1), B_RGBA32);
	bitmap->SetBits(fz_pixmap_samples(fRenderContext, image),
		imageWidth * imageHeight * fz_pixmap_components(fRenderContext, image), 0, B_RGBA32);

	fz_drop_device(fRenderContext, dev);
	fz_drop_pixmap(fRenderContext, image);
	fz_drop_display_list(fRenderContext, list);
	fz_drop_page(fRenderContext, page);

	pthread_mutex_unlock(&fRendermutex);
	return unique_ptr<BBitmap>(bitmap);
}


std::pair<BBitmap*, bool>
PDFEngine::_RenderBitmap(int const& pageNumber)
{
	unique_ptr<BBitmap> bitmap = RenderBitmap(pageNumber, fDefaultRect.Width(),
		fDefaultRect.Height(), 0);

    return std::pair<BBitmap*, bool>(bitmap.release(), false);
}
