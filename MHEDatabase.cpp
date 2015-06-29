#include <cfloat>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <math.h>
#include "MHEDatabase.h"

MHEDatabase::MHEDatabase() : _db(NULL)
{
    const char *fileName = "params.db";
	bool isExistingDB;
	int rc;

	_log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHEDatabase"));
	isExistingDB = boost::filesystem::exists(fileName);
	LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("MHEDatabase::MHEDatabase - Opening database..."));
	rc = sqlite3_open(fileName, &_db);
	if (rc != 0)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::MHEDatabase - sqlite3_open() failed: " << sqlite3_errmsg(_db)));
	else
	{
		this->createTables();
		if (!isExistingDB)
            this->insertDefaultData();
	}
}

MHEDatabase::~MHEDatabase()
{
    if (_db != NULL)
		sqlite3_close(_db);
}

void MHEDatabase::createTables()
{
	const char sqls[][512] =
	{
            "CREATE TABLE IF NOT EXISTS hardware(type TEXT INTEGER NOT NULL, name TEXT NOT NULL, param1 TEXT)",
            "CREATE TABLE IF NOT EXISTS device(type INTEGER NOT NULL, name TEXT NOT NULL, hardware_id INTEGER NOT NULL REFERENCES hardware(rowid), cache_lifetime INTEGER NOT NULL, param1 TEXT)",

			"CREATE TABLE IF NOT EXISTS graph(description TEXT, position INTEGER DEFAULT 99)",
			"CREATE TABLE IF NOT EXISTS graph_data(graph_id INTEGER REFERENCES graph(rowid), time INTEGER NOT NULL, value REAL)",

			"CREATE TABLE IF NOT EXISTS condition(name TEXT, device_id INTEGER REFERENCES device(rowid), temperature_min REAL, temperature_max REAL, days INTEGER, use_calendar INTEGER NOT NULL DEFAULT 0)",
			"CREATE TABLE IF NOT EXISTS condition_calendar(condition_id INTEGER REFERENCES condition(rowid), date INTEGER)",

            "CREATE TABLE IF NOT EXISTS room(name TEXT NOT NULL, device_temperature_id INTEGER NOT NULL REFERENCES device(rowid), device_heater_id INTEGER REFERENCES device(rowid), device_heating_id INTEGER REFERENCES device(rowid))",
			"CREATE TABLE IF NOT EXISTS room_graphcond(room_id INTEGER NOT NULL REFERENCES room(rowid), graph_id INTEGER NOT NULL REFERENCES graph(rowid), condition_id INTEGER NOT NULL REFERENCES condition(rowid))",
	};
	int nbElements = (sizeof(sqls) / sizeof(*sqls));

	for (int i = 0; i < nbElements; i++)
	{
		char *zErrMsg = NULL;
		int rc;

		rc = sqlite3_exec(_db, sqls[i], NULL, NULL, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::createTables - SQL error: " << zErrMsg << " ### " << sqls[i]));
			sqlite3_free(zErrMsg);
		}
		else
			LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("MHEDatabase::createTables - table created successfully"));
	}
}

void MHEDatabase::insertDefaultData()
{
	const char sqls[][256] =
	{
            "INSERT INTO hardware(type,name,param1) VALUES('domoticz','MyDomoticz','http://127.0.0.1:8080/')",
            "INSERT INTO device(type,name,hardware_id,cache_lifetime,param1) VALUES(0,'TempIn',1,300,8),(0,'TempOut',1,300,5),(0,'TempHeating',1,300,6),(1,'Heater',1,300,7)",
            "INSERT INTO hardware(type,name) VALUES('shell','Scripts shell')",
            "INSERT INTO device(type,name,hardware_id,cache_lifetime,param1) VALUES(0,'DHT22',2,240,'echo \"Humidity = 50 % Temperature = 19.5 *C\"')",

            "INSERT INTO graph(description,position) VALUES('Default week', 0)",
			"INSERT INTO graph_data(graph_id,time,value) VALUES(1,0000,19),(1,0600,19.5),(1,1900,20),(1,2100,19)",
			"INSERT INTO graph(description,position) VALUES('Default week-end',1)",
			"INSERT INTO graph_data(graph_id,time,value) VALUES(2,0000,19),(2,1000,19.5),(2,1900,20),(2,2100,19)",
			"INSERT INTO graph(description,position) VALUES('Hot week',2)",
			"INSERT INTO graph_data(graph_id,time,value) VALUES(3,0000,18),(3,0600,18.5),(3,1900,19),(3,2100,18)",
			"INSERT INTO graph(description,position) VALUES('Hot week-end',3)",
			"INSERT INTO graph_data(graph_id,time,value) VALUES(4,0000,18),(4,1000,18.5),(4,1900,19),(4,2100,18)",
			"INSERT INTO graph(description, position) VALUES('Absence short',98)",
			"INSERT INTO graph_data(graph_id,time,value) VALUES(5,0000,18)",
			"INSERT INTO graph(description,position) VALUES('Absence long',99)",
			"INSERT INTO graph_data(graph_id,time,value) VALUES(6,0000,12)",

			"INSERT INTO condition(name,days) VALUES('Monday to friday',124),('Saturday to sunday',3)",
			"INSERT INTO condition(name,device_id,temperature_min,days) VALUES('Monday to friday - hot',2,14,124),('Saturday to sunday - hot',2,14,3)",
			"INSERT INTO condition(name,use_calendar) VALUES('Absence less than 1 week',1),('Absence more than 1 week',1)",
			"INSERT INTO condition_calendar(condition_id,date) VALUES(6, 20150605),(6, 20150606),(6, 20150623)",

            "INSERT INTO room(name,device_temperature_id,device_heater_id,device_heating_id) VALUES('Home',1,4,3)",
			"INSERT INTO room_graphcond(room_id,graph_id,condition_id) VALUES(1,1,1),(1,2,2),(1,3,3),(1,4,4),(1,5,5),(1,6,6)",
	};
	int nbElements = (sizeof(sqls) / sizeof(*sqls));

	for (int i = 0; i < nbElements; i++)
	{
		char *zErrMsg = NULL;
		int rc;

		rc = sqlite3_exec(_db, sqls[i], NULL, NULL, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::insertDefaultData - SQL error: " << zErrMsg << " ### " << sqls[i]));
			sqlite3_free(zErrMsg);
		}
		else
			LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("MHEDatabase::createTables - data inserted successfully"));
	}
}

