/*
 * Copyright 2011-2011 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <String.h>
#include <Point.h>

using std::endl;

#if DEBUG == 1

class Debug
{
public:
    Debug(void)
        :
        fStr(""),
        fState(true)
    {

    }

    Debug(BString str)
        :
        fStr(""),
        fState(true)
    {
        std::cout << str.String() << std::endl;
    }


    Debug& operator<<(std::ostream& (*f)(std::ostream&))
    {
        std::cout<<fStr<< *f;
        fStr = "";
        return *this;
    }

    Debug&	operator ! ()
    {
        fStr = BString();
        std::cout.clear();
        std::cout << std::flush;
        return *this;
    }

    Debug& operator<<(BString str)
    {
        fStr += str;
        return *this;
    }

    Debug& operator<<(BPoint const& point)
    {
        std::cout << "(" << point.x << ";" << point.y << ")";
        return *this;
    }

    Debug& operator<<(float number)
    {
        fStr << number;
        return *this;
    }

    Debug& operator<<(int number)
    {
        fStr << number;

        return( *this );
    }

    Debug& operator<<(unsigned int number)
    {
        fStr << number;

        return( *this );
    }

    Debug& operator<<(char insert)
    {
        fStr << insert;

        return( *this );
    }

    Debug& operator<<(int32 number)
    {
        fStr << number;
        return *this;
    }

    bool const&  state(void)
    {
        return(fState);
    }

    void Clear(void)
    {
        fStr = "";
    }

private:
    BString     fStr;
    bool        fState;
};

// Else create Class which does nothing
#else
class Debug
{
public:
    Debug(void)
        :
        fState(false)
    {

    }

    Debug(BString str)
        :
        fState(false)
        {

        }

    Debug& operator<<(std::ostream& (*f)(std::ostream&))
    {
        return *this;
    }

    Debug&	operator ! ()               { return *this; }
    Debug& operator<<(BString)          { return *this; }
    Debug& operator<<(BPoint)           { return *this; }
    Debug& operator<<(float)            { return *this; }
    Debug& operator<<(int)              { return *this; }
    Debug& operator<<(uint)             { return *this; }
    Debug& operator<<(char)             { return *this; }
    Debug& operator<<(int32)            { return *this; }
    bool const&  state(void)            { return fState; }
    void Clear(void)                    {               }

private:
    bool    fState;
};

#endif

#endif
