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


class MHEDevice
{
    protected:
        int _id;
        DBDeviceType _type;
        std::string _name;
        int _cacheLifetime;
        bool _hidden;
        MHEDevice *_cloneTo;
        time_t _lastUpdate;

    public:
        MHEDevice(int id, DBDeviceType type, std::string &name, int cacheLifetime, bool hidden);
        virtual ~MHEDevice();

        int getId();
        DBDeviceType getType();
        std::string getName();
        time_t getLastUpdate();
        bool isHidden();

        virtual void setCloneTo(MHEDevice *cloneTo);
        virtual float getTemperature();
        virtual float getHumidity();
        virtual bool setTempHum(float temperature, float humidity);
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
