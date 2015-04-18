#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <string>
#include <vector>

#include <microhttpd.h>
#include "DataBase.h"

class WebServer
{
private:
	struct MHD_Daemon *daemon;
	DataBase *db;

public:
	WebServer(int port, DataBase *dbConnection);
	~WebServer();

	DataBase *getDataBase();
};

#endif /* WEBSERVER_H_ */
