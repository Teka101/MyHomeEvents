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
