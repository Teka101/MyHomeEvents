#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <boost/algorithm/string/predicate.hpp>
#include "WebServer.h"

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	const char *page = "<html><body>Hello, browser!</body></html>";
	struct MHD_Response *response = nullptr;
	int ret;

	std::cout << "HTTP-request: url=" << url << " method=" << method << " version=" << version << std::endl;
	//MHD_create_response_from_fd
	//MHD_add_response_header
	if (boost::starts_with(url, "/static/"))
	{
		int fd;

		if ((fd = open(&url[1], O_RDONLY)) >= 0)
		{
			struct stat st;

			fstat(fd, &st);
			std::cout << "Send file: size=" << st.st_size << std::endl;
			response = MHD_create_response_from_fd(st.st_size, fd);
		}
	}
	else
		response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
	if (response == nullptr)
	{
		const char *errorPage = "Ressource not found";

		response = MHD_create_response_from_buffer(strlen(errorPage), (void *)errorPage, MHD_RESPMEM_PERSISTENT);
		ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
	}
	else
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

WebServer::WebServer(int port)
{
	daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port, nullptr, nullptr, &answer_to_connection, nullptr, MHD_OPTION_END);
}

WebServer::~WebServer()
{
	if (daemon != nullptr)
		MHD_stop_daemon(daemon);
}
