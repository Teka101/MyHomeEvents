#ifndef MHEWEB_H
#define MHEWEB_H

#include <log4cplus/logger.h>
#include <microhttpd.h>
#include <string>
#include <vector>

class MHEWeb
{
    private:
        struct MHD_Daemon *_daemon;

    public:
        log4cplus::Logger log;

        MHEWeb(int port);
        ~MHEWeb();

        int sendNotFound(struct MHD_Response **response);
        int sendPermanentRedirectTo(struct MHD_Response **response, const char *location);
        int sendFile(struct MHD_Response **response, const char *url);
};

#endif // MHEWEB_H
