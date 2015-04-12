#ifndef DOMOTICZ_H_
#define DOMOTICZ_H_

#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

struct sDomoticzDevice
{
	int idx;
	std::string name;
	time_t lastUpdate;
	bool statusIsOn;
	float temperature;
	float humidity;

	sDomoticzDevice() : idx(-1), lastUpdate(0), statusIsOn(false), temperature(0), humidity(0)
	{
	}
};

class Domoticz
{
private:
	std::string serverUrl;
	std::string serverAuth;
	time_t lastUpdateDevices;
	std::vector<sDomoticzDevice*> devices;
	int idxDHT22, idxHeating, idxHeater;

	void removeDevices();
	void refreshData();

public:
	boost::signals2::signal<void(sDomoticzDevice *dev)> listenerDHT22;

	Domoticz(std::string &domoURL, std::string &domoAuth, int deviceIdxDHT22, int deviceIdxHeating, int deviceIdxHeater);
	~Domoticz();

	sDomoticzDevice *getDeviceDTH22();
	bool setValuesDHT22(time_t timestamp, float temperature, float humidity);
	bool setStatusHeater(bool activate);
};

#endif /* DOMOTICZ_H_ */
