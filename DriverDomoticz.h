#ifndef DRIVERDOMOTICZ_H
#define DRIVERDOMOTICZ_H

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
	bool statusIsOn;
	float temperature, temperatureRaw;
	float humidity;

	CacheDeviceDomoticz() : name(""), statusIsOn(false), temperature(0), temperatureRaw(0), humidity(0)
	{
	}

	operator std::string() const
        {
            std::stringstream ss;

            ss << "CacheDeviceDomoticz["
                << "name=" << name
                << " statusIsOn=" << statusIsOn
                << " temperature=" << temperature
                << " temperatureRaw=" << temperatureRaw
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
        bool _isDayAverage;
        CacheDeviceDomoticz _cache;
        log4cplus::Logger _log;

        bool parseGraphData(const std::string &url, tMHEDeviceValues *values, const std::string &fieldDate, const std::string &dateFormat);
        void refreshCache();
        bool refreshNormal();
        bool refreshDayAverage();

    public:
        DeviceDomoticz(std::string &urlDomoticz, bool isDayAverage, DBDevice &dev);
        ~DeviceDomoticz();

        float getCachedRawTemperature();
        float getCachedTemperature();
        float getCachedHumidity();
        bool isCachedActivated();
        float getRawTemperature();
        float getTemperature();
        float getHumidity();
        bool setTempHum(float temperature, float humidity);
        bool isActivated();
        bool setStatus(bool activate);
        bool sendCommand(const std::string &command, const std::string &value, void *output);

        operator std::string() const
        {
            std::stringstream ss;

            ss << "MHEDevice#DeviceDomoticz["
                << "id=" << _id
                << " type=" << _type
                << " deviceIdx=" << _deviceIdx
                << " isDayAverage=" << _isDayAverage
                << "]";
            return ss.str();
        }
};

#endif // DRIVERDOMOTICZ_H