static int selectHardware(void *param, int ac, char **av, char **column)
{
	std::vector<DBHarware> *r = static_cast<std::vector<DBHarware>*>(param);

	if (ac == 4)
	{
		DBHarware hw;

        hw.id = boost::lexical_cast<int>(av[0]);
        hw.name = av[1];
		hw.type = av[2];
		if (av[3] != NULL)
            hw.param1 = av[3];
		r->push_back(hw);
	}
	return 0;
}

std::vector<DBHarware> MHEDatabase::getHardwares()
{
    std::vector<DBHarware> ret;
	int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,name,type,param1 FROM hardware ORDER BY rowid", selectHardware, &ret, NULL);
	if (rc != SQLITE_OK)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getHardwares - SQL error: " << sqlite3_errmsg(_db)));
	return ret;
}

static int selectDevice(void *param, int ac, char **av, char **column)
{
	std::vector<DBDevice> *r = static_cast<std::vector<DBDevice>*>(param);

	if (ac == 6)
	{
		DBDevice dev;

        dev.id = boost::lexical_cast<int>(av[0]);
        dev.type = static_cast<DBDeviceType>(boost::lexical_cast<int>(av[1]));
        dev.name = av[2];
        dev.hardwareId = boost::lexical_cast<int>(av[3]);
        dev.cacheLifetime = boost::lexical_cast<int>(av[4]);
		dev.param1 = av[5];
		r->push_back(dev);
	}
	return 0;
}

std::vector<DBDevice> MHEDatabase::getDevices()
{
    std::vector<DBDevice> ret;
	int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,type,name,hardware_id,cache_lifetime,param1 FROM device ORDER BY rowid", selectDevice, &ret, NULL);
	if (rc != SQLITE_OK)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getDevices - SQL error: " << sqlite3_errmsg(_db)));
	return ret;
}

static int selectGraph(void *param, int ac, char **av, char **column)
{
	std::vector<DBGraph> *r = static_cast<std::vector<DBGraph>*>(param);

	if (ac == 3)
	{
		DBGraph g;

        g.id = boost::lexical_cast<int>(av[0]);
        g.description = av[1];
		g.position= boost::lexical_cast<int>(av[2]);
		r->push_back(g);
	}
	return 0;
}

static int selectGraphData(void *param, int ac, char **av, char **column)
{
	std::vector<DBGraphData> *r = static_cast<std::vector<DBGraphData>*>(param);

	if (ac == 2)
	{
		DBGraphData data;

		data.time = boost::lexical_cast<int>(av[0]);
		data.value = boost::lexical_cast<float>(av[1]);
		r->push_back(data);
	}
	return 0;
}

std::vector<DBGraph> MHEDatabase::getGraphs()
{
    std::vector<DBGraph> ret;
    int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,description,position FROM graph ORDER BY position,rowid", selectGraph, &ret, NULL);
	if (rc == SQLITE_OK)
	{
        BOOST_FOREACH(DBGraph &graph, ret)
        {
            std::stringstream ss;

            ss << "SELECT time,value FROM graph_data WHERE graph_id=" << graph.id;
            rc = sqlite3_exec(_db, ss.str().c_str(), selectGraphData, &graph.data, NULL);
            if (rc != SQLITE_OK)
                LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getGraphs - SQL error graph_data: " << sqlite3_errmsg(_db)));
        }
	}
	else
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getGraphs - SQL error: " << sqlite3_errmsg(_db)));
    return ret;
}

