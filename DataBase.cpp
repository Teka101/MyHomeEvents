#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "DataBase.h"

DataBase::DataBase() : db(nullptr)
{
	const char *fileName = "test.db";
	bool isExistingDB;
	int rc;

	isExistingDB = boost::filesystem::exists(fileName);
	rc = sqlite3_open(fileName, &db);
	if (rc != 0)
		std::cerr << "sqlite3_open() failed: " << sqlite3_errmsg(db) << std::endl;
	else if (!isExistingDB)
	{
		this->createTables();
		this->insertDefaultData();
	}
	else
		std::cout << "Opening existing database..." << std::endl;
}

DataBase::~DataBase()
{
	if (db != nullptr)
		sqlite3_close(db);
}

void DataBase::createTables()
{
	const char sqls[][256] =
	{
			"CREATE TABLE calendar(date INTEGER)",
			"CREATE TABLE condition(description TEXT, domoticz_device_type INTEGER, temperature_min REAL, temperature_max REAL, day INTEGER, calendar_id INTEGER REFERENCES calendar(rowid))",
			"CREATE TABLE graph(description TEXT, position INTEGER DEFAULT 99, condition_id INTEGER REFERENCES condition(rowid))",
			"CREATE TABLE graph_data(graph_id INTEGER REFERENCES graph(rowid), time INTEGER NOT NULL, value REAL)",
	};
	int nbElements = (sizeof(sqls) / sizeof(*sqls));

	for (int i = 0; i < nbElements; i++)
	{
		char *zErrMsg = nullptr;
		int rc;

		rc = sqlite3_exec(db, sqls[i], nullptr, nullptr, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			std::cerr << "SQL error: " << zErrMsg << " ### " << sqls[i] << std::endl;
			sqlite3_free(zErrMsg);
		}
		else
			std::cout << "Table created successfully" << std::endl;
	}
}

void DataBase::insertDefaultData()
{
	const char sqls[][256] =
	{
			"INSERT INTO condition(description, domoticz_device_type, day) VALUES('Monday to friday', NULL, 124),('Saturday to sunday', NULL, 3)",
			"INSERT INTO graph(description, position, condition_id) VALUES('Default week', 0, 1)",
			"INSERT INTO graph_data(graph_id, time, value) VALUES(1, 0000, 19),(1, 0600, 19.5),(1, 1900, 20),(1, 2100, 19)",
			"INSERT INTO graph(description, position, condition_id) VALUES('Default week-end', 1, 2)",
			"INSERT INTO graph_data(graph_id, time, value) VALUES(2, 0000, 19),(2, 1000, 19.5),(2, 1900, 20),(2, 2100, 19)"
	};
	int nbElements = (sizeof(sqls) / sizeof(*sqls));

	for (int i = 0; i < nbElements; i++)
	{
		char *zErrMsg = nullptr;
		int rc;

		rc = sqlite3_exec(db, sqls[i], nullptr, nullptr, &zErrMsg);
		if (rc != SQLITE_OK)
		{
			std::cerr << "SQL error: " << zErrMsg << " ### " << sqls[i] << std::endl;
			sqlite3_free(zErrMsg);
		}
		else
			std::cout << "Data inserted successfully" << std::endl;
	}
}

bool DataBase::addCondition(std::string &description)
{
	sqlite3_stmt *stmt = nullptr;
	const char *sql = "INSERT INTO condition(description) VALUES(?)";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, nullptr);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, description.c_str(), description.length(), nullptr);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		return (rc == SQLITE_DONE);
	}
	else
		std::cerr << "addCondition()-SQL error prepare: " << sqlite3_errmsg(db) << std::endl;
	return false;
}

bool DataBase::addGraph(std::string &description)
{
	sqlite3_stmt *stmt = nullptr;
	const char *sql = "INSERT INTO graph(description,condition_id) VALUES(?,NULL)";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, nullptr);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, description.c_str(), description.length(), nullptr);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		return (rc == SQLITE_DONE);
	}
	else
		std::cerr << "addGraph()-SQL error prepare: " << sqlite3_errmsg(db) << std::endl;
	return false;
}

