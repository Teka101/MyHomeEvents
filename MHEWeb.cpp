#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/regex.hpp>
#include <log4cplus/loggingmacros.h>
#include <iostream>
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
	}
	else
        postData = NULL;
    if (boost::starts_with(url, "/admin/"))
        response = ws->doAdmin(method, url, postData, httpCode);
    else if (boost::equals(url, "/runningDevices"))
    {
        boost::property_tree::ptree pt;
        std::vector<MHEDevice*> devices = ws->getHardDevContainer()->getDevices();
        std::stringstream ss;

        if (devices.size() > 0)
        {
            boost::property_tree::ptree ptChildren;

            BOOST_FOREACH(MHEDevice *dev, devices)
            {
                boost::property_tree::ptree ptChild;

                writeToPTree(ptChild, *dev, dev->isCacheRunning());
                ptChildren.push_back(std::make_pair("", ptChild));
            }
            pt.add_child("devices", ptChildren);
        }
        boost::property_tree::write_json(ss, pt, false);
        response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
    }
    else if (boost::equals(method, "GET") && boost::starts_with(url, "/mobile/"))
	{
		boost::regex expression("^/mobile/(.*)/(.*)/(.*)$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression))
		{
            std::string type = what[1];
            std::string user = what[2];
            std::string token = what[3];

            response = ws->buildStatusResponse(ws->getDataBase()->addMobile(type, user, token), httpCode);
        }
	}
	else if (boost::equals(method, "GET") && boost::starts_with(url, "/notify/"))
	{
		boost::regex expression("^/notify/(.+)/(.+)/(.+)$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression))
		{
            std::string event = what[1];
            std::string type = what[2];
            std::string message = what[3];

            if (ws->getMobileNotify() != NULL)
            {
                ws->getMobileNotify()->notifyDevices(event, type, message);
                response = ws->buildStatusResponse(true, httpCode);
            }
            else
                response = ws->buildStatusResponse(false, httpCode);
		}
	}
	else if (boost::equals(method, "GET") && boost::starts_with(url, "/setChannel/"))
	{
		boost::regex expression("^/setChannel/([0-9]+)/(.+)$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression))
		{
            int deviceId = boost::lexical_cast<int>(what[1]);
            std::string channelId = what[2];
            MHEDevice *dev = ws->getHardDevContainer()->getDeviceById(deviceId);

            response = ws->buildStatusResponse(dev != NULL && dev->sendCommand("channel", channelId, NULL), httpCode);
		}
	}
	else if (boost::equals(method, "GET") && boost::starts_with(url, "/setStatus/"))
	{
		boost::regex expression("^/setStatus/([0-9]+)/(false|true)$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression))
		{
            int deviceId = boost::lexical_cast<int>(what[1]);
            std::string activate = what[2];
            MHEDevice *dev = ws->getHardDevContainer()->getDeviceById(deviceId);

            response = ws->buildStatusResponse(dev != NULL && dev->setStatus(activate == "true"), httpCode);
		}
	}
	else if (boost::equals(method, "GET") && boost::starts_with(url, "/getSpecial/"))
	{
		boost::regex expressionGraph("^/getSpecial/([0-9]+)/graph/(last24h|lastMonth|lastYear)$", boost::regex::perl);
		boost::regex expressionVol("^/getSpecial/([0-9]+)/volume$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expressionGraph))
		{
            int deviceId = boost::lexical_cast<int>(what[1]);
            std::string type = what[2];
            tMHEDeviceValues values;
            MHEDevice *dev = ws->getHardDevContainer()->getDeviceById(deviceId);

            if (dev != NULL && dev->sendCommand("values", type, &values))
            {
                boost::property_tree::ptree pt;
                std::stringstream ss;

                if (values.size() > 0)
                {
                    boost::property_tree::ptree ptChildren;

                    BOOST_FOREACH(sMHEDeviceValue &value, values)
                    {
                        boost::property_tree::ptree ptChild;

                        writeToPTree(ptChild, value);
                        ptChildren.push_back(std::make_pair("", ptChild));
                    }
                    pt.add_child("values", ptChildren);
                }
                boost::property_tree::write_json(ss, pt, false);
                response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
                MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
            }
		}
		else if (boost::regex_match(url, what, expressionVol))
		{
            int deviceId = boost::lexical_cast<int>(what[1]);
            sMHEDeviceVolume value;
            MHEDevice *dev = ws->getHardDevContainer()->getDeviceById(deviceId);

            if (dev != NULL && dev->sendCommand("volume", "", &value))
            {
                boost::property_tree::ptree pt;
                std::stringstream ss;

                writeToPTree(pt, value);
                boost::property_tree::write_json(ss, pt, false);
                response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
                MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
            }
		}
	}
	else if (boost::equals(method, "GET") && boost::starts_with(url, "/setSpecial/"))
	{
		boost::regex expression("^/setSpecial/([0-9]+)/volume/([0-9]+)$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression))
		{
            int deviceId = boost::lexical_cast<int>(what[1]);
            std::string volume = what[2];
            tMHEDeviceValues values;
            MHEDevice *dev = ws->getHardDevContainer()->getDeviceById(deviceId);

            response = ws->buildStatusResponse(dev != NULL && dev->sendCommand("volume", volume, NULL), httpCode);
		}
	}
	else if (boost::equals(method, "GET") && boost::starts_with(url, "/order/"))
	{
		boost::regex expression("^/order/(.*)$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression))
		{
            std::string order = what[1];
            sSpeechOrder value = ws->getSpeechService()->parseAndExecute(ws, order);
            std::stringstream ss;

            ss << "{ \"debug\": \"" << value.debug << "\", \"isSuccess\": " << (value.isSuccess ? "true" : "false") << ", \"result\": \"" << value.result << "\" }";
            response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
            MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
		}
	}
	else
	{
        std::stringstream filePath;

        filePath << "static/";
        if (boost::equals(url, "/"))
            filePath << "index.html";
        else
        {
            std::string s = &url[1];

            boost::replace_all(s, "../", "");
            filePath << s;
        }
        httpCode = ws->sendFile(&response, filePath.str().c_str());
	}
	if (response == NULL)
		httpCode = ws->sendNotFound(&response);
	MHD_add_response_header(response, "Age", "0");
	MHD_add_response_header(response, "Server", "MyHomeEvents");
	ret = MHD_queue_response(connection, httpCode, response);
	MHD_destroy_response(response);
	if (postData != NULL)
        delete postData;
	return ret;
}

MHEWeb::MHEWeb(int port, MHEDatabase *db, MHEHardDevContainer *hardDev, MHEMobileNotify *notify, MHESpeechService *ss) : _db(db), _hardDev(hardDev), _notify(notify), _ss(ss)
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

bool MHEWeb::isStarted() const
{
    return (_daemon != NULL);
}

int MHEWeb::sendNotFound(struct MHD_Response **response)
{
	const std::string errorPage = "Ressource not found";

	*response = MHD_create_response_from_buffer(errorPage.length(), (void *)errorPage.c_str(), MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(*response, "Content-Type", "text/html");
	return MHD_HTTP_NOT_FOUND;
}

int MHEWeb::sendPermanentRedirectTo(struct MHD_Response **response, const char *location)
{
	const std::string errorPage = "Redirect to home";

	*response = MHD_create_response_from_buffer(errorPage.length(), (void *)errorPage.c_str(), MHD_RESPMEM_PERSISTENT);
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

MHEDatabase *MHEWeb::getDataBase() const
{
    return _db;
}

MHEHardDevContainer *MHEWeb::getHardDevContainer() const
{
    return _hardDev;
}

MHEMobileNotify *MHEWeb::getMobileNotify() const
{
    return _notify;
}

MHESpeechService *MHEWeb::getSpeechService() const
{
    return _ss;
}

struct MHD_Response *MHEWeb::buildStatusResponse(bool status, int &httpCode)
{
    struct MHD_Response *response;
    std::stringstream ss;

    ss << "{\"success\": "<< (status ? "true" : "false") << "}";
    response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
    return response;
}

struct MHD_Response *MHEWeb::doAdmin(const char *method, const char *url, std::stringstream *postData, int &httpCode)
{
    MHD_Response *response = NULL;

    if (boost::equals(url, "/admin/world"))
    {
        std::stringstream ss;

        buildJsonWorld(ss);
        response = MHD_create_response_from_buffer(ss.tellp(), (void *)ss.str().c_str(), MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", "text/json; charset=UTF-8");
    }
    else if (postData != NULL && boost::starts_with(url, "/admin/hardware/"))
	{
		boost::property_tree::ptree pTree;
		DBHarware hard;

        boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, hard);
        response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
        if (!getDataBase()->updateHardware(hard))
            httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}
    else if (postData != NULL && boost::starts_with(url, "/admin/graph/"))
	{
		boost::property_tree::ptree pTree;
		DBGraph graph;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, graph);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!getDataBase()->updateGraph(graph))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}
	else if (postData != NULL && boost::starts_with(url, "/admin/device/"))
	{
		boost::property_tree::ptree pTree;
		DBDevice dev;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, dev);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!getDataBase()->updateDevice(dev))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}
	else if (postData != NULL && boost::starts_with(url, "/admin/condition/"))
	{
		boost::property_tree::ptree pTree;
		DBCondition cond;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, cond);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!getDataBase()->updateCondition(cond))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}
	else if (postData != NULL && boost::starts_with(url, "/admin/room/"))
	{
		boost::property_tree::ptree pTree;
		DBRoom room;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, room);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!getDataBase()->updateRoom(room))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}
	else if (postData != NULL && boost::starts_with(url, "/admin/rgc/"))
	{
		boost::property_tree::ptree pTree;
		DBRoomGraphCond rgc;

		boost::property_tree::read_json(*postData, pTree);
		readFromPTree(pTree, rgc);
		response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
		if (!getDataBase()->updateRoomGraphCond(rgc))
			httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}
	else if (boost::starts_with(url, "/admin/condition/"))
	{
		boost::regex expression1d("^/admin/condition/(\\d+)/addDate/(\\d{8})$", boost::regex::perl);
		boost::regex expression2d("^/admin/condition/(\\d+)/addDate/(\\d{12})/(\\d{12})$", boost::regex::perl);
		boost::cmatch what;

		if (boost::regex_match(url, what, expression1d))
		{
            int conditionId = boost::lexical_cast<int>(what[1]);
            int dateYYYYMMDD = boost::lexical_cast<int>(what[2]);

            response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
            if (!getDataBase()->addConditionDate(conditionId, dateYYYYMMDD))
                httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
		}
		else if (boost::regex_match(url, what, expression2d))
		{
            int conditionId = boost::lexical_cast<int>(what[1]);
            u_int64_t dateStartYYYYMMDDHHMM = boost::lexical_cast<u_int64_t>(what[2]);
            u_int64_t dateEndYYYYMMDDHHMM = boost::lexical_cast<u_int64_t>(what[3]);

            response = MHD_create_response_from_buffer(2, (void *)"{}", MHD_RESPMEM_PERSISTENT);
            if (!getDataBase()->addConditionDate(conditionId, dateStartYYYYMMDDHHMM, dateEndYYYYMMDDHHMM))
                httpCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
		}
	}
    return response;
}
