/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MHEDEVICE_H
#define MHEDEVICE_H

#include "MHEDatabase.h"

struct sMHEDeviceValue
{
    time_t date;
    float temperature;
    float humdity;
    std::string status;
};
typedef std::vector<sMHEDeviceValue> tMHEDeviceValues;

struct sMHEDeviceVolume
{
    int currentVol;
    int minVol;
    int maxVol;
};


class MHEDevice
{
    protected:
        int _id;
        DBDeviceType _type;
        std::string _name;
        int _cacheLifetime;
        bool _hidden;
        bool _cacheRunning;
        MHEDevice *_cloneTo;
        time_t _lastUpdate;

    public:
        MHEDevice(int id, DBDeviceType type, std::string &name, int cacheLifetime, bool hidden, bool cacheRunning);
        virtual ~MHEDevice();

        int getId() const;
        DBDeviceType getType() const;
        std::string getName() const;
        time_t getLastUpdate() const;
        bool isHidden() const;
        bool isCacheRunning() const;

        virtual void setCloneTo(MHEDevice *cloneTo);
        virtual float getCachedRawTemperature();
        virtual float getCachedTemperature();
        virtual float getCachedHumidity();
        virtual bool isCachedDimmable();
        virtual int getCachedDimmableValue();
        virtual int getCachedDimmableMax();
        virtual bool isCachedActivated();
        virtual float getRawTemperature();
        virtual float getTemperature();
        virtual float getHumidity();
        virtual bool setTempHum(float temperature, float humidity);
        virtual bool isDimmable();
        virtual int getDimmableValue();
        virtual int getDimmableMax();
        virtual bool isActivated();
        virtual bool setStatus(bool activate);
        virtual bool sendCommand(const std::string &command, const std::string &value, void *output);
        virtual operator std::string() const
        {
            std::stringstream ss;

            ss << "MHEDevice["
                << "id=" << _id
                << " type=" << _type
                << "]";
            return ss.str();
        }
};

#endif // MHEDEVICE_H
