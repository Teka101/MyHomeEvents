#include "MHEWeb.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <sstream>
#include "MHEWeb.h"
#include "DataBaseHelpers.h"

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	std::stringstream *postData = static_cast<std::stringstream*>(*con_cls);
	MHEWeb *ws = static_cast<MHEWeb*>(cls);
	struct MHD_Response *response = NULL;
	int httpCode = MHD_HTTP_OK;
	int ret;

	LOG4CPLUS_DEBUG(ws->log, LOG4CPLUS_TEXT("MHEWeb::answer_to_connection - HTTP-request: url=" << url << " method=" << method << " version=" << version));
    if (boost::equals(url, "/world"))
    {
        std::stringstream ss;

        ws->buildJsonWorld(ss);
        response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
    }
    else if (boost::equals(method, "POST") && boost::starts_with(url, "/hardware/"))
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
		DBHarware hard;

        boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, hard);
        response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
        if (!ws->getDataBase()->updateHardware(hard))
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
		DBGraph graph;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, graph);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!ws->getDataBase()->updateGraph(graph))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
		delete postData;
	}
	else if (boost::equals(method, "POST") && boost::starts_with(url, "/device/"))
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
		DBDevice dev;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, dev);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!ws->getDataBase()->updateDevice(dev))
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
		DBCondition cond;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, cond);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!ws->getDataBase()->updateCondition(cond))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
		delete postData;
	}
	else if (boost::equals(method, "POST") && boost::starts_with(url, "/room/"))
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
		DBRoom room;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, room);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!ws->getDataBase()->updateRoom(room))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
		delete postData;
	}
	else if (boost::equals(method, "POST") && boost::starts_with(url, "/rgc/"))
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
		DBRoomGraphCond rgc;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, rgc);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!ws->getDataBase()->updateRoomGraphCond(rgc))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
		delete postData;
	}
	else
	{
        std::stringstream filePath;

        filePath << "static/";
        if (boost::equals(url, "/"))
            filePath << "index.html";
        else
            filePath << &url[1];
        httpCode = ws->sendFile(&response, filePath.str().c_str());
        /*if (boost::equals(url, "/") || boost::equals(url, "/static/"))
		httpCode = ws->sendPermanentRedirectTo(&response, "/static/index.html");
	else if (boost::starts_with(url, "/static/"))
		httpCode = ws->sendFile(&response, &url[1]);*/
	}
	/*else if (boost::starts_with(url, "/condition/"))
	{
		if (boost::equals(method, "POST") && boost::starts_with(url, "/condition/"))
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
				boost::property_tree::write_json(ss, pt, false);
				response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
				MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
			}
		}
	}*/
	if (response == NULL)
		httpCode = ws->sendNotFound(&response);
	MHD_add_response_header(response, "Age", "0");
	MHD_add_response_header(response, "Server", "MyHomeEvents");
	ret = MHD_queue_response(connection, httpCode, response);
	MHD_destroy_response(response);
	return ret;
}

MHEWeb::MHEWeb(int port, MHEDatabase *db, MHEHardDevContainer *hardDev) : _db(db), _hardDev(hardDev)
{
    this->log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHEWeb"));
	LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("MHEWeb::MHEWeb - start web server on port " << port));
	_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port, NULL, NULL, &answer_to_connection, this, MHD_OPTION_END);
}

MHEWeb::~MHEWeb()
{
    if (_daemon != NULL)
		MHD_stop_daemon(_daemon);
}

