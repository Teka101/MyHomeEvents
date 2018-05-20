/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
	bool dimmable;
	int dimmableValue, dimmableMax;

	CacheDeviceDomoticz() : name(""), statusIsOn(false), temperature(0), temperatureRaw(0), humidity(0), dimmable(false), dimmableValue(0), dimmableMax(0)
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
                << " dimmable=" << dimmable
                << " dimmableValue=" << dimmableValue
                << " dimmableMax=" << dimmableMax
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
        bool isCachedDimmable();
        int getCachedDimmableValue();
        int getCachedDimmableMax();
        bool isCachedActivated();
        float getRawTemperature();
        float getTemperature();
        float getHumidity();
        bool setTempHum(float temperature, float humidity);
        bool isDimmable();
        int getDimmableValue();
        int getDimmableMax();
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
