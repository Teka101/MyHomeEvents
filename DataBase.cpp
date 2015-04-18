#include <iostream>

#include <boost/lexical_cast.hpp>
#include "DataBase.h"

DataBase::DataBase() : db(nullptr)
{
	int rc;

	rc = sqlite3_open("test.db", &db);
	if (rc != 0)
		std::cerr << "sqlite3_open() failed: " << sqlite3_errmsg(db) << std::endl;
	else
		this->createTables();
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
			"CREATE TABLE IF NOT EXISTS condition(type INTEGER, device_domoticz_idx INTEGER, min REAL, max REAL)",
			"CREATE TABLE IF NOT EXISTS graph(description TEXT, position INTEGER, condition_id INTEGER REFERENCES condition(rowid))",
			"CREATE TABLE IF NOT EXISTS graph_data(graph_id INTEGER REFERENCES graph(rowid), value REAL)",
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

static int selectGraph(void *param, int ac, char **av, char **column)
{
	std::vector<sGraph> *r = static_cast<std::vector<sGraph>*>(param);
	sGraph graph;

	graph.id = boost::lexical_cast<int>(av[0]);
	graph.description = av[1];
	r->push_back(graph);
	/*for (int i = 0; i < ac; i++)
	        printf("[%s]%s,\t", column[i] , av[i]);
	    printf("\n");*/
	return 0;
}

std::vector<sGraph> DataBase::getGraphs()
{
	std::vector<sGraph> ret;
	int rc;

	rc = sqlite3_exec(db, "SELECT rowid,description FROM graph ORDER BY position", selectGraph, &ret, nullptr);
	return ret;
}
