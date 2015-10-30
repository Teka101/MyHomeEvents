#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <time.h>
#include "CurlHelpers.h"
#include "DriverPhilipsTV.h"

HardwarePhilipsTV::HardwarePhilipsTV(DBHarware &hw) : MHEHardware(hw.id), _hw(hw)
{
}

HardwarePhilipsTV::~HardwarePhilipsTV()
{
}

MHEDevice *HardwarePhilipsTV::buildObject(DBDevice &dev)
{
    return new DevicePhilipsTV(dev);
}

DevicePhilipsTV::DevicePhilipsTV(DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.name, dev.cacheLifetime, dev.hidden), _ip(dev.param1), _lastStatus(false)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DevicePhilipsTV"));
}

DevicePhilipsTV::~DevicePhilipsTV()
{
}

bool DevicePhilipsTV::isActivated()
{
    time_t now = time(NULL) - _cacheLifetime;

    if (_lastUpdate < now)
    {
        std::stringstream ssUrl;
        bool status = false;

        ssUrl << "http://" << _ip << ":1925/1/channels/current";
        if (curlExecute(ssUrl.str()))
            status = true;
        _lastUpdate = time(NULL);
        _lastStatus = status;
        return status;
    }
    return _lastStatus;
}

bool DevicePhilipsTV::setStatus(bool activate)
{
    if (!activate)
        return sendKey("Standby");
    return false;
}

bool DevicePhilipsTV::sendCommand(const std::string &command, const std::string &value, void *output)
{
    if (command == "channel")
        return sendNewChannel(value);
    else if (command == "volume" && output == NULL)
        return setVolume(value);
    else if (command == "volume" && output != NULL)
    {
        sMHEDeviceVolume *data = static_cast<sMHEDeviceVolume*>(output);

        return getVolume(data->currentVol, data->minVol, data->maxVol);
    }
    return false;
}

bool DevicePhilipsTV::sendKey(const std::string &key)
{
    std::stringstream ssUrl, ssPost;
    std::string postData;

    ssUrl << "http://" << _ip << ":1925/1/input/key";
    ssPost << "{\"key\": \""<< key << "\"}";
    postData = ssPost.str();
    if (curlExecute(ssUrl.str(), NULL, &postData))
    {
        time_t now = time(NULL);

        _lastUpdate = now;
        _lastStatus = true;
        return true;
    }
    return false;
}

bool DevicePhilipsTV::sendNewChannel(const std::string &channelId)
{
    std::stringstream ssUrl, ssPost;
    std::string postData;

    ssUrl << "http://" << _ip << ":1925/1/channels/current";
    ssPost << "{\"id\": \""<< channelId << "\"}";
    postData = ssPost.str();
    if (curlExecute(ssUrl.str(), NULL, &postData))
    {
        time_t now = time(NULL);

        _lastUpdate = now;
        _lastStatus = true;
        return true;
    }
    return false;
}

bool DevicePhilipsTV::setVolume(const std::string &volume)
{
    std::stringstream ssUrl, ssPost;
    std::string postData;

    ssUrl << "http://" << _ip << ":1925/1/audio/volume";
    ssPost << "{\"muted\": false, \"current\":"<< volume << "}";
    postData = ssPost.str();
    if (curlExecute(ssUrl.str(), NULL, &postData))
    {
        time_t now = time(NULL);

        _lastUpdate = now;
        _lastStatus = true;
        return true;
    }
    return false;
}

bool DevicePhilipsTV::getVolume(int &volume, int &minVol, int &maxVol)
{
    std::stringstream ssUrl, ssOut;

    ssUrl << "http://" << _ip << ":1925/1/audio/volume";
    if (curlExecute(ssUrl.str(), &ssOut))
    {
        boost::property_tree::ptree pTree;
        time_t now = time(NULL);

        boost::property_tree::read_json(ssOut, pTree);
        volume = pTree.get<int>("current", 0);
        minVol = pTree.get<int>("min", 0);
        maxVol = pTree.get<int>("max", 0);
        _lastUpdate = now;
        _lastStatus = true;
        return true;
    }
    return false;
}
