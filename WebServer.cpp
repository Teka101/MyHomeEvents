#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "DataBaseHelpers.h"
#include "WebServer.h"

typedef std::pair<const std::string, boost::property_tree::ptree> ptreePair;

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	std::stringstream *postData = static_cast<std::stringstream*>(*con_cls);
	WebServer *ws = static_cast<WebServer*>(cls);
	struct MHD_Response *response = NULL;
	int httpCode = MHD_HTTP_OK;
	int ret;

	std::cout << "HTTP-request: url=" << url << " method=" << method << " version=" << version << std::endl;
	if (boost::equals(url, "/"))
		httpCode = ws->sendPermanentRedirectTo(&response, "/static/index.html");
	else if (boost::starts_with(url, "/static/"))
		httpCode = ws->sendFile(&response, &url[1]);
	else if (boost::equals(url, "/conditions"))
	{
		boost::property_tree::ptree pt;
		boost::property_tree::ptree ptChildren;
		std::ostringstream ss;

		BOOST_FOREACH(sCondition cond, ws->getDataBase()->getConditions())
		{
			boost::property_tree::ptree ptChild;

			writeToPTree(ptChild, cond);
			ptChildren.push_back(std::make_pair("", ptChild));
		}
		pt.add_child("conditions", ptChildren);
		boost::property_tree::write_json(ss, pt, false);
		response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
	}
	else if (boost::starts_with(url, "/condition/"))
	{
		if (boost::equals(method, "POST") && boost::equals(url, "/condition/"))
		{
			if (postData == NULL)
			{
				postData = new std::stringstream();
				*con_cls = postData;
				return MHD_YES;
			}
			if (*upload_data_size > 0)
			{
				*postData << upload_data;
				*upload_data_size = 0;
				return MHD_YES;
			}
			boost::property_tree::ptree pTree;
			std::string description;

			boost::property_tree::read_json(*postData, pTree);
			description = pTree.get<std::string>("description");
			response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
			if (!ws->getDataBase()->addCondition(description))
				httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
			delete postData;
		}
		else if (boost::equals(method, "POST") && boost::starts_with(url, "/condition/"))
		{
			if (postData == NULL)
			{
				postData = new std::stringstream();
				*con_cls = postData;
				return MHD_YES;
			}
			if (*upload_data_size > 0)
			{
				*postData << upload_data;
				*upload_data_size = 0;
				return MHD_YES;
			}
			boost::property_tree::ptree pTree;
			std::string description;
			sCondition cond;

			boost::property_tree::read_json(*postData, pTree);
			readFromPTree(pTree, cond);
			response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
			if (!ws->getDataBase()->updateCondition(cond))
				httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
			delete postData;
		}
		else
		{
			const char *idSTR = &url[11];

			if (*idSTR != '\0')
			{
				boost::property_tree::ptree pt;
				boost::property_tree::ptree ptChildren;
				std::ostringstream ss;
				sCondition cond;
				int id = boost::lexical_cast<int>(idSTR);

				cond = ws->getDataBase()->getCondition(id);
				writeToPTree(pt, cond);
				/*for (sGraphData data : graph.data)
				{
					boost::property_tree::ptree ptChild;

					writeToPTree(ptChild, data);
					ptChildren.push_back(std::make_pair("", ptChild));
				}
				pt.add_child("data", ptChildren);*/
				boost::property_tree::write_json(ss, pt, false);
				response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
				MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
			}
		}
	}
	else if (boost::equals(url, "/graphs"))
	{
		if (boost::equals(method, "POST"))
		{
			if (postData == NULL)
			{
				postData = new std::stringstream();
				*con_cls = postData;
				return MHD_YES;
			}
			if (*upload_data_size > 0)
			{
				*postData << upload_data;
				*upload_data_size = 0;
				return MHD_YES;
			}
			boost::property_tree::ptree pTree;

			boost::property_tree::read_json(*postData, pTree);
			BOOST_FOREACH(ptreePair it, pTree.get_child("graphs"))
			{
				sGraph graph;

				readFromPTree(it.second, graph);
				if (!ws->getDataBase()->updateGraph(graph))
					httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
			}
			response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
			delete postData;
		}
		else
		{
			boost::property_tree::ptree pt;
			boost::property_tree::ptree ptChildren;
			std::ostringstream ss;

			BOOST_FOREACH(sGraph graph, ws->getDataBase()->getGraphs())
			{
				boost::property_tree::ptree ptChild;

				writeToPTree(ptChild, graph);
				ptChildren.push_back(std::make_pair("", ptChild));
			}
			pt.add_child("graphs", ptChildren);
			boost::property_tree::write_json(ss, pt, false);
			response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
			MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
		}
	}
	else if (boost::starts_with(url, "/graph/"))
	{
		if (boost::equals(method, "POST") && boost::equals(url, "/graph/"))
		{
			if (postData == NULL)
			{
				postData = new std::stringstream();
				*con_cls = postData;
				return MHD_YES;
			}
			if (*upload_data_size > 0)
			{
				*postData << upload_data;
				*upload_data_size = 0;
				return MHD_YES;
			}
			boost::property_tree::ptree pTree;
			std::string description;

			boost::property_tree::read_json(*postData, pTree);
			description = pTree.get<std::string>("description");
			response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
			if (!ws->getDataBase()->addGraph(description))
				httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
			delete postData;
		}
		else if (boost::equals(method, "POST") && boost::starts_with(url, "/graph/"))
		{
			if (postData == NULL)
			{
				postData = new std::stringstream();
				*con_cls = postData;
				return MHD_YES;
			}
			if (*upload_data_size > 0)
			{
				*postData << upload_data;
				*upload_data_size = 0;
				return MHD_YES;
			}
			boost::property_tree::ptree pTree;
			std::string description;
			sGraph graph;

			boost::property_tree::read_json(*postData, pTree);
			readFromPTree(pTree, graph, true);
			response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
			if (!ws->getDataBase()->updateGraphData(graph))
				httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
			delete postData;
		}
		else
		{
			const char *idSTR = &url[7];

			if (*idSTR != '\0')
			{
				boost::property_tree::ptree pt;
				std::ostringstream ss;
				sGraph graph;
				int id = boost::lexical_cast<int>(idSTR);

				graph = ws->getDataBase()->getGraph(id);
				writeToPTree(pt, graph, true);
				boost::property_tree::write_json(ss, pt, false);
				response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
				MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
			}
		}
	}
	if (response == NULL)
		httpCode = ws->sendNotFound(&response);
	MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
	MHD_add_response_header(response, "Age", "0");
	MHD_add_response_header(response, "Server", "MyHomeEvents");
	ret = MHD_queue_response(connection, httpCode, response);
	MHD_destroy_response(response);
	return ret;
}

