/*
 * Copyright 2012-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef FLAGS_H
#define FLAGS_H

inline bool GHasFlag(int const& kValue, int const& kFlag)
{
    return ((kValue & kFlag) == kFlag);
}

const int SEARCH_MATCH_CASE 	= 1 << 0	;
const int SEARCH_WHOLE_WORD 	= 1 << 1	;

#endif
