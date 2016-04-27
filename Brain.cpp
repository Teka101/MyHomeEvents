/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <log4cplus/loggingmacros.h>
#include <cmath>
#include "Brain.h"

#define DEFAULT_TEMP_HYSTERESIS 0.5

Brain::Brain(MHEDatabase *db, MHEHardDevContainer *hardDevContainer, int refreshInSeconds, MHEMobileNotify *notify)
    : _db(db), _hardDevContainer(hardDevContainer), _notify(notify), _refreshInSeconds(refreshInSeconds), _timer(_io), _signals(_io)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Brain"));
    _signals.add(SIGINT);
    _signals.add(SIGTERM);
#if defined(SIGQUIT)
    _signals.add(SIGQUIT);
#endif //SIGQUIT
    _signals.async_wait(boost::bind(&Brain::stop, this));
    setHysteresis(DEFAULT_TEMP_HYSTERESIS, DEFAULT_TEMP_HYSTERESIS);
}

Brain::~Brain()
{
    boost::system::error_code ec;

    _timer.cancel(ec);
    if (errorCode != boost::system::errc::success)
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("~Brain - timer.cancel: errorCode=" << errorCode));
    _io.stop();
}

void Brain::start()
{
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain start..."));
    computeNextLaunch();
    _io.run();
}

void Brain::stop()
{
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain stop..."));
    _io.stop();
}

void Brain::setHysteresis(float hysteresisMin, float hysteresisMax)
{
	LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::setHysteresis - hysteresisMin=" << hysteresisMin << " hysteresisMax=" << hysteresisMax));
	_hysteresisMin = hysteresisMin;
	_hysteresisMax = hysteresisMax;
}

void Brain::computeNextLaunch()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	long currentMS = now.time_of_day().minutes() * 60L + now.time_of_day().seconds();

	LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::computeNextLaunch - currentTime=" << to_simple_string(now.time_of_day())));
	currentMS = _refreshInSeconds - (currentMS % _refreshInSeconds);
	LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::computeNextLaunch - waiting=" << currentMS));
	_timer.expires_from_now(boost::posix_time::seconds(currentMS));
	_timer.async_wait(boost::bind(&Brain::launch, this));
}

void Brain::launch()
{
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::launch - run..."));
    std::vector<DBRoomGraphCond> roomGraphConds = _db->getRoomGraphCondByActiveDaysAndCalendar();
    std::vector<DBCondition> conditions = _db->getConditions();
    std::vector<DBGraph> graphs = _db->getGraphs();
    std::vector<DBRoom> rooms = _db->getRooms();
    std::map<int,DBRoomGraphCond> graphByRoomId;
    std::map<int,DBCondition> conditionById;
    std::map<int,DBGraph> graphById;
    std::map<int,DBRoom> roomById;
    std::pair<int, DBRoomGraphCond> kv;

    BOOST_FOREACH(DBCondition &cond, conditions)
    {
        conditionById[cond.id] = cond;
    }
    BOOST_FOREACH(DBGraph &graph, graphs)
    {
        graphById[graph.id] = graph;
    }
    BOOST_FOREACH(DBRoom &room, rooms)
    {
        roomById[room.id] = room;
    }
    BOOST_FOREACH(DBRoomGraphCond &rgc, roomGraphConds)
    {
        DBCondition condition = conditionById[rgc.conditionId];

        if (condition.deviceId != -1 && (std::isnormal(condition.temperatureMin) || std::isnormal(condition.temperatureMax)))
        {
            MHEDevice *dev = _hardDevContainer->getDeviceById(condition.deviceId);

            if (dev != NULL)
            {
                float deviceTemperature = dev->getTemperature();

                if (condition.temperatureMin <= deviceTemperature && deviceTemperature <= condition.temperatureMax)
                    graphByRoomId[rgc.roomId] = rgc;
            }
        }
        else
            graphByRoomId[rgc.roomId] = rgc;
    }

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    BOOST_FOREACH(kv, graphByRoomId)
    {
        DBCondition condition = conditionById[kv.second.conditionId];
        DBGraph graph = graphById[kv.second.graphId];
        DBRoom room = roomById[kv.second.roomId];
        MHEDevice *devIn = _hardDevContainer->getDeviceById(room.deviceTemperatureId);
        MHEDevice *devHeater = _hardDevContainer->getDeviceById(room.deviceHeaterId);
        MHEDevice *devHeating = _hardDevContainer->getDeviceById(room.deviceHeatingId);
		long currentHHMM = now.time_of_day().hours() * 100L + now.time_of_day().minutes();
		float temperatureOrder = NAN;

		BOOST_FOREACH(DBGraphData data, graph.data)
			if (currentHHMM >= data.time)
				temperatureOrder = data.value;
        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::launch - room=" << (std::string)room << " condition=" << (std::string)condition));
		if (isnan(temperatureOrder))
			LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("Brain::launch - no graph data found !!! currentHHMM=" << currentHHMM));
		else
		{
            if (_notify != NULL)
            {
                int lastConditionId = _lastConditionIdByRoomId[room.id];

                if (lastConditionId > 0 && lastConditionId != condition.id)
                {
                    _notify->notifyDevices(conditionLeave, conditionById[lastConditionId]);
                    _notify->notifyDevices(conditionEnter, condition);
                }
            }
            _lastConditionIdByRoomId[room.id] = condition.id;
            if (devHeating != NULL)
            {
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::launch - set heating temperature to: " << temperatureOrder));
                devHeating->setTempHum(temperatureOrder, devHeating->getHumidity());
            }
            if (devHeater != NULL)
            {
                float temperatureMeasured = devIn->getTemperature();

                if (temperatureMeasured <= (temperatureOrder - _hysteresisMin))
                {
                    if (_notify != NULL && !devHeater->isActivated())
                        _notify->notifyDevices(conditionEnter, *devHeater);
                    devHeater->setStatus(true);
                }
                else if (temperatureMeasured >= (temperatureOrder + _hysteresisMax))
                {
                    if (_notify != NULL && devHeater->isActivated())
                        _notify->notifyDevices(conditionLeave, *devHeater);
                    devHeater->setStatus(false);
                }
            }
            else
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("Brain::launch - no heater to control"));
		}
    }
    computeNextLaunch();
}
