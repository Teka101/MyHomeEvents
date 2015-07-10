# MyHomeEvents
work with Domoticz

Note: should work (at least for me)


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
Copy 'config.ini.sample' to 'config.ini'.

Run
---
	./MyHomeEvents
