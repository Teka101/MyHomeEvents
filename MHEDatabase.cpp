#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <log4cplus/loggingmacros.h>
#include <cmath>
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
			"CREATE TABLE IF NOT EXISTS device(type INTEGER NOT NULL, name TEXT NOT NULL, hardware_id INTEGER NOT NULL REFERENCES hardware(rowid), cache_lifetime INTEGER NOT NULL, param1 TEXT, param2 TEXT, clone_to_device_id INTEGER REFERENCES device(rowid), hidden INTEGER NOT NULL DEFAULT 0, cache_running INTEGER NOT NULL DEFAULT 0)",

			"CREATE TABLE IF NOT EXISTS graph(description TEXT, position INTEGER DEFAULT 99)",
			"CREATE TABLE IF NOT EXISTS graph_data(graph_id INTEGER REFERENCES graph(rowid), time INTEGER NOT NULL, value REAL)",

			"CREATE TABLE IF NOT EXISTS condition(name TEXT, device_id INTEGER REFERENCES device(rowid), temperature_min REAL, temperature_max REAL, days INTEGER, use_calendar INTEGER NOT NULL DEFAULT 0)",
			"CREATE TABLE IF NOT EXISTS condition_calendar(condition_id INTEGER REFERENCES condition(rowid), datetime_begin INTEGER, datetime_end INTEGER)",

			"CREATE TABLE IF NOT EXISTS room(name TEXT NOT NULL, device_temperature_id INTEGER NOT NULL REFERENCES device(rowid), device_heater_id INTEGER REFERENCES device(rowid), device_heating_id INTEGER REFERENCES device(rowid))",
			"CREATE TABLE IF NOT EXISTS room_graphcond(room_id INTEGER NOT NULL REFERENCES room(rowid), graph_id INTEGER NOT NULL REFERENCES graph(rowid), condition_id INTEGER NOT NULL REFERENCES condition(rowid))",

			"CREATE TABLE IF NOT EXISTS mobile(type TEXT NOT NULL, user TEXT NOT NULL, token TEXT NOT NULL, is_enabled INTEGER NOT NULL, datetime_last_success INTEGER, UNIQUE(type,user) ON CONFLICT REPLACE)",
			"CREATE TABLE IF NOT EXISTS mobile_event(mobile_user INTEGER REFERENCES mobile(user), name TEXT NOT NULL, object_id INTEGER NOT NULL)",
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
			"INSERT INTO device(type,name,hardware_id,cache_lifetime,param1,param2) VALUES(0,'TempIn',1,300,'8',NULL),(0,'TempOut',1,300,'5','24havg'),(0,'TempHeating',1,300,'6',NULL),(1,'Heater',1,300,'7',NULL)",
			"INSERT INTO hardware(type,name) VALUES('shell','Scripts shell')",
			"INSERT INTO device(type,name,hardware_id,cache_lifetime,param1,param2,clone_to_device_id) VALUES(0,'DHT22',2,240,'echo \"Humidity = 50 % Temperature = 19.5 *C\"','offset_temperature=-1.0', 1)",
			"INSERT INTO hardware(type,name) VALUES('philipsTV','Philips')",
			"INSERT INTO device(type,name,hardware_id,cache_lifetime,param1) VALUES(3,'TV',3,300,'192.168.0.5')",

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
			"INSERT INTO condition_calendar(condition_id,datetime_begin,datetime_end) VALUES(6, 201506050000, 201506232359)",

			"INSERT INTO room(name,device_temperature_id,device_heater_id,device_heating_id) VALUES('Home',5,4,3)",
			"INSERT INTO room_graphcond(room_id,graph_id,condition_id) VALUES(1,1,1),(1,2,2),(1,3,3),(1,4,4),(1,5,5),(1,6,6)",

			"INSERT INTO mobile(type,user,token,is_enabled) VALUES('gcm','Test','GCMTOKEN',0)",
			"INSERT INTO mobile_event(mobile_user,name,object_id) VALUES('Test','condition',1)",
			"INSERT INTO mobile_event(mobile_user,name,object_id) VALUES('Test','transmission',-1)",
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

	if (ac == 10)
	{
		DBDevice dev;

        dev.id = boost::lexical_cast<int>(av[0]);
        dev.type = static_cast<DBDeviceType>(boost::lexical_cast<int>(av[1]));
        dev.name = av[2];
        dev.hardwareId = boost::lexical_cast<int>(av[3]);
        dev.cacheLifetime = boost::lexical_cast<int>(av[4]);
		dev.param1 = av[5];
		if (av[6] != NULL)
            dev.param2 = av[6];
		dev.cloneToDeviceId = (av[7] == NULL ? -1 : boost::lexical_cast<int>(av[7]));
		dev.hidden = boost::lexical_cast<int>(av[8]) == 1;
		dev.cacheRunning = boost::lexical_cast<int>(av[9]) == 1;
		r->push_back(dev);
	}
	return 0;
}

