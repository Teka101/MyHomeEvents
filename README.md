# MyHomeEvents
work with Domoticz


Before
------

Install and configure [Domoticz](http://www.domoticz.com/).

Compilation
-----------
Install packages for Debian/Raspbian/Ubuntu:
* libboost-date-time-dev
* libboost-dev
* libboost-filesystem-dev
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
