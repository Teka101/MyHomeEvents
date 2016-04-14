/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DRIVERSHELL_H
#define DRIVERSHELL_H

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

        float getCachedTemperature();
        float getCachedHumidity();
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
