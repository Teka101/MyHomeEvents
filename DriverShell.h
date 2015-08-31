#ifndef DRIVERSHELL_H
#define DRIVERSHELL_H

#include "MHEDatabase.h"
#include "MHEHardware.h"

class HardwareShell : public MHEHardware
{
    private:
        DBHarware _hw;

    public:
        HardwareShell(DBHarware &hw);
        ~HardwareShell();

        MHEDevice *buildObject(DBDevice &dev);
};

class DeviceShell : public MHEDevice
{
    private:
        std::string _shellCmd;
        float _temperature;
        float _humidity;
        float _offsetTemperature;
        log4cplus::Logger _log;

        void refreshCache();
        bool checkValidity(float newHumidty, float newTemperature);

    public:
        DeviceShell(DBDevice &dev);
        ~DeviceShell();

        float getTemperature();
        float getHumidity();

        operator std::string() const
        {
            std::stringstream ss;

            ss << "MHEDevice#DeviceShell["
                << "id=" << _id
                << " type=" << _type
                << " shellCmd=" << _shellCmd
                << " offsetTemperature=" << _offsetTemperature
                << "]";
            return ss.str();
        }
};

#endif // DRIVERSHELL_H
