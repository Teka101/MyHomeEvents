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
