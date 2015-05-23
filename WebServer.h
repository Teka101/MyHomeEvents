#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <string>
#include <vector>

#include <log4cplus/loggingmacros.h>
#include <microhttpd.h>
#include "DataBase.h"

class WebServer
{
private:
	struct MHD_Daemon *daemon;
	DataBase *db;

public:
	log4cplus::Logger log;

	WebServer(int port, DataBase *dbConnection);
	~WebServer();

	DataBase *getDataBase();
	int sendNotFound(struct MHD_Response **response);
	int sendPermanentRedirectTo(struct MHD_Response **response, const char *location);
	int sendFile(struct MHD_Response **response, const char *url);
};

#endif /* WEBSERVER_H_ */