WebServer::WebServer(int port, DataBase *dbConnection) : db(dbConnection)
{
	daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port, NULL, NULL, &answer_to_connection, this, MHD_OPTION_END);
}

WebServer::~WebServer()
{
	if (daemon != NULL)
		MHD_stop_daemon(daemon);
}

DataBase *WebServer::getDataBase()
{
	return this->db;
}

int WebServer::sendNotFound(struct MHD_Response **response)
{
	const char *errorPage = "Ressource not found";

	*response = MHD_create_response_from_buffer(strlen(errorPage), (void *)errorPage, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(*response, "Content-Type", "text/html");
	return MHD_HTTP_NOT_FOUND;
}

int WebServer::sendPermanentRedirectTo(struct MHD_Response **response, const char *location)
{
	const char *errorPage = "Redirect to home";

	*response = MHD_create_response_from_buffer(strlen(errorPage), (void *)errorPage, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(*response, "Location", "/static/index.html");
	return MHD_HTTP_MOVED_PERMANENTLY;
}

int WebServer::sendFile(struct MHD_Response **response, const char *url)
{
	int fd;

	if ((fd = open(url, O_RDONLY)) >= 0)
	{
		struct stat st;

		fstat(fd, &st);
		std::cout << "Send file: size=" << st.st_size << std::endl;
		*response = MHD_create_response_from_fd(st.st_size, fd);
		if (boost::ends_with(url, ".css"))
			MHD_add_response_header(*response, "Content-Type", "text/css");
		else if (boost::ends_with(url, ".js"))
			MHD_add_response_header(*response, "Content-Type", "application/javascript");
		else
			MHD_add_response_header(*response, "Content-Type", "text/html");
		return MHD_HTTP_OK;
	}
	return MHD_HTTP_NOT_FOUND;
}