std::vector<DBDevice> MHEDatabase::getDevices()
{
    std::vector<DBDevice> ret;
	int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,type,name,hardware_id,cache_lifetime,param1,param2,clone_to_device_id,hidden,cache_running FROM device ORDER BY rowid", selectDevice, &ret, NULL);
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
        c.temperatureMin = (av[3] == NULL ? NAN : boost::lexical_cast<float>(av[3]));
        c.temperatureMax = (av[4] == NULL ? NAN : boost::lexical_cast<float>(av[4]));
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
	int dayMask = pow(2, 6 - weekDayStartMonday);

    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("MHEDatabase::getRoomGraphCondByActiveDaysAndCalendar - dateYYYYMMDD=" << dateYYYYMMDD << " weekDayStartMonday=" << weekDayStartMonday << " dayMask=" << dayMask));
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

std::vector<DBRoomGraphCond> MHEDatabase::getRoomGraphCondByActiveDaysAndCalendar(int dayMask, int dateYYYYMMDDHHMM)
{
    std::vector<DBRoomGraphCond> ret;
    std::stringstream ss;
    int rc;

    ss << "SELECT rgc.rowid,rgc.room_id,rgc.graph_id,rgc.condition_id FROM room_graphcond rgc,condition c,condition_calendar cc WHERE rgc.condition_id=c.rowid AND (c.days IS NULL OR (c.days&" << dayMask << ")=" << dayMask
        << ") AND c.use_calendar=1 AND c.rowid=cc.condition_id AND " << dateYYYYMMDDHHMM << " BETWEEN cc.datetime_begin AND cc.datetime_end";
    ss << " UNION SELECT rgc.rowid,rgc.room_id,rgc.graph_id,rgc.condition_id FROM room_graphcond rgc,condition c WHERE rgc.condition_id=c.rowid AND (c.days IS NULL OR (c.days&" << dayMask << ")=" << dayMask << ") AND c.use_calendar=0";
    rc = sqlite3_exec(_db, ss.str().c_str(), selectRoomGraphCond, &ret, NULL);
    if (rc != SQLITE_OK)
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getRoomGraphCondByActiveDaysAndCalendar - SQL error: " << sqlite3_errmsg(_db)));
    return ret;
}

std::vector<DBRoomGraphCond> MHEDatabase::getRoomGraphConds()
{
    std::vector<DBRoomGraphCond> ret;
	int rc;

	rc = sqlite3_exec(_db, "SELECT rowid,room_id,graph_id,condition_id FROM room_graphcond ORDER BY rowid", selectRoomGraphCond, &ret, NULL);
	if (rc != SQLITE_OK)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getRoomGraphConds - SQL error: " << sqlite3_errmsg(_db)));
	return ret;
}

