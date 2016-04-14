/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DRIVERPHILIPSTV_H
#define DRIVERPHILIPSTV_H

#include <map>
#include "MHEHardware.h"

class HardwarePhilipsTV : public MHEHardware
{
    private:
        DBHarware _hw;

    public:
        HardwarePhilipsTV(DBHarware &hw);
        ~HardwarePhilipsTV();

        MHEDevice *buildObject(DBDevice &dev);
};

class DevicePhilipsTV : public MHEDevice
{
    private:
        std::map<std::string,std::string> _channelCodeToChannelId;
        std::string _ip;
        bool _lastStatus;
        log4cplus::Logger _log;

        bool sendKey(const std::string &key);
        bool sendNewChannel(const std::string &channelId);
        bool getVolume(int &volume, int &minVol, int &maxVol);
        bool setVolume(const std::string &volume);

    public:
        DevicePhilipsTV(DBDevice &dev);
        ~DevicePhilipsTV();

        bool isCachedActivated();
        bool isActivated();
        bool setStatus(bool activate);
        bool sendCommand(const std::string &command, const std::string &value, void *output);

        operator std::string() const
        {
            std::stringstream ss;

            ss << "MHEDevice#DevicePhilipsTV["
                << "id=" << _id
                << " type=" << _type
                << " ip=" << _ip
                << "]";
            return ss.str();
        }
};

#endif // DRIVERPHILIPSTV_H
