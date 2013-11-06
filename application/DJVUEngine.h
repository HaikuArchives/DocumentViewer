/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef DJVUENGINE_H
#define DJVUENGINE_H

#include <utility>

#include <String.h>

#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>

#include "BaseEngine.h"
#include "Debug.h"

class DJVUEngine : public BaseEngine
{
public:
                            DJVUEngine(BString filename, BString& password);

    virtual                 ~DJVUEngine();

    virtual BString         FileName(void) const;
    int						PageCount(void) const;

    virtual BString         GetProperty(BString name);
                                
 	virtual	void			WriteOutline(BOutlineListView* list);

 	virtual std::unique_ptr<BBitmap> 	RenderBitmap(int const& pageNumber,int const& width,
    										int const& height, int const& rotation = 0);
 
private:
    virtual std::pair<BBitmap*, bool>   _RenderBitmap(int const& pageNumber);
    
    virtual std::tuple< std::vector<BString>, std::vector<BRect> >
    									_FindString(BString const& name, int const& page);
    									
    void								_HandleDjvuMessages(ddjvu_context_t *context,
    										int wait = false);

    BString                 fFileName;
    BString               	fPassword;
    
    ddjvu_context_t*		fContext;
    ddjvu_document_t*		fDocument;
    
    static pthread_mutex_t 	gRendermutex;

    Debug             		out;
};

#endif