std::vector<DBMobile> MHEDatabase::getMobileActivatedToNotify(const std::string &event, int objectId)
{
    std::vector<DBMobile> ret;
    sqlite3_stmt *stmt = NULL;
    const std::string sql = "SELECT DISTINCT m.rowid,m.type,m.user,m.token,m.is_enabled,m.datetime_last_success FROM mobile m,mobile_event me"
                            " WHERE m.is_enabled=1 AND m.user=me.mobile_user AND me.name=? AND (me.object_id=? OR me.object_id=-1)"
                            " ORDER BY m.rowid";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
        sqlite3_bind_text(stmt, 1, event.c_str(), event.size(), NULL);
		sqlite3_bind_int(stmt, 2, objectId);
		rc = sqlite3_step(stmt);
		while (rc == SQLITE_ROW)
		{
            DBMobile m;

            m.id = sqlite3_column_int(stmt, 0);
            m.type = (char *)sqlite3_column_text(stmt, 1);
            m.user= (char *)sqlite3_column_text(stmt, 2);
            m.token= (char *)sqlite3_column_text(stmt, 3);
            m.isActivated = (sqlite3_column_int(stmt, 4) == 1);
            m.lastSuccessYYYYMMDDHHSS = sqlite3_column_int64(stmt, 5);
            ret.push_back(m);
            rc = sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}
	else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::getMobileActivatedToNotify - SQL error: " << sqlite3_errmsg(_db)));
	return ret;
}

bool MHEDatabase::updateHardware(DBHarware &hard)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE hardware SET name=?,type=?,param1=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, hard.name.c_str(), hard.name.length(), NULL);
		sqlite3_bind_text(stmt, 2, hard.type.c_str(), hard.type.length(), NULL);
		sqlite3_bind_text(stmt, 3, hard.param1.c_str(), hard.param1.length(), NULL);
		sqlite3_bind_int(stmt, 4, hard.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateHardware - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::updateDevice(DBDevice &dev)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE device SET type=?,name=?,cache_lifetime=?,param1=?,param2=?,clone_to_device_id=?,hidden=?,cache_running=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
        sqlite3_bind_int(stmt, 1, dev.type);
		sqlite3_bind_text(stmt, 2, dev.name.c_str(), dev.name.length(), NULL);
		sqlite3_bind_int(stmt, 3, dev.cacheLifetime);
		if (dev.param1.length() == 0)
            sqlite3_bind_null(stmt, 4);
        else
            sqlite3_bind_text(stmt, 4, dev.param1.c_str(), dev.param1.length(), NULL);
        if (dev.param2.length() == 0)
            sqlite3_bind_null(stmt, 5);
        else
            sqlite3_bind_text(stmt, 5, dev.param2.c_str(), dev.param2.length(), NULL);
        if (dev.cloneToDeviceId == -1)
            sqlite3_bind_null(stmt, 6);
        else
            sqlite3_bind_int(stmt, 6, dev.cloneToDeviceId);
        sqlite3_bind_int(stmt, 7, dev.hidden ? 1 : 0);
        sqlite3_bind_int(stmt, 8, dev.cacheRunning ? 1 : 0);
		sqlite3_bind_int(stmt, 9, dev.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateDevice - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::updateGraph(DBGraph &graph)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE graph SET position=?,description=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, graph.position);
		sqlite3_bind_text(stmt, 2, graph.description.c_str(), graph.description.length(), NULL);
		sqlite3_bind_int(stmt, 3, graph.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return updateGraphData(graph);
	}
	if (rc != SQLITE_OK && rc != SQLITE_DONE)
		LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateGraph - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::updateGraphData(DBGraph &graph)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sqlDelete = "DELETE FROM graph_data WHERE graph_id=?";
	const std::string sql = "INSERT INTO graph_data(graph_id, time, value) VALUES(?,?,?)";
	int rc;

	rc = sqlite3_prepare(_db, sqlDelete.c_str(), sqlDelete.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, graph.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
		{
			rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
			if (rc == SQLITE_OK)
			{
				BOOST_FOREACH(DBGraphData &data, graph.data)
				{
					sqlite3_bind_int(stmt, 1, graph.id);
					sqlite3_bind_int(stmt, 2, data.time);
					sqlite3_bind_double(stmt, 3, data.value);
					rc = sqlite3_step(stmt);
					sqlite3_reset(stmt);
				}
				sqlite3_finalize(stmt);
			}
		}
	}
	if (rc != SQLITE_OK && rc != SQLITE_DONE)
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateGraphData - SQL error: " << sqlite3_errmsg(_db)));
	return (rc == SQLITE_OK || rc == SQLITE_DONE);
}

