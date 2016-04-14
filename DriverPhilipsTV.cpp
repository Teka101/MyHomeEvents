/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <log4cplus/loggingmacros.h>
#include <time.h>
#include "CurlHelpers.h"
#include "DriverPhilipsTV.h"

typedef std::pair<const std::string, boost::property_tree::ptree> ptreePair;

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

DevicePhilipsTV::DevicePhilipsTV(DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.name, dev.cacheLifetime, dev.hidden, dev.cacheRunning), _ip(dev.param1), _lastStatus(false)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DevicePhilipsTV"));
}

DevicePhilipsTV::~DevicePhilipsTV()
{
}

bool DevicePhilipsTV::isCachedActivated()
{
    return _lastStatus;
}

bool DevicePhilipsTV::isActivated()
{
    time_t now = time(NULL) - _cacheLifetime;

    if (_lastUpdate < now)
    {
        std::stringstream ssUrl, ssOut;
        bool status = false;

        ssUrl << "http://" << _ip << ":1925/1/channels";
        if (curlExecute(ssUrl.str(), &ssOut))
        {
            boost::property_tree::ptree pTree;

            boost::property_tree::read_json(ssOut, pTree);
            BOOST_FOREACH(ptreePair &it, pTree)
            {
                std::string channelId = it.first;
                std::string channelCode = it.second.get("preset", "");

                _channelCodeToChannelId[channelCode] = channelId;
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("isActivated - channelCode=" << channelCode << " -> channelId=" << channelId));
            }
            status = true;
        }
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
    std::string philipsChannelId = _channelCodeToChannelId[channelId];

    if (philipsChannelId.length() > 0)
    {
        ssUrl << "http://" << _ip << ":1925/1/channels/current";
        ssPost << "{\"id\": \""<< philipsChannelId << "\"}";
        postData = ssPost.str();
        if (curlExecute(ssUrl.str(), NULL, &postData))
        {
            time_t now = time(NULL);

            _lastUpdate = now;
            _lastStatus = true;
            return true;
        }
    }
    else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("sendNewChannel - unable to find id for channelCode=" << channelId));
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
