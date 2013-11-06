/*
 * Copyright 2011-2012 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ciprian Nedisan (cipri)
 *
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <String.h>

#include "Debug.h"
#include "Tools.h"

class Settings
{
public:
    Settings(BString name)
        :
        fName(name),
        fLabel("")
    {
        fLoadMessage = nullptr;
        fSaveMessage = nullptr;

        fAppName = Tools::AppName().Capitalize();
        fSettings = new BMessage();
        _Load(fSettings);
    }


    ~Settings()
    {
       delete fLoadMessage;
       delete fSaveMessage;
       delete fSettings;
    }


    void Clear(void)
    {
        if (fName == "")
            return;

        delete fSaveMessage;
        fSaveMessage = new BMessage();
        _Save();

    }


    void ClearAll(void)
    {
        delete fSaveMessage;
        fSaveMessage = new BMessage();
        delete fSettings;
        fSettings = new BMessage();
        _Save();
        // TODO: better delete file and directory

    }


    Settings& operator<<(std::ostream& (*f)(std::ostream&))
    {
        if (f == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            _Save();
            delete fSettings;
            fSettings = nullptr;
            delete fLoadMessage;
            fLoadMessage = nullptr;
            delete fSaveMessage;
            fSaveMessage = nullptr;
        }

        return *this;
    }


    Settings& operator<<(BString const& str)
    {
        if (fLabel.Length() == 0) {
            fLabel = str;
            return *this;
        }

        _CheckSaveNull();

        BString temp;
        if (fSaveMessage->FindString(fLabel, 0, &temp) == B_OK)
            fSaveMessage->ReplaceString(fLabel, str);
        else
            fSaveMessage->AddString(fLabel, str);

        fLabel = "";

        return *this;
    }

/*
    Settings& operator<<(bool value)
    {
        int32 newvalue = static_cast<int32>(value);

        if (fLabel.Length() == 0) {
            return *this;
        } else {
            if (fSaveMessage == nullptr) {
                fSaveMessage = new BMessage();
                fSettings->FindMessage(fName, 0, fSaveMessage);
            }

            int32 temp = 0;
            if (fSaveMessage->FindInt32(fLabel, 0, &temp) == B_OK)
                fSaveMessage->ReplaceInt32(fLabel, newvalue);
            else
                fSaveMessage->AddInt32(fLabel, newvalue);

            fLabel = "";
        }

        return *this;
    }
*/

    Settings& operator<<(int32 value)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckSaveNull();

        int32 temp = 0;
        if (fSaveMessage->FindInt32(fLabel, 0, &temp) == B_OK)
            fSaveMessage->ReplaceInt32(fLabel, value);
        else
            fSaveMessage->AddInt32(fLabel, value);

        fLabel = "";

        return *this;
    }


    Settings& operator<<(float value)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckSaveNull();

        float temp = 0;
        if (fSaveMessage->FindFloat(fLabel, 0, &temp) == B_OK)
            fSaveMessage->ReplaceFloat(fLabel, value);
        else
            fSaveMessage->AddFloat(fLabel, value);

        fLabel = "";

        return *this;
    }


    Settings& operator<<(BRect value)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckSaveNull();

        BRect temp;
        if (fSaveMessage->FindRect(fLabel, 0, &temp) == B_OK)
            fSaveMessage->ReplaceRect(fLabel, value);
        else
            fSaveMessage->AddRect(fLabel, value);

        fLabel = "";

        return *this;
    }


    Settings& operator>>(BString & str)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckLoadNull();

        BString temp = "";
        if (fLoadMessage->FindString(fLabel, 0, &temp) == B_OK)
            str = temp;

        fLabel = "";
        return *this;
    }

/*
    Settings& operator>>(bool & value)
    {
        if (fLabel.Length() == 0)
            return *this;

        if (fLoadMessage == nullptr) {
            fLoadMessage = new BMessage();
            fSettings->FindMessage(fName, 0, fLoadMessage);
        }

        bool temp = 0;
        if (fLoadMessage->FindBool(fLabel, 0, &temp) == B_OK)
            value = temp;

        fLabel = "";
        return *this;
    }
*/

    Settings& operator>>(int32 & value)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckLoadNull();

        int32 temp = 0;
        if (fLoadMessage->FindInt32(fLabel, 0, &temp) == B_OK)
            value = temp;

        fLabel = "";
        return *this;
    }


    Settings& operator>>(float & value)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckLoadNull();

        float temp = 0;
        if (fLoadMessage->FindFloat(fLabel, 0, &temp) == B_OK)
            value = temp;

        fLabel = "";
        return *this;
    }


    Settings& operator>>(BRect & value)
    {
        if (fLabel.Length() == 0)
            return *this;

        _CheckLoadNull();

        BRect temp;
        if (fLoadMessage->FindRect(fLabel, 0, &temp) == B_OK)
            value = temp;

        fLabel = "";
        return *this;
    }

private:

    status_t _Open(BFile& file, uint32 mode)
    {
        BPath path;
        if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
            return B_ERROR;

        path.Append(fAppName);
        create_directory(path.Path(), 0755);
        path.Append(fAppName);
        return file.SetTo(path.Path(), mode);
    }


    status_t _Save()
    {
        BFile file;
        status_t status = _Open(file, B_WRITE_ONLY | B_CREATE_FILE
            | B_ERASE_FILE);
        if (status < B_OK)
            return status;

        BMessage msg;

        if (fSettings->FindMessage(fName, &msg) == B_OK )
            fSettings->ReplaceMessage(fName, fSaveMessage);
        else
            fSettings->AddMessage(fName, fSaveMessage);

        status = fSettings->Flatten(&file);
        if (status < B_OK)
            return status;

        return status;
    }


    status_t _Load(BMessage * settings)
    {
        BFile file;
        status_t status = _Open(file, B_READ_ONLY);
        if (status < B_OK)
            return status;

        return settings->Unflatten(&file);
    }


    void _CheckSaveNull(void)
    {
        if (fSettings == nullptr) {
            fSettings = new BMessage();
            _Load(fSettings);
        } else if (fSaveMessage == nullptr) {
            fSaveMessage = new BMessage();
            fSettings->FindMessage(fName, 0, fSaveMessage);
        }
    }

    void _CheckLoadNull(void)
    {
        if (fSettings == nullptr) {
            fSettings = new BMessage();
            _Load(fSettings);
        } else if (fLoadMessage == nullptr) {
            fLoadMessage = new BMessage();
            fSettings->FindMessage(fName, 0, fLoadMessage);
        }
    }


    BString                 fName;
    BString                 fLabel;
    BString                 fAppName;
    BMessage*               fSettings;
    BMessage*               fSaveMessage;
    BMessage*               fLoadMessage;

    Debug                   out;

};

#endif
