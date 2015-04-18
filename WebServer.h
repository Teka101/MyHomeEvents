#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <string>
#include <vector>

#include <microhttpd.h>

class WebServer
{
private:
	struct MHD_Daemon *daemon;

public:
	WebServer(int port);
	~WebServer();
};

#endif /* WEBSERVER_H_ */
