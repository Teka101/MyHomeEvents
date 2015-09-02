#ifndef MHEWEB_H
#define MHEWEB_H

#include <log4cplus/logger.h>
#include <microhttpd.h>
#include <sstream>
#include <string>
#include <vector>
#include "MHEHardDevContainer.h"
#include "MHEMobileNotify.h"

class MHEWeb
{
    private:
        struct MHD_Daemon *_daemon;
        MHEDatabase *_db;
        MHEHardDevContainer *_hardDev;
        MHEMobileNotify *_notify;

        void buildJsonWorld(std::stringstream &ss);

    public:
        log4cplus::Logger log;

        MHEWeb(int port, MHEDatabase *db, MHEHardDevContainer *hardDev, MHEMobileNotify *notify);
        ~MHEWeb();

        struct MHD_Response *doAdmin(const char *method, const char *url, std::stringstream *postData, int &httpCode);

        int sendNotFound(struct MHD_Response **response);
        int sendPermanentRedirectTo(struct MHD_Response **response, const char *location);
        int sendFile(struct MHD_Response **response, const char *url);

        MHEDatabase *getDataBase();
        MHEHardDevContainer *getHardDevContainer();
        MHEMobileNotify *getMobileNotify();
};

#endif // MHEWEB_H
