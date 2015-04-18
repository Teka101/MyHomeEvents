#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>

#include <sqlite3.h>

struct sGraph
{
	int id;
	std::string description;
};

class DataBase
{
private:
	sqlite3 *db;

	void createTables();

public:
	DataBase();
	~DataBase();

	std::vector<sGraph> getGraphs();
};

#endif /* DATABASE_H_ */
