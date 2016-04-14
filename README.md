# MyHomeEvents [![Build Status](https://travis-ci.org/Teka101/MyHomeEvents.svg?branch=master)](https://travis-ci.org/Teka101/MyHomeEvents) [![Code Quality](https://img.shields.io/badge/code%20quality-link-blue.svg)](https://gist.github.com/Teka101/94c7b6a4408e1f601899#file-report-md) [![Coverity Scan](https://scan.coverity.com/projects/8385/badge.svg)](https://scan.coverity.com/projects/teka101-myhomeevents)
work with Domoticz

License
-------
[MIT License](https://opensource.org/licenses/MIT)


Before
------

Install and configure [Domoticz](http://www.domoticz.com/).

Compilation
-----------
Install packages for Debian/Raspbian/Ubuntu:
* libboost-date-time-dev
* libboost-dev
* libboost-filesystem-dev
* libboost-iostreams-dev
* libboost-program-options-dev
* libboost-regex-dev
* libboost-serialization-dev
* libboost-system-dev
* libboost-thread-dev
* libcurl4-gnutls-dev
* liblog4cplus-dev
* libmicrohttpd-dev
* libsqlite3-dev

Do:

	make all

Configuration
-------------
You have to customize your 'config.ini' (a sample is available in 'config.ini.sample').
Some features are not visible in web pages, so you have to edit the SQLITE database (file 'params.db').

Run
---
	./MyHomeEvents

Features
--------
* Web configuration - [http://localhost:8080/](http://localhost:8080/)
* "virtual" devices (can connect to Domoticz or execute shell script)
* create room to link devices (heat control => ambiant temperature => heater)
* Android notification with GCM (check table "mobile_event" to register to events)
* Send external notification to mobile (ex: when download is finished) - [http://localhost:8080/notify/myevent/mytype/mymessage](http://localhost:8080/notify/myevent/mytype/mymessage)


Service REST/JSON:
* /runningDevices : list declared device
* /setStatus/{device.id}/(false|true) : turn on/off a device
* /getSpecial/{device.id}/graph/(last24h|lastMonth|lastYear) : get graph data of a device (only supported for Domoticz)
* /getSpecial/{device.id}/volume) : get volume of your device (only supported for TV)
* /setSpecial/{device.id}/volume/{volume 0-100} : set volume of your device (only supported for TV)
* /setChannel/{device.id}/{channel.id} : change channel of your TV
* /mobile/{type}/{user}/{token} : declare new mobile to notify (only GCM - should activated device in table "mobile")
* /notify/{event}/{type}/{message} : notify an event to registered mobile
* /order/{message} : send an order to your server


Migration from previous version:
* ALTER TABLE device ADD COLUMN hidden INTEGER NOT NULL DEFAULT 0
* ALTER TABLE device ADD COLUMN cache_running INTEGER NOT NULL DEFAULT 0
