#ifndef MHEWEB_H
#define MHEWEB_H

#include <log4cplus/logger.h>
#include <microhttpd.h>
#include <sstream>
#include <string>
#include <vector>
#include "MHEHardDevContainer.h"

class MHEWeb
{
    private:
        struct MHD_Daemon *_daemon;
        MHEDatabase *_db;
        MHEHardDevContainer *_hardDev;

    public:
        log4cplus::Logger log;

        MHEWeb(int port, MHEDatabase *db, MHEHardDevContainer *hardDev);
        ~MHEWeb();

        int sendNotFound(struct MHD_Response **response);
        int sendPermanentRedirectTo(struct MHD_Response **response, const char *location);
        int sendFile(struct MHD_Response **response, const char *url);

        void buildJsonWorld(std::stringstream &ss);
};

#endif // MHEWEB_H
