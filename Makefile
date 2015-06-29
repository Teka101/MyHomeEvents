NAME=MyHomeEvents
SRC=Brain.cpp CurlHelpers.cpp DataBaseHelpers.cpp DriverDomoticz.cpp DriverShell.cpp Main.cpp MHEDatabase.cpp MHEDevice.cpp MHEHardDevContainer.cpp MHEHardware.cpp MHEWeb.cpp
OBJ=$(SRC:.cpp=.o)

DEPS_PATH=$(HOME)/deps
CPPFLAGS=-g3 -Wall -Wunused -Wpointer-arith -Wno-uninitialized `curl-config --cflags` -I$(DEPS_PATH)/include
LDFLAGS=`curl-config --libs` -lmicrohttpd -lpthread -lsqlite3 -lboost_program_options -lboost_filesystem -lboost_system -lboost_thread -lboost_date_time -lboost_regex -llog4cplus -L$(DEPS_PATH)/lib
CPP=g++
RM=rm -f

help:
	@echo "Rules:"
	@echo " - clean : clean objects (*.o)"
	@echo " - prepare : configure and compile dependencies (installed in ./deps/)"
	@echo " - all : compile"
	@echo ""
	@echo "Contents of ./deps/:"
	@echo " ./deps/boost : download at http://www.boost.org/"
	@echo " ./deps/log4cplus : download at http://log4cplus.sourceforge.net/"

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Compile binary	[$(NAME)]"
	@$(CPP) -o $(NAME) $(OBJ) $(LDFLAGS)

prepare: prepare-boost prepare-log4cplus
	@echo "Done."

prepare-boost:
	@echo "Prepare boost"
	@cd $(DEPS_PATH)/boost && ./bootstrap.sh --without-icu --with-libraries=chrono,date_time,filesystem,program_options,regex,serialization,system,thread --prefix=$(DEPS_PATH)
	@echo "Compilation and installation of boost"
	@cd $(DEPS_PATH)/boost && ./b2 install

prepare-log4cplus:
	@echo "Prepare log4cplus"
	@cd $(DEPS_PATH)/log4cplus && CPPFLAGS=-O3 CXXFLAGS=-O3 CFLAGS=-O3 ./configure --prefix=$(DEPS_PATH) --disable-shared
	@echo "Compilation and installation of log4cplus"
	@cd $(DEPS_PATH)/log4cplus && make install

clean:
	@echo "Delete all objects"
	@$(RM) $(OBJ)

distclean: clean
	@echo "Delete all unecessary files"
	@$(RM) $(NAME)
	@$(RM) -i `$(FIND) . | grep -F '~'` *.tgz || true

.cpp.o:
	@echo "Compile		[$<]"
	@$(CPP) $(CPPFLAGS) -c -o $@ $<
