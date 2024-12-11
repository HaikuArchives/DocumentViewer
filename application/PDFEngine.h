/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PDFENGINE_H
#define PDFENGINE_H

extern "C" {
#include <mupdf/fitz.h>
//#include <fitz/fitz-internal.h>
}


#include <utility>

#include <String.h>

#include "BaseEngine.h"
#include "Debug.h"

class PDFEngine : public BaseEngine
{
public:
                            PDFEngine(BString filename, BString& password);

    virtual                 ~PDFEngine();

    virtual BString         FileName(void) const;
    int						PageCount(void) const;

    virtual BString         GetProperty(BString name);

 	virtual	void			WriteOutline(BOutlineListView* list);

 	virtual std::unique_ptr<BBitmap> RenderBitmap(int const& pageNumber,uint32 const& width,
    										uint32 const& height, int const& rotation = 0);

private:
    virtual std::pair<BBitmap*, bool>   _RenderBitmap(int const& pageNumber);
    virtual std::tuple< std::vector<BString>, std::vector<BRect> >
										_FindString(BString const& name, int const& page);

    fz_document*			fDocument;
    fz_context*				fContext;
	fz_context*				fRenderContext;

    fz_page*				fPage;
	fz_display_list*		fList;
	fz_device*				fDev;

    fz_colorspace*			fColorSpace;

    BString                 fFileName;
    BString               	fPassword;

	pthread_mutex_t			fRendermutex = PTHREAD_MUTEX_INITIALIZER;

    Debug             		out;
};

#endif