bool DataBase::updateCondition(sCondition &cond)
{
	sqlite3_stmt *stmt = nullptr;
	const char *sql = "UPDATE condition SET description=?,domoticz_device_type=?,temperature_min=?,temperature_max=?,day=?,calendar_id=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, nullptr);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, cond.description.c_str(), cond.description.length(), nullptr);
		if (cond.domoticzDeviceType != -1)
			sqlite3_bind_int(stmt, 2, cond.domoticzDeviceType);
		else
			sqlite3_bind_null(stmt, 2);
		if (cond.temperatureMin != -1)
			sqlite3_bind_double(stmt, 3, cond.temperatureMin);
		else
			sqlite3_bind_null(stmt, 3);
		if (cond.temperatureMax != -1)
			sqlite3_bind_double(stmt, 4, cond.temperatureMax);
		else
			sqlite3_bind_null(stmt, 4);
		if (cond.day != -1)
			sqlite3_bind_int(stmt, 5, cond.day);
		else
			sqlite3_bind_null(stmt, 5);
		if (cond.calendarId != -1)
			sqlite3_bind_int(stmt, 6, cond.calendarId);
		else
			sqlite3_bind_null(stmt, 6);
		sqlite3_bind_int(stmt, 7, cond.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		return (rc == SQLITE_DONE);
	}
	else
		std::cerr << "updateCondition()-SQL error prepare: " << sqlite3_errmsg(db) << std::endl;
	return false;
}

bool DataBase::updateGraph(sGraph &graph)
{
	sqlite3_stmt *stmt = nullptr;
	const char *sql = "UPDATE graph SET position=?,description=?,condition_id=? WHERE rowid=?";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, nullptr);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, graph.position);
		sqlite3_bind_text(stmt, 2, graph.description.c_str(), graph.description.length(), nullptr);
		if (graph.conditionId != -1)
			sqlite3_bind_int(stmt, 3, graph.conditionId);
		else
			sqlite3_bind_null(stmt, 3);
		sqlite3_bind_int(stmt, 4, graph.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		return (rc == SQLITE_DONE);
	}
	else
		std::cerr << "updateGraph()-SQL error prepare: " << sqlite3_errmsg(db) << std::endl;
	return false;
}

bool DataBase::updateGraphData(sGraph &graph)
{
	sqlite3_stmt *stmt = nullptr;
	const char *sqlDelete = "DELETE FROM graph_data WHERE graph_id=?";
	const char *sql = "INSERT INTO graph_data(graph_id, time, value) VALUES(?,?,?)";
	int rc;

	rc = sqlite3_prepare(db, sqlDelete, strlen(sqlDelete), &stmt, nullptr);
	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, graph.id);
		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
		{
			rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, nullptr);
			if (rc == SQLITE_OK)
			{
				for (sGraphData data : graph.data)
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
		std::cerr << "updateGraphData()-SQL error prepare: " << sqlite3_errmsg(db) << std::endl;
	return (rc == SQLITE_OK || rc == SQLITE_DONE);
}

static int selectCondition(void *param, int ac, char **av, char **column)
{
	std::vector<sCondition> *r = static_cast<std::vector<sCondition>*>(param);

	if (ac == 7)
	{
		sCondition cond;

		cond.id = boost::lexical_cast<int>(av[0]);
		cond.description = av[1];
		cond.domoticzDeviceType = static_cast<eDomoticzDeviceType>(av[2] == nullptr ? -1 : boost::lexical_cast<int>(av[2]));
		cond.temperatureMin = (av[3] == nullptr ? -1 : boost::lexical_cast<float>(av[3]));
		cond.temperatureMax = (av[4] == nullptr ? -1 : boost::lexical_cast<float>(av[4]));
		cond.day = (av[5] == nullptr ? -1 : boost::lexical_cast<int>(av[5]));
		cond.calendarId = (av[6] == nullptr ? -1 : boost::lexical_cast<int>(av[6]));
		r->push_back(cond);
	}
	else
		std::cerr << "selectCondition()-not expected arguments: []" << ac << std::endl;
	return 0;
}

