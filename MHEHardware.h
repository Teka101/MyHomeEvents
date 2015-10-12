#ifndef MHEHARDWARE_H
#define MHEHARDWARE_H

#include "MHEDevice.h"

class MHEHardware
{
    private:
        int _id;

    public:
        MHEHardware(int id);
        virtual ~MHEHardware();

        virtual MHEDevice *buildObject(DBDevice &dev) = 0;
        virtual operator std::string() const
        {
            std::stringstream ss;

            ss << "MHEHardware["
                << "id=" << _id
                << "]";
            return ss.str();
        }
};

#endif // MHEHARDWARE_H
