#ifndef DRIVERPHILIPSTV_H
#define DRIVERPHILIPSTV_H

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
        std::string _ip;
        bool _lastStatus;
        log4cplus::Logger _log;

        bool sendKey(const std::string &key);

    public:
        DevicePhilipsTV(DBDevice &dev);
        ~DevicePhilipsTV();

        bool isActivated();
        bool setStatus(bool activate);

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
