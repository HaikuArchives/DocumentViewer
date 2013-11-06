/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#include "MainApplication.h"

int main( void )
{
	new MainApplication();

	be_app->Run();

	delete be_app;

	return 0;
}