int MHEWeb::sendNotFound(struct MHD_Response **response)
{
	const char *errorPage = "Ressource not found";

	*response = MHD_create_response_from_buffer(strlen(errorPage), (void *)errorPage, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(*response, "Content-Type", "text/html");
	return MHD_HTTP_NOT_FOUND;
}

int MHEWeb::sendPermanentRedirectTo(struct MHD_Response **response, const char *location)
{
	const char *errorPage = "Redirect to home";

	*response = MHD_create_response_from_buffer(strlen(errorPage), (void *)errorPage, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(*response, "Location", "/static/index.html");
	return MHD_HTTP_MOVED_PERMANENTLY;
}

int MHEWeb::sendFile(struct MHD_Response **response, const char *url)
{
	int fd;

	if ((fd = open(url, O_RDONLY)) >= 0)
	{
		struct stat st;

		fstat(fd, &st);
		LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("MHEWeb::sendFile - send file: size=" << st.st_size));
		*response = MHD_create_response_from_fd(st.st_size, fd);
		if (boost::ends_with(url, ".css"))
			MHD_add_response_header(*response, "Content-Type", "text/css");
		else if (boost::ends_with(url, ".js"))
			MHD_add_response_header(*response, "Content-Type", "application/javascript");
		else
			MHD_add_response_header(*response, "Content-Type", "text/html");
		return MHD_HTTP_OK;
	}
	LOG4CPLUS_ERROR(log, LOG4CPLUS_TEXT("MHEWeb::sendFile - unable to send file: " << url));
	return MHD_HTTP_NOT_FOUND;
}

void MHEWeb::buildJsonWorld(std::stringstream &ss)
{
    std::vector<DBHarware> hards = _db->getHardwares();
    std::vector<DBDevice> devs = _db->getDevices();
    std::vector<DBGraph> graphs = _db->getGraphs();
    std::vector<DBCondition> conds = _db->getConditions();
    std::vector<DBRoom> rooms = _db->getRooms();
    std::vector<DBRoomGraphCond> rgcs = _db->getRoomGraphConds();
    boost::property_tree::ptree pt;

    if (hards.size() > 0)
    {
        boost::property_tree::ptree ptChildren;

        BOOST_FOREACH(DBHarware hard, hards)
        {
            boost::property_tree::ptree ptChild;

            writeToPTree(ptChild, hard);
            ptChildren.push_back(std::make_pair("", ptChild));
        }
        pt.add_child("hardwares", ptChildren);
    }
    if (devs.size() > 0)
    {
        boost::property_tree::ptree ptChildren;

        BOOST_FOREACH(DBDevice dev, devs)
        {
            boost::property_tree::ptree ptChild;

            writeToPTree(ptChild, dev);
            ptChildren.push_back(std::make_pair("", ptChild));
        }
        pt.add_child("devices", ptChildren);
    }
    if (graphs.size() > 0)
    {
        boost::property_tree::ptree ptChildren;

        BOOST_FOREACH(DBGraph graph, graphs)
        {
            boost::property_tree::ptree ptChild;

            writeToPTree(ptChild, graph);
            ptChildren.push_back(std::make_pair("", ptChild));
        }
        pt.add_child("graphics", ptChildren);
    }
    if (conds.size() > 0)
    {
        boost::property_tree::ptree ptChildren;

        BOOST_FOREACH(DBCondition cond, conds)
        {
            boost::property_tree::ptree ptChild;

            writeToPTree(ptChild, cond);
            ptChildren.push_back(std::make_pair("", ptChild));
        }
        pt.add_child("conditions", ptChildren);
    }
    if (rooms.size() > 0)
    {
        boost::property_tree::ptree ptChildren;

        BOOST_FOREACH(DBRoom room, rooms)
        {
            boost::property_tree::ptree ptChild;

            writeToPTree(ptChild, room);
            ptChildren.push_back(std::make_pair("", ptChild));
        }
        pt.add_child("rooms", ptChildren);
    }
    if (rgcs.size() > 0)
    {
        boost::property_tree::ptree ptChildren;

        BOOST_FOREACH(DBRoomGraphCond rgc, rgcs)
        {
            boost::property_tree::ptree ptChild;

            writeToPTree(ptChild, rgc);
            ptChildren.push_back(std::make_pair("", ptChild));
        }
        pt.add_child("roomGraphConds", ptChildren);
    }
	boost::property_tree::write_json(ss, pt, false);
}

MHEDatabase *MHEWeb::getDataBase()
{
    return _db;
}
