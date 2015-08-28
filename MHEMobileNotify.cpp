#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include "CurlHelpers.h"
#include "MHEMobileNotify.h"

MHEMobileNotify::MHEMobileNotify(std::string gcmAppId) : _gcmAppId(gcmAppId)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHEMobileNotify"));
}

MHEMobileNotify::~MHEMobileNotify()
{
}

void MHEMobileNotify::notifyDevices(std::vector<DBMobile> &devices, MHEMobileNotifyType type, DBCondition &condition)
{
    if (devices.size() == 0)
        return;
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("notifyDevices - devices[]=" << devices.size()));
    BOOST_FOREACH(DBMobile &device, devices)
    {
        notityDevice(device, type, condition);
    }
}

void MHEMobileNotify::notityDevice(DBMobile &device, MHEMobileNotifyType type, DBCondition &condition)
{
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("notityDevice - device=" << (std::string)device << " type=" << type << " condition=" << (std::string)condition));
    if (device.type == "gcm")
    {
        std::stringstream ss, ssOut;
        std::string postData;
        std::string url = "https://android.googleapis.com/gcm/send";

        ss << "{"
            << "to: \"" << device.token << "\","
            << "data: {"
            << "conditionType: \"" << (type == conditionEnter ? "conditionEnter" : "conditionLeave") << "\","
            << "conditionName: \"" << condition.name << "\""
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
}
