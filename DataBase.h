#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>

#include <sqlite3.h>
#include "Domoticz.h"

struct sCondition
{
	int id;
	std::string description;
	eDomoticzDeviceType domoticzDeviceType;
	float temperatureMin, temperatureMax;
	int day;
	int calendarId;
};

struct sGraphData
{
	int time;
	float value;
};

struct sGraph
{
	int id;
	int position;
	std::string description;
	std::vector<sGraphData> data;
	int conditionId;
};

class DataBase
{
private:
	sqlite3 *db;

	void createTables();
	void insertDefaultData();

public:
	DataBase();
	~DataBase();

	bool addGraph(std::string &description);
	bool updateGraph(sGraph &graph);
	bool updateGraphData(sGraph &graph);
	std::vector<sCondition> getConditions();
	std::vector<sGraph> getGraphs();
	sGraph getGraph(int id);
};

#endif /* DATABASE_H_ */
