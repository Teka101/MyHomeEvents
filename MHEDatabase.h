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
    devInterruptor = 1
};

struct DBDevice
{
    int id;
    DBDeviceType type;
    std::string name;
    int hardwareId;
    int cacheLifetime;
    std::string param1;

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
        std::vector<DBRoomGraphCond> getRoomGraphCondByActiveDaysAndCalendar();
        std::vector<DBRoomGraphCond> getRoomGraphCondByActiveDaysAndCalendar(int dayMask, int dateYYYYMMDD);
};

#endif // MHEDATABASE_H