std::vector<sCondition> DataBase::getConditions()
{
	std::vector<sCondition> ret;
	int rc;

	rc = sqlite3_exec(db, "SELECT rowid,description,domoticz_device_type,temperature_min,temperature_max,day,calendar_id FROM condition ORDER BY rowid", selectCondition, &ret, nullptr);
	if (rc != SQLITE_OK)
		std::cerr << "getGraphs()-SQL error" << std::endl;
	return ret;
}

static int selectGraph(void *param, int ac, char **av, char **column)
{
	std::vector<sGraph> *r = static_cast<std::vector<sGraph>*>(param);

	if (ac == 4)
	{
		sGraph graph;

		graph.id = boost::lexical_cast<int>(av[0]);
		graph.position = boost::lexical_cast<int>(av[1]);
		graph.description = av[2];
		graph.conditionId = (av[3] == nullptr ? -1 : boost::lexical_cast<int>(av[3]));
		r->push_back(graph);
	}
	else
		std::cerr << "selectGraph()-not expected arguments: []" << ac << std::endl;
	return 0;
}

std::vector<sGraph> DataBase::getGraphs()
{
	std::vector<sGraph> ret;
	int rc;

	rc = sqlite3_exec(db, "SELECT rowid,position,description,condition_id FROM graph ORDER BY position", selectGraph, &ret, nullptr);
	if (rc != SQLITE_OK)
		std::cerr << "getGraphs()-SQL error" << std::endl;
	return ret;
}

static int selectGraphData(void *param, int ac, char **av, char **column)
{
	sGraph *r = static_cast<sGraph*>(param);

	if (ac == 2)
	{
		sGraphData data;

		data.time = boost::lexical_cast<int>(av[0]);
		data.value = boost::lexical_cast<float>(av[1]);
		r->data.push_back(data);
	}
	else
		std::cerr << "selectGraphData()-not expected arguments: []" << ac << std::endl;
	return 0;
}

sCondition DataBase::getCondition(int id)
{
	std::stringstream ss;
	std::vector<sCondition> conds;
	sCondition ret;
	int rc;

	ss << "SELECT rowid,description,domoticz_device_type,temperature_min,temperature_max,day,calendar_id FROM condition WHERE rowid=" << id;
	rc = sqlite3_exec(db, ss.str().c_str(), selectCondition, &conds, nullptr);
	if (rc != SQLITE_OK)
		std::cerr << "getCondition()-SQL error graph" << std::endl;
	else if (conds.size() == 1)
		ret = conds[0];
	return ret;
}

sGraph DataBase::getGraph(int id)
{
	std::stringstream ss;
	std::vector<sGraph> graphs;
	sGraph ret;
	int rc;

	ss << "SELECT rowid,position,description,condition_id FROM graph WHERE rowid=" << id;
	rc = sqlite3_exec(db, ss.str().c_str(), selectGraph, &graphs, nullptr);
	if (rc != SQLITE_OK)
		std::cerr << "getGraph()-SQL error graph" << std::endl;
	else if (graphs.size() == 1)
	{
		ret = graphs[0];
		ss.str(std::string());
		ss << "SELECT time,value FROM graph_data WHERE graph_id=" << id;
		rc = sqlite3_exec(db, ss.str().c_str(), selectGraphData, &ret, nullptr);
		if (rc != SQLITE_OK)
			std::cerr << "getGraph()-SQL error graph_data" << std::endl;
	}
	else
		std::cerr << "getGraph()-no data..." << std::endl;
	return ret;
}


