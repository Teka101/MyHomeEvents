/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <log4cplus/loggingmacros.h>
#include "CurlHelpers.h"
#include "MHEMobileNotify.h"

MHEMobileNotify::MHEMobileNotify(std::string gcmAppId, MHEDatabase *db, SpeechRecognize *sr) : _gcmAppId(gcmAppId), _db(db), _sr(sr)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHEMobileNotify"));
}

MHEMobileNotify::~MHEMobileNotify()
{
}

void MHEMobileNotify::notifyDevices(const std::string &event, const std::string &type, const std::string &msg)
{
    std::vector<DBMobile> devices = _db->getMobileActivatedToNotify(event, -1);

    if (devices.size() > 0)
        notifyDevices(devices, type, msg);
}

void MHEMobileNotify::notifyDevices(MHEMobileNotifyType type, DBCondition &condition)
{
    std::vector<DBMobile> devices = _db->getMobileActivatedToNotify("condition", condition.id);

    if (devices.size() > 0)
    {
        std::string typeStr = (type == conditionEnter ? "conditionEnter" : "conditionLeave");
        std::string msg;

        if (_sr != NULL)
        {
            std::stringstream ssMsg1, ssMsg2;

            ssMsg1 << "condition_" << typeStr << "_" << condition.id;
            ssMsg2 << "condition_" << typeStr;
            msg = _sr->getResponse(ssMsg1.str(), ssMsg2.str());
            boost::replace_all(msg, "%name%", condition.name);
        }
        else
            msg = typeStr;
        notifyDevices(devices, typeStr, msg);
    }
}

void MHEMobileNotify::notifyDevices(MHEMobileNotifyType type, MHEDevice &device)
{
    std::vector<DBMobile> devices = _db->getMobileActivatedToNotify("device", device.getId());

    if (devices.size() > 0)
    {
        std::string typeStr = (type == conditionEnter ? "deviceActivated" : "deviceDesactivated");
        std::string msg;

        if (_sr != NULL)
        {
            std::stringstream ssMsg1, ssMsg2;

            ssMsg1 << "device_" << typeStr << "_" << device.getId();
            ssMsg2 << "device_" << typeStr;
            msg = _sr->getResponse(ssMsg1.str(), ssMsg2.str());
            boost::replace_all(msg, "%name%", device.getName());
        }
        else
            msg = typeStr;
        notifyDevices(devices, typeStr, msg);
    }
}

void MHEMobileNotify::notifyDevices(std::vector<DBMobile> &devices, const std::string &type, const std::string &msg)
{
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("notifyDevices - devices[]=" << devices.size() << " type=" << type << " msg=" << msg));
    BOOST_FOREACH(DBMobile &device, devices)
    {
        notifyDevice(device, type, msg);
    }
}

void MHEMobileNotify::notifyDevice(DBMobile &device, const std::string &type, const std::string &msg)
{
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("notityDevice - device=" << (std::string)device << " type=" << type << " msg=" << msg));
    if (device.type == "gcm")
    {
        std::vector<std::string> headers;
        std::stringstream ss, ssOut;
        std::string postData;
        std::string url = "https://android.googleapis.com/gcm/send";
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        boost::gregorian::date currentDate = now.date();
        u_int64_t dateYYYYMMDD = currentDate.year() * 10000 + currentDate.month() * 100 + currentDate.day();
        u_int64_t currentHHMMM = now.time_of_day().hours() * 100 + now.time_of_day().minutes();
	u_int64_t currentYYYYMMDDHHMMM = dateYYYYMMDD * (u_int64_t)10000 + currentHHMMM;

        ss << "Authorization: key=" << _gcmAppId;
        headers.push_back(ss.str());
        headers.push_back("Content-Type: application/json");
        ss.str(std::string());
        ss << "{"
                << "\"to\": \"" << device.token << "\","
                << "\"delay_while_idle\": false,"
                << "\"time_to_live\": 86400,"
                << "\"data\": {"
                    << "\"type\": \"" << type << "\","
                    << "\"datetime\": \"" << currentYYYYMMDDHHMMM << "\","
                    << "\"msg\": \"" << msg << "\""
                << "}"
            << "}";
        postData = ss.str();
        if (curlExecute(url, &headers, &postData, &ssOut))
        {
            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("notityDevice - success post=" << ss.str() << " return: " << ssOut.str()));
            device.lastSuccessYYYYMMDDHHSS = currentYYYYMMDDHHMMM;
            _db->updateMobileLastSuccess(device);
        }
        else
            LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("notityDevice - failed post=" << ss.str() << " result=" << ssOut.str()));
    }
    else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("notityDevice - unkown type of device - " << (std::string)device));
}