static int selectCondition(void *param, int ac, char **av, char **column)
{
	std::vector<DBCondition> *r = static_cast<std::vector<DBCondition>*>(param);

	if (ac == 7)
	{
		DBCondition c;

        c.id = boost::lexical_cast<int>(av[0]);
        c.name = av[1];
        c.deviceId = (av[2] == NULL ? -1 : boost::lexical_cast<int>(av[2]));
        c.temperatureMin = (av[3] == NULL ? FLT_MIN : boost::lexical_cast<float>(av[3]));
        c.temperatureMax = (av[4] == NULL ? FLT_MAX : boost::lexical_cast<float>(av[4]));
        c.days = (av[5] == NULL ? -1 : boost::lexical_cast<int>(av[5]));
        c.useCalendar = boost::lexical_cast<int>(av[6]) != 0;
		r->push_back(c);
	}
	return 0;
}

std::vector<DBCondition> MHEDatabase::getConditions()
{
    std::vector<DBCondition> ret;
	int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,name,device_id,temperature_min,temperature_max,days,use_calendar FROM condition ORDER BY rowid", selectCondition, &ret, NULL);
	if (rc != SQLITE_OK)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getConditions - SQL error: " << sqlite3_errmsg(_db)));
	return ret;
}

static int selectRoom(void *param, int ac, char **av, char **column)
{
	std::vector<DBRoom> *r = static_cast<std::vector<DBRoom>*>(param);

	if (ac == 5)
	{
		DBRoom ro;

        ro.id = boost::lexical_cast<int>(av[0]);
        ro.name = av[1];
        ro.deviceTemperatureId = (av[2] == NULL ? -1 : boost::lexical_cast<int>(av[2]));
        ro.deviceHeaterId = (av[3] == NULL ? -1 : boost::lexical_cast<int>(av[3]));
        ro.deviceHeatingId = (av[4] == NULL ? -1 : boost::lexical_cast<int>(av[4]));
		r->push_back(ro);
	}
	return 0;
}

std::vector<DBRoom> MHEDatabase::getRooms()
{
    std::vector<DBRoom> ret;
	int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,name,device_temperature_id,device_heater_id,device_heating_id FROM room ORDER BY rowid", selectRoom, &ret, NULL);
	if (rc != SQLITE_OK)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getRooms - SQL error: " << sqlite3_errmsg(_db)));
	return ret;
}

std::vector<DBRoomGraphCond> MHEDatabase::getRoomGraphCondByActiveDaysAndCalendar()
{
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	boost::gregorian::date currentDate = now.date();
	int dateYYYYMMDD = (int)currentDate.year() * 10000 + (int)currentDate.month() * 100 + (int)currentDate.day();
	int weekDayStartMonday = ((int)currentDate.day_of_week() - 1 + 7) % 7;
	int dayMask = pow(2, weekDayStartMonday);

	return getRoomGraphCondByActiveDaysAndCalendar(dayMask, dateYYYYMMDD);
}

static int selectRoomGraphCond(void *param, int ac, char **av, char **column)
{
	std::vector<DBRoomGraphCond> *r = static_cast<std::vector<DBRoomGraphCond>*>(param);

	if (ac == 4)
	{
		DBRoomGraphCond rgc;

        rgc.id = boost::lexical_cast<int>(av[0]);
        rgc.roomId = boost::lexical_cast<int>(av[1]);
        rgc.graphId = boost::lexical_cast<int>(av[2]);
        rgc.conditionId = boost::lexical_cast<int>(av[3]);
		r->push_back(rgc);
	}
	return 0;
}

std::vector<DBRoomGraphCond> MHEDatabase::getRoomGraphCondByActiveDaysAndCalendar(int dayMask, int dateYYYYMMDD)
{
    std::vector<DBRoomGraphCond> ret;
    std::stringstream ss;
    int rc;

    ss << "SELECT rgc.rowid,rgc.room_id,rgc.graph_id,rgc.condition_id FROM room_graphcond rgc,condition c,condition_calendar cc WHERE rgc.condition_id=c.rowid AND (c.days IS NULL OR (c.days&" << dayMask << ")=" << dayMask << ") AND c.use_calendar=1 AND c.rowid=cc.condition_id AND cc.date=" << dateYYYYMMDD;
    ss << " UNION SELECT rgc.rowid,rgc.room_id,rgc.graph_id,rgc.condition_id FROM room_graphcond rgc,condition c WHERE rgc.condition_id=c.rowid AND (c.days IS NULL OR (c.days&" << dayMask << ")=" << dayMask << ") AND c.use_calendar=0";
    rc = sqlite3_exec(_db, ss.str().c_str(), selectRoomGraphCond, &ret, NULL);
    if (rc != SQLITE_OK)
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getRoomGraphCondByActiveDaysAndCalendar - SQL error: " << sqlite3_errmsg(_db)));
    return ret;
}
