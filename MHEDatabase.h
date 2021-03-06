/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MHEDATABASE_H
#define MHEDATABASE_H

#include <sstream>
#include <log4cplus/logger.h>
#include <sqlite3.h>
#include <vector>

struct DBHarware
{
    int id;
    std::string type;
    std::string name;
    std::string param1;

    operator std::string() const
    {
        std::stringstream ss;

        ss << "DBHarware["
            << "id=" << id
            << " type=" << type
            << " name=" << name
            << " param1=" << param1
            << "]";
        return ss.str();
    }
};

enum DBDeviceType
{
    devTempHum = 0,
    devInterruptor = 1,
    devDimmer = 2,
    devTV = 3
};

struct DBDevice
{
    int id;
    DBDeviceType type;
    std::string name;
    int hardwareId;
    int cacheLifetime;
    std::string param1;
    std::string param2;
    int cloneToDeviceId;
    bool hidden;
    bool cacheRunning;

    operator std::string() const
    {
        std::stringstream ss;

        ss << "DBDevice["
            << "id=" << id
            << " type=" << type
            << " name=" << name
            << " hardwareId=" << hardwareId
            << " cacheLifetime=" << cacheLifetime
            << " param1=" << param1
            << " param2=" << param2
            << " cloneToDeviceId=" << cloneToDeviceId
            << " hidden=" << hidden
            << "]";
        return ss.str();
    }
};

struct DBGraphData
{
	int time;
	float value;
};

struct DBGraph
{
	int id;
	int position;
	std::string description;
	std::vector<DBGraphData> data;

	operator std::string() const
    {
        std::stringstream ss;

        ss << "DBGraph["
            << "id=" << id
            << " position=" << position
            << " description=" << description
            << " data[]=" << data.size()
            << "]";
        return ss.str();
    }
};

struct DBCondition
{
	int id;
	std::string name;
    int deviceId;
    float temperatureMin;
    float temperatureMax;
    int days;
    bool useCalendar;

    operator std::string() const
    {
        std::stringstream ss;

        ss << "DBCondition["
            << "id=" << id
            << " name=" << name
            << " deviceId=" << deviceId
            << " temperatureMin=" << temperatureMin
            << " temperatureMax=" << temperatureMax
            << " days=" << days
            << " useCalendar=" << useCalendar
            << "]";
        return ss.str();
    }
};

struct DBRoom
{
	int id;
	std::string name;
    int deviceTemperatureId;
    int deviceHeaterId;
    int deviceHeatingId;

	operator std::string() const
    {
        std::stringstream ss;

        ss << "DBRoom["
            << "id=" << id
            << " name=" << name
            << " deviceTemperatureId=" << deviceTemperatureId
            << " deviceHeaterId=" << deviceHeaterId
            << " deviceHeatingId=" << deviceHeatingId
            << "]";
        return ss.str();
    }
};

struct DBRoomGraphCond
{
    int id;
    int roomId;
    int graphId;
    int conditionId;

    operator std::string() const
    {
        std::stringstream ss;

        ss << "DBRoomGraphCond["
            << "id=" << id
            << " roomId=" << roomId
            << " graphId=" << graphId
            << " conditionId=" << conditionId
            << "]";
        return ss.str();
    }
};

struct DBMobile
{
    int id;
    std::string type;
    std::string user;
    std::string token;
    bool isActivated;
    u_int64_t lastSuccessYYYYMMDDHHSS;

    operator std::string() const
    {
        std::stringstream ss;

        ss << "DBMobile["
            << "id=" << id
            << " type=" << type
            << " user=" << user
            << " token=" << token
            << " isActivated=" << isActivated
            << " lastSuccessYYYYMMDDHHSS=" << lastSuccessYYYYMMDDHHSS
            << "]";
        return ss.str();
    }
};

class MHEDatabase
{
    private:
        log4cplus::Logger _log;
        sqlite3 *_db;

        void createTables();
        void insertDefaultData();

    public:
        MHEDatabase();
        ~MHEDatabase();

        std::vector<DBHarware> getHardwares();
        std::vector<DBDevice> getDevices();
        std::vector<DBGraph> getGraphs();
        std::vector<DBCondition> getConditions();
        std::vector<DBRoom> getRooms();
        std::vector<DBRoomGraphCond> getRoomGraphConds();
        std::vector<DBRoomGraphCond> getRoomGraphCondByActiveDaysAndCalendar();
        std::vector<DBRoomGraphCond> getRoomGraphCondByActiveDaysAndCalendar(int dayMask, u_int64_t dateYYYYMMDDHHMM);
        std::vector<DBMobile> getMobileActivatedToNotify(const std::string &event, int objectId);
        bool addConditionDate(int condId, int dateYYYYMMDD);
        bool addConditionDate(int condId, u_int64_t dateBeginYYYYMMDDHHMM, u_int64_t dateEndYYYYMMDDHHMM);
        bool addMobile(std::string &type, std::string &user, std::string &token);
        bool updateHardware(DBHarware &hard);
        bool updateDevice(DBDevice &dev);
        bool updateGraph(DBGraph &graph);
        bool updateGraphData(DBGraph &graph);
        bool updateCondition(DBCondition &cond);
        bool updateRoom(DBRoom &room);
        bool updateRoomGraphCond(DBRoomGraphCond &rgc);
        bool updateMobileLastSuccess(DBMobile &mob);
	bool deleteDevice(int devId);
};

#endif // MHEDATABASE_H
