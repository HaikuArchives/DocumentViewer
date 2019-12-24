/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef PDF_FILTER_H
#define PDF_FILTER_H

#include <FilePanel.h>

class PDFFilter : public BRefFilter {
public:
	bool	Filter(const entry_ref* ref, BNode* node, struct stat_beos* st,
				const char* filetype);
};

#endif
