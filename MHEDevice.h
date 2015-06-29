#ifndef MHEDEVICE_H
#define MHEDEVICE_H

#include "MHEDatabase.h"

class MHEDevice
{
    protected:
        int _id;
        DBDeviceType _type;
        int _cacheLifetime;

    public:
        MHEDevice(int id, DBDeviceType type, int cacheLifetime);
        virtual ~MHEDevice();

        virtual float getTemperature();
        virtual float getHumidity();
        virtual bool setTempHum(float temperature, float humidity);
        virtual bool isActivated();
        virtual bool setStatus(bool activate);
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
