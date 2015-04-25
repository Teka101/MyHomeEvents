#ifndef DATABASEHELPERS_H_
#define DATABASEHELPERS_H_

struct sGraph;
struct sGraphData;

void readFromPTree(boost::property_tree::ptree &pTree, sGraph &graph, bool readData = false);

void writeToPTree(boost::property_tree::ptree &pTree, sGraph &graph);
void writeToPTree(boost::property_tree::ptree &pTree, sGraphData &data);

#endif /* DATABASEHELPERS_H_ */
