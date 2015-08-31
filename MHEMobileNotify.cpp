#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include "CurlHelpers.h"
#include "MHEMobileNotify.h"

MHEMobileNotify::MHEMobileNotify(std::string gcmAppId, MHEDatabase *db) : _gcmAppId(gcmAppId), _db(db)
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
        std::string msg = typeStr;

        notifyDevices(devices, typeStr, msg);
    }
}

void MHEMobileNotify::notifyDevices(MHEMobileNotifyType type, MHEDevice &device)
{
    std::vector<DBMobile> devices = _db->getMobileActivatedToNotify("device", device.getId());

    if (devices.size() > 0)
    {
        std::string typeStr = (type == conditionEnter ? "deviceActivated" : "deviceDesactivated");
        std::string msg = typeStr;

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
        std::stringstream ss, ssOut;
        std::string postData;
        std::string url = "https://android.googleapis.com/gcm/send";

        ss << "{"
                << "to: \"" << device.token << "\","
                << "data: {"
                    << "type: \"" << type << "\","
                    << "msg: \"" << msg << "\""
                << "}"
            << "}";
        postData = ss.str();
        if (curlExecute(url, &postData, &ssOut))
        {
            boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
            boost::gregorian::date currentDate = now.date();
            long dateYYYYMMDD = (long)currentDate.year() * 10000L + (long)currentDate.month() * 100L + (long)currentDate.day();
            long currentHHMMM = now.time_of_day().hours() * 100L + now.time_of_day().minutes();

            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("notityDevice - success post=" << ss.str() << " return: " << ssOut));
            device.lastSuccessYYYYMMDDHHSS = dateYYYYMMDD * 10000 + currentHHMMM;
        }
        else
            LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("notityDevice - failed post=" << ss.str()));
    }
    else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("notityDevice - unkown type of device - " << (std::string)device));
}
