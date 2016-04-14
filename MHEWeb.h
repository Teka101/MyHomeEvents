/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MHEWEB_H
#define MHEWEB_H

#include <microhttpd.h>
#include <sstream>
#include <string>
#include "MHEHardDevContainer.h"
#include "MHEMobileNotify.h"
#include "MHESpeechService.h"

class MHESpeechService;

class MHEWeb
{
    private:
        struct MHD_Daemon *_daemon;
        MHEDatabase *_db;
        MHEHardDevContainer *_hardDev;
        MHEMobileNotify *_notify;
        MHESpeechService *_ss;

        void buildJsonWorld(std::stringstream &ss);

    public:
        log4cplus::Logger log;

        MHEWeb(int port, MHEDatabase *db, MHEHardDevContainer *hardDev, MHEMobileNotify *notify, MHESpeechService *ss);
        ~MHEWeb();

        struct MHD_Response *buildStatusResponse(bool status, int &httpCode);
        struct MHD_Response *doAdmin(const char *method, const char *url, std::stringstream *postData, int &httpCode);

        int sendNotFound(struct MHD_Response **response);
        int sendPermanentRedirectTo(struct MHD_Response **response, const char *location);
        int sendFile(struct MHD_Response **response, const char *url);

        MHEDatabase *getDataBase() const;
        MHEHardDevContainer *getHardDevContainer() const;
        MHEMobileNotify *getMobileNotify() const;
        MHESpeechService *getSpeechService() const;
        bool isStarted() const;
};

#endif // MHEWEB_H
