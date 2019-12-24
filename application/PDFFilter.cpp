/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */

#include "PDFFilter.h"

#include <string.h>

#include <MimeType.h>
#include <SymLink.h>

static const char* valid_filetypes[] = {
	"application/pdf",
	"application/djvu",
	"application/xps",
	NULL
};

bool
PDFFilter::Filter(const entry_ref* ref, BNode* node, struct stat_beos* st,
	const char* filetype)
{
	BEntry entry(ref, true);
	if (entry.InitCheck() != B_OK)
		return false;
	if (entry.IsDirectory())
		return true;

	entry_ref linkedRef;
	if (entry.GetRef(&linkedRef) != B_OK)
		return false;

	BMimeType mimeType;
	if (BMimeType::GuessMimeType(&linkedRef, &mimeType) != B_OK)
		return false;

	for (int i = 0; valid_filetypes[i] != nullptr; i++)
		if (strcmp(mimeType.Type(), valid_filetypes[i]) == 0)
			return true;

	return false;
}
