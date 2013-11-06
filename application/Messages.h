/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef MESSAGES_H
#define MESSAGES_H

enum
{
        MSG_ACTIVE_PAGE             	= 'ga01'    ,
        MSG_ANSWER_CURRENT_PAGENUMBER	= 'ga02'	,

		MSG_APP_QUIT                	= 'ga02'    ,

        MSG_FIT_PAGE_HEIGHT         	= 'gf01'    ,
        MSG_FIT_PAGE_WIDTH          	= 'gf02'    ,
        MSG_FULLSCREEN              	= 'gf03'    ,

        MSG_GOTO_PAGE               	= 'gg01'    ,
        
        MSG_HELP               			= 'gh01'    ,
        MSG_HIGHLIGHT_RECT				= 'gh02'    ,

        MSG_NO_FIT                  	= 'gn01'    ,
        MSG_NO_ZOOM						= 'ge02'	, 
        
        
        MSG_OPEN_FILE					= 'go01'	,
        MSG_OPEN_FILE_PANEL				= 'go02'	,
        
        MSG_PRINT_DOCUMENT				= 'gp01'	,
        
        MSG_SEARCH_RESULT				= 'gS01'	,
        MSG_SETUP_PRINTER				= 'gS02'	,
        MSG_SUPPORT						= 'gS03'	,
        
        MSG_TELL_CURRENT_PAGENUMBER		= 'gt01'	,

        // View-Menu
        MSG_VIEW_ALWAYSONTOP        	= 'gv01'    ,
		MSG_ZOOM_IN                 	= 'gz01'    ,
		
        MSG_ZOOM_OUT                	= 'gz03'    
};

#endif