bool MHEDatabase::updateCondition(DBCondition &cond)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE condition SET name=?,device_id=?,temperature_min=?,temperature_max=?,days=?,use_calendar=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, cond.name.c_str(), cond.name.length(), NULL);
		if (cond.deviceId == -1)
            sqlite3_bind_null(stmt, 2);
        else
            sqlite3_bind_int(stmt, 2, cond.deviceId);
		if (!std::isnormal(cond.temperatureMin))
            sqlite3_bind_null(stmt, 3);
        else
            sqlite3_bind_double(stmt, 3, cond.temperatureMin);
        if (!std::isnormal(cond.temperatureMax))
            sqlite3_bind_null(stmt, 4);
        else
            sqlite3_bind_double(stmt, 4, cond.temperatureMax);
        if (cond.days <= 0)
            sqlite3_bind_null(stmt, 5);
        else
            sqlite3_bind_int(stmt, 5, cond.days);
        sqlite3_bind_int(stmt, 6, cond.useCalendar ? 1 : 0);
		sqlite3_bind_int(stmt, 7, cond.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateCondition - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::updateRoom(DBRoom &room)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE room SET name=?,device_temperature_id=?,device_heater_id=?,device_heating_id=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, room.name.c_str(), room.name.length(), NULL);
		sqlite3_bind_int(stmt, 2, room.deviceTemperatureId);
		sqlite3_bind_int(stmt, 3, room.deviceHeaterId);
		sqlite3_bind_int(stmt, 4, room.deviceHeatingId);
		sqlite3_bind_int(stmt, 5, room.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateRoom - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::updateRoomGraphCond(DBRoomGraphCond &rgc)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE room_graphcond SET room_id=?,graph_id=?,condition_id=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, rgc.roomId);
		sqlite3_bind_int(stmt, 2, rgc.graphId);
		sqlite3_bind_int(stmt, 3, rgc.conditionId);
		sqlite3_bind_int(stmt, 4, rgc.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateRoomGraphCond - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::updateMobileLastSuccess(DBMobile &mob)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "UPDATE mobile SET datetime_last_success=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int64(stmt, 1, mob.lastSuccessYYYYMMDDHHSS);
		sqlite3_bind_int(stmt, 2, mob.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::updateMobileLastSuccess - SQL error: " << sqlite3_errmsg(_db)));
    return false;
}

bool MHEDatabase::addConditionDate(int condId, int dateYYYYMMDD)
{
    u_int64_t dateBeginYYYYMMDDHHMM = dateYYYYMMDD;
    u_int64_t dateEndYYYYMMDDHHMM;

    dateBeginYYYYMMDDHHMM *= (u_int64_t)10000;
    dateEndYYYYMMDDHHMM = dateBeginYYYYMMDDHHMM + (u_int64_t )2359;
    return addConditionDate(condId, dateBeginYYYYMMDDHHMM, dateEndYYYYMMDDHHMM);
}

bool MHEDatabase::addConditionDate(int condId, u_int64_t dateBeginYYYYMMDDHHMM, u_int64_t dateEndYYYYMMDDHHMM)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "INSERT INTO condition_calendar(condition_id,datetime_begin,datetime_end) VALUES(?,?,?)";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, condId);
		sqlite3_bind_int64(stmt, 2, dateBeginYYYYMMDDHHMM);
		sqlite3_bind_int64(stmt, 3, dateEndYYYYMMDDHHMM);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::addConditionDate - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}

bool MHEDatabase::addMobile(std::string &type, std::string &user, std::string &token)
{
    sqlite3_stmt *stmt = NULL;
	const std::string sql = "INSERT INTO mobile(type,user,token,is_enabled,datetime_last_success) VALUES(?,?,?,0,0)";
	int rc;

	rc = sqlite3_prepare(_db, sql.c_str(), sql.size(), &stmt, NULL);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), NULL);
		sqlite3_bind_text(stmt, 2, user.c_str(), user.length(), NULL);
		sqlite3_bind_text(stmt, 3, token.c_str(), token.length(), NULL);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
            return true;
	}
	LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("MHEDatabase::addMobile - SQL error: " << sqlite3_errmsg(_db)));
	return false;
}
