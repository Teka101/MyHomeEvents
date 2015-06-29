#ifndef DRIVERDOMOTICZ_H
#define DRIVERDOMOTICZ_H

#include <log4cplus/logger.h>
#include "MHEDatabase.h"
#include "MHEHardware.h"

class HardwareDomoticz : public MHEHardware
{
    private:
        DBHarware _hw;

    public:
        HardwareDomoticz(DBHarware &hw);
        ~HardwareDomoticz();

        MHEDevice *buildObject(DBDevice &dev);
};

struct CacheDeviceDomoticz
{
	std::string name;
	time_t lastUpdate;
	bool statusIsOn;
	float temperature;
	float humidity;

	CacheDeviceDomoticz() : lastUpdate(0), statusIsOn(false), temperature(0), humidity(0)
	{
	}

	operator std::string() const
        {
            std::stringstream ss;

            ss << "CacheDeviceDomoticz["
                << "name=" << name
                << " lastUpdate=" << lastUpdate
                << " statusIsOn=" << statusIsOn
                << " temperature=" << temperature
                << " humidity=" << humidity
                << "]";
            return ss.str();
        }
};

class DeviceDomoticz : public MHEDevice
{
    private:
        std::string _urlDomoticz;
        std::string _deviceIdx;
        CacheDeviceDomoticz _cache;
        log4cplus::Logger _log;

        void refreshCache();

    public:
        DeviceDomoticz(std::string &urlDomoticz, DBDevice &dev);
        ~DeviceDomoticz();

        float getTemperature();
        float getHumidity();
        bool setTempHum(float temperature, float humidity);
        bool isActivated();
        bool setStatus(bool activate);

        operator std::string() const
        {
            std::stringstream ss;

            ss << "MHEDevice#DeviceDomoticz["
                << "id=" << _id
                << " type=" << _type
                << " deviceIdx=" << _deviceIdx
                << "]";
            return ss.str();
        }
};

#endif // DRIVERDOMOTICZ_H
