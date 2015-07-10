#ifndef DATABASEHELPERS_H_
#define DATABASEHELPERS_H_

#include "MHEDatabase.h"

void readFromPTree(boost::property_tree::ptree &pTree, DBHarware &hard);
void readFromPTree(boost::property_tree::ptree &pTree, DBDevice &dev);
void readFromPTree(boost::property_tree::ptree &pTree, DBGraph &graph);
void readFromPTree(boost::property_tree::ptree &pTree, DBCondition &cond);
void readFromPTree(boost::property_tree::ptree &pTree, DBRoom &room);
void readFromPTree(boost::property_tree::ptree &pTree, DBRoomGraphCond &rgc);

void writeToPTree(boost::property_tree::ptree &pTree, DBHarware &hard);
void writeToPTree(boost::property_tree::ptree &pTree, DBDevice &dev);
void writeToPTree(boost::property_tree::ptree &pTree, DBGraph &graph);
void writeToPTree(boost::property_tree::ptree &pTree, DBCondition &cond);
void writeToPTree(boost::property_tree::ptree &pTree, DBRoom &room);
void writeToPTree(boost::property_tree::ptree &pTree, DBRoomGraphCond &rgc);

#endif /* DATABASEHELPERS_H_ */
